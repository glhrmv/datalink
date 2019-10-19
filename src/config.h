/**
 * @file config.h
 * @brief Program configuration structure type definition
 *
 * This is the application layer.
 *
 */

#pragma once

#include "conn_mode.h"

/// Program config struct
typedef struct {
  int fd;         ///< Serial port file descriptor
  char *filename; ///< Path of file to be transferred
  conn_mode_t cm; ///< Connection mode
} config_t;
