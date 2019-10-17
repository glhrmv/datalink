#include <termios.h>

#include "conn_mode.h"
#include "util.h"
#include "ll.h"



const int FLAG = 0x7E;
const int A = 0x03;


int initLinkLayer(const char* port, int baudRate,
		int messageDataMaxSize, int numTransmissions, int timeout) {

	LinkLayer* ll = (LinkLayer*) malloc(sizeof(LinkLayer));

	strcpy(ll->port, port);
	ll->baudRate = baudRate;
	ll->messageDataMaxSize = messageDataMaxSize;
	ll->sequenceNumber = 0;
	ll->timeout = timeout;
	ll->numTransmissions = 1 + numTransmissions;

    //Trama nao definido porque nao é preciso na inicializacao
	return 0;
}


int llopen(config* config) {
    printf("Trying to start connection\n");

    int connected = 0;
    int try = 0;
    switch (config->cm){
        case SEND:
        {
            while(!connected){
                //....//
                if(!sendCommand(config->fd, SET))
                
                break;
            }
            break;
        }
        
        case RECEIVE:
        {
            break;
        }
    }
    return 0;
}

int llwrite(int fd, const unsigned char* buf, unsigned int buf_size) {

}

int llread(int fd, unsigned char** msg) {

}

int llclose(int fd, conn_mode cm) {

}



int createCommand(control_field C, char *command){
    command[0] = FLAG;
    command[1] = A;
    command[2] = C;
    command[3] = command[1]^command[2];
    command[4] = FLAG;
}


int sendCommand(int fd, msg_command msg_command){
    char msg_commandstr[MAX_SIZE];

    control_field ctrl_field = getCommandControl_Field(msg_commandstr, msg_command);
    char* commandbuffer = malloc(COMMAND_SIZE);

    createCommand(ctrl_field, commandbuffer);

    return 0;
    //..........//
}


control_field getCommandControl_Field(char* msg_commandstr, msg_command msg_command){
    switch (msg_command){
        case SET:
            strcpy(msg_commandstr, "SET");
            return C_SET;
        case UA:
            strcpy(msg_commandstr, "UA");
            return C_UA;
        case RR:
            strcpy(msg_commandstr, "RR");
            return C_RR;
        case REJ:
            strcpy(msg_commandstr, "REJ");
            return C_REJ;
        case DISC:
            strcpy(msg_commandstr, "DISC");
            return C_DISC;
        default:
            strcpy(msg_commandstr, "ERROR");
            return C_SET;
    }
}