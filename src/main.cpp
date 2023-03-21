/**
 * @file main.cpp
 * @author KorigamiK
 * @brief A cURL test for the PSP which allows a Net Dialog to connect to a
 * netowrk and then downloads a file from the internet
 * @version 0.1
 * @date 2023-01-01
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifdef __PSP__
#include <pspdebug.h>
#include <pspkernel.h>
#include <pspctrl.h>
#include "callbacks.hpp"
#include "graphics.hpp"

PSP_MODULE_INFO("cURL Test", 0, 1, 1);
#else
#define pspDebugScreenPrintf printf
#define SceSize size_t
#endif

#include <iostream>
#include <string.h>

#include "file_handler.hpp"
#include "network.hpp"

#define SCR_WIDTH (480)
#define SCR_HEIGHT (272)
#define URL_FIlE "./url.txt"

int main_thread(SceSize args, void *argp)
{
	(void)args;
	(void)argp;

	// Get the URL from the file
	std::string url = "https://www.google.com/robots.txt";

	pspDebugScreenPrintf("\n%s\n", url.c_str());

	// Download the file
	pspDebugScreenPrintf("\nDownloading...\n");
	curlDownload(url, "robots.txt") < 0 ? pspDebugScreenPrintf("Failed!\n") : pspDebugScreenPrintf("Success!\n");

	return 0;
}

#ifdef __PSP__
void init()
{
	setupGu();

	pspDebugScreenInit();
	pspDebugScreenSetXY(0, 0);

	pspDebugScreenSetBackColor(0xFFCCCF);
	pspDebugScreenSetTextColor(0x000000);
	pspDebugScreenClear();

	startNetworking();
}

void close()
{
	stopNetworking();
}
#else
void init()
{
}
void close()
{
}
#endif

int main(int argc, char const *argv[])
{
	(void)argc;
	(void)argv;

	init();
	pspDebugScreenPrintf("cURL Test");

	main_thread(0, NULL);

	close();

#ifdef __PSP__
	printf("Press X to exit\n");
	pspDebugScreenPrintf("Press X to exit");
	while (1)
	{
		SceCtrlData pad;
		sceCtrlReadBufferPositive(&pad, 1);
		if (pad.Buttons & PSP_CTRL_CROSS)
			break;
	}

	sceKernelExitGame();
#endif

	return 0;
}
