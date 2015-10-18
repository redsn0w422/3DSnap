#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <setjmp.h>
#include <3ds.h>
#include <3ds/services/cam.h>
#include <arpa/inet.h>
#include <sys/dirent.h>
#include <sys/errno.h>
#include <sys/unistd.h>
#include <fcntl.h>
#include <stdbool.h>

#include "drawing.h"
#include "inet_pton.h"
#include "network.h"

#define CONFIG_3D_SLIDERSTATE (*(volatile float*)0x1FF81080)

static jmp_buf exitJmp;

static char buffer[128];

void hang(char *message) {
    while (aptMainLoop()) {
        hidScanInput();

        clearScreen();
        drawString(10, 10, "%s", message);
        drawString(10, 20, "Press start to exit");

        u32 kHeld = hidKeysHeld();
        if (kHeld & KEY_START) longjmp(exitJmp, 1);

        gfxFlushBuffers();
        gspWaitForVBlank();
        gfxSwapBuffers();
    }
}

void cleanup() {
    enableBacklight();
    SOC_Shutdown();
    //svcCloseHandle(fileHandle);
    fsExit();
    camExit();
    gfxExit();
    acExit();
}

void displayMessage(int x, int y, char *msg) {
    clearScreen();
    drawString(x, y, msg);
    gfxFlushBuffers();
    gfxSwapBuffers();
}

/*
r = (uint8_t) ((data >> 11) & 0x1F) << 3;
r |= (r >> 5);
g = (uint8_t) ((data >> 5) & 0x3F) << 2;
g |= (g >> 6);
b = (uint8_t) (data & 0x1F) << 3;
b |= (b >> 5);
*/

void writePictureToFramebufferRGB565(void *fb, u16 *img) {
    u8 *framebuf = (u8*) fb;
    int i, j, x, y;
    for(j = 0; j < 240; j++) {
        for(i = 0; i < 400; i++) {
            y = 240 - j;
            x = i;
            u32 v = (y + x * 240) * 3;
            u16 data = img[j * 400 + i];
            uint8_t b = ((data >> 11) & 0x1F) << 3;
            uint8_t g = ((data >> 5) & 0x3F) << 2;
            uint8_t r = (data & 0x1F) << 3;
            framebuf[v] = r;
            framebuf[v+1] = g;
            framebuf[v+2] = b;
        }
    }
}

void writePictureToFramebufferRGB888(void *fb, void *img) {
    memcpy(fb, img, 400*240*3);
}

u8* takePicture3D() {
    u16 width = 400;
    u16 height = 240;
    u32 screen_size = width * height * 2;
    u32 mem_size = screen_size * 2;
    CAMU_Size size = SIZE_CTR_TOP_LCD;
    CAMU_CameraSelect select = SELECT_OUT1_OUT2;

    u32 bufSize;
    printf("CAMU_GetMaxBytes: 0x%08X\n", (unsigned int) CAMU_GetMaxBytes(&bufSize, width, height));
    printf("CAMU_SetTransferBytes: 0x%08X\n", (unsigned int) CAMU_SetTransferBytes(PORT_CAM1, bufSize, width, height));
    printf("CAMU_SetTransferBytes: 0x%08X\n", (unsigned int) CAMU_SetTransferBytes(PORT_CAM2, bufSize, width, height));

    printf("CAMU_Activate: 0x%08X\n", (unsigned int) CAMU_Activate(select));

    u8* buf = (u8*) malloc(mem_size);

    Handle camReceiveEvent = 0;
    Handle camReceiveEvent2 = 0;

    printf("CAMU_ClearBuffer: 0x%08X\n", (unsigned int) CAMU_ClearBuffer(PORT_CAM1));
    printf("CAMU_ClearBuffer: 0x%08X\n", (unsigned int) CAMU_ClearBuffer(PORT_CAM2));
    printf("CAMU_SetReceiving: 0x%08X\n", (unsigned int) CAMU_SetReceiving(&camReceiveEvent, buf, PORT_CAM1, width * height * 2, (s16) bufSize));
    printf("CAMU_SetReceiving: 0x%08X\n", (unsigned int) CAMU_SetReceiving(&camReceiveEvent2, buf + width * height * 2, PORT_CAM2, width * height * 2, (s16) bufSize));
    printf("CAMU_StartCapture: 0x%08X\n", (unsigned int) CAMU_StartCapture(PORT_CAM1));
    printf("CAMU_StartCapture: 0x%08X\n", (unsigned int) CAMU_StartCapture(PORT_CAM2));
    printf("svcWaitSynchronization: 0x%08X\n", (unsigned int) svcWaitSynchronization(camReceiveEvent, U64_MAX));
    printf("svcWaitSynchronization: 0x%08X\n", (unsigned int) svcWaitSynchronization(camReceiveEvent2, U64_MAX));

    printf("CAMU_SetReceiving: 0x%08X\n", (unsigned int) CAMU_SetReceiving(&camReceiveEvent, buf, PORT_CAM1, width * height * 2, (s16) bufSize));
    printf("CAMU_SetReceiving: 0x%08X\n", (unsigned int) CAMU_SetReceiving(&camReceiveEvent2, buf + width * height * 2, PORT_CAM2, width * height * 2, (s16) bufSize));
    printf("svcWaitSynchronization: 0x%08X\n", (unsigned int) svcWaitSynchronization(camReceiveEvent, U64_MAX));
    printf("svcWaitSynchronization: 0x%08X\n", (unsigned int) svcWaitSynchronization(camReceiveEvent2, U64_MAX));

    printf("CAMU_StopCapture: 0x%08X\n", (unsigned int) CAMU_StopCapture(PORT_CAM1));
    printf("CAMU_StopCapture: 0x%08X\n", (unsigned int) CAMU_StopCapture(PORT_CAM2));
    printf("CAMU_Activate: 0x%08X\n", (unsigned int) CAMU_Activate(SELECT_NONE));

    gfxFlushBuffers();
    gspWaitForVBlank();
    gfxSwapBuffers();

    return buf;
}

u8* takePictureInner() {
    u16 width = 400;
    u16 height = 240;
    u32 screen_size = width * height * 2;
    u32 mem_size = screen_size;
    CAMU_Size size = SIZE_CTR_TOP_LCD;
    CAMU_CameraSelect select = SELECT_IN1;

    u32 bufSize;
    printf("CAMU_GetMaxBytes: 0x%08X\n", (unsigned int) CAMU_GetMaxBytes(&bufSize, width, height));
    printf("CAMU_SetTransferBytes: 0x%08X\n", (unsigned int) CAMU_SetTransferBytes(PORT_CAM1, bufSize, width, height));
    printf("CAMU_SetTransferBytes: 0x%08X\n", (unsigned int) CAMU_SetTransferBytes(PORT_CAM2, bufSize, width, height));

    printf("CAMU_Activate: 0x%08X\n", (unsigned int) CAMU_Activate(select));

    u8* buf = (u8*) malloc(mem_size);

    Handle camReceiveEvent = 0;

    printf("CAMU_ClearBuffer: 0x%08X\n", (unsigned int) CAMU_ClearBuffer(PORT_CAM1));
    printf("CAMU_SetReceiving: 0x%08X\n", (unsigned int) CAMU_SetReceiving(&camReceiveEvent, buf, PORT_CAM1, width * height * 2, (s16) bufSize));
    printf("CAMU_StartCapture: 0x%08X\n", (unsigned int) CAMU_StartCapture(PORT_CAM1));
    printf("svcWaitSynchronization: 0x%08X\n", (unsigned int) svcWaitSynchronization(camReceiveEvent, U64_MAX));

    printf("CAMU_SetReceiving: 0x%08X\n", (unsigned int) CAMU_SetReceiving(&camReceiveEvent, buf, PORT_CAM1, width * height * 2, (s16) bufSize));
    printf("svcWaitSynchronization: 0x%08X\n", (unsigned int) svcWaitSynchronization(camReceiveEvent, U64_MAX));

    printf("CAMU_StopCapture: 0x%08X\n", (unsigned int) CAMU_StopCapture(PORT_CAM1));
    printf("CAMU_Activate: 0x%08X\n", (unsigned int) CAMU_Activate(SELECT_NONE));

    gfxFlushBuffers();
    gspWaitForVBlank();
    gfxSwapBuffers();

    return buf;
}

int main() {
    // Initializations
    acInit();
    gfxInitDefault();

    // Enable double buffering to remove screen tearing
    gfxSetDoubleBuffering(GFX_TOP, true);
    gfxSetDoubleBuffering(GFX_BOTTOM, true);

    // Save current stack frame for easy exit
    if(setjmp(exitJmp)) {
        cleanup();
        return 0;
    }

    // Set up file system
    displayMessage(10, 10, "Initializing file system");
    fsInit();

    // Set up Wi-Fi
    displayMessage(10, 10, "Initializing SOC");
    SOC_Initialize((u32*) memalign(0x1000, 0x100000), 0x100000);

    u32 wifiStatus = 0;
    ACU_GetWifiStatus(NULL, &wifiStatus);
    if(!wifiStatus) {
        hang("Wi-Fi is not enabled or not connected");
    }

    u32 kDown;
    u32 kHeld;
    //u32 kUp;
    circlePosition circlePad;
    circlePosition cStick;
    touchPosition touch;

    clearScreen();

    u16 width = 400;
    u16 height = 240;
    u32 screen_size = width * height * 2;
    u32 mem_size = screen_size * 2;
    CAMU_Size size = SIZE_CTR_TOP_LCD;
    CAMU_CameraSelect select = SELECT_OUT1_OUT2;
    printf("camInit: 0x%08X\n", (unsigned int) camInit());

    printf("CAMU_SetSize: 0x%08X\n", (unsigned int) CAMU_SetSize(select, size, CONTEXT_A));
    printf("CAMU_SetOutputFormat: 0x%08X\n", (unsigned int) CAMU_SetOutputFormat(select, OUTPUT_RGB_565, CONTEXT_A));

    printf("CAMU_SetNoiseFilter: 0x%08X\n", (unsigned int) CAMU_SetNoiseFilter(select, true));
    printf("CAMU_SetAutoExposure: 0x%08X\n", (unsigned int) CAMU_SetAutoExposure(select, true));
    printf("CAMU_SetAutoWhiteBalance: 0x%08X\n", (unsigned int) CAMU_SetAutoWhiteBalance(select, true));

    printf("CAMU_SetTrimming: 0x%08X\n", (unsigned int) CAMU_SetTrimming(PORT_CAM1, false));
    printf("CAMU_SetTrimming: 0x%08X\n", (unsigned int) CAMU_SetTrimming(PORT_CAM2, false));
    //printf("CAMU_SetTrimmingParamsCenter: 0x%08X\n", (unsigned int) CAMU_SetTrimmingParamsCenter(PORT_CAM1, 512, 240, 512, 384));

    u8 *buf = takePicture();

    gfxFlushBuffers();
    gspWaitForVBlank();
    gfxSwapBuffers();

    // Main loop
    while (aptMainLoop()) {
        clearScreen();

        // Read which buttons are currently pressed or not
        hidScanInput();
        kDown = hidKeysDown();
        kHeld = hidKeysHeld();
        //kUp = hidKeysUp();
        float slider3D = CONFIG_3D_SLIDERSTATE;

        // Read circle pad and c stick positions
        hidCstickRead(&cStick);
        hidCircleRead(&circlePad);
        hidTouchRead(&touch);

        // If START button is pressed, break loop and quit
        if (kDown & KEY_START) {
            break;
        }

        if (kHeld & KEY_A) {
            drawString(20, 20, "A");
        }
        if (kHeld & KEY_B) {
            drawString(35, 20, "B");
        }
        if (kHeld & KEY_X) {
            drawString(50, 20, "X");
        }
        if (kHeld & KEY_Y) {
            drawString(65, 20, "Y");
        }
        if (kHeld & KEY_L) {
            drawString(80, 20, "L");
        }
        if (kHeld & KEY_R) {
            drawString(95, 20, "R");
        }
        if (kHeld & KEY_ZL) {
            drawString(110, 20, "ZL");
        }
        if (kHeld & KEY_ZR) {
            drawString(135, 20, "ZR");
        }

        if (kHeld & KEY_DUP) {
            drawString(20, 30, "UP");
        }
        if (kHeld & KEY_DDOWN) {
            drawString(45, 30, "DOWN");
        }
        if (kHeld & KEY_DLEFT) {
            drawString(90, 30, "LEFT");
        }
        if (kHeld & KEY_DRIGHT) {
            drawString(135, 30, "RIGHT");
        }

        if(CONFIG_3D_SLIDERSTATE > 0.0f) {
            gfxSet3D(true);
            writePictureToFramebufferRGB565Left(gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL), buf);
            writePictureToFramebufferRGB565Right(gfxGetFramebuffer(GFX_TOP, GFX_RIGHT, NULL, NULL), buf + screen_size);
            //writePictureToFramebufferRGB565Left(gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL), buf, (u16) offset);
            //writePictureToFramebufferRGB565Right(gfxGetFramebuffer(GFX_TOP, GFX_RIGHT, NULL, NULL), buf + screen_size, (u16) offset);
        } else {
            gfxSet3D(false);
            writePictureToFramebufferRGB565Left(gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL), buf);
        }

        drawString(20, 50, itoa(circlePad.dx, buffer, 10));
        drawString(55, 50, itoa(circlePad.dy, buffer, 10));
        drawString(20, 60, itoa(cStick.dx, buffer, 10));
        drawString(55, 60, itoa(cStick.dy, buffer, 10));
        sprintf(buffer, "%f %f", slider3D, offset);
        drawString(20, 70, buffer);

        if(touch.px && touch.py) {
            drawBox(touch.px - 2, touch.py - 2, 5, 5, 255, 255, 255);
        }

        // Reset framebuffers
        //fbTopLeft = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
        //fbTopRight = gfxGetFramebuffer(GFX_TOP, GFX_RIGHT, NULL, NULL);
        //fbBottom = gfxGetFramebuffer(GFX_BOTTOM, 0, NULL, NULL);
        //memset(fbTopLeft, 0, 240 * 400 * 3);
        //memset(fbTopRight, 0, 240 * 400 * 3);
        //memset(fbBottom, 0, 240 * 320 * 3);

        // Flush and swap framebuffers
        gfxFlushBuffers();
        gspWaitForVBlank();
        gfxSwapBuffers();
    }

    // Exit
    free(buf);
    cleanup();

    // Return to hbmenu
    return 0;
}
