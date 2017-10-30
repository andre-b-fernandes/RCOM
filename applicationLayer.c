#include "functions.h"

int openFile(char * filename){
        int filesenddescriptor = open(filename, O_RDONLY);
        if(filesenddescriptor == -1) {
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
        if(err == -1) {
                printf("ERROR ON applicationLayer(): stat failed\n");
                return err;
        }
        else{
                printf("Stat struct was filled!\n");
        }
        return filinfo.st_size;
}

int fillControlPacket(char * buf, char content, char length, void * value, int index){
        buf[index] = content;
        index++;
        buf[index] = length;
        index++;
        memcpy(&buf[index], value, length);
        index += length;
        return index;
}

int sendControlPackage(int fd, int size, char * filename, char type){
        printf("PREPARING START CONTROL PACKAGE\n");
        char fileSizeStr[MAX_SIZE_OF_FILESIZE];
        sprintf(fileSizeStr, "%x", size);
        printf("fileSizeStr: %s\n", fileSizeStr);
        int INT_SIZE_PACKET_CONTROL = 5*sizeof(char) + strlen(filename) + strlen(fileSizeStr);
        printf("Size of PACKET_CONTROL: %d\n", INT_SIZE_PACKET_CONTROL);
        char controlPacket_START[INT_SIZE_PACKET_CONTROL];
        int index = 1;
        controlPacket_START[0] = type;
        index = fillControlPacket(controlPacket_START,FILENAME,strlen(filename),filename, index);
        fillControlPacket(controlPacket_START, FILESIZE, strlen(fileSizeStr), (unsigned int *)&size, index);

        int ret = llwrite(fd, controlPacket_START, INT_SIZE_PACKET_CONTROL);

        return ret;

}

int readFile(int filesenddescriptor, char * dataPackage){
        printf("Reading from File!\n");
        char * data = (char *) malloc(DATA_FRAGMENT_SIZE);
        int r = read(filesenddescriptor, data, DATA_FRAGMENT_SIZE);
        if(r == -1)
        {
                printf("ERROR applicationLayer(),read failed!Counter\n");
                return -1;
        }
        else{
                printf("File was read!\n");
        }
        data = (char *) realloc(data, r);
        dataPackage[0] = DATA_PACKET;
        dataPackage[1] = 0x00;
        char l1 = (char) DATA_PACKET_SIZE / 256;
        char l2 = (char) DATA_PACKET_SIZE % 256;
        dataPackage[2] = l2;
        dataPackage[3] = l1;
        int newDataPackage = r+4;
        printf("New Data Package: %d\n", newDataPackage);
        memcpy(&dataPackage[4], data, DATA_FRAGMENT_SIZE);
        dataPackage = (char *)realloc(dataPackage,newDataPackage);
        return newDataPackage;
}

int retfileSize(char * buffer){
  int fileSize;
  int c = 5;
  while(buffer[c] != FILESIZE){
    c++;
    char inc = buffer[c];
    c+=inc;
    c++;
    sleep(1);
  }
  memcpy(&fileSize,&buffer[c+2],buffer[c+1]);
  printf("FILESIZE: %d\n", fileSize);
  return fileSize;
}


//TODO send DISC AND UA
int sequenceWriter(int fd, int size, char * filename){
  printf("Sequence writer\n");
  int filesenddescriptor = openFile(filename);
  if(filesenddescriptor == -1)
  {
    printf("Reading Data Package ERROR!\n!");
      return -1;
  }
  int test = 0;
  do {
    test = sendControlPackage(fd, size, filename, START_CONTROL_PACKET);
    if(test == -1)
      return -1;
  } while(test == 0);
  test = 0;
  int aux = 0;
  do {
    char* dataPackage = (char *) malloc(DATA_PACKET_SIZE);
    test = readFile(filesenddescriptor, dataPackage);
    if(test == -1)
      return -1;
    int ret = 0;
    do {
      ret = llwrite(fd, dataPackage, test);
      if( ret == -1)
        return -1;
      aux += ret - 10;//removing byte Flags basically
    } while(ret == 0);
  } while(aux < size);

  do {
    test = sendControlPackage(fd, size, filename, END_CONTROL_PACKET);
  } while(test == 0);

  return 0;
}
//TODO DISC
int sequenceReader(int fd, int newFileDiscriptor){
  printf("Sequence reader\n");
  char* buffer = (char *) malloc(FRAME_I_SIZE);
  int test;
  int fileSize;
  do { //Start COntrol package
    test = llread(fd, buffer);
    if(test == -1)
      return -1;
  } while(test == 1);
  fileSize = retfileSize(buffer);
  buffer = (char *) malloc(FRAME_I_SIZE);
  test = 0;
  do {
    int aux = llread(fd, buffer) - 10;//- FLAGS BASICALLY
    printf("Aux: %d\n", aux);
    if(aux == -1)
      return -1;
    test+= aux;
    printf("Test: %d\n", test);
    int lel = write(newFileDiscriptor, &buffer[8],aux);
    if(lel == -1)
      return lel;
  } while(test < fileSize );
  do { //End COntrol package
    test = llread(fd, buffer);
    if(test == -1)
      return -1;
  } while(test == 1);
  close(newFileDiscriptor);
  return 0;
}

int applicationLayer(int role, int fd, char * filename){

        if(role == 0) //writer
        {
          int size;
          printf("FILENAME: %s\n", filename);
          size = getFileSize(filename);
          if(size == -1)
                  return -1;
          printf("FILESIZE: %d\n", size);
          printf("WRITER!\n");
          sequenceWriter(fd, size, filename);
        }

        else if(role == 1) { //reader
          printf("READER\n");
          int newFileDiscriptor = open(filename, O_WRONLY|  O_CREAT);
          if(newFileDiscriptor == -1){
            printf("APPLAYER READER ERROR CREATING FILE!\n");
            return -1;
          }
          sequenceReader(fd, newFileDiscriptor);
        }
        else{
            printf("ERROR ROLE HAS TO BE 0 OR 1");
            return -1;
        }
        return 0;
}
