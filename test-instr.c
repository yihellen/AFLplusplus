/*
   american fuzzy lop - a trivial program to test the build
   --------------------------------------------------------

   Written and maintained by Michal Zalewski <lcamtuf@google.com>

   Copyright 2014 Google Inc. All rights reserved.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at:

     http://www.apache.org/licenses/LICENSE-2.0

 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>


int main(int argc, char** argv) {

  char buf[8];
  int port = 0, fd = 0, sock, yes = 1;
  
  if (argc > 1) port = atoi(argv[1]);

  if (port > 0 && port < 65536) {
    struct sockaddr_in server;
    if ((sock = socket(AF_INET, SOCK_STREAM , 0)) < 0) {
      perror("socket");
      exit(-1);
    }
    memset((char*)&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(0x7f000001);
    server.sin_port = htons(port);
    if (bind(sock, (struct sockaddr *) &server, sizeof(server)) < 0) {
      perror("bind");
      exit(-1);
    }
#ifdef SO_REUSEADDR
  setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void *) &yes, sizeof(yes));
#endif
#ifdef SO_REUSEPORT
  setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, (void *) &yes, sizeof(yes));
#endif
    if (listen(sock, 1) < 0) {
      perror("listen");
      exit(-1);
    }
    if ((fd = accept(sock, NULL, NULL)) < 0) {
      perror("accept");
      exit(-1);
    }
  }

  if (read(fd, buf, sizeof(buf)) < 1) {
    printf("Hum?\n");
    exit(1);
  }

  if (buf[0] == '0')
    printf("Looks like a zero to me!\n");
  else if (buf[0] == '1')
    printf("Pretty sure that is a one!\n");
  else
    printf("Neither one or zero? How quaint!\n");

  exit(0);

}
