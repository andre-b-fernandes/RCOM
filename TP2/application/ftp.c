#include "ftp.h"

int connectftp(const char * ip, int port){
	struct	sockaddr_in server_addr;
	int sock;
	/*server address handling*/
	bzero((char*)&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip);	/*32 bit Internet address network byte ordered*/
	server_addr.sin_port = htons(port);		/*server TCP port must be network byte ordered */

	/*open an TCP socket*/
	if ((sock= socket(AF_INET,SOCK_STREAM,0)) < 0) {
    perror("socket() ERROR");
    return -1;
  }

	/*connect to the server*/
  if(connect(sock,(struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
    perror("connect() ERROR");
		return -1;
	}

  printf("Connected!\n");
  return sock;
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

	printf("Logged in!\n");
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

	printf("Changed directory!\n");
	return 0;
}

int passiveMode(ftp * ftp){
	char pasv[1024] = "PASV\r\n";
	int sendPassive = sendftp(ftp, pasv);
	if (sendPassive == -1)
		return -1;

	sendPassive = receiveftp(ftp, pasv, sizeof(pasv));
	if (sendPassive == -1)
		return -1;

	int ipPart1, ipPart2, ipPart3, ipPart4;
	int port1, port2;
	if ((sscanf(pasv, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)", &ipPart1,
			&ipPart2, &ipPart3, &ipPart4, &port1, &port2)) < 0) {
		printf("ERROR parsing information from Passive Mode response.\n");
		return -1;
	}

	memset(pasv, 0, sizeof(pasv));

	if ((sprintf(pasv, "%d.%d.%d.%d", ipPart1, ipPart2, ipPart3, ipPart4))
			< 0) {
		printf("ERROR forming ip address\n");
		return -1;
	}

	int portResult = port1 * 256 + port2;

	//printf("IP: %s\n", pasv);
	//printf("PORT: %d\n", portResult);

	ftp->data_socket_fd = connectftp(pasv, portResult);
	if (ftp->data_socket_fd == -1)
		return -1;

	printf("Entered passive mode!\n");

	return 0;
}

int retrftp(ftp* ftp, const char* filename) {
	char retr[1024];
	sprintf(retr, "RETR %s\r\n", filename);
	if (sendftp(ftp, retr))
		return -1;

	if (receiveftp(ftp, retr, sizeof(retr)))
		return -1;

	printf("Preparing download of %s\n", filename);
	return 0;
}

int downloadftp(ftp* ftp, const char * filename){
	FILE * file = fopen(filename, "w");
	if(file == NULL)
	{
		printf("Could not create local file: %s\n", filename);
		return -1;
	}

	char buffer[1024];
	int bytes = 1;
	printf("Downloading file: ");
	while(bytes > 0){
		printf("...");
		bytes = recv(ftp->data_socket_fd, buffer, 1024, 0);
		int fwriteStatus = fwrite(buffer, bytes, 1, file);
		if(fwriteStatus < 0)
		{
			printf("Error on fwrite!\n");
			return -1;
		}
	}
	printf("\n");
	fclose(file);

	char response[1024];
	int receiveUserResponse;
	if((receiveUserResponse = receiveftp(ftp, response, sizeof(response))) == -1)
		return -1;
	return 0;
}

int disconnectftp(ftp * ftp){
	int sendQuit;
	char response[1024];
	sprintf(response, "QUIT\r\n");

  if((sendQuit = sendftp(ftp, response)) == -1)
    return -1;

	close(ftp->data_socket_fd);
	close(ftp->control_socket_fd);
	printf("QUIT\n");
  return 0;
}

int sendftp(ftp * ftp, const char * str){
  int bytesWritten;
	if ((bytesWritten = write(ftp->control_socket_fd, str, strlen(str))) <= 0) {
		printf("ERROR: Nothing was sent. \n");
		return -1;
	}
	//printf("Bytes sent: %d\nInfo: %s\n", bytesWritten, str);
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
		//printf("%s", resultStr);
	} while (!('1' <= resultStr[0] && resultStr[0] <= '5') || resultStr[3] != ' ');

  return 0;
}
