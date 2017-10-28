#include "functions.h"

static int state = 0;
static int stop = 0;





int stateMachineLinkLayer(int fd, char * buffer){
  switch (state) {
    case 0:
    {
      char r;
      int test = read(fd, &r, 1);
      if(test == -1){
        printf("STATE MACHINE LINK LAYER CASE 1 READ ERROR!\n");
        return -1;
      }
      if(r == FLAG){
        printf("FLAG READ\n");
        state = 1;
      }
      else {
        state = 0;
      }
    }
      break;
    case 1:
    {
      char r;
      int test = read(fd, &r, 1);
      if(test == -1){
        printf("STATE MACHINE LINK LAYER CASE 1 READ ERROR!\n");
        return -1;
      }
      if(r == A){
        printf("A READ\n");
        state = 2;
      }
      else {
        state = 0;
      }
    }
      break;
    case 2:
    {
      char r;
      int test = read(fd, &r, 1);
      if(test == -1){
        printf("STATE MACHINE LINK LAYER CASE 1 READ ERROR!\n");
        return -1;
      }
      if(r == DISC){
        printf("DISC READ\n");
        state = 3;
      }
      else if(r == C_UA){
        printf("UA READ!\n");
        state = 3;
      }
      else if(r == 0x00){
        printf("READ ControlFieldWrite\n");
        //ControlFieldRead = 0x40;
        state = 3;
      }
      else if(r == 0x40){
        printf("READ ControlFieldRead\n");
      //  ControlFieldRead = 0x00;
        state = 3;
      }
      else {
        state = 0;
      }
    }
      break;
  case 3:
  {
    char r;
    int test = read(fd, &r, 1);
    if(test == -1){
      printf("STATE MACHINE LINK LAYER CASE 1 READ ERROR!\n");
      return -1;
    }
    char cenas = 0x00;
    if(r == (A^cenas)){
      printf("Control Field A^cenas\n");
      state = 4;
    }
    if(r == (A^C_UA)){
      printf("Control Field A^c_ua\n");
      state = 5;
    }
    if(r == (A^DISC)){
      printf("Control Field A^disc\n");
      state = 5;
    }
  }
      break;

  case 4:
  {
    printf("Byte destuffing\n");
    char r;
    int counter = 0;
    int test = 0;
    int bcc2 = 0;
    int t = read(fd, &r, 1);
    if(t == -1){
      printf("STATE MACHINE LINK LAYER CASE 1 READ ERROR!\n");
      return -1;
    }
    if(r == START_CONTROL_PACKET){

    }
    else if(r == END_CONTROL_PACKET){

    }
    else if(r == DATA_PACKET){

    }
    
    while (test < DATA_FRAGMENT_SIZE) {
      printf("Iteration!\n");
      test += read(fd, &r, 1);
      if(test == -1){
        printf("STATE MACHINE LINK LAYER CASE 4 READ ERROR!\n");
        return -1;
      }
      if(r == 0x7d){
        char r2;
        test += read(fd, &r2, 1);
        if(test == -1){
          printf("STATE MACHINE LINK LAYER CASE 4 READ ERROR!\n");
          return -1;
        }
        if(r2 == 0x5e){
          buffer[counter] = FLAG;
          bcc2 ^= FLAG;
          counter++;
        }
        else if(r2 == 0x5d){
          buffer[counter] = ESCAPE_BYTE;
          bcc2 ^= ESCAPE_BYTE;
          counter++;
        }
        else{
          buffer[counter] = r;
          bcc2 ^= r;
          buffer[counter + 1] = r2;
          bcc2 ^= r2;
          counter+=2;
        }

      }
      else {
        buffer[counter] = r;
        bcc2^= r;
        counter++;
      }

    }
    char r;
    test = read(fd, &r, 1);
    if(test == -1){
      printf("STATE MACHINE LINK LAYER CASE 4 READ ERROR!\n");
      return -1;
    }
    if(r != bcc2){
      printf("ERRORS ON DATA FIELD!\n");
      char REJ[TRAME_SIZE];
      REJ[0] = FLAG;
      REJ[1] = A;
      REJ[2] = REJ_2;
      REJ[3] = REJ[2] ^REJ[1];
      REJ[4] = FLAG;

      int test = write(fd, REJ, TRAME_SIZE);
      if(test == -1){
        printf("LINK LAYER STATE MACHINE CASE 4 ERROR ON WRITE REJ\n");
        return -1;
      }
      else{
        printf("SENT REJ\n");
      }

    }

    else{
      printf("NO ERRORS ON DATA FIELD!\n");
      state = 5;
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
  case 6:
  {
    stop = 1;
  }
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
