#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

int llopen(int fileDescriptor, int mode);

int readByte(int fileDescriptor, char comparison);

int send_UA(int fileDescriptor);

int send_SET(int fileDescriptor);
