/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define SIZE 255 // Frame size
#define FLAG 0x7e // Flag Delimiter
#define A 0x01 // Receiver Adress Field Headr

// Control Field Messages
#define SET 0x03 // SET (set up)
#define DISC 0x0B // DISC (disconnect)
#define UA 0x07 // UA (Unnumbered acknowledgement)
#define BCC A^C
#define TRANSMITTER 0
#define RECEIVER 1

volatile int STOP = FALSE;

int llopen(int port, char f) {
	struct termios oldtio, newtio;
	int fd; 
	
	if (f == TRANSMITTER) {
		// Transmitter code
		fd = open(port, O_RDWR | O_NOCTTY)	
		
		if (fd < 0)
			return -1;
			
		
	} else {
		// Receiver code
		fd = open(port, O_RDWR | O_NOCTTY);
			
		if (fd < 0)
			return -1;
		
		// save current port settings
		tcgetattr(fd, &oldtio); 

		bzero(&newtio, sizeof(newtio));
		
		newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
		newtio.c_iflag = IGNPAR;
		newtio.c_oflag = 0;

		/* set input mode (non-canonical, no echo,...) */
		newtio.c_lflag = 0;

		newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
		newtio.c_cc[VMIN]     = 5;   /* blocking read until 5 chars received */

		tcflush(fd, TCIFLUSH);
		tcsetattr(fd,TCSANOW,&newtio);
	}
	
	return fd;
}

int llread(int port, unsigned char** frame) {
	int res;
	
	// loop for input
	while (STOP==FALSE) {
      res = read(fd,frame,255);
      
      // so we can printf...
      frame[res] = 0; 
      printf(":%s:%d\n", frame, res);
      
      if (frame[0] == 'z') 
		STOP = TRUE;
    }
    
    tcsetattr(fd, TCSANOW, &oldtio);
}

int llclose(int port) {
	return 	close(port);
}

int main(int argc, char** argv) {
	if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }

	/*
		Open serial port device for reading and writing and not as controlling tty
		because we don't want to get killed if linenoise sends CTRL-C.
	*/
	fd = llopen(argv[1], RECEIVER);    
    if (fd <0) {
		perror(argv[1]); exit(-1); 
	}
    
    printf("New termios structure set\n");

	unsigned char frame[SIZE];  
	llread(fd, frame);
    
    llclose(fd);
    
    return 0;
}
