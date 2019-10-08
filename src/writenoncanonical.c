/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define FLAG 0x7e
#define A 0x03
#define C 0x03

#define SIZE 255

#define TRANSMITTER 0
#define RECEIVER 1

volatile int STOP=FALSE;

int llopen(int porta, int flag){
  
  int fd;
  if(flag == TRANSMITTER){
    fd = open(porta, O_RDWR | O_NOCTTY );
    if (fd <0) {perror(porta); return -1; }
  }
  else if(flag == RECEIVER){
    fd = open(porta, O_RDWR | O_NOCTTY );
    if (fd <0) {perror(porta); return -1; }
  }

  return fd;

}

int llwrite(int fd, char * buffer, int length){
  int res = -1;
  res = write(fd, buffer, length);

  return res;
}

int llread(int fd, char * buffer){

}

int llclose(int fd){
  return close(fd);
}



int main(int argc, char** argv)
{
    int fd,c, res;
    struct termios oldtio,newtio;
    char buf[SIZE];
    unsigned char SET[5];

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

    fd = llopen(argv[1], TRANSMITTER);
    if(fd < 0){
      printf("Error llopen\n");
      exit(1);
    }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 5 chars received */



  /*
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
    leitura do(s) pr�ximo(s) caracter(es)
  */



    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    printf("New termios structure set\n");


    SET[0] = FLAG;
    SET[1] = A;
    SET[2] = C;
    SET[3] = A^C;
    SET[4] = FLAG;

    if(llwrite(fd, SET, 5) < 0){
      printf("Error llwrite\n");
      exit(1);
    }

    gets(buf);

    buf[strlen(buf)] = '\0';
    
    res = write(fd,buf,strlen(buf)+1);   
    printf("%d bytes written\n", res);

    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    if(llclose(fd) != 0){
      printf("Error llclose\n");
      exit(1);
    }
    return 0;
}
