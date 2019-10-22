/**
 * @file link_layer.h
 * @brief The datalink program data link layer header file
 *
 * This is the data link layer of the project.
 *
 * The API is defined in the documentation below.
 *
 */
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "conn_type.h"

#define MAX_SIZE 256

typedef enum state { START, FLAG_RCV, A_RCV, C_RCV, BCC_OK, STOP } state_t;

typedef enum command { I, SET, DISC, UA, RR, REJ } command_t;

typedef enum type { COMMAND, DATA, INVALID } type_t;

typedef enum err { IO_ERR, BCC1_ERR, BCC2_ERR } err_t;

typedef enum control_field {
  C_SET = 0x03,
  C_UA = 0x07,
  C_RR = 0x05,
  C_REJ = 0x01,
  C_DISC = 0x0B
} control_field_t;

typedef enum message_size {
  COMMAND_SIZE = 5 * sizeof(char),
  MESSAGE_SIZE = 6 * sizeof(char)
} message_size_t;

typedef struct message {
  struct {
    char *message;
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
typedef struct link_layer {
  int fd;         ///< Serial port device file descriptor
  conn_type_t ct; ///< Connection type (SEND, RECEIVE)

  unsigned int seq_number;   ///< Frame sequence number (0, 1)
  int baud_rate;             ///< Baud rate
  int message_data_max_size; ///< Maximum message data size
  unsigned int timeout;      ///< Timeout interval (seconds)
  unsigned int retries; ///< Number of connection attempts in case of failure
  char frame[MAX_SIZE]; ///< Frame

  struct termios old_termios, new_termios; ///< Old and new termio
} link_layer_t;

/**
 * @brief Initializes link layer struct
 *
 * Requires the serial port device file descriptor and
 * the connection type to be provided as parameters.
 *
 * @param ll Link layer struct
 * @param port Serial port device filepath
 * @param ct Connection type
 * @return int 0 if successful, error otherwise
 */
int set_link_layer(link_layer_t *ll, char *port, const conn_type_t ct);

/**
 * @brief Establish a serial port connection
 *
 * @param ll Link layer struct
 * @return int File descriptor for the serial port on success, -1 on error
 */
int llopen(link_layer_t *ll);

/**
 * @brief Writes a message through the serial port
 *
 * @param ll Link layer struct
 * @param buf Character buffer to write
 * @param buf_size Size of character buffer
 * @return int Number of characters written on success, -1 on error
 */
int llwrite(link_layer_t *ll, char *buf, int buf_size);

/**
 * @brief Reads a message through the serial port
 *
 * @param ll Link layer struct
 * @param buf Received character buffer
 * @return int Number of characters read on success, -1 on error
 */
int llread(link_layer_t *ll, char *buf);

/**
 * @brief Closes the serial port connection
 *
 * @param ll Link layer struct
 * @return int 0 if successful, error otherwise
 */
int llclose(link_layer_t *ll);

char *create_command(link_layer_t *ll, control_field_t cf);
int send_command(link_layer_t *ll, command_t command);
command_t get_command(control_field_t cf);
control_field_t get_command_w_control_field(char *command_str,
                                            command_t command);

int receive_message(link_layer_t *ll, message_t *msg);

char process_bcc(const char* buf, unsigned int buf_size);

unsigned int stuff_buffer(char *buf, unsigned int buf_size);
unsigned int destuff_buffer(char *buf, unsigned int buf_size);
