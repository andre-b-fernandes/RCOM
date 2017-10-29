#include "functions.h"

static char ControlByte = 0x40;

int readByte(int fd, char* r){
  int test = read(fd, r, 1);
  if(test == -1){
    printf("Error function read Byte\n");
    return -1;
  }
  else{
    printf("%x was read", *r);
    return 0;
  }
}

int processBuffer(char * buff, char * buffer, int buffLength){
  int count = 0;
  int bufferLength = 0;
  while(count < buffLength){
    char r = buff[count];
    if( r == 0x7d){
      char r1 = buff[count + 1];
      if(r1 == 0x5e){
        buffer[count] = FLAG;
      }
      else if (r1 == 0x5d){
        buffer[count] = ESCAPE_BYTE;
      }
      else{
        printf("Oh boy...This cannot happen. AND HIS NAME IS JOHN CENA: %d !\n", count);
        return -1;
      }
      count += 2;
    }
    else {
      buffer[count] = r;
      count++;
    }
    bufferLength++;
  }

  return bufferLength;
}

int checkHeadErrors(char* buffer, int bufferLength){
  if(buffer[0] != FLAG){
    printf("FIRST FLAG HEADER MISSING! %x \n", buffer[0]);
    return -1;
  }
  if(buffer[1] != A){
    printf("Adress FIELD HEADER MISSING! %x\n",buffer[1]);
    return -1;
  }
  if(buffer[2] == 0x00){
    printf("Control byte %x!\n", buffer[2]);
    ControlByte = 0x40;
  }
  else if(buffer[2] == 0x40){
    printf("Control byte %x!\n", buffer[2]);
    ControlByte = 0x00;
  }
  else{
      printf("Error Control byte%x!\n", buffer[2]);
      return 1;
  }
  if(buffer[3] != (buffer[1]^buffer[2])){
    printf("Error BCC1: %x\n", buffer[3]);
    return 1;
  }
  int counter = 4;
  char bcc2 = 0;
  while(counter < bufferLength -6){
    bcc2 ^= buffer[counter];
    counter++;
  }
  if(buffer[bufferLength - 2] != bcc2){
    printf("ERROR ON DATA SEGMENT! %x\n", buffer[bufferLength - 2]);
    return 1;
  }
  if(buffer[bufferLength - 1] != FLAG){
    printf("Final Flag missing! %x\n", buffer[bufferLength - 2]);
    return -1;
  }
  return 0;
}

int readTrame(int fd, char * buffer){
  char * buff = (char *) malloc(FRAME_I_SIZE);
  int test = read(fd,buff,FRAME_I_SIZE);
  if(test == -1){
    printf("Error LLREAD() reading trame!\n");
    return -1;
  }
  printf("Read: %d bytes\n", test);
  buff = (char *)realloc(buff, test);
  int bufferLen = processBuffer(buff, buffer, test);
  printf("BufferLength: %d\n", bufferLen);
  if(bufferLen == -1){
    return -1;
  }

  else{
    buffer = (char*) realloc(buffer, bufferLen);
    int cmp = checkHeadErrors(buffer,bufferLen);
    if(cmp == -1)
      return -1;
    else if(cmp == 0)
      return bufferLen;
    else
      return 1;
  }
}

int sendRR(int fd){
  char trame[TRAME_SIZE];
  trame[0] = FLAG;
  trame[1] = A;
  if(ControlByte == 0x00){
    trame[2] = RR_1;
  }
  else if(ControlByte == 0x40){
    trame[2] = RR_2;
  }
  trame[3] = trame[1] ^trame[2]; //XOR OF 1 AND 2
  trame[4] = FLAG;
  int err = write(fd, trame, TRAME_SIZE);
  if(err == -1){
    printf("ERROR ON WRITE RESPONSE LLREAD\n");
    return err;
  }
  else{
    printf("RR TRAME WRITTEN %x\n", trame[2]);
    return 0;
  }
}

int sendREJ(int fd){
  char trame[TRAME_SIZE];
  trame[0] = FLAG;
  trame[1] = A;
  if(ControlByte == 0x00){
    trame[2] = REJ_1;
  }
  else if(ControlByte == 0x40){
    trame[2] = REJ_2;
  }
  trame[3] = trame[1] ^trame[2]; //XOR OF 1 AND 2
  trame[4] = FLAG;
  int err = write(fd, trame, TRAME_SIZE);
  if(err == -1){
    printf("ERROR ON WRITE RESPONSE LLREAD\n");
    return err;
  }
  else{
    printf("REJ TRAME WRITTEN %x\n", trame[2]);
    return 0;
  }
}

int llread(int fd, char * buffer){
  printf("LLREAD()\n");
  int test = readTrame(fd,buffer);
  if( test == -1){
    return -1;
  }
  else if(test == 1){
    int cmp = sendREJ(fd);
    if(cmp == -1)
      return -1;
    else return 0;
  }
  else{
    int cmp = sendRR(fd);
    if(cmp == -1){
      return -1;
    }
    else return test;
  }
}