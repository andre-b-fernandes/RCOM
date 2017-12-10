#include "ip.h"

int getip(url * url){
  struct hostent *h;

  if ((h=gethostbyname(url->hostname)) == NULL) {
    herror("gethostbyname ERROR");
    return -1 ;
  }

  strcpy(url->ip, inet_ntoa(*((struct in_addr *)h->h_addr)));
  return 0;
}
