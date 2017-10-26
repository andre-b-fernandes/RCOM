#include "functions.h"


int openFile(char * filename){
  int filesenddescriptor = open(filename, O_RDONLY);
  if(filesenddescriptor == -1){
    printf("ERROR applicationLayer(), opening %s\n", filename);
    return -1;
  }
  else{
    printf("%s was opened!\n", filename);
  }

  return filesenddescriptor;
}

int getFileSize(char * filename){
  struct stat filinfo;
  int err = stat(filename, &filinfo);
  if(err == -1){
    printf("ERROR ON applicationLayer(): stat failed\n");
    return err;
  }
  else{
    printf("Stat struct was filled!\n");
  }
  return filinfo.st_size;
}

int fillControlPacket(char * buf, char content, char length, char * value, int index){
  buf[index] = content;
  buf[index + 1] = length;
  memcpy(&buf[index + 2], value, length);
  index = 3;
  return index;
}

int sendControlPackage(int fd, int size, char * filename, char type){
  printf("PREPARING START CONTROL PACKAGE\n");

  char fileSizeStr[MAX_SIZE_OF_FILESIZE];
  sprintf(fileSizeStr, "%x", size);
  printf("fileSizeStr: %s\n", fileSizeStr);

  int INT_SIZE_PACKET_CONTROL = 5*sizeof(char) + strlen(filename) + strlen(fileSizeStr);
  printf("Size of PACKET_CONTROL: %d\n", INT_SIZE_PACKET_CONTROL);
  char controlPacket_START[INT_SIZE_PACKET_CONTROL] ;
  int index = 1;
  controlPacket_START[0] = type;
  index = fillControlPacket(controlPacket_START,FILENAME,strlen(filename),filename, index);
  fillControlPacket(controlPacket_START, FILESIZE, strlen(fileSizeStr),fileSizeStr, index);

  int ret = llwrite(fd, controlPacket_START, INT_SIZE_PACKET_CONTROL);

  return ret;

}

int readTrame(int fd, int size){
  char trameRead[size];
  int test = read(fd,trameRead,size);
  if(test == -1){
    printf("ApplicationLayer() reading Response Trame failed!\n");
    return -1;
  }
  else{
    printf("ApplicationLayer() reading Response Trame successfull!\n");
  }
  llread(fd, trameRead);
  return 0;
}



int readFile(int fd, int size, int filesenddescriptor){
  int counter = 0;
  while(counter <= size){
    char  data[DATA_FRAGMENT_SIZE];
    int r = read(filesenddescriptor, data, DATA_FRAGMENT_SIZE);
    if(r == -1)
    {
      printf("ERROR applicationLayer(),read failed!Counter: %d\n", counter);
      return -1;
    }
    else{
      printf("File was read!\n");
    }
    char dataPackage[DATA_PACKET_SIZE];
    dataPackage[0] = DATA_PACKET;
    dataPackage[1] = 0x00;
    char l1 = (char) DATA_PACKET_SIZE / 256;
    char l2 = (char) DATA_PACKET_SIZE % 256;
    dataPackage[2] = l2;
    dataPackage[3] = l1;
    memcpy(&dataPackage[4], data, DATA_FRAGMENT_SIZE);
    llwrite(fd, dataPackage, DATA_PACKET_SIZE);
    counter+=DATA_FRAGMENT_SIZE;
  }
  return 0;
}

int sendDataPackage(int fd, int size, char * filename){
  int filesenddescriptor = openFile(filename);
  if(filesenddescriptor == -1)
    return -1;

  int test = readFile(fd, size, filesenddescriptor);
  if(test == -1)
    return -1;

  return 0;
}

int applicationLayer(int fd, char * filename){
  //TODO Change the layer to divide between writer and reader!!
  printf("FILENAME: %s\n", filename);

  int size = getFileSize(filename);
  if(size == -1)
    return -1;

  printf("FILESIZE: %d\n", size);


  int s = sendControlPackage(fd, size,filename, START_CONTROL_PACKET);
  if(s == -1)
    return -1;

  readTrame(fd, s);

  sendDataPackage(fd,size, filename);

  s = sendControlPackage(fd, size,filename, END_CONTROL_PACKET);
  readTrame(fd, s);

  return 0;
}
