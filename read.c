#include "functions.h"

static int state = 0;
static int stop = 0;
static char controlField = 0x00;

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

char destuff(int fd){
  printf("Destuffing!\n");
  char ret;
  char r;
  cmp = readByte(fd, &r);
  if(cmp == -1){
    return -1;
  }
  if( r == 0x7d){
    cmp = readByte(fd, &r);
    if(cmp == -1){
      return -1;
    }
    if(r == 0x5e){
      ret = FLAG;
    }
    else if (r == 0x5d){
      ret = ESCAPE_BYTE;
    }
    else{
      printf("Oh boy...This cannot happen. AND HIS NAME IS JOHN CENA!\n")
      return -1;
    }
  }
  else ret = r;
  return ret;
}

int getSizeOfPackage(int fd, char * bcc2){
  printf("Getting size of package\n");
  char l1;
  char l2;
  l2 = destuff(fd);
  l1 = destuff(fd);
  printf("l1: %x\n", l1);
  printf("l2: %x\n", l2);
  int size = 256*l2 + l1;
  printf("Packtet Size: %d\n", size);
  return size;
}

int readDataPacket(int fd, char * buffer, char * bcc2){
  printf("Reading Data Packet\n");
  char r;
  int cmp = readByte(fd, &r);//Numero de sequencia.
  if(cmp == -1){
    return -1;
  }
  int sizePackage = getSizeOfPackage(fd);
  if(sizePackage == -1){
    return -1;
  }
  int counter = 0;
  while(counter < sizePackage){
    printf("Iterating: %d\n", counter);
    char byte;
    byte = destuff(fd);
    if (byte == FLAG || byte == ESCAPE_BYTE){
      counter += 2;
    }
    else counter ++;
    buffer[counter] = byte;
  }
  return 0;
}

int readHeader(int fd){

}



int stateMachineLinkLayer(int fd, char * buffer){
  switch (state) {
    case 0: //Reading initial Flag
    {
      printf("State Machine Link Layer Case 0\n");
      char r;
      int cmp = readByte(fd, &r);
      if(cmp == -1){
        return -1;
      }
      if(r == FLAG){
        printf("FLAG was read!\n");
        state = 1;
      }
      else{
        printf("Error on frame header!\n");
        state = 6;
      }
    }
      break;
    case 1: //Reading Address Field
    {
      printf("State Machine Link Layer Case 1\n");
      char r;
      int cmp = readByte(fd, &r);
      if(cmp == -1){
        return -1;
      }
      if(r == A){
        state = 1;
      }
      else{
        printf("Error on frame header!\n");
        state = 6;
      }
    }
      break;
    case 2: //Control Field
    {
      printf("State Machine Link Layer Case 2\n");
      char r;
      int cmp = readByte(fd, &r);
      if(cmp == -1){
        return -1;
      }
      if(r == DISC){
        printf("DISC READ\n");
        state = 7;
      }
      else if(r == C_UA){
        printf("UA READ!\n");
        state = 6;
      }
      else if(r == ControlField){
        printf("READ ControlField\n");
        state = 3;
      }
      else {
        printf("Error on frame header!\n");
        state = 0;
      }
    }
      break;
  case 3: //BCC 1
  {
    printf("State Machine Link Layer Case 3\n");
    char r;
    int cmp = readByte(fd, &r);
    if(cmp == -1){
      return -1;
    }

    if(r == (A^ControlField)){
      printf("Control Field A^cenas\n");
      state = 4;
    }
    else{
      printf("Error on function header!\n");
      state = 6;
    }
  }
      break;

  case 4: //type of data package
  {
    printf("State Machine Link Layer Case 4\n");
    char r;
    int cmp = readByte(fd, &r);
    if(cmp == -1){
      return -1;
    }
    if(r == DATA_PACKET){
      state = 10;
    }
    else if(r == START_CONTROL_PACKET){
      state = 11;
    }

    else if(r == END_CONTROL_PACKET){
      state = 12;
    }

  }
      break;
  case 5: //FINAL FLAG
  {
    char r;
    int test = read(fd, &r, 1);
    if(test == -1){
      printf("STATE MACHINE LINK LAYER CASE 5 READ ERROR!\n");
      return -1;
    }
    if(r == FLAG){
      printf("FLAG READ");
      state = 6;
    }
  }
      break;
  case 6: //END
  {
    stop = 1;
  }
    break;
  case 7: //send Disc
  {

  }
    break;

  case 8: //Send RR
  {

  }
    break;
  case 9: //send REJ
  {

  }
    break;
  case 10: //DATA PACKET
  {

  }
    break;
  case 11: //CONTROL PACKET START
  {

  }
    break;

  case 12: //CONTROL PACKET END
  {

  }
    break;
  default:
    break;
  }

  return 0;
}


int llread(int fd, char * buffer){
  printf("LLREAD()\n");
  do {
    stateMachineLinkLayer(fd, buffer);
  } while(!stop);
  return 0;
}
