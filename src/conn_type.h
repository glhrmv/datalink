/**
 * @file conn_type.h
 * @brief Connection mode enum definition
 *
 */

#pragma once

/// Connection mode enum
typedef enum conn_type {
  SEND,   ///< To be used by the sender
  RECEIVE ///< To be used by the receiver
} conn_type_t;
