#include "ip.h"
#include "ftp.h"

int main(int argc , char * argv[]){

  url url;
  int parseurl = parseURL(argv[1], &url);
  if(parseurl == -1)
    return -1;

  int ipParse = getip(&url);
  if(ipParse == -1)
    return -1;

  ftp ftp;
  int connectionStatus = connectftp(url.ip, url.port);
  if(connectionStatus == -1)
    return -1;
  ftp.control_socket_fd = connectionStatus;
  char response[1024];
  int receiveConnectResponse;
  if((receiveConnectResponse = receiveftp(&ftp, response, sizeof(response))) == -1)
    return -1;

  if(strlen(url.password) == 0){
      char pass[MAX_STRING];
      printf("Please insert random password: ");
      fgets(pass, MAX_STRING + 1, stdin);
      strcpy(url.password, pass);
    }

  int loginStatus = loginftp(&ftp, url.username, url.password);
  if(loginStatus == -1)
    return -1;

  int changeCWDStatus = changedirectoryftp(&ftp, url.path);
  if(changeCWDStatus == -1)
      return -1;

  int passiveModeStatus =  passiveMode(&ftp);
  if(passiveModeStatus == -1)
    return -1;

  int retrStatus = retrftp(&ftp, url.filename);
  if(retrStatus == -1)
    return -1;

  int downloadStatus  = downloadftp(&ftp, url.filename);
  if(downloadStatus == -1)
    return -1;

  int disconnect = disconnectftp(&ftp);
  if(disconnect == -1)
    return -1;

  return 0;
}
