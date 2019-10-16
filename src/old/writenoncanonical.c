/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <signal.h>

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1


#define FLAG 0x7e
#define A 0x03


#define C_SET 0x03
#define C_UA 0x07 // Valor do UA
#define C_DISC 0x011
#define N 5    // Tamanho trama SET

//State machine
#define START     0
#define FLAG_RCV  1
#define A_RCV     2
#define C_RCV     3
#define BCC_OK    4
#define STOP      5
#define Other_RCV 6

#define SIZE 255

volatile int STOP_=FALSE;


int counter = 1;

struct termios oldtio,newtio;

void atende() {
  printf("contador # %d\n", counter);
  counter++;
}

int llopen(char* porta){
    int fd;
    char buffer[5];
    unsigned char frame_S[] = {FLAG, A, C_SET, A^C_SET, FLAG};

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

    printf("New termios structure set\n");

    write(fd, frame_S, strlen(frame_S));

    read(fd, buffer, N);

    if (stateMachine(buffer) != 0){
      exit(-1);
    }

    return fd;
}

unsigned char calculateBcc( char *data) {
  unsigned char bcc = data[0];
  for(int i = 1; i < strlen(data); i++) {
    bcc ^= data[i];
  }
  return bcc;
}

int llwrite(int fd, char * buffer, int length){
  unsigned char c = 0x00;
  unsigned char bcc2 = calculateBcc(buffer);
  unsigned char buff[5];
  unsigned char frame_I[] = {FLAG, A, c, A^c, buffer, bcc2, FLAG};

  write(fd, buffer, length);

  read(fd, buff, 5);

  c = 0x40;
}



int llread(int fd, char * buffer){
  
}

int stateMachine(char *buffer) {
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
        else if(c == C_UA || c == C_DISC)
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

int llclose(int fd){
  unsigned char frame_D[] = {FLAG, A, C_DISC, A^C_DISC, FLAG};
  unsigned char frame_U[] = {FLAG, A, C_UA, A^C_UA, FLAG};
  unsigned char buff[5];

  write(fd, frame_D, strlen(frame_D));

  read(fd, buff, 5);

  if (stateMachine(buff) != 0) {
    perror("Error disconect\n");
    return -1;
  }

  write(fd, frame_U, strlen(frame_U));

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
  	     ((strcmp("/dev/ttyS2", argv[1])!=0) &&
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }

    (void) signal(SIGALRM, atende);


  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */

    

    fd = llopen(argv[1]);
    if(fd < 0){
      printf("Error llopen\n");
      exit(1);
    }
    else {
      sleep(0);
      STOP_ = TRUE;
    }

    FILE *file = fopen("pinguim.gif", "r");

    fseek(file, 0, SEEK_END);
    unsigned long file_size = ftell(file);

    llwrite(fd, file, file_size);

    

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
