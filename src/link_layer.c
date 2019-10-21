#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "link_layer.h"

#define FLAG 0x7E
#define A 0x03
#define ESCAPE 0x7D

#define DEFAULT_BAUDRATE B38400
#define DEFAULT_MSG_MAX_SIZE 512
#define DEFAULT_RETRIES 3
#define DEFAULT_TIMEOUT 3

int set_link_layer(link_layer_t *ll, char *port, const conn_type_t ct) {
  // Open serial port device for reading and writing, and not as controlling
  ll->fd = open(port, O_RDWR | O_NOCTTY);
  // Set connection type
  ll->ct = ct;

  // Below are default values
  // TODO: Make these configurable by the user
  ll->seq_number = 0;
  ll->baud_rate = DEFAULT_BAUDRATE;
  ll->message_data_max_size = DEFAULT_MSG_MAX_SIZE;
  ll->retries = DEFAULT_RETRIES;
  ll->timeout = DEFAULT_TIMEOUT;

  return 0;
}

int llopen(link_layer_t *ll) {
  // Save current termios settings
  if (tcgetattr(ll->fd, &ll->old_termios) != 0) {
    printf("Could not save current termios settings.\n");
    return -1;
  }

  // Configure new termios settings
  bzero(&ll->new_termios, sizeof(ll->new_termios));
  ll->new_termios.c_cflag = ll->baud_rate | CS8 | CLOCAL | CREAD;
  ll->new_termios.c_iflag = IGNPAR;
  ll->new_termios.c_oflag = 0;
  // Input mode (non-canonical)
  ll->new_termios.c_lflag = 0;

  // Inter-character timer unused
  ll->new_termios.c_cc[VTIME] = 3;

  // Blocking read until x chars received
  ll->new_termios.c_cc[VMIN] = 0;

  if (tcflush(ll->fd, TCIOFLUSH) != 0)
    return -1;

  if (tcsetattr(ll->fd, TCSANOW, &ll->new_termios) != 0)
    return -1;

  printf("Establishing connection...\n");

  unsigned int connected = 0, tries = 0;
  switch (ll->ct) {
  case SEND: {
    while (!connected) {
      if (tries == 0) {

        if (tries >= ll->retries) {
          printf("Connection timeout. (Too many tries)\n");
          return -1;
        }

        send_command(ll, SET);

        tries++;
      }

      // TODO: Receive UA
    }
    break;
  }

  case RECEIVE: {
    while (!connected) {
      // TODO: Receive SET
    }
    break;
  }
  }

  return ll->fd;
}

int llwrite(link_layer_t *ll) { return 0; }

int llread(link_layer_t *ll) { return 0; }

int llclose(link_layer_t *ll) {
  printf("Closing connection...\n");

  // Switch back to old termios
  if (tcsetattr(ll->fd, TCSANOW, &ll->old_termios) < 0)
		return -1;

  // Close serial port file descriptor
  return close(ll->fd);
}

unsigned char *create_command(link_layer_t *ll, control_field_t cf) {
  unsigned char *command = malloc(COMMAND_SIZE);

  command[0] = FLAG;
  command[1] = A;
  command[2] = cf;
  if (cf == C_REJ || cf == C_RR)
    command[2] |= (ll->seq_number << 7);
  command[3] = command[1] ^ command[2];
  command[4] = FLAG;

  return command;
}

int send_command(link_layer_t *ll, command_t command) {
  char command_str[MAX_SIZE];
  control_field_t cf = get_command_w_control_field(command_str, command);

  unsigned char *command_buf = create_command(ll, cf);
  unsigned int command_buf_size = stuff_buffer(&command_buf, COMMAND_SIZE);

  write(ll->fd, command_buf, command_buf_size);

  free(command_buf);

  return 0;
}

command_t get_command(control_field_t cf) {
  switch (cf & 0x0F) {
  case C_SET:
    return SET;
  case C_UA:
    return UA;
  case C_RR:
    return RR;
  case C_REJ:
    return REJ;
  case C_DISC:
    return DISC;
  default:
    return SET;
  }
}

control_field_t get_command_w_control_field(char *command_str,
                                          command_t command) {
  switch (command) {
  case SET:
    strcpy(command_str, "SET");
    return C_SET;
  case UA:
    strcpy(command_str, "UA");
    return C_UA;
  case RR:
    strcpy(command_str, "RR");
    return C_RR;
  case REJ:
    strcpy(command_str, "REJ");
    return C_REJ;
  case DISC:
    strcpy(command_str, "DISC");
    return C_DISC;
  default:
    strcpy(command_str, "* ERROR *");
    return C_SET;
  }
}

unsigned int stuff_buffer(unsigned char **buf, unsigned int buf_size) {
  unsigned int new_buf_size = buf_size;

  for (unsigned int i = 1; i < buf_size - 1; i++)
    if ((*buf)[i] == FLAG || (*buf)[i] == ESCAPE)
      new_buf_size++;

  *buf = (unsigned char *)realloc(*buf, new_buf_size);

  for (unsigned int i = 1; i < buf_size - 1; i++) {
    if ((*buf)[i] == FLAG || (*buf)[i] == ESCAPE) {
      memmove(*buf + i + 1, *buf + i, buf_size - i);

      buf_size++;

      (*buf)[i] = ESCAPE;
      (*buf)[i + 1] ^= 0x20;
    }
  }

  return new_buf_size;
}

unsigned int destuff_buffer(unsigned char **buf, unsigned int buf_size) {
  for (unsigned int i = 1; i < buf_size - 1; ++i) {
    if ((*buf)[i] == ESCAPE) {
      memmove(*buf + i, *buf + i + 1, buf_size - i - 1);

      buf_size--;

      (*buf)[i] ^= 0x20;
    }
  }

  *buf = (unsigned char *)realloc(*buf, buf_size);

  return buf_size;
}
