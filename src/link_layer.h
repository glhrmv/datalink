/**
 * @file link_layer.h
 * @brief Data link layer protocol definition
 *
 */
#pragma once

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>

#include "conn_mode.h"
#include "util.h"

#define FLAG 0x7E
#define A 0x03
#define ESCAPE 0x7D

typedef enum { START, FLAG_RCV, A_RCV, C_RCV, BCC_OK, STOP } state_t;

typedef enum { SET, UA, RR, REJ, DISC } command_t;

typedef enum { COMMAND, DATA, INVALID } type_t;

typedef enum { IO_ERR, BCC1_ERR, BCC2_ERR } err_t;

typedef enum {
  C_SET = 0x03,
  C_UA = 0x07,
  C_RR = 0x05,
  C_REJ = 0x01,
  C_DISC = 0x0B
} control_field_t;

typedef enum {
  COMMAND_SIZE = 5 * sizeof(char),
  MESSAGE_SIZE = 6 * sizeof(char)
} message_size_t;

typedef struct {
  struct {
    unsigned char *message;
    unsigned int message_size;
  } data;

  type_t type;
  command_t command;
  err_t err;
  
  int ns;
  int nr;
} message_t;

/**
 * @brief Link layer structure
 * 
 */
typedef struct {
  int fd;         ///< Serial device file descriptor
  conn_mode_t cm; ///< Connection mode (SEND, RECEIVE)

  int baud_rate;           ///< Baud rate
  unsigned int seq_number; ///< Frame sequence number (0, 1)
  unsigned int timeout;    ///< Timeout interval (seconds)
  unsigned int retries;    ///< Number of connection attempts in case of failure
  char frame[MAX_SIZE];    ///< Frame

  int message_data_max_size; ///< Maximum message data size

  struct termios old_tio, new_tio; ///< Old and new termio
} link_layer_t;

/**
 * @brief Initializes link layer struct from the program config
 *
 * @param ll Link layer struct
 * @return int 0 if successful, error otherwise
 */
int set_link_layer(link_layer_t *ll, int fd, const conn_mode_t cm);

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

unsigned char *create_command(link_layer_t *ll, control_field_t cf);
int send_command(link_layer_t *ll, command_t command);
command_t get_command(control_field_t cf);
control_field_t get_command_w_control_field(char *command_str, command_t command);

unsigned int stuff_buffer(unsigned char **buf, unsigned int buf_size);
unsigned int destuff_buffer(unsigned char **buf, unsigned int buf_size);
