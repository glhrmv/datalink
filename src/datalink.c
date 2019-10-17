/**
 * @file datalink.c
 * @brief The datalink program starting point
 *
 */

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "config.h"
#include "util.h"

void set_program_config(config *config, const char **argv) {
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
  char fname[64];
  sprintf(fname, "/dev/ttys%s", argv[2]);

  // Check if serial port exists
  if (!file_exists(fname)) {
    printf("Invalid port number given. Serial port does not exist.\n");
    exit(1);
  }

  // Set the serial port file descriptor
  config->fd = open(fname, O_RDWR | O_NOCTTY);

  // If receiving, we're done setting the config
  if (config->cm == RECEIVE)
    return;

  // Prompt user for filename of file to be sent
  printf("Name of file to send ? ");
  char str[64];
  scanf("%[^\n]%*c", str);

  // Check if file exists
  if (!file_exists(str)) {
    printf("Invalid file name given. File does not exist.\n");
    exit(1);
  }

  // Set the filename
  config->filename = str;
}

int run(config config) {
  // TODO: Program logic

  return 0;
}

int main(int argc, const char **argv) {
  if (argc != 3) {
    printf("Usage: %s <send|receive> <serial port number>\n", argv[0]);
    return 1;
  }

  // Set the program config
  config config;
  set_program_config(&config, argv);

  return run(config);
}
