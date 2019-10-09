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

// Control Field Messages
#define SET 0x03   // SET (set up)
#define DISC 0x0B  // DISC (disconnect)
#define UA 0x07    // UA (Unnumbered acknowledgement)

volatile int STOP = FALSE;
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
  while(true)
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
        if(a^b == buffer[0])
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
      case default:
      return 1;
    }
  }
}


int llclose(int port) { return close(port); }

int main(int argc, char **argv) {
  if ((argc < 2) || ((strcmp("/dev/ttyS0", argv[1]) != 0) &&
                     (strcmp("/dev/ttyS1", argv[1]) != 0))) {
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
    exit(1);
  }

  // Registo

  /*
          Open serial port device for reading and writing and not as controlling
     tty because we don't want to get killed if linenoise sends CTRL-C.
  */

  int port = llopen(argv[1], RECEIVER);
  if (port < 0) {
    perror(argv[1]);
    exit(-1);
  }

  printf("New termios structure set\n");

  unsigned char frame[SIZE];
  llread(port, frame);

  llclose(port);

  return 0;
}
