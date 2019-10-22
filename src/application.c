/**
 * @file application.c
 * @brief The datalink program application layer source file
 *
 * This is the application layer of the project.
 *
 * It is also the starting point of the built program.
 *
 */

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "application.h"

void set_config(config_t *config, const char **argv) {
  // Initialize pointers
  config->port = (char*) malloc(256);
  config->file_name = (char*) malloc(256);

  // Set the connection mode
  if (strcmp(argv[1], "send") == 0)
    config->ct = SEND;
  else if (strcmp(argv[1], "receive") == 0)
    config->ct = RECEIVE;
  else {
    printf("Invalid connection mode. Must be 'send' or 'receive'\n");
    exit(-1);
  }

  // Build the serial port file name from the port number given
  char str[64];
  sprintf(str, "/dev/%s", argv[2]);

  // Check if serial port exists
  if (!file_exists(str)) {
    printf("Invalid port number given. Serial port %s does not exist.\n", str);
    exit(-1);
  }

  // Set the serial port device file path
  strcpy(config->port, str);

  // If receiving, keep config file name as recognisable string
  strcpy(config->file_name, "null");

  // If receiving, we're done setting the config
  if (config->ct == RECEIVE)
    return;

  // Prompt user for name of file to be sent
  printf("Name of file to send ? ");
  memset(str, 0, sizeof(str));
  scanf("%[^\n]%*c", str);

  // Check if file exists
  if (!file_exists(str)) {
    printf("Invalid file name given. File %s does not exist.\n", str);
    exit(-1);
  }

  // Set the file name
  strcpy(config->file_name, str);
}

int run(const config_t *config) {
  // Set the data link layer struct
  link_layer_t *ll = (link_layer_t *)malloc(sizeof(link_layer_t));
  if (set_link_layer(ll, config->port, config->ct) < 0) {
    perror("set_link_layer");
    return -1;
  }

  // Establish connection
  if (llopen(ll) < 0)
    return -1;

  // Perform transfer
  if (config->ct == SEND) {
    if (send_file(ll, config->file_name) < 0)
      return -1;
  } else if (config->ct == RECEIVE) {
    if (receive_file(ll) < 0)
      return -1;
  }

  // Close connection
  return llclose(ll);
}

int send_file(link_layer_t *ll, const char *file_name) {
  // Open file
  FILE *file = fopen(file_name, "rb");

  // TODO: get size of file to be sent,
  //       and allocate a character buffer with
  //       the same size

  // TODO: send START control packet
  //       (with file size and name in value field)

  // TODO: send file chunks to llwrite

  // TODO: free the character buffer

  // Close file
  fclose(file);

  // TODO: send END control packet

  return 0;
}

int receive_file(link_layer_t *ll) {
  // TODO: receive START control packet
  //       (with file size and name in value field)

  // TODO: create a file with the proper name

  // TODO: read from from llread
  //       (should read as many bytes as file size given)

  // TODO: create

  return 0;
}

int send_control_packet(link_layer_t *ll, const packet_t *packet) {
  // Calculate control packet size
  int packet_buf_size =
      5 + strlen(packet->file_size_buf) + strlen(packet->file_name);

  // Create the control packet buffer
  char packet_buf[packet_buf_size];
  unsigned int pos = 0;

  // Set packet type
  packet_buf[pos++] = packet->type;

  // Set file size field
  packet_buf[pos++] = FILE_SIZE_FIELD;
  packet_buf[pos++] = strlen(packet->file_size_buf);
  for (unsigned int i = 0; i < strlen(packet->file_size_buf); i++)
    packet_buf[pos++] = packet->file_size_buf[i];

  // Set file name field
  packet_buf[pos++] = FILE_NAME_FIELD;
  packet_buf[pos++] = strlen(packet->file_name);
  for (unsigned int i = 0; i < strlen(packet->file_name); i++)
    packet_buf[pos++] = packet->file_name[i];

  // Write it to the serial port
  if (llwrite(ll, packet_buf, packet_buf_size) < 0)
    return -1;

  free(packet_buf);

  return 0;
}

int receive_control_packet(link_layer_t *ll, packet_t *packet) {
  // Receive packet
  char *packet_buf = NULL;
  int packet_buf_size = llread(ll, packet_buf);
  if (packet_buf_size < 0) {
    printf("Could not read from link layer.\n");
    return -1;
  }

  // Get packet type
  unsigned int pos = 0;
  packet->type = packet_buf[pos++];

  // Go through packet fields
  while (true) {
    switch (packet_buf[pos++]) {
    case FILE_SIZE_FIELD: {
      unsigned int octets = (unsigned int)packet_buf[pos++];

      char *length = malloc(octets);
      memcpy(length, &packet_buf[pos], octets);

      packet->file_size = atoi(length);
      free(length);

      break;
    }
    case FILE_NAME_FIELD: {
      unsigned char octets = (unsigned char)packet_buf[pos++];

      memcpy(packet->file_name, &packet_buf[pos], octets);

      break;
    }
    default:
      return -1;
    }
  }

  return 0;
}

int main(int argc, const char **argv) {
  if (argc != 3) {
    printf("Usage: %s <send|receive> <serial port device name>\n", argv[0]);
    return 1;
  }

  // Set the program config struct
  config_t *config = (config_t *)malloc(sizeof(config_t));
  set_config(config, argv);

  // We're good to go
  return run(config);
}
