/**
 * @file config.h
 * @brief The datalink program application layer header file
 *
 * This is the application layer of the project.
 *
 * It is also the starting point of the built program.
 *
 */

#pragma once

#include "conn_type.h"
#include "link_layer.h"
#include "util.h"

typedef enum packet_type {
  PACKET_TYPE_DATA = 1,
  PACKET_TYPE_START = 2,
  PACKET_TYPE_END = 3
} packet_type_t;

typedef enum packet_field { FILE_SIZE_FIELD, FILE_NAME_FIELD } packet_field_t;

typedef struct packet {
  packet_type_t type;
  char *file_name;
  int file_size;
  char *file_size_buf;
} packet_t;

/**
 * @brief Program config struct
 *
 */
typedef struct {
  char *port;      ///< Path of serial port device
  char *file_name; ///< Path of file to be transferred
  conn_type_t ct;  ///< Connection type
} config_t;

/**
 * @brief Set the program config struct
 *
 * Parses argv and sets the program config struct
 * accordingly. If the user is running in SEND mode,
 * prompts the user for path of file to transfer.
 *
 * Exits with -1 in case of error.
 *
 * @param config Program config struct
 * @param argv Program arguments
 */
void set_config(config_t *config, const char **argv);

/**
 * @brief Run the main program logic
 *
 * Essentially, uses the link layer API to perform
 * the file transfer.
 *
 * In case of SEND, calls send_file.
 * In case of RECEIVE, calls receive_file.
 *
 * @param config Program config struct
 * @return int 0 on success, error otherwise
 */
int run(const config_t *config);

int send_file(link_layer_t *ll, char *file_name);
int receive_file(link_layer_t *ll);

int send_control_packet(link_layer_t *ll, const packet_t *packet);
int receive_control_packet(link_layer_t *ll, packet_t *packet);

int send_data_packet(link_layer_t *ll, int N, const char *buf, int length);
int receive_data_packet(link_layer_t *ll, int *N, char **buf, int *length);
