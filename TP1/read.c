#include "functions.h"

static unsigned char ControlByte = 0x00;

int processBuffer(unsigned char * buff, unsigned char * buffer, int buffLength){
//  printf("Process Buffer!\n");
  int count = 0;
  int bufferLength = 0;
  while(count < buffLength){
    unsigned char r = buff[count];
    if( r == 0x7d){
    // printf("Needs to destuff!\n");
      unsigned char r1 = buff[count + 1];
      if(r1 == 0x5e){
        buffer[bufferLength] = FLAG;
      }
      else if (r1 == 0x5d){
        buffer[bufferLength] = ESCAPE_BYTE;
      }
      else{
        printf("  Probably going to give bcc2 error!: %d, %x !  ", count,r1);
        buffer[bufferLength] = 0x7d;
      }
      count += 2;
    }
    else {
    //  printf("No need to destuff!\n");
      buffer[bufferLength] = r;
      count++;
    }
    //printf("Buffer[%d]: %x\n",bufferLength, buffer[bufferLength]);
    bufferLength++;
  }

  return bufferLength;
}

int checkHeadErrors(unsigned char* buffer, int bufferLength){
  int randomNumber = rand()%100;
  printf("Random Number: %d\n", randomNumber);
  if(randomNumber < 50){
      printf("Random fail!\n");
      return 1;
  }
  if(buffer[0] != FLAG){
    printf("FIRST FLAG HEADER MISSING! %x \n", buffer[0]);
    return 3;
  }
  if(buffer[1] != A){
    printf("Adress FIELD HEADER MISSING! %x\n",buffer[1]);
    return 3;
  }
  if(buffer[2] != 0x00 && buffer[2] != 0x40){
      printf("  Invalid frame CONTROL BYTE! %x ", buffer[2]);
      return 3;
  }
  if(buffer[2] != ControlByte){
    printf("  Repeated frame! %x  !=  %x", buffer[2], ControlByte);
    return 2;
  }
  else if(buffer[2] == 0x00 && buffer[2]==ControlByte){
    printf("  Control byte %x!  ", buffer[2]);
    ControlByte = 0x40;
  }
  else if(buffer[2] == 0x40 && buffer[2]==ControlByte){
    printf("Control byte %x!  ", buffer[2]);
    ControlByte = 0x00;
  }
  else{
      printf("  Error Control byte%x! ", buffer[2]);
      return 3;
  }
  if(buffer[3] != (buffer[1]^buffer[2])){
    printf(" Error BCC1: %x ", buffer[3]);
    return 1;
  }
  int counter = 4;
  unsigned char bcc2 = 0;
  while((counter-4) < bufferLength -6){
    //printf("bcc2 ^= Buffer[%d]: %x\n", counter, buffer[counter]);
    bcc2 ^= buffer[counter];
    counter++;
  }

  printf(" BCC2: %x  ", bcc2);

  if(buffer[bufferLength - 2] != bcc2){
    printf("ERROR ON DATA SEGMENT! %x read instead of %x ", buffer[bufferLength - 2], bcc2);
    return 1;
  }
  if(buffer[bufferLength - 1] != FLAG){
    printf("Final Flag missing! %x ", buffer[bufferLength - 2]);
    return 3;
  }
  return 0;
}

int receiveMessageRead(int fd, unsigned char * buff){
  //printf("Receiving message!\n");
  unsigned char r;
  int newSize = 1;
  int test = read(fd,&r,1);
  if(test == -1){
    printf("Error LLREAD() reading trame!\n");
    return -1;
  }
  buff[0] = r;
  do {
    //printf("Reading next byte from message!\n");
    int test = read(fd,&r,1);
    //usleep(100);
    if(test == -1){
      printf("Error LLREAD() reading trame!\n");
      return -1;
    }
    //printf("R: %x\n", r);
    buff[newSize] = r;
    newSize++;
  } while(r != FLAG && newSize < FRAME_I_SIZE);
  return newSize;
}

int readTrame(int fd, unsigned char * buffer){
  unsigned char * buff = (unsigned char *) malloc(FRAME_I_SIZE);
  int test = receiveMessageRead(fd, buff);
  if(test == -1){
    return -1;
  }
//  printf("Read: %d bytes\n", test);
  buff = (unsigned char *)realloc(buff, test);
  int bufferLen = processBuffer(buff, buffer, test);
//  printf("BufferLength: %d\n", bufferLen);
  if(bufferLen == -1){
    return -1;
  }
  else{
    buffer = (unsigned char*) realloc(buffer, bufferLen);
    int cmp = checkHeadErrors(buffer,bufferLen);
    if(cmp == 0)
    {
      //printf("Everything is ok no errors on head\n");
      return bufferLen;
    }
    else return cmp;

  }
}

int sendRR(int fd){
  printf(" Send RR! ");
  unsigned char trame[TRAME_SIZE];
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
    //printf("RR TRAME WRITTEN %x\n", trame[2]);
    return 0;
  }
}

int sendREJ(int fd){
  printf(" Send rej! ");
  unsigned char trame[TRAME_SIZE];
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

int llread(int fd, unsigned char * buffer){
  //sleep(1);
//  printf("LLREAD()\n");
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
  else if(test == 2){
    int cmp = sendRR(fd);
    if(cmp == -1){
      return -1;
    }
    return 0;
  }
  else if(test == 3){
    return 0;
  }
  else{
    int cmp = sendRR(fd);
    if(cmp == -1){
      return -1;
    }
    else return test;//ou length ou 2
  }
}
