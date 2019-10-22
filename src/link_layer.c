/**
 * @file link_layer.h
 * @brief The datalink program data link layer source file
 *
 * This is the data link layer of the project.
 *
 */
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "link_layer.h"
#include "util.h"

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
  if (ll->fd < 0) {
    perror("open");
    printf("trying to open port %s\n", port);
    return -1;
  }

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
  if (tcgetattr(ll->fd, &ll->old_termios) < 0) {
    perror("tcgetattr");
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

  if (tcflush(ll->fd, TCIOFLUSH) != 0) {
    perror("tcflush");
    return -1;
  }

  if (tcsetattr(ll->fd, TCSANOW, &ll->new_termios) != 0) {
    perror("tcsetattr");
    return -1;
  }

  printf("Establishing connection...\n");

  unsigned int connected = 0, tries = 0;
  switch (ll->ct) {
  case SEND: {
    while (!connected) {
      if (tries == 0) {
        // Send SET
        printf("Sending SET...\n");
        send_command(ll, SET);
      }

      // Receive UA
      printf("Waiting for UA...\n");
      message_t *msg = (message_t *)malloc(sizeof(message_t));
      receive_message(ll, msg);

      if (msg->command == UA) {
        printf("Received UA!\n");
        connected = 1;
        break;
      }

      tries++;
    }
    break;
  }

  case RECEIVE: {
    while (!connected) {
      // Receive SET
      printf("Waiting for SET...\n");
      message_t *msg = (message_t *)malloc(sizeof(message_t));
      receive_message(ll, msg);

      if (msg->command == SET) {
        printf("Received SET, sending back UA...\n");
        // Send UA
        sleep(1);
        send_command(ll, UA);

        connected = 1;
        break;
      }

      tries++;
    }
    break;
  }
  }

  printf("Connection established.\n");

  return ll->fd;
}

int llwrite(link_layer_t *ll, char *buf, int buf_size) { return 0; }

int llread(link_layer_t *ll, char *buf) { return 0; }

int llclose(link_layer_t *ll) {
  printf("Closing connection...\n");

  unsigned int disconnected = 0, tries = 0;
  switch (ll->ct) {
  case SEND: {
    while (!disconnected) {
      // Send DISC
      printf("Sending DISC...\n");
      send_command(ll, DISC);

      // Receive DISC
      printf("Waiting for DISC...\n");
      message_t *msg = (message_t *)malloc(sizeof(message_t));
      receive_message(ll, msg);

      if (msg->command == DISC) {
        // Send UA
        printf("Received DISC, sending back UA...\n");
        sleep(1);
        send_command(ll, UA);

        disconnected = 1;
        continue;
      }

      tries++;
    }
    break;
  }
  case RECEIVE: {
    while (!disconnected) {
      // Receive DISC
      printf("Waiting for DISC...\n");
      message_t *msg = (message_t *)malloc(sizeof(message_t));
      receive_message(ll, msg);

      if (msg->command == DISC) {
        printf("Received DISC, sending back DISC...\n");
        // Send DISC
        sleep(1);
        send_command(ll, DISC);

        // Receive UA
        printf("Waiting for UA...\n");
        receive_message(ll, msg);
        if (msg->command == UA) {
          printf("Received UA. Done.\n");
          disconnected = 1;
          continue;
        }
      }

      tries++;
    }
    break;
  }
  }
  printf("Connection closed.\n");

  // Switch back to old termios
  if (tcsetattr(ll->fd, TCSANOW, &ll->old_termios) < 0) {
    perror("tcsetattr");
    return -1;
  }

  // Close serial port file descriptor
  if (close(ll->fd) < 0) {
    perror("close");
    return -1;
  }

  return 0;
}

char *create_command(link_layer_t *ll, control_field_t cf) {
  char *command = malloc(COMMAND_SIZE);

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

  char *command_buf = create_command(ll, cf);
  unsigned int command_buf_size = stuff_buffer(command_buf, COMMAND_SIZE);

  if (write(ll->fd, command_buf, command_buf_size) != COMMAND_SIZE) {
    printf("Could not send command: %s  \n", command_str);
    return -1;
  }

  free(command_buf);

  printf("Sent command: %s\n", command_str);

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
    strcpy(command_str, "ERROR");
    return C_SET;
  }
}

int receive_message(link_layer_t *ll, message_t *msg) {
  msg->type = INVALID;
  msg->ns = msg->nr = -1;

  char *msg_buf = malloc(ll->message_data_max_size);

  unsigned int size = 0;
  state_t state = START;
  volatile int done = 0;
  while (!done) {
    // Byte to read
    char c;

    if (state != STOP) {
      // Read 1 byte into c
      int n = read(ll->fd, &c, 1);

      // If no bytes read
      if (!n) {
        free(msg);

        msg->type = INVALID;
        msg->err = IO_ERR;

        return -1;
      }
    }

    switch (state) {
    case START: {
      if (c == FLAG) {
        printf("START: FLAG received. Going to FLAG_RCV.\n");
        msg_buf[size++] = c;
        state = FLAG_RCV;
      }
      break;
    }
    case FLAG_RCV: {
      if (c == A) {
        printf("FLAG_RCV: A received. Going to A_RCV.\n");
        msg_buf[size++] = c;
        state = A_RCV;
      } else if (c != FLAG) {
        printf("C_RCV: ? received. Going back to START.\n");
        size = 0;
        state = START;
      }
      break;
    }
    case A_RCV: {
      if (c != FLAG) {
        printf("A_RCV: C received. Going to C_RCV.\n");
        msg_buf[size++] = c;
        state = C_RCV;
      } else if (c == FLAG) {
        printf("C_RCV: FLAG received. Going back to FLAG_RCV.\n");
        size = 1;
        state = FLAG_RCV;
      } else {
        printf("C_RCV: ? received. Going back to START.\n");
        size = 0;
        state = START;
      }
      break;
    }
    case C_RCV: {
      if (c == (msg_buf[1] ^ msg_buf[2])) {
        printf("C_RCV: BCC received. Going to BCC_OK.\n");
        msg_buf[size++] = c;
        state = BCC_OK;
      } else if (c == FLAG) {
        printf("C_RCV: FLAG received. Going back to FLAG_RCV.\n");
        size = 1;
        state = FLAG_RCV;
      } else {
        printf("C_RCV: ? received. Going back to START.\n");
        size = 0;
        state = START;
      }
      break;
    }
    case BCC_OK: {
      if (c == FLAG) {
        if (msg->type == INVALID)
          msg->type = COMMAND;

        printf("BCC_OK: FLAG received. Going to STOP.\n");

        msg_buf[size++] = c;

        state = STOP;
      } else {
        if (msg->type == INVALID)
          msg->type = DATA;
        else if (msg->type == COMMAND) {
          printf("WANING?? something unexpected happened.\n");
          state = START;
          continue;
        }

        // If more bytes are to be received
        if (size % ll->message_data_max_size == 0) {
          int mFactor = size / ll->message_data_max_size + 1;
          msg_buf =
              (char *)realloc(msg_buf, mFactor * ll->message_data_max_size);
        }

        msg_buf[size++] = c;
      }
      break;
    }
    case STOP: {
      msg_buf[size] = 0;
      done = 1;
      break;
    }
    default:
      break;
    }
  }

  printf("Destuffing message...\n");
  size = destuff_buffer(msg_buf, size);

  char address_field = msg_buf[1];
  char control_field = msg_buf[2];
  char bcc1 = msg_buf[3];

  if (bcc1 != (address_field ^ control_field)) {
    printf("Invalid BCC1.\n");

    free(msg_buf);

    msg->type = INVALID;
    msg->err = BCC1_ERR;

    return -1;
  }

  printf("If message is command or data\n");
  if (msg->type == COMMAND) {
    // Get message command
    msg->command = get_command(msg_buf[2]);

    // Get command control field
    control_field_t cf = msg_buf[2];

    char command_str[MAX_SIZE];
    get_command_w_control_field(command_str, msg->command);

    printf("Received command: %s.\n", command_str);

    if (msg->command == RR || msg->command == REJ)
      msg->nr = (cf >> 7) & BIT(0);
  } else if (msg->type == DATA) {
    msg->data.message_size = size - MESSAGE_SIZE;

    char calc_bcc2 = process_bcc(&msg_buf[4], msg->data.message_size);
    char bcc2 = msg_buf[4 + msg->data.message_size];

    if (calc_bcc2 != bcc2) {
      printf("Invalid BCC2: 0x%02x != 0x%02x.\n", calc_bcc2, bcc2);

      free(msg_buf);

      msg->type = INVALID;
      msg->err = BCC2_ERR;

      return -1;
    }

    msg->ns = (msg_buf[2] >> 6) & BIT(0);

    // Copy message
    msg->data.message = malloc(msg->data.message_size);
    memcpy(msg->data.message, &msg_buf[4], msg->data.message_size);
  }

  free(msg_buf);

  return 0;
}

unsigned int stuff_buffer(char *buf, unsigned int buf_size) {
  unsigned int new_buf_size = buf_size;

  for (unsigned int i = 1; i < buf_size - 1; i++)
    if (buf[i] == FLAG || buf[i] == ESCAPE)
      new_buf_size++;

  buf = (char *)realloc(buf, new_buf_size);

  for (unsigned int i = 1; i < buf_size - 1; i++) {
    if (buf[i] == FLAG || buf[i] == ESCAPE) {
      memmove(buf + i + 1, buf + i, buf_size - i);

      buf_size++;

      buf[i] = ESCAPE;
      buf[i + 1] ^= 0x20;
    }
  }

  return new_buf_size;
}

unsigned int destuff_buffer(char *buf, unsigned int buf_size) {
  for (unsigned int i = 1; i < buf_size - 1; ++i) {
    if (buf[i] == ESCAPE) {
      memmove(buf + i, buf + i + 1, buf_size - i - 1);

      buf_size--;

      buf[i] ^= 0x20;
    }
  }

  buf = (char *)realloc(buf, buf_size);

  return buf_size;
}

char process_bcc(const char *buf, unsigned int buf_size) {
  char bcc = 0;

  unsigned int i = 0;
  for (; i < buf_size; i++)
    bcc ^= buf[i];

  return bcc;
}
