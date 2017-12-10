#include <string.h>
#include <stdio.h>
#include <regex.h>
#include <stdlib.h>
#include <sys/types.h>

#define MAX_STRING 256
#define ANONYMOUS_SIZE 10
#define ANONYMOUS_STRING "anonymous\0"
#define COLLEGE_EMAIL_LENGTH 20



typedef char url_parameter[MAX_STRING];
typedef struct URL {
  url_parameter username;
  url_parameter password;
  url_parameter hostname;
  url_parameter ip;
  url_parameter path;
  url_parameter filename;
  int port;
}url;

int parseURL(char * url_param, url* url);
int parseUser(char * url_param, url * url);
int parseAnonymous(char * url_param, url * url);
int parseHost(char * url_param , url* url);
int parsePath(char * url_param, url* url);
int parseFilename(char * url_param, url * url);
