/**
 * @file conn_type.h
 * @brief Connection mode enum definition
 *
 */

#pragma once

/// Connection mode enum
typedef enum {
  SEND,   ///< To be used by the sender
  RECEIVE ///< To be used by the receiver
} conn_type_t;
