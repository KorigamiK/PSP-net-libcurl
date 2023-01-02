#include "network.hpp"

#include <pspdebug.h>
#include <psputility.h>
#include <pspdisplay.h>

#include <pspnet.h>
#include <pspnet_inet.h>
#include <pspnet_apctl.h>
#include <pspnet_resolver.h>
#include <psphttp.h>

#include <string.h>
#include <stdlib.h>

#define printf pspDebugScreenPrintf

int netDialogActive = -1;
int httpTemplate;

void loadNetworkingLibs()
{
  int rc = sceUtilityLoadNetModule(PSP_NET_MODULE_COMMON);
  if (rc < 0)
    printf("net common didn't load.\n");
  rc = sceUtilityLoadNetModule(PSP_NET_MODULE_INET);
  if (rc < 0)
    printf("inet didn't load.\n");
}

int goOnline()
{
  sceNetInit(128 * 1024, 42, 4 * 1024, 42, 4 * 1024);
  sceNetInetInit();
  sceNetApctlInit(0x8000, 48);
  sceNetResolverInit();
  if (!netDialog())
  {
    printf("Could not access networking dialog! %d", 30000);
    stopNetworking();
    return 1;
  }
  sceHttpInit(20000);
  return 0;
}

pspUtilityNetconfData data;

int netDialog()
{
  memset(&data, 0, sizeof(data));
  data.base.size = sizeof(data);
  data.base.language = PSP_SYSTEMPARAM_LANGUAGE_ENGLISH;
  data.base.buttonSwap = PSP_UTILITY_ACCEPT_CROSS;
  data.base.graphicsThread = 17;
  data.base.accessThread = 19;
  data.base.fontThread = 18;
  data.base.soundThread = 16;
  data.action = PSP_NETCONF_ACTION_CONNECTAP;

  netDialogActive = -1;
  int result = sceUtilityNetconfInitStart(&data);
  printf("sceUtilityNetconfInitStart: %08x\n", result);
  if (result < 0)
  {
    data.base.size = sizeof(pspUtilityNetconfData) - 12;
    result = sceUtilityNetconfInitStart(&data);
    printf("sceUtilityNetconfInitStart again: %08x\n", result);
    if (result < 0)
      return 0;
  }
  netDialogActive = 0;

  return 1;
}

// returns -1 on quit, 0 on active, and 1 on success
int drawNetDialog()
{
  int done = 0;

  switch (sceUtilityNetconfGetStatus())
  {
  case PSP_UTILITY_DIALOG_NONE:
    printf("None\n");
    break;
  case PSP_UTILITY_DIALOG_INIT:
    break;
  case PSP_UTILITY_DIALOG_VISIBLE:
    sceUtilityNetconfUpdate(1);
    break;
  case PSP_UTILITY_DIALOG_QUIT:
    printf("NetDialog was quit.\n");
    sceUtilityNetconfShutdownStart();
    done = -1;
    break; // cancelled??
  case PSP_UTILITY_DIALOG_FINISHED:
    printf("NetDialog completed successfully.\n");
    sceUtilityNetconfShutdownStart();
    done = 1;
    break;
  default:
    printf("NetconfGetStatus: %08x\n", sceUtilityNetconfGetStatus());
    break;
  }

  return done;
}

void startNetworking()
{
  printf("Loading Netowrk libs...\n");
  loadNetworkingLibs();
  printf("Initing networking\n");
  goOnline();

  printf("Starting dialog\n");
  while (netDialogActive != 1)
  {
    netDialogActive = drawNetDialog();

    if (netDialogActive == -1)
    {
      printf("Dialog was quit.\n");
      break;
    }

    sceDisplayWaitVblankStart();
  }

  printf("Dialog finished.\n");

  atexit(stopNetworking);
}

void stopNetworking()
{
  printf("Shutting down networking.\n");
  sceNetResolverTerm();
  sceNetApctlTerm();
  sceNetInetTerm();
  sceNetTerm();
}