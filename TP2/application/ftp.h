#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <errno.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#include <netinet/in.h>

typedef struct FTP
{
    int control_socket_fd; // file descriptor to control socket
    int data_socket_fd; // file descriptor to data socket
} ftp;

int connectftp(const char * ip, int port);
int loginftp(ftp * ftp, const char * username, const char * password);
int sendftp(ftp * ftp, const char * str);
int receiveftp(ftp * ftp, char * resultStr, size_t size);
int changedirectoryftp(ftp * ftp, const char * path);
int passiveMode(ftp * ftp);
int retrftp(ftp* ftp, const char* filename);
int downloadftp(ftp* ftp, const char * filename);
int disconnectftp(ftp * ftp);
