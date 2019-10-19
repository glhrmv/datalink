/**
 * @file config.h
 * @brief The datalink program application layer header file
 *
 * This is the application layer.
 *
 */

#pragma once

#include "conn_mode.h"
#include "link_layer.h"

/// Program config struct
typedef struct {
  int fd;         ///< Serial port file descriptor
  char *filename; ///< Path of file to be transferred
  conn_mode_t cm; ///< Connection mode
} config_t;

void set_config(config_t *config, const char **argv);

int run(const config_t *config);

int send_file(link_layer_t *ll, const char *filename);

int receive_file(link_layer_t *ll);

int send_control_packet(int fd, int C, char* fileSize, char* fileName);
int receive_control_packet(int fd, int ctrl, int fileLength, char** fileName);

int send_data_packet(int fd, int N, const char* buf, int length);
int receive_data_packet(int fd, int* N, char** buf, int length);
