/**
 * @file config.h
 * @brief Program configuration structure definition
 *
 */

#pragma once

#include "conn_mode.h"

/// Program config struct
typedef struct config {
  int fd;          ///< Serial port file descriptor
  char* filename;  ///< Name of file to be transferred
  conn_mode cm;    ///< Connection mode
} config;
