#include "functions.h"

static char ControlByte = 0x00;
static int count = 0;
static int FD;

void alarmHandlerWrite(int sig){
 printf("Attempt number %d \n", count + 1);
 count ++;
 unblockReadPortSettings(FD);
}



int stuffTrame(char* trame, char * buffer, int length){
  printf("Stuffing Frame!!\n");
  trame[0] = FLAG;
  trame[1] = A;
  trame[2] = ControlByte;
  trame[3] = trame[1]^trame[2];
  int counter = 0;
  int realocSize = 4;
  int startPoint = 4;
  char bcc2 = 0x00;
  while(counter < length){
    char v = buffer[counter];
    printf("Buffer[%d]: %x \n", counter, v);
    if(v == FLAG){
      printf("EQUALS TO FLAG!");
      char subs[2] = {0x7d,0x5e};
      memcpy(&trame[startPoint+counter], subs, 2);
      startPoint++;
      realocSize+=2;
    }
    else if(v == ESCAPE_BYTE){
      printf("EQUALS TO ESCAPE BYTE!");
      char subs[2] = {0x7d,0x5d};
      memcpy(&trame[startPoint+counter], subs, 2);
      startPoint++;
      realocSize+=2;
    }
    else{
      printf("No stuffing needed!\n");
      trame[startPoint+counter] = v;
      realocSize++;
    }
    bcc2 ^= v;
    counter++;
  }
  printf("BCC2: %x\n", bcc2);
  int newSize = realocSize + 2;
  printf("New TRame Size: %d\n", newSize);
  trame = (char*) realloc(trame, newSize);
  trame[newSize - 2] = bcc2;
  trame[newSize - 1] = FLAG;
  return newSize;
}

int readResponse(int fd){
  int ret;
  char responseTrame[TRAME_SIZE];
  int test = read(fd, responseTrame,TRAME_SIZE);
  if( test == 0)
  {
    defaultPortSettings(fd);
    return -1;
  }

  else if(test == -1){
    return test;
  }
  if(responseTrame[0] != FLAG){
      printf("ERROR RECEIVER RESPONSE INVALID!\n");
      return -1;
  }
  if(responseTrame[1] != A){
      printf("ERROR RECEIVER RESPONSE INVALID!\n");
      return -1;
  }
  if(responseTrame[2] == RR_2){//Nr = 1
    printf("Receiver Ready 1\n");
    ControlByte = 0x40;
    alarm(0);
    ret = 0;
  }
  else if(responseTrame[2] == RR_1){//Nr = 0
    printf("Receiver Ready 0\n");
    ControlByte = 0x00;
    alarm(0);
    ret = 0;
  }

  else if(responseTrame[2] == REJ_2){//Nr = 1
    printf("Receiver REJECT 1\n");
    ControlByte = 0x40;
    alarm(0);
    ret = 1;
  }

  else if(responseTrame[2] == REJ_1){//Nr = 0
    printf("Receiver REJECT 0\n");
    ControlByte = 0x00;
    alarm(0);
    ret = 1;
  }
  else{
    printf("ERROR RECEIVER RESPONSE INVALID!\n");
    return -1;
  }
  char bcc1 = responseTrame[2] ^ responseTrame[1];
  if(bcc1 != responseTrame[3]){
    printf("ERROR RECEIVER RESPONSE INVALID!\n");
    return -1;
  }
  if(responseTrame[4] != FLAG){
      printf("ERROR RECEIVER RESPONSE INVALID!\n");
      return -1;
  }
  return ret;
}

int llwrite(int fd, char * buffer, int length){
  signal(SIGALRM, alarmHandlerWrite);
  FD = fd;
  printf("LLWRITE()");
  char* trame_I = (char *) malloc(FRAME_I_SIZE);
  int newSize = stuffTrame(trame_I, buffer, length);
  printf("New trame Size: %d\n", newSize);
  int test = write(fd,trame_I,newSize);

  if(test == -1){
    printf("Error LLWRITE()! Trame could not be written!\n");
    return -1;
  }
  else{
    printf("LLWRITE()! %d  bytes WAS WRITTEN.!\n", test);
    alarm(3);
  }
  int r;
  do {
    r = readResponse(fd);
  } while(count < 3);
  if( r == 0)
    return newSize;
  else if(r == 1)
    return 0;
  else return -1;
}
