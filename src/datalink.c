/**
 * @file datalink.c
 * @brief The datalink program starting point
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ll.h"

int main(int argc, char **argv) {
  if ((argc < 2) || ((strcmp("/dev/ttyS0", argv[1]) != 0) &&
                     (strcmp("/dev/ttyS1", argv[1]) != 0))) {
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
    exit(1);
  }

  printf("hello world\n");

  return 0;
}
