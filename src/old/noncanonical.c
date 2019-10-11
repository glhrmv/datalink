/*Non-Canonical Input Processing*/

#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define TRANSMITTER 0
#define RECEIVER 1

#define SIZE 255   // Frame size
#define FLAG 0x7e  // Flag Delimiter
#define A 0x01     // Receiver Adress Field Headr

//State machine
#define START     0
#define FLAG_RCV  1
#define A_RCV     2
#define C_RCV     3
#define BCC_OK    4
#define STOP      5
#define Other_RCV 6

// Control Field Messages
#define SET 0x03   // SET (set up)
#define DISC 0x0B  // DISC (disconnect)
//#define UA 0x07    // UA (Unnumbered acknowledgement)

volatile int STOP_ = FALSE;
struct termios oldtio, newtio;

int llopen(int port, char f) {
  int fd;

  if (f == TRANSMITTER) {
    // Transmitter code
    fd = open(port, O_RDWR | O_NOCTTY);
    if (fd < 0) return -1;


  } else {
    // Receiver code
    fd = open(port, O_RDWR | O_NOCTTY);
    if (fd < 0) return -1;

    // save current port settings
    tcgetattr(fd, &oldtio);

    bzero(&newtio, sizeof(newtio));

    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME] = 0; /* inter-character timer unused */
    newtio.c_cc[VMIN] = 5;  /* blocking read until 5 chars received */

    tcflush(fd, TCIFLUSH);
    tcsetattr(fd, TCSANOW, &newtio);
  }

  return fd;
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


int llclose(int fd) { 
  if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      return -1;
  }

  return close(fd); 
}

int main(int argc, char **argv) {
  unsigned char UA[5];
  unsigned char frame[SIZE];
  char buf[SIZE];
  int res;

  // Registo

  /*
          Open serial port device for reading and writing and not as controlling
     tty because we don't want to get killed if linenoise sends CTRL-C.
  */

  int fd = llopen(argv[1], RECEIVER);
  if (fd < 0) {
    perror(argv[1]);
    exit(-1);
  }

  printf("New termios structure set\n");

  if(llread(fd, frame) != 0){
    perror("llread\n");
    exit(1);
  }

  if(llwrite(fd, UA, 5) < 0){
    perror("llwrite");
    exit(1);
  }

  while (STOP_==FALSE) {       /* loop for input */
      res = read(fd,buf,255);   /* returns after 5 chars have been input */
      buf[res]=0;               /* so we can printf... */
      printf(":%s:%d\n", buf, res);
      if (buf[0]=='z') STOP_=TRUE;
    }

  if(llclose(fd) != 0){
    perror("llclose\n");
    exit(1);
  }

  return 0;
}
