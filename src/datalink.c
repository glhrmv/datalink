/**
 * @file datalink.c
 * @brief The datalink program starting point
 *
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "util.h"

void set_program_config(config *config, const char *cm,
                        const char *port_number) {
  if (strcmp(cm, "send") == 0)
    config->cm = SEND;
  else if (strcmp(cm, "receive") == 0)
    config->cm = RECEIVE;
  else {
    printf("Invalid connection mode. Must be 'send' or 'receive'\n");
    exit(1);
  }

  // TODO: Set config->fd to the file descriptor of "/dev/ttySx", x= port_number

  // TODO: If sender, prompt user for filename,
  //       check if it exists, and set config->filename to it
}

int main(int argc, char **argv) {
  if (argc != 3) {
    printf("Usage: %s <send|receive> <serial port number>\n", argv[0]);
    return 1;
  }

  config *config;
  set_program_config(config, argv[1], argv[2]);
  return run(config);
}

int run(config *config) {
  // TODO: Program logic

  return 0;
}
