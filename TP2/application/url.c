#include "url.h"

//ftp://[<user>:<password>@]<host>/<url-path>
//ftp://ftp.up.pt/pub/...
//ftp://<user>:<password>@ftp.up.pt/pub/...

const char * regExpUser = "ftp://([A-Za-z0-9])*:([A-Za-z0-9])*@([A-Za-z0-9.~-])+/([[A-Za-z0-9/~._-])+";

const char * regExpAnonymous = "ftp://([A-Za-z0-9.~-])+/([[A-Za-z0-9/~._-])+";

int parseFilename(char * url_param, url * url){
  strcpy(url->filename, url_param);
  return 0;
}

int parsePath(char * url_param, url* url){
  char* path = (char*) malloc(strlen(url_param));
  path[0] = 0;
  int first = 1;
  while (strchr(url_param, '/')) {
      char aux[MAX_STRING];
      int counter;
      for (counter = 0; url_param[counter] != '/'; counter++) {
          aux[counter] = url_param[counter];
      }
      aux[counter] = '/';
      aux[counter + 1] = '\0';
      if(first){
        first = 0;
        strcpy(path, aux);
      }
      else{
        strcat(path, aux);
      }
      memmove(url_param, url_param + counter + 1, strlen(url_param)); //advance path and /
	}
  if(strlen(path)!=0) strcpy(url->path, path);

  else url->path[0] = 0;

  free(path);
  return 0;
}

int parseHost(char * url_param , url* url){
  int counter;
  char hostname[MAX_STRING];
  for(counter = 0; url_param[counter] != '/'; counter++){
    hostname[counter] = url_param[counter];
  }
  hostname[counter] = '\0';
  memcpy(url->hostname, hostname, counter + 1);
  memmove(url_param, url_param + counter + 1, strlen(url_param)); //advance the hostname
  return 0;
}

int parseAnonymous(char * url_param, url * url){
  url->password[0] = 0;
  memcpy(url->username, ANONYMOUS_STRING, ANONYMOUS_SIZE);
  memmove(url_param, url_param + 6, strlen(url_param)); //advance ftp
  return 0;
}

int parseUser(char * url_param, url * url){
  memmove(url_param, url_param + 6, strlen(url_param)); //advance ftp
  int counterUsername;
  int counterPassword;
  char username[MAX_STRING];
  for(counterUsername = 0; url_param[counterUsername] != ':'; counterUsername++){
    username[counterUsername] = url_param[counterUsername];
  }
  memcpy(url->username, username, counterUsername);
  char password[MAX_STRING];
  memmove(url_param, url_param + counterUsername + 1, strlen(url_param)); //advance the username and :
  for(counterPassword = 0; url_param[counterPassword] != '@'; counterPassword++){
    password[counterPassword] = url_param[counterPassword];
  }
  password[counterPassword] = '\0';
  memcpy(url->password, password, counterPassword + 1);
  memmove(url_param, url_param + counterPassword + 1, strlen(url_param)); //advance password and @
  return 0;
}


int parseURL(char * url_param, url* url){
  int reti;
  size_t nmatch = strlen(url_param);
  regmatch_t pmatch[nmatch];
  char * tempURL = (char * ) malloc(strlen(url_param));
  memcpy(tempURL, url_param, strlen(url_param));
  regex_t * regexUser = (regex_t*) malloc(sizeof(regex_t));
  regex_t  * regexAnonymous = (regex_t*) malloc(sizeof(regex_t));
	if ((reti = regcomp(regexUser, regExpUser, REG_EXTENDED)) != 0) {
		printf("URL format is wrong: regExpUser");
		return -1;
  }
  if ((reti = regcomp(regexAnonymous, regExpAnonymous, REG_EXTENDED)) != 0) {
		printf("URL format is wrong: regExpAnonymous");
		return -1;
  }
  if ((reti = regexec(regexUser, tempURL, nmatch, pmatch, REG_EXTENDED)) == 0) {
    printf("URL is of type User\n");
    parseUser(tempURL, url);
  }
  else if ((reti = regexec(regexAnonymous, tempURL, nmatch, pmatch, REG_EXTENDED)) == 0) {
    printf("URL is of type anonymous\n");
    parseAnonymous(tempURL, url);
  }
  else{
    printf("Error invalid URL!\n");
    return -1;
  }

  free(regexUser);
  free(regexAnonymous);

  parseHost(tempURL, url);
  parsePath(tempURL, url);
  parseFilename(tempURL, url);

  free(tempURL);
  return 0;
}
