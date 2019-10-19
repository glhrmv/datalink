#include <stdlib.h>
#include <termios.h>

#include "conn_mode.h"
#include "link_layer.h"

#include "util.h"

const int FLAG = 0x7E;
const int A = 0x03;

int set_link_layer(link_layer_t *ll, const config_t *config) {
  ll->fd = config->fd;
  // Below are default values
  // TODO: Make these configurable by the user
  ll->baud_rate = B38400;
  ll->message_data_max_size = 512;
  ll->seq_number = 0;
  ll->timeout = 3;
  ll->retries = 3;

  return 0;
}

int llopen(link_layer_t *ll) {
  printf("Establishing connection..\n");

  int connected = 0, tries = 0;
  switch (ll->cm) {
  case SEND: {
    while (!connected) {
      
    }
    break;
  }

  case RECEIVE: {
    break;
  }
  }
  return 0;
}

int llwrite(link_layer_t *ll) {
  return 0;
}

int llread(link_layer_t *ll) {
  return 0;
}

int llclose(link_layer_t *ll) {
  printf("Closing connection..\n");
  
  return 0;
}
