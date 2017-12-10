#include "url.h"

int main(int argc , char * argv[]){

  url url;
  int parse = parseURL(argv[1], &url);
  if(parse == -1)
    return -1;

  printf("Username: %s\n", url.username);
  printf("Hostname: %s\n", url.hostname);
  printf("Path: %s\n", url.path);
  printf("Filename: %s\n", url.filename);

  return 0;
}
