#include "functions.h"

static char ControlByte = 0x00;
static int count = 0;
static int FD;
static int stop = 0;

void alarmHandlerWrite(int sig){
 printf("Attempt number %d \n", count);
 count ++;
 unblockReadPortSettings(FD);
}

int receiveMessageWrite(int fd, unsigned char * buffer){
  char r;
  int count = 0;
  int test = read(fd,&r,1);
  if(test == -1){
    printf("ERROR READING RESPONSE BYTE!\n");
    return -1;
  }
  if( test == 0){
    printf("this is a 0\n");
    defaultPortSettings(fd);
    return 1;
  }
  printf("TEST_ %d\n", test);
  printf("R: %x\n" ,r);
  buffer[count] = r;
  count++;
  do {
    int test = read(fd,&r,1);
    if(test == -1){
      printf("ERROR READING RESPONSE BYTE!\n");
      return -1;
    }
    printf("R: %x\n", r);
    buffer[count]=r;
    count++;
  } while(r != FLAG);
  return 0;
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
  printf("New Trame Size: %d\n", newSize);
  trame = (char*) realloc(trame, newSize);
  trame[newSize - 2] = bcc2;
  trame[newSize - 1] = FLAG;
  return newSize;
}

int readResponse(int fd){
  printf("Reading response!\n");
  int ret;
  unsigned char responseTrame[TRAME_SIZE];
  int test = receiveMessageWrite(fd, responseTrame);
  if(test == -1){
    return test;
  }
  else if(test == 1){
    return 1;
  }
  if(responseTrame[0] != FLAG){
      printf("ERROR RECEIVER RESPONSE INVALID FLAG INITIAL!\n");
      return -1;
  }
  if(responseTrame[1] != A){
      printf("ERROR RECEIVER RESPONSE INVALID A!\n");
      return -1;
  }
  if(responseTrame[2] == RR_2){//Nr = 1
    printf("Receiver Ready 1\n");
    ControlByte = 0x40;
    ret = 0;
  }
  else if(responseTrame[2] == RR_1){//Nr = 0
    printf("Receiver Ready 0\n");
    ControlByte = 0x00;
    ret = 0;
  }

  else if(responseTrame[2] == REJ_2){//Nr = 1
    printf("Receiver REJECT 1\n");
    ControlByte = 0x40;
    ret = 1;
  }

  else if(responseTrame[2] == REJ_1){//Nr = 0
    printf("Receiver REJECT 0\n");
    ControlByte = 0x00;
    ret = 1;
  }
  else{
    printf("ERROR RECEIVER RESPONSE INVALID CONTROL FIELD!\n");
    return -1;
  }
  unsigned char bcc1 = responseTrame[2] ^ responseTrame[1];
  if(bcc1 != responseTrame[3]){
    printf("ERROR RECEIVER RESPONSE INVALID BCC1!\n");
    return -1;
  }
  if(responseTrame[4] != FLAG){
      printf("ERROR RECEIVER RESPONSE INVALID FLAG FINAL!\n");
      return -1;
  }
  stop = 1;
  alarm(0);
  return ret;
}

int llwrite(int fd, char * buffer, int length){
  signal(SIGALRM, alarmHandlerWrite);
  FD = fd;
  printf("LLWRITE()");
  char* trame_I = (char *) malloc(FRAME_I_SIZE);
  int newSize = stuffTrame(trame_I, buffer, length);
  int test = write(fd,trame_I,newSize);
  if(test == -1){
    printf("Error LLWRITE()! Trame could not be written!\n");
    return -1;
  }
  else{
    printf("LLWRITE()! %d  bytes were WRITTEN.!\n", test);
  }
  int r;
  do {
    printf("Trying to read response %d!\n", count);
    alarm(3);
    r = readResponse(fd);
  } while(count < 5 && !stop);
  if( r == 0)
  {
    return newSize;
  }
  else if(count >= 5)
  {
    printf("TIMEOUT!\n");
    return -1;
  }
  else if(r == 1)
  {
    return 0;
  }
  else return -1;
}
