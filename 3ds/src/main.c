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
#define WAIT_TIMEOUT 300000000ULL

static jmp_buf exitJmp;

void hang(char *message) {
    clearScreen();
    printf("%s", message);
    printf("Press start to exit");

    while (aptMainLoop()) {
        hidScanInput();

        u32 kHeld = hidKeysHeld();
        if (kHeld & KEY_START) longjmp(exitJmp, 1);
    }
}

void cleanup() {
    enableBacklight();
    SOC_Shutdown();
    //svcCloseHandle(fileHandle);
    httpcExit();
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

void convertRGB565ToRGB888(u8 *dst, u16 *src) {
    int i;
    for(i = 0; i < 400 * 240 * 2; i++) {
        u16 data = src[i];
        uint8_t r = ((data >> 11) & 0x1F) << 3;
        uint8_t g = ((data >> 5) & 0x3F) << 2;
        uint8_t b = (data & 0x1F) << 3;
        dst[3 * i + 0] = r;
        dst[3 * i + 1] = g;
        dst[3 * i + 2] = b;
    }
}

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

void writePictureToFramebufferRGB888(void *fb, u8 *img) {
    u8 *framebuf = (u8*) fb;
    int i, j, x, y;
    for(j = 0; j < 240; j++) {
        for(i = 0; i < 400; i++) {
            y = 240 - j;
            x = i;
            u32 v = (y + x * 240) * 3;
            uint8_t b = img[(j * 400 + i) * 3];
            uint8_t g = img[(j * 400 + i) * 3 + 1];
            uint8_t r = img[(j * 400 + i) * 3 + 2];
            framebuf[v] = r;
            framebuf[v+1] = g;
            framebuf[v+2] = b;
        }
    }
}

u8* takePicture3D(u8 *buf) {
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

    if(!buf) {
        buf = (u8*) malloc(mem_size);
        if(buf == NULL) {
            hang("Unable to allocate memory!");
        }
    }

    Handle camReceiveEvent = 0;
    Handle camReceiveEvent2 = 0;

    printf("CAMU_ClearBuffer: 0x%08X\n", (unsigned int) CAMU_ClearBuffer(PORT_CAM1));
    printf("CAMU_ClearBuffer: 0x%08X\n", (unsigned int) CAMU_ClearBuffer(PORT_CAM2));
    printf("CAMU_SynchronizeVsyncTiming: 0x%08X\n", (unsigned int) CAMU_SynchronizeVsyncTiming(SELECT_OUT1, SELECT_OUT2));
    printf("CAMU_SetReceiving: 0x%08X\n", (unsigned int) CAMU_SetReceiving(&camReceiveEvent, buf, PORT_CAM1, width * height * 2, (s16) bufSize));
    printf("CAMU_SetReceiving: 0x%08X\n", (unsigned int) CAMU_SetReceiving(&camReceiveEvent2, buf + width * height * 2, PORT_CAM2, width * height * 2, (s16) bufSize));
    printf("CAMU_StartCapture: 0x%08X\n", (unsigned int) CAMU_StartCapture(PORT_CAM1));
    printf("CAMU_StartCapture: 0x%08X\n", (unsigned int) CAMU_StartCapture(PORT_CAM2));
    printf("svcWaitSynchronization: 0x%08X\n", (unsigned int) svcWaitSynchronization(camReceiveEvent, WAIT_TIMEOUT));
    printf("svcWaitSynchronization: 0x%08X\n", (unsigned int) svcWaitSynchronization(camReceiveEvent2, WAIT_TIMEOUT));

    printf("CAMU_SetReceiving: 0x%08X\n", (unsigned int) CAMU_SetReceiving(&camReceiveEvent, buf, PORT_CAM1, width * height * 2, (s16) bufSize));
    printf("CAMU_SetReceiving: 0x%08X\n", (unsigned int) CAMU_SetReceiving(&camReceiveEvent2, buf + width * height * 2, PORT_CAM2, width * height * 2, (s16) bufSize));
    printf("CAMU_PlayShutterSound: 0x%08X\n", (unsigned int) CAMU_PlayShutterSound(SHUTTER_SOUND_TYPE_NORMAL));
    printf("svcWaitSynchronization: 0x%08X\n", (unsigned int) svcWaitSynchronization(camReceiveEvent, WAIT_TIMEOUT));
    printf("svcWaitSynchronization: 0x%08X\n", (unsigned int) svcWaitSynchronization(camReceiveEvent2, WAIT_TIMEOUT));
    printf("CAMU_StopCapture: 0x%08X\n", (unsigned int) CAMU_StopCapture(PORT_CAM1));
    printf("CAMU_StopCapture: 0x%08X\n", (unsigned int) CAMU_StopCapture(PORT_CAM2));

    printf("CAMU_Activate: 0x%08X\n", (unsigned int) CAMU_Activate(SELECT_NONE));

    return buf;
}

Result http_download(httpcContext *context, u8 *buf) {
	Result ret=0;
	u8* framebuf_top;
	u32 statuscode=0;
	u32 size=0, contentsize=0;

	ret = httpcBeginRequest(context);
	if(ret!=0)return ret;

	ret = httpcGetResponseStatusCode(context, &statuscode, 0);
	if(ret!=0)return ret;

	if(statuscode!=200)return -2;

	ret=httpcGetDownloadSizeState(context, NULL, &contentsize);
	if(ret!=0)return ret;

	//printf("size: %"PRId32"\n",contentsize);
	gfxFlushBuffers();

	if(buf==NULL)return -1;
	memset(buf, 0, contentsize);


	ret = httpcDownloadData(context, buf, contentsize, NULL);
	if(ret!=0)
	{
		free(buf);
		return ret;
	}

	size = contentsize;
	if(size>(240*400*3*2))size = 240*400*3*2;

    return 0;
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
    printf("svcWaitSynchronization: 0x%08X\n", (unsigned int) svcWaitSynchronization(camReceiveEvent, WAIT_TIMEOUT));

    printf("CAMU_SetReceiving: 0x%08X\n", (unsigned int) CAMU_SetReceiving(&camReceiveEvent, buf, PORT_CAM1, width * height * 2, (s16) bufSize));
    printf("CAMU_PlayShutterSound: 0x%08X\n", (unsigned int) CAMU_PlayShutterSound(SHUTTER_SOUND_TYPE_NORMAL));
    printf("svcWaitSynchronization: 0x%08X\n", (unsigned int) svcWaitSynchronization(camReceiveEvent, WAIT_TIMEOUT));

    printf("CAMU_StopCapture: 0x%08X\n", (unsigned int) CAMU_StopCapture(PORT_CAM1));
    printf("CAMU_Activate: 0x%08X\n", (unsigned int) CAMU_Activate(SELECT_NONE));

    return buf;
}

int main() {
    // Initializations
    acInit();
    gfxInitDefault();
    consoleInit(GFX_BOTTOM, NULL);

    // Enable double buffering to remove screen tearing
    gfxSetDoubleBuffering(GFX_TOP, true);
    gfxSetDoubleBuffering(GFX_BOTTOM, false);

    // Save current stack frame for easy exit
    if(setjmp(exitJmp)) {
        cleanup();
        return 0;
    }

    // Set up file system
    printf("Initializing file system\n");
    fsInit();

    // Set up Wi-Fi
    printf("Initializing SOC\n");
    SOC_Initialize((u32*) memalign(0x1000, 0x100000), 0x100000);

    printf("Initializing Wi-Fi/network\n");
    u32 wifiStatus = 0;
    ACU_GetWifiStatus(NULL, &wifiStatus);
    if(!wifiStatus) {
        hang("Wi-Fi is not enabled or not connected");
    }

    httpcInit();

    u32 kDown;
    u32 kHeld;
    //u32 kUp;
    circlePosition circlePad;
    circlePosition cStick;
    touchPosition touch;

    printf("Initializing camera\n");

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
    //
    u8 *buf = malloc(400*240*3*2);
    u8 *cam_buf = malloc(400*240*2*2);

    FILE *file = fopen("logo.bin","rb");
    fseek(file,0,SEEK_END);
    off_t file_size = ftell(file);
    fseek(file,0,SEEK_SET);
    off_t bytesRead = fread(buf,1,file_size,file);
    fclose(file);
    memcpy(buf + (400 * 320 * 3), buf, 400 * 320 * 3);

    gfxFlushBuffers();
    gspWaitForVBlank();
    gfxSwapBuffers();

    bool held_A = false;
    bool held_B = false;
    bool held_R = false;

    printf("\n3DSnap build 85\n");
    printf("Welcome wchill\n");

    printf("\nPress R to take a new picture\n");
    printf("Press A to download received snap\n");
    printf("Press B to send current snap\n");
    printf("Use slider to enable/disable 3D\n");
    printf("Press Start to exit to Homebrew Launcher\n");

    // Main loop
    while (aptMainLoop()) {
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
            if(!held_A) {
                held_A = true;
                printf("Fetching new snap\n");
                gfxFlushBuffers();
                gspWaitForVBlank();
                gfxSwapBuffers();
                char *url = "http://intense.io:6001/recv/wchill";
                httpcContext context;
                Result ret = httpcOpenContext(&context, url, 0);
                ret |= http_download(&context, buf);
                if(ret) {
                    printf("An error occurred (no new snaps?)\n");
                } else {
                    printf("Snap downloaded\n");
                }
                httpcCloseContext(&context);
            }
        }
        if (!(kHeld & KEY_A)) {
            held_A = false;
        }
        if (kHeld & KEY_B) {
            if(!held_B) {
                held_B = true;
                printf("Press up/down to change letter\n");
                printf("Press left/right to change position\n");
                printf("Press A to send to this recipient\n");
                char alphabet[] = "abcdefghijklmnopqrstuvwxyz0123456789";
                char recipient[32];
                memset(recipient, 0, 32);
                recipient[0] = 1;
                gfxSet3D(false);
                int pos = 0;
                bool held_up = false;
                bool held_down = false;
                bool held_left = false;
                bool held_right = false;
                int i;
                while(aptMainLoop()) {
                    hidScanInput();
                    kDown = hidKeysDown();
                    kHeld = hidKeysHeld();
                    memset(gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL), 0, 400 * 240 * 3);
                    drawStringFramebuffer(gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL), 10, 10, "Recipient: ");
                    char tmpstr[2];
                    tmpstr[1] = 0;
                    for(i = 0; i <= pos; i++) {
                        tmpstr[0] = alphabet[recipient[i] - 1];
                        drawStringFramebuffer(gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL), 120 + 9 * i, 10, tmpstr);
                    }
                    if(kHeld & KEY_A) {
                        break;
                    }
                    if((kHeld & KEY_DUP) && !held_up) {
                        recipient[pos]--;
                        if(recipient[pos] <= 0) recipient[pos] += strlen(alphabet);
                        held_up = true;
                    } else if((kHeld & KEY_DDOWN) && !held_down) {
                        recipient[pos] = ((recipient[pos]) % strlen(alphabet)) + 1;
                        held_down = true;
                    } else if((kHeld & KEY_DLEFT) && !held_left && pos > 0) {
                        recipient[pos--] = 0;
                        held_left = true;
                    } else if((kHeld & KEY_DRIGHT) && !held_right && pos < 29) {
                        recipient[pos+1] = recipient[pos];
                        pos++;
                        held_right = true;
                    }
                    if(!(kHeld & KEY_DUP)) {
                        held_up = false;
                    }
                    if(!(kHeld & KEY_DDOWN)) {
                        held_down = false;
                    }
                    if(!(kHeld & KEY_DLEFT)) {
                        held_left = false;
                    }
                    if(!(kHeld & KEY_DRIGHT)) {
                        held_right = false;
                    }
                    gfxFlushBuffers();
                    gspWaitForVBlank();
                    gfxSwapBuffers();
                }
                printf("Sending snap\n");
                gfxFlushBuffers();
                gspWaitForVBlank();
                gfxSwapBuffers();
                if(!openSocket("104.131.69.59", 6000)) {
                    printf("ERROR: Failed to connect!\n");
                } else {
                    // write data
                    char send[] = "wchill-";
                    int count = sendBuf(send, strlen(send));
                    recipient[strlen(recipient)] = '\n';
                    count += sendBuf(recipient, strlen(recipient));
                    count += sendBuf(buf, 400 * 240 * 3 * 2);
                    closeSocket();
                    if(count != strlen(send) + strlen(recipient) + (400 * 240 * 3 * 2)) {
                        hang("WARNING: Data transmission error!\n");
                    }
                    printf("Snap upload complete\n");
                }
            }
        }
        if (!(kHeld & KEY_B)) {
            held_B = false;
        }
        if (kHeld & KEY_R) {
            if(!held_R) {
                printf("Capturing new image (may hang!)\n");
                gfxFlushBuffers();
                gspWaitForVBlank();
                gfxSwapBuffers();
                held_R = true;
                takePicture3D(cam_buf);
                convertRGB565ToRGB888(buf, cam_buf);

                gfxFlushBuffers();
                gspWaitForVBlank();
                gfxSwapBuffers();
            }
        }
        if (!(kHeld & KEY_R)) {
            held_R = false;
        }

        if(CONFIG_3D_SLIDERSTATE > 0.0f) {
            gfxSet3D(true);
            writePictureToFramebufferRGB888(gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL), buf);
            writePictureToFramebufferRGB888(gfxGetFramebuffer(GFX_TOP, GFX_RIGHT, NULL, NULL), buf + (400 * 240 * 3));
        } else {
            gfxSet3D(false);
            writePictureToFramebufferRGB888(gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL), buf);
        }

        // Flush and swap framebuffers
        gfxFlushBuffers();
        gspWaitForVBlank();
        gfxSwapBuffers();
    }

    // Exit
    free(buf);
    free(cam_buf);
    cleanup();

    // Return to hbmenu
    return 0;
}
