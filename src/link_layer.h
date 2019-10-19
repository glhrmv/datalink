/**
 * @file link_layer.h
 * @brief Data link layer protocol definition
 * 
 */
#pragma once

#include <termios.h>

#include "config.h"
#include "conn_mode.h"
#include "util.h"

typedef enum { START, FLAG_RCV, A_RCV, C_RCV, BCC_OK, STOP } state_t;

typedef enum {
  C_SET = 0x03,
  C_UA = 0x07,
  C_RR = 0x05,
  C_REJ = 0x01,
  C_DISC = 0x0B
} control_field_t;

typedef enum { SET, UA, RR, REJ, DISC } message_command_t;

typedef enum { COMMAND, DATA, INVALID } message_type_t;

typedef enum { INPUT_OUTPUT_ERROR, BCC1_ERROR, BCC2_ERROR } message_err_t;

typedef enum {
  COMMAND_SIZE = 5 * sizeof(char),
  MESSAGE_SIZE = 6 * sizeof(char)
} message_size_t;

typedef struct {
  message_type_t type;
  message_command_t command;
  message_err_t err;
  int ns;
  int nr;
  struct {
    unsigned char *message;
    unsigned int message_size;
  } data;
} message_t;

typedef struct {
  conn_mode_t cm;          ///< Connection mode (SEND, RECEIVE)
  int fd;                  ///< Serial device file descriptor
  int baud_rate;           ///< Baud rate
  unsigned int seq_number; ///< Frame sequence number (0, 1)
  unsigned int timeout;    ///< Timeout limit
  unsigned int retries;    ///< Number of connection attempts in case of failure
  char frame[MAX_SIZE];    ///< Frame
  int message_data_max_size;
} link_layer_t;

/**
 * @brief Initializes link layer struct from the program config
 *
 * @param ll Link layer struct
 * @param config Program config struct
 * @return int 0 if successful, error otherwise
 */
int set_link_layer(link_layer_t *ll, const config_t *config);

/**
 * @brief Establish a serial port connection
 *
 * @param ll Link layer struct
 * @return int File descriptor for the serial port, 0 on error
 */
int llopen(link_layer_t *ll);

/**
 * @brief Writes a message through the serial port
 *
 * @param ll Link layer struct
 * @return int 0 if successful, error otherwise
 */
int llwrite(link_layer_t *ll);

/**
 * @brief Reads a message through the serial port
 *
 * @param ll Link layer struct
 * @return int 0 if successful, error otherwise
 */
int llread(link_layer_t *ll);

/**
 * @brief Closes the serial port connection
 *
 * @param ll Link layer struct
 * @return int 0 if successful, error otherwise
 */
int llclose(link_layer_t *ll);
