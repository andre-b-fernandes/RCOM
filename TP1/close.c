#include "functions.h"


int llclose(int fd){
  int test = close(fd);
  if(test == -1){
    printf("Error closing!\n");
    return -1;
  }

  printf("THE END!\n");
  return 0;
}
