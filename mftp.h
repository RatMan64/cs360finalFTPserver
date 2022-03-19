//
// Created by Riley on 4/19/2021.
//

#ifndef CS360FINAL_MFTP_H
#define CS360FINAL_MFTP_H
#include <sys/stat.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/un.h>
#include <sys/socket.h>
#define DATASIZE 1024
#define BACKLOG 4


#define MY_PORT_NUMBER 49999

#define PACKET_SIZE 1024
#define CLIENTPORT "49999"

#endif //CS360FINAL_MFTP_H
