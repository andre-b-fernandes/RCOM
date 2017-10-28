#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_SIZE_OF_FILESIZE 255

#define START_CONTROL_PACKET 0x02
#define END_CONTROL_PACKET 0x03
#define DATA_PACKET 0x01

#define FILENAME 0x00
#define FILESIZE 0x01

#define DATA_FRAGMENT_SIZE 1014
#define DATA_PACKET_SIZE DATA_FRAGMENT_SIZE + 4
#define FRAME_I_SIZE 2*DATA_FRAGMENT_SIZE + 4 + 6

#define RR_1 0x05
#define RR_2 0x85
#define DISC 0X0b
#define REJ_1 0x01
#define REJ_2 0x81


#define ESCAPE_BYTE 0x7d
#define TRAME_SIZE 5
#define FLAG 0x7e
#define A 0x03
#define C_SET 0x03
#define C_UA 0x07


//char ControlFieldWrite = 0x00;
//char ControlFieldRead = 0x40;

void alarmHandler(int sig);

int llopen(int fileDescriptor, int mode);

int send_UA(int fileDescriptor);

int send_SET(int fileDescriptor);

int stateMachineWrite(char c,int fileDescriptor);

int stateMachineRead(char c, int fileDescriptor);

int readByteWrite(int fd, char *c);

int readByteRead(int fd, char *c);

int defaultPortSettings(int fd);

int unblockReadPortSettings(int fd);

int llwrite(int fd, char * buffer, int length);

int llread(int fd, char * buffer);

int getFileSize(char * filename);

int fillControlPacket(char * buf, char content, char length, char * value, int index);

int sendControlPackage(int fd, int size, char * filename, char type);

int sendDataPackage(int fd, char * filename);

int applicationLayer(int role, int fd, char * filename);

int writeDataPackage(int fd, char * buffer);

int openFile(char * filename);

int stateMachineApplicationLayer(int fd, int fileSize, char * filename);

int readByte(int fd, char* r);
