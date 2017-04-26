#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <ctype.h>
#include <time.h>
#include <sys/sendfile.h>


typedef struct Port
{
  int p1;
  int p2;
} Port;


#ifndef BUFFERSIZE
  #define BUFFERSIZE 1024
#endif
