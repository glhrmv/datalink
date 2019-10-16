#pragma once

#include <termios.h>

#include "conn_mode.h"

typedef enum state {
	START, 
	FLAG_RCV, 
	A_RCV, 
	C_RCV, 
	BCC_OK, 
	STOP
} state;

typedef enum control_field {
	C_SET = 0x03, 
	C_UA = 0x07, 
	C_RR = 0x05, 
	C_REJ = 0x01, 
	C_DISC = 0x0B
} control_field;

typedef enum msg_command {
	SET, UA, RR, REJ, DISC
} msg_command;

typedef enum msg_type {
	COMMAND, DATA, INVALID
} msg_type;

typedef enum msg_err {
	INPUT_OUTPUT_ERROR, BCC1_ERROR, BCC2_ERROR
} msg_err;

typedef enum msg_size {
	COMMAND_SIZE = 5 * sizeof(char), 
	MESSAGE_SIZE = 6 * sizeof(char)
} msg_size;

typedef struct msg {
	msg_type type;
	msg_command command;
	
	int ns;
	int nr;

	struct {
		unsigned char* msg;
		unsigned int msg_size;
	} data;

	msg_err err;
} msg;

/**
 * @brief Establish a serial port connection
 * 
 * @param cm Connection mode
 * @return int File descriptor for the serial port, 0 on error
 */
int llopen(conn_mode cm);

/**
 * @brief Writes a message through the serial port
 * 
 * @param fd File descriptor for the serial port
 * @param buf Buffer of data to write
 * @param buf_size Size of buffer
 * @return int 0 if successful, error otherwise
 */
int llwrite(int fd, const unsigned char* buf, unsigned int buf_size);

/**
 * @brief Reads a message through the serial port
 * 
 * @param fd File descriptor for the serial port
 * @param msg Message to be read 
 * @return int 0 if successful, error otherwise
 */
int llread(int fd, unsigned char** msg);

/**
 * @brief Close the serial port connection
 * 
 * @param fd File descriptor for the serial port
 * @param mode Connection mode
 * @return int 0 if successful, error otherwise
 */
int llclose(int fd, conn_mode cm);
