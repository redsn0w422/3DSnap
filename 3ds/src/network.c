#include <stddef.h>

#include "network.h"

int sock;
struct sockaddr_in sain, saout;
char outBuf[128], rcvBuf[128];

socklen_t sockaddr_in_sizePtr = (int)sizeof(struct sockaddr_in);

bool openSocket(int port) {
    sock = socket(AF_INET, SOCK_DGRAM, 0);

    saout.sin_family = sain.sin_family = AF_INET;
    saout.sin_port = sain.sin_port = htons(port);
    sain.sin_addr.s_addr = INADDR_ANY;

    bind(sock, (struct sockaddr *)&sain, sizeof(sain));

    fcntl(sock, F_SETFL, O_NONBLOCK);

    return true;
}

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