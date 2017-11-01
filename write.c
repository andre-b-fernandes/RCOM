#include "functions.h"

static unsigned char ControlByte = 0x00;
int counter = 0;
static int FD;
int stop = 0;

void alarmHandlerWrite(int sig){
 printf("Attempt number %d \n", counter);
 counter++;
 unblockReadPortSettings(FD);
}

int receiveMessageWrite(int fd, unsigned char * buffer){
//  printf("Receiving MESSAGE....!\n");
  unsigned char r;
  int count = 0;
  int test = read(fd,&r,1);
  if(test == -1){
    printf("ERROR READING RESPONSE BYTE!\n");
    return -1;
  }
  if( test == 0){
    printf("Could not be read\n");
    defaultPortSettings(fd);
    return 1;
  }
  //printf("TEST: %d\n", test);
//  printf("R: %x\n" ,r);
  if(r != FLAG){
    printf("NOT A FLAG\n");
    return 1;
  }

  buffer[count] = r;
  count++;
  do {
    int test = read(fd,&r,1);
    if(test == -1){
      printf("ERROR READING RESPONSE BYTE!\n");
      return -1;
    }
    //printf("R2: %x\n", r);
    buffer[count]=r;
    count++;
  } while(r != FLAG && count < TRAME_SIZE);
  return 0;
}

int stuffTrame(unsigned char* trame, unsigned char * buffer, int length){
  //+printf("Stuffing Frame!!\n");
  trame[0] = FLAG;
  trame[1] = A;
  trame[2] = ControlByte;
  trame[3] = trame[1]^trame[2];
  int c = 0;
  int realocSize = 4;
  int startPoint = 4;
  unsigned char bcc2 = 0x00;
  while(c < length){
    unsigned char v = buffer[c];
  //  printf("Buffer[%d]: %x \n", c, v);
    if(v == FLAG){
  //    printf("EQUALS TO FLAG!");
      unsigned char subs[2] = {0x7d,0x5e};
      memcpy(&trame[startPoint+c], subs, 2);
      startPoint++;
      realocSize+=2;
    }
    else if(v == ESCAPE_BYTE){
//      printf("EQUALS TO ESCAPE BYTE!");
      unsigned char subs[2] = {0x7d,0x5d};
      memcpy(&trame[startPoint+c], subs, 2);
      startPoint++;
      realocSize+=2;
    }
    else{
    //  printf("No stuffing needed!\n");
      trame[startPoint+c] = v;
      realocSize++;
    }
    //printf("Trame[%d+%d]: %x\n", startPoint, c, trame[startPoint+c]);
    bcc2 ^= v;
    c++;
  }
  printf("  BCC2: %x ", bcc2);
  if(bcc2 == FLAG){
    unsigned char subs[2] = {0x7d,0x5e};
    memcpy(&trame[startPoint+c], subs, 2);
    realocSize+=2;
  }
  else if(bcc2 == ESCAPE_BYTE){
    unsigned char subs[2] = {0x7d,0x5d};
    memcpy(&trame[startPoint+c], subs, 2);
    realocSize+=2;
  }
  else {
    trame[startPoint + c] = bcc2;
    realocSize++;
  }
  int newSize = realocSize + 1;
//  printf("New Trame Size: %d\n", newSize);
  trame = (unsigned char*) realloc(trame, newSize);
  trame[newSize - 1] = FLAG;
  return newSize;
}

int readResponse(int fd){
//  printf("Reading response!\n");
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
      return 1;
  }
  if(responseTrame[1] != A){
      printf("ERROR RECEIVER RESPONSE INVALID A!\n");
      return 1;
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
    return 1;
  }
  unsigned char bcc1 = responseTrame[2] ^ responseTrame[1];
  if(bcc1 != responseTrame[3]){
    printf("ERROR RECEIVER RESPONSE INVALID BCC1!\n");
    return 1;
  }
  if(responseTrame[4] != FLAG){
      printf("ERROR RECEIVER RESPONSE INVALID FLAG FINAL!\n");
      return 1;
  }
  stop = 1;
  alarm(0);
  return ret;
}

int writeTrame(int fd, unsigned char * buffer, int length){
  unsigned int accum = 0;
  while(accum < length){
  //  printf("Writing: buffer[%d]: %x\n", accum, buffer[accum]);
    int t = write(fd, &buffer[accum], 1);
    if( t == -1){
      printf("LLWRITE() -> writeTrame error!\n");
      return -1;
    }
    accum++;
  }
  return accum;
}


int llwrite(int fd, unsigned char * buffer, int length){
  signal(SIGALRM, alarmHandlerWrite);
  FD = fd;
  //printf("LLWRITE()");
  int newSize;
  int r;
  do {
    unsigned char* trame_I = (unsigned char *) malloc(FRAME_I_SIZE);
    newSize = stuffTrame(trame_I, buffer, length);
    int test = write(fd,trame_I,newSize);
    if(test == -1){
      return -1;
    }
    else{
    //  printf("LLWRITE()! %d  bytes were WRITTEN.!\n", test);
    }
    //printf("Trying to read response %d!\n", counter);
    alarm(3);
    r = readResponse(fd);
    free(trame_I);
  } while(counter < 5 && !stop);
  if(counter >= 5)
  {
    printf("TIMEOUT!\n");
    return -1;
  }
  if(r == 0)
  {
    //printf("RR!\n");
    counter = 0;
    return newSize;
  }
  else if(r == 1)
  {
    //printf("REJ!\n");
    return 0;
  }
  else return -1;
}
