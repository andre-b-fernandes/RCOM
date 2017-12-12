#include "ftp.h"

int connectftp(ftp * ftp, const char * ip, int port){
	struct	sockaddr_in server_addr;

	/*server address handling*/
	bzero((char*)&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip);	/*32 bit Internet address network byte ordered*/
	server_addr.sin_port = htons(port);		/*server TCP port must be network byte ordered */

	/*open an TCP socket*/
	if ((ftp->control_socket_fd = socket(AF_INET,SOCK_STREAM,0)) < 0) {
    perror("socket() ERROR");
    return -1;
  }

	/*connect to the server*/
  if(connect(ftp->control_socket_fd,(struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
    perror("connect() ERROR");
		return -1;
	}

  ftp->data_socket_fd = 0;

	char response[1024];
  int receiveConnectResponse;
  if((receiveConnectResponse = receiveftp(ftp, response, sizeof(response))) == -1)
    return -1;

  printf("Connected!\n");
  return 0;
}


int loginftp(ftp * ftp, const char * username, const char * password){
  int sendLoginUser;
	char response[1024];
	sprintf(response, "USER %s\r\n", username);

  if((sendLoginUser = sendftp(ftp, response)) == -1)
    return -1;
  else printf("Username was sent\n");
	memset(response, 0 ,sizeof(response));
  int receiveUserResponse;
  if((receiveUserResponse = receiveftp(ftp, response, sizeof(response))) == -1)
    return -1;

	memset(response, 0 ,sizeof(response));
	int sendLoginPass;
	sprintf(response, "PASS %s\r\n", password);

	 if((sendLoginPass = sendftp(ftp, response)) == -1)
	   return -1;
  else printf("Password was sent\n");

	memset(response, 0 ,sizeof(response));
  int receivePassResponse;
  if((receivePassResponse = receiveftp(ftp, response, sizeof(response))) == -1)
    return -1;

  return 0;
}

int changedirectoryftp(ftp * ftp, const char * path){
	int sendPath;
	char cwd[1024];
	sprintf(cwd, "CWD %s\r\n", path);

  if((sendPath = sendftp(ftp, cwd)) == -1)
    return -1;

  else printf("Path was sent\n");

	memset(cwd, 0 ,sizeof(cwd));
  int receiveCWDResponse;
  if((receiveCWDResponse = receiveftp(ftp, cwd, sizeof(cwd))) == -1)
    return -1;

	return 0;
}


int sendftp(ftp * ftp, const char * str){
  int bytesWritten;
	if ((bytesWritten = write(ftp->control_socket_fd, str, strlen(str))) <= 0) {
		printf("ERROR: Nothing was sent. \n");
		return -1;
	}
	printf("Bytes sent: %d\nInfo: %s\n", bytesWritten, str);
  return 0;
}

int receiveftp(ftp * ftp, char * resultStr, size_t size){
  FILE * fd = fdopen(ftp->control_socket_fd, "r");
  if(fd == NULL)
  {
    printf("fdopen() error!");
    return -1;
  }
  do {
		memset(resultStr, 0, size);
		resultStr = fgets(resultStr, size, fd);
		printf("%s", resultStr);
	} while (!('1' <= resultStr[0] && resultStr[0] <= '5') || resultStr[3] != ' ');

  return 0;
}
