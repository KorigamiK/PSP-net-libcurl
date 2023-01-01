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
#include <pspnet.h>
#include <pspnet_apctl.h>
#include <pspnet_inet.h>
#include <psputility.h>
#include <pspdisplay.h>

#include <iostream>
#include <string.h>

#include "callbacks.hpp"

#define SCR_WIDTH (480)
#define SCR_HEIGHT (272)

PSP_MODULE_INFO("cURL Test", 0, 1, 1);

int dialogDone = 0;

void netInit(void)
{
	sceNetInit(128 * 1024, 42, 4 * 1024, 42, 4 * 1024);

	sceNetInetInit();

	sceNetApctlInit(0x8000, 48);
}

void netTerm(void)
{
	sceNetApctlTerm();

	sceNetInetTerm();

	sceNetTerm();
}

void networkDialog()
{
	std::cout << "Starting Network Dialog" << std::endl;
	pspUtilityNetconfData data;

	memset(&data, 0, sizeof(data));
	data.base.size = sizeof(data);
	data.base.language = PSP_SYSTEMPARAM_LANGUAGE_ENGLISH;
	data.base.buttonSwap = PSP_UTILITY_ACCEPT_CIRCLE;
	data.action = PSP_NETCONF_ACTION_CONNECTAP;
	data.base.graphicsThread = 17;
	data.base.accessThread = 19;
	data.base.fontThread = 18;
	data.base.soundThread = 16;

	pspUtilityNetconfAdhoc adhocparam;
	memset(&adhocparam, 0, sizeof(adhocparam));
	data.adhocparam = &adhocparam;

	sceUtilityNetconfInitStart(&data);

	int status;
	while (!dialogDone)
	{
		status = sceUtilityNetconfGetStatus();
		switch (status)
		{
		case PSP_UTILITY_DIALOG_NONE:
			break;

		case PSP_UTILITY_DIALOG_VISIBLE:
			sceUtilityNetconfUpdate(1);
			break;

		case PSP_UTILITY_DIALOG_QUIT:
			sceUtilityNetconfShutdownStart();
			break;

		case PSP_UTILITY_DIALOG_FINISHED:
			dialogDone = 1;
			break;

		default:
			break;
		}

		sceDisplayWaitVblankStart();
	}
	std::cout << "Network Dialog Done" << std::endl;
}

void mainThread()
{
	bool running = true;

	while (running)
	{
		switch (dialogDone)
		{
		case 0:
			pspDebugScreenPrintf(" Dialog not done yet");
			// sleep 3 second
			sceKernelDelayThread(1000 * 1000 * 3);
			break;

		default:
			running = false;
			break;
		}
	}
}

int main(int argc, char const *argv[])
{
	(void)argc;
	(void)argv;

	pspDebugScreenInit();
	// debug screent in bottom half of screen
	pspDebugScreenSetXY(SCR_WIDTH / 2, SCR_HEIGHT / 2);
	SetupCallbacks();
	pspDebugScreenPrintf("Hello World!");

	sceUtilityLoadNetModule(PSP_NET_MODULE_COMMON);
	sceUtilityLoadNetModule(PSP_NET_MODULE_INET);
	netInit();

	// Start the main thread in a new thread in background
	// SceUID mainThreadID = sceKernelCreateThread("mainThread",
	// (SceKernelThreadEntry)mainThread, 0x11, 0xFA0, 0, 0);
	// sceKernelStartThread(mainThreadID, 0, 0);

	networkDialog();

	netTerm();

	sceKernelSleepThread();
	sceKernelExitGame();

	return 0;
}
