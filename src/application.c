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
  sprintf(str, "/dev/ttys%s", argv[2]);

  // Check if serial port exists
  if (!file_exists(str)) {
    printf("Invalid port number given. Serial port %s does not exist.\n", str);
    exit(-1);
  }

  // Set the serial port device file path
  config->port = str;

  // If receiving, we're done setting the config
  if (config->ct == RECEIVE)
    return;

  // Prompt user for filename of file to be sent
  printf("Name of file to send ? ");
  memset(str, 0, sizeof(str));
  scanf("%[^\n]%*c", str);

  // Check if file exists
  if (!file_exists(str)) {
    printf("Invalid file name given. File %s does not exist.\n", str);
    exit(-1);
  }

  // Set the filename
  config->filename = str;
}

int run(const config_t *config) {
  // Set the data link layer struct
  link_layer_t *ll = (link_layer_t *)malloc(sizeof(link_layer_t));
  set_link_layer(ll, config->port, config->ct);

  // Establish connection
  if (llopen(ll) < 0)
    return -1;

  // Perform transfer
  if (config->ct == SEND) {
    if (send_file(ll, config->filename) < 0)
      return -1;
  } else if (config->ct == RECEIVE) {
    if (receive_file(ll) < 0)
      return -1;
  }

  // Close connection
  return llclose(ll);
}

int send_file(link_layer_t *ll, const char *filename) {
  // Open file
  FILE *file = fopen(filename, "rb");

  // TODO: get size of file to be sent,
  //       and allocate a character buffer with
  //       the same size

  // TODO: send START control package
  //       (with file size and name in value field)

  // TODO: send file chunks to llwrite

  // TODO: free the character buffer

  // Close file
  fclose(file);

  // TODO: send END control package

  return 0;
}

int receive_file(link_layer_t *ll) {
  // TODO: receive START control package
  //       (with file size and name in value field)

  // TODO: create a file with the proper name

  // TODO: read from from llread
  //       (should read as many bytes as file size given)

  // TODO: create

  return 0;
}

int main(int argc, const char **argv) {
  if (argc != 3) {
    printf("Usage: %s <send|receive> <serial port number>\n", argv[0]);
    return 1;
  }

  // Set the program config struct
  config_t *config = (config_t *)malloc(sizeof(config_t));
  set_config(config, argv);

  // We're good to go
  return run(config);
}
