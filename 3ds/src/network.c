#include <stddef.h>

#include "network.h"

int sock = -1;
struct sockaddr_in server;
char outBuf[128], rcvBuf[128];

socklen_t sockaddr_in_sizePtr = (int)sizeof(struct sockaddr_in);

bool openSocket(char *addr, int port) {
    if(sock > 0) return false;
    sock = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(addr);

    int ret = connect(sock, (struct sockaddr *)&server, sizeof(server));
    if(ret) return false;

    //fcntl(sock, F_SETFL, O_NONBLOCK);

    return true;
}

void closeSocket() {
    close(sock);
    sock = -1;
}

u32 sendBuf(void* buffer, size_t count)
{
  size_t left_to_write = count;
  while (left_to_write > 0) {
    size_t written = send (sock, buffer, count, 0);
    if (written == -1)
      /* An error occurred; bail.  */
      return -1;
    else
      /* Keep count of how much more we need to write.  */
      left_to_write -= written;
  }
  /* The number of bytes written is exactly COUNT.  */
  return (u32) count;
}

/*
void sendBuf(int length) {
    sendto(sock, (char *)&outBuf, length, 0, (struct sockaddr *)&saout, sizeof(saout));
}

int receiveBuffer(int length) {
    return recvfrom(sock, (char *)&rcvBuf, length, 0, (struct sockaddr *)&sain, &sockaddr_in_sizePtr);
}

void sendKeys(unsigned int keys, circlePosition circlePad, circlePosition cStick, touchPosition touch) {
    memcpy(&(outBuf[0]), &keys, 4);
    memcpy(&(outBuf[4]), &(circlePad.dx), 2);
    memcpy(&(outBuf[6]), &(circlePad.dy), 2);
    memcpy(&(outBuf[8]), &(cStick.dx), 2);
    memcpy(&(outBuf[10]), &(cStick.dy), 2);
    memcpy(&(outBuf[12]), &(touch.px), 2);
    memcpy(&(outBuf[14]), &(touch.py), 2);
    outBuf[16] = '\r';
    outBuf[17] = '\n';
    sendBuf(18);
}
*/
