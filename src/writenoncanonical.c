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
#define N 5

//State machine
#define START     0
#define FLAG_RCV  1
#define A_RCV     2
#define C_RCV     3
#define BCC_OK    4
#define STOP      5
#define Other_RCV 6

#define SIZE 255

#define TRANSMITTER 0
#define RECEIVER 1

volatile int STOP_=FALSE;


struct termios oldtio,newtio;

int llopen(int porta, int flag){

  int fd;
  if(flag == TRANSMITTER){
    fd = open(porta, O_RDWR | O_NOCTTY );
    if (fd <0) {perror(porta); return -1; }

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

    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }
  }
  else if(flag == RECEIVER){
    fd = open(porta, O_RDWR | O_NOCTTY );
    if (fd <0) {perror(porta); return -1; }
  }

  return fd;
}

int llwrite(int fd, char * buffer, int length){
  return write(fd, buffer, length);
}

int llread(int fd, char * buffer){
  int state = 0;
  char flag1, flag2, a, c;
  while(TRUE)
  {
    switch (state) {

      //START*******************************
      case START:
        if(read(fd, &flag1, 1))
        {
          if(flag1 == FLAG)
            state = FLAG_RCV;
        }
        else
          state = Other_RCV;
        break;

      //FLAG_RCV****************************
      case FLAG_RCV:
        if(read(fd, &a, 1))
        {
          if(a == FLAG)
            state = FLAG_RCV;
          else
          state = A_RCV;
        }
        else
          state = Other_RCV;
        break;

      //A_RCV*******************************
      case A_RCV:
      if(read(fd, &c, 1))
      {
        if(c == FLAG)
          state  = FLAG_RCV;
        else
        state = C_RCV;
      }
      else
      state = Other_RCV;
      break;


      //C_RCV********************************
      case C_RCV:
        if(read(fd, buffer, 1))
        {
          if(a^c == buffer[0])
            state = BCC_OK;
          else
          if(buffer[0] == FLAG)
            state = FLAG_RCV;
        }
        else
          state = Other_RCV;
        break;

      //BCC_OK********************************
      case BCC_OK:
        if(read(fd, &flag2, 1))
        {
          if(flag2 == FLAG)
            state = STOP;
        }
        else
          state = Other_RCV;
        break;

      //STOP***********************************
      case STOP:
        return 0;


      //Other_RCV******************************
      case Other_RCV:
        state = START;
        break;

      //default********************************
      default:
      return 1;
    }
  }
}

int close(int fd){
  if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      return -1;
  }
  
  return close(fd);
}


int main(int argc, char** argv)
{
    int fd,c, res;
    char buf[SIZE];
    unsigned char SET[N];

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

    printf("New termios structure set\n");


    SET[0] = FLAG;
    SET[1] = A;
    SET[2] = C;
    SET[3] = A^C;
    SET[4] = FLAG;

    if(llwrite(fd, SET, N) < 0){
      printf("Error llwrite\n");
      exit(1);
    }

    if(llread(fd, buf) != 0){
      printf("Error llread\n");
      exit(1);
    }

    gets(buf);

    buf[strlen(buf)] = '\0';

    res = write(fd,buf,strlen(buf)+1);
    printf("%d bytes written\n", res);

    

    if(llclose(fd) != 0){
      printf("Error llclose\n");
      exit(1);
    }
    return 0;
}
