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
    config->cm = SEND;
  else if (strcmp(argv[1], "receive") == 0)
    config->cm = RECEIVE;
  else {
    printf("Invalid connection mode. Must be 'send' or 'receive'\n");
    exit(1);
  }

  // Build the serial port file name from the port number given
  char str[64];
  sprintf(str, "/dev/ttys%s", argv[2]);

  // Check if serial port exists
  if (!file_exists(str)) {
    printf("Invalid port number given. Serial port %s does not exist.\n", str);
    exit(1);
  }

  // Set the serial port file descriptor
  config->fd = open(str, O_RDWR | O_NOCTTY);

  // If receiving, we're done setting the config
  if (config->cm == RECEIVE)
    return;

  // Prompt user for filename of file to be sent
  printf("Name of file to send ? ");
  memset(str, 0, sizeof(str));
  scanf("%[^\n]%*c", str);

  // Check if file exists
  if (!file_exists(str)) {
    printf("Invalid file name given. File %s does not exist.\n", str);
    exit(1);
  }

  // Set the filename
  config->filename = str;
}

int run(const config_t *config) {
  // Set the data link layer struct
  link_layer_t *ll = (link_layer_t*) malloc(sizeof(link_layer_t));
  set_link_layer(ll, config->fd, config->cm);

  // Establish connection
  if (llopen(ll) < 0)
    return -1;

  // Perform transfer
  if (config->cm == SEND)
    send_file(ll, config->filename);
  else
    receive_file(ll);

  // Close connection
  if (llclose(ll) <0 )
    return -1;

  // We're done
  return close(config->fd);
}

int send_file(link_layer_t *ll, const char *filename) {
  // Open file
  FILE* file = fopen(filename, "rb");

  // Close file
  fclose(file);

  return 0;
}

int receive_file(link_layer_t *ll) {


  return 0;
}

int main(int argc, const char **argv) {
  if (argc != 3) {
    printf("Usage: %s <send|receive> <serial port number>\n", argv[0]);
    return 1;
  }

  // Set the program config struct
  config_t *config = (config_t*) malloc(sizeof(config_t));
  set_config(config, argv);

  // We're good to go
  return run(config);
}
