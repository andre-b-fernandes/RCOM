#include "functions.h"

int llread(int fd, char * buffer){
  static char ControlField = 0x00;
  if(buffer[0] != FLAG){
    return -1;
  }

  if(buffer[1] != A){
    return -1;
  }

  if(buffer[2] == ControlField)
  {
    ControlField = 0x01; //R 0 0 0 0 1 0 1
  }
  else{
    ControlField = buffer[2];
  }


  return 0;
}
