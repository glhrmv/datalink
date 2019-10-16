/*Non-Canonical Input Processing*/

#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <signal.h>

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define SIZE 255   // Frame size

#define FLAG 0x7e  // Flag Delimiter
#define A 0x03     // Receiver Adress Field Headr

// Control field messages
#define C_SET 0x03    // Valor do SET
#define C_UA 0x07
#define C_DISC 0x0b


#define N 5       // Tamanho da trama I

//State machine
#define START     0
#define FLAG_RCV  1
#define A_RCV     2
#define C_RCV     3
#define BCC_OK    4
#define STOP      5
#define Other_RCV 6

volatile int STOP_ = FALSE;
struct termios oldtio, newtio;

int llopen(char* port) {
    int fd;
    char buffer[N];
    unsigned char UA[N];

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

    read(fd, buffer, N);

    if(stateMachine(buffer) != 0){
      exit(-1);
    }

    UA[0] = FLAG;
    UA[1] = A;
    UA[2] = C_UA;
    UA[3] = A^C_UA;
    UA[4] = FLAG;

    write(fd, UA, N);

    return fd;
}

int llread(int fd, char * buffer){

}

int stateMachine(char* buffer) {
  int state = 0;
  char flag1, flag2, a, c, bcc;

  flag1 = buffer[0];
  a = buffer[1];
  c = buffer[2];
  bcc = buffer[3];
  flag2 = buffer[4];
  
  while(TRUE)
  {
    switch (state) {
      //START*******************************
      case START:
        if(flag1 == FLAG)
          state = FLAG_RCV;
        else
          state = START;
      break;

      //FLAG_RCV****************************
      case FLAG_RCV:
        if(a == FLAG)
          state = FLAG_RCV;
        else if(a == A)
          state = A_RCV;
        else
          state = START;
      break;

      //A_RCV*******************************
      case A_RCV:
        if(c == FLAG)
          state  = FLAG_RCV;
        else if(c == C_SET)
          state = C_RCV;
        else
          state = START;
      break;

      //C_RCV********************************
      case C_RCV:
        if(a^c == bcc)
          state = BCC_OK;
        else if(a^c == FLAG)
          state = FLAG_RCV;
        else
          state = START;
      break;

      //BCC_OK********************************
      case BCC_OK:
        if(flag2 == FLAG)
          state = STOP;
        else
         state = START;
      break;

      //STOP***********************************
      case STOP:
        return 0;

      //default********************************
      default:
        return 1;
    }
  }
}

int llwrite(int fd, char* buffer, int length) {
  
}


int llclose(int fd) { 
  if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      return -1;
  }

  return close(fd); 
}

int main(int argc, char **argv) {
  if ((argc < 2) || ((strcmp("/dev/ttyS2", argv[1]) != 0) &&
                     (strcmp("/dev/ttyS1", argv[1]) != 0))) {
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
    exit(1);
  }

  unsigned char UA[N];
  unsigned char frame[SIZE];
  char buf[SIZE];
  int res;


  int fd = llopen(argv[1]);
  if (fd < 0) {
    perror(argv[1]);
    exit(-1);
  }

  printf("New termios structure set\n");

  if(llread(fd, frame) != 0){
    perror("llread\n");
    exit(1);
  }

  printf("Read trama I from writer\n");


  if(llwrite(fd, UA, N) < 0){
    perror("llwrite");
    exit(1);
  }

  printf("Returnded confirmation for writer\n");



  while (STOP_==FALSE) {       /* loop for input */
      res = read(fd,buf,255);   /* returns after 5 chars have been input */
      buf[res]=0;               /* so we can printf... */
      printf(":%s:%d\n", buf, res);
      if (buf[res]=='\0') STOP_=TRUE;
    }





  if(llclose(fd) != 0){
    perror("llclose\n");
    exit(1);
  }

  return 0;
}
