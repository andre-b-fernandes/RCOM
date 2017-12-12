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
  int connectionStatus = connectftp(&ftp,url.ip, url.port);
  if(connectionStatus == -1)
    return -1;

  if(strlen(url.password) == 0){
      char pass[COLLEGE_EMAIL_LENGTH];
      printf("Please insert your college email as password: ");
      fgets(pass, COLLEGE_EMAIL_LENGTH + 1, stdin);
      strcpy(url.password, pass);
    }

  int loginStatus = loginftp(&ftp, url.username, url.password);
  if(loginStatus == -1)
    return -1;

  int changeCWDStatus = changedirectoryftp(&ftp, url.path);
  if(changeCWDStatus == -1)
      return -1;



   printf("Username: %s\n", url.username);
   printf("Password: %s\n", url.password);
   printf("Hostname: %s\n", url.hostname);
   printf("Path: %s\n", url.path);
   printf("Filename: %s\n", url.filename);
   printf("IP: %s\n", url.ip);

  return 0;
}
