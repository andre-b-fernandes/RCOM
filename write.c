#include "functions.h"

char ControlField;

int stuffTrame(char* trame, char * buffer, int length){
  printf("Stuffing Frame!!\n");
  trame[0] = FLAG;
  trame[1] = A;
  trame[2] = ControlField;
  trame[3] = trame[1]^trame[2];
  int counter = 0;
  int realocSize = 0;
  int startPoint = 4;
  char bcc2 = 0x00;
  while(counter < length){
    char v = buffer[counter];
    printf("Buffer[%d]: %x ", counter, v);
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

int llwrite(int fd, char * buffer, int length){
  printf("LLWRITE()");
  ControlField = 0x00; //ou é 0x00 ou é 0x40
  char* trame_I = (char *) malloc(FRAME_I_SIZE);
  int newSize = stuffTrame(trame_I, buffer, length);

  int test = write(fd,trame_I,newSize);

  if(test == -1){
    printf("Error LLWRITE()! Trame could not be written!\n");
    return -1;
  }

  else{
    printf("LLWRITE()! TRAME WAS WRITTEN!\n");
    alarm(3);
  }
  return newSize;
}
