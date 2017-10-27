#include "functions.h"

int state;
int lastState;
int count = 0;
int stop = 0;

int FD;

void AppLayerAlarmHandler(int sig){
 printf("function timed out\n");
 state = lastState;
 count ++;
 unblockReadPortSettings(FD);
}
//TODO Complete with resending bad trames/ Sequence numbers
int stateMachineApplicationLayer(int fd, int fileSize, char * filename){
        static int counterWriter = 0;
        switch (state) {
        case 0: //llwrite start Package control
                printf("State 0\n");
                sendControlPackage(fd, fileSize, filename, START_CONTROL_PACKET);
                state = 4;
                lastState = 0;
                break;
        case 1: //Read Frame
        {
                printf("State 1\n");
                char r[FRAME_I_SIZE];
                int test = read(fd, r, FRAME_I_SIZE);
                if(test == -1) {
                        printf("State machine app layer case 1 read failed!\n");
                }
                else{
                        printf("State machine app layer case 1 read %d bytes\n", test);
                }
                if(r[2] == C_UA){
                  printf("Finished reading!\n");
                  state = 5;
                }
                else{
                  printf("continue reading!\n");
                  llread(fd,r);
                  state = 1;
                }

        }
        break;
        case 2: //Data Package
        {
                printf("State 2\n");
                sendDataPackage(fd,filename);
                counterWriter += DATA_FRAGMENT_SIZE;
                if(counterWriter >= fileSize){
                  printf("Finished writing\n");
                  state = 3;
                }
                else state = 4;
                lastState = 2;
        }
        break;
        case 3: //Leave Writer
        {
                printf("State 3\n");
                send_UA(fd);
                stop = 1;

        }
        break;
        case 4: //Read Response
        {
                printf("State 4\n");
                char r[TRAME_SIZE];
                int test = read(fd, r, TRAME_SIZE);
                if(test == -1) {
                        printf("State machine app layer case 4 read failed!\n");
                }
                else{
                        printf("State machine app layer case 4 read %d bytes\n", test);
                }
                if(r[2] == RR_1 || r[2] == RR_2){
                  state = 2;
                }
                else if(r[2] == REJ_1 || r[2] == REJ_2){

                  state = lastState;
                }
                else if(r[2] == DISC){
                  state = 3;
                }

        }
                break;
        case 5:{ //Leave Reader
                printf("State 5\n");
                stop = 1;
                lastState = 5;
        }
                break;
        default:
                break;
        }

        return 0;
}

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
        char controlPacket_START[INT_SIZE_PACKET_CONTROL];
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
        if(test == -1) {
                printf("ApplicationLayer() reading Response Trame failed!\n");
                return -1;
        }
        else{
                printf("ApplicationLayer() reading Response Trame successfull!\n");
        }
        llread(fd, trameRead);
        return 0;
}



int readFile(int fd, int filesenddescriptor){
        char data[DATA_FRAGMENT_SIZE];
        int r = read(filesenddescriptor, data, DATA_FRAGMENT_SIZE);
        if(r == -1)
        {
                printf("ERROR applicationLayer(),read failed!Counter\n");
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
        int ret = llwrite(fd, dataPackage, DATA_PACKET_SIZE);
        return ret;
}

int sendDataPackage(int fd, char * filename){
        int filesenddescriptor = openFile(filename);
        if(filesenddescriptor == -1)
                return -1;

        int test = readFile(fd,filesenddescriptor);
        if(test == -1)
                return -1;

        return test;
}

int applicationLayer(int role, int fd, char * filename){
        FD = fd;
        printf("FILENAME: %s\n", filename);
        int size = getFileSize(filename);
        if(size == -1)
                return -1;

        printf("FILESIZE: %d\n", size);
        if(role == 0) //writer
        {
                state = 0;
        }


        else if(role == 1) { //reader
                state = 1;
        }

        else{ //rip
                return -1;
        }

        do {
          stateMachineApplicationLayer(fd, size, filename);
        } while(!stop && count < 3);


        return 0;
}
