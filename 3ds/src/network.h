#pragma once

#include <string.h>

#include <3ds.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include "inet_pton.h"

extern int sock;
extern struct sockaddr_in sain, saout;
extern char outBuf[128], rcvBuf[128];

extern socklen_t sockaddr_in_sizePtr;

bool openSocket(int port);
void sendBuf(int length);
int receiveBuffer(int length);
void sendKeys(unsigned int keys, circlePosition circlePad, circlePosition cStick, touchPosition touch);