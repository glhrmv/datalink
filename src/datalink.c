/**
 * @file datalink.c
 * @brief The datalink program starting point
 *
 */

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "ll.h"

conn_mode get_conn_mode(const char *cm) {
  if (strcmp(cm, "send") == 0)
    return SEND;
  else if (strcmp(cm, "receive") == 0)
    return RECEIVE;
  
  printf("Invalid connection mode. Must be 'send' or 'receive'\n");

  return 1;
}

int main(int argc, char **argv) {
  if (argc != 3) {
    printf("Usage: %s <send|receive> <serial port number>\n", argv[0]);
    return 1;
  }

  conn_mode cm = get_conn_mode(argv[1]);

  return 0;
}
