/**
 * @file conn_mode.h
 * @brief Connection mode enum definition
 *
 */

/// Connection mode type
typedef enum conn_mode {
  SEND,   ///< To be used by the sender
  RECEIVE ///< To be used by the receiver
} conn_mode;
