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

#include <pspdebug.h>
#include <pspkernel.h>
#include <pspctrl.h>

#include <iostream>
#include <string.h>

#include "callbacks.hpp"
#include "file_handler.hpp"
#include "graphics.hpp"
#include "network.hpp"

#define SCR_WIDTH (480)
#define SCR_HEIGHT (272)
#define URL_FIlE "./url.txt"

PSP_MODULE_INFO("cURL Test", 0, 1, 1);

int main_thread(SceSize args, void *argp)
{
	(void)args;
	(void)argp;

	// Get the URL from the file
	std::string url = get_file_contents(URL_FIlE);

	pspDebugScreenPrintf("\n%s\n", url.c_str());

	return 0;
}

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

int main(int argc, char const *argv[])
{
	(void)argc;
	(void)argv;

	init();
	pspDebugScreenPrintf("cURL Test");

	// Main thread loop
	// SceUID thid = sceKernelCreateThread("net_thread", main_thread, 0x18, 0x10000, PSP_THREAD_ATTR_USER, NULL);
	// if (thid < 0)
	// {
	// 	pspDebugScreenPrintf("Error, could not create thread\n");
	// 	sceKernelSleepThread();
	// }
	// _sceKernelExitThread(thid, 0, NULL);

	main_thread(0, NULL);

	close();

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

	return 0;
}
