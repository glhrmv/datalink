#pragma once

#include <termios.h>

#include "util.h"
#include "conn_mode.h"
#include "config.h"

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

typedef struct link_layer {
	char port[20]; /*Dispositivo /dev/ttySx, x = 0, 1*/

	int baudRate; /*Velocidade de transmissão*/
	unsigned int sequenceNumber; /*Número de sequência da trama: 0, 1*/
	unsigned int timeout; /*Valor do temporizador: 1 s*/
	unsigned int numTransmissions; /*Número de tentativas em caso de falha*/

	char frame[MAX_SIZE]; /*Trama*/

	int messageDataMaxSize;
} link_layer;

extern link_layer* ll;

/**
 * @brief Establish a serial port connection
 * 
 * @param cm Connection mode
 * @return int File descriptor for the serial port, 0 on error
 */
int llopen(config* config);

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


/**
 * @brief 
 *
 *
*/
int initLinkLayer(const char* port, int baudRate,
		int messageDataMaxSize, int numTransmissions, int timeout);

/**
 * @brief Create command to send
 *
 * @param command buffer where the command will be saved
 * @return int 0 if successfull, error otherwise
*/
int createCommand(control_field C, char *command);

/**
 * @brief Send command mensage to certain port
 *
 * @param fd File descriptor for the serial port
 * @param msg_command type of command to be send
 * @return int 0 if successful, error otherwise
*/
int sendCommand(int fd, msg_command msg_command);

/**
 * @brief get control field for msg
 * 
 * @param msg_commandstr	string to be updated with the control field
 * @param msg_command		type of command 
 * @return control_field of the msg
 * 
 * if the msg_command is not recugnized it will return C_SET, but
 * the string msg_commandstr will be "ERROR" so we can detect if it is wrong
*/
control_field getCommandControl_Field(char* msg_commandstr, msg_command msg_command);
