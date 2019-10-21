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

typedef enum {
  CONTROL_PACKET_DATA = 1,
  CONTROL_PACKET_START = 2,
  CONTROL_PACKET_END = 3
} packet_type_t;

/**
 * @brief Program config struct
 *
 */
typedef struct {
  char *port;     ///< Path of serial port device
  char *filename; ///< Path of file to be transferred
  conn_type_t ct; ///< Connection type
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

int send_file(link_layer_t *ll, const char *filename);

int receive_file(link_layer_t *ll);

int send_control_packet(int fd, int C, char *file_size, char *filename);
int receive_control_packet(int fd, int ctrl, int *file_length, char **filename);

int send_data_packet(int fd, int N, const char *buf, int length);
int receive_data_packet(int fd, int *N, char **buf, int length);
