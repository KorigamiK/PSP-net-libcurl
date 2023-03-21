#include "network.hpp"

#ifdef __PSP__
#include <pspdebug.h>
#include <psputility.h>
#include <pspdisplay.h>

#include <pspnet.h>
#include <pspnet_inet.h>
#include <pspnet_apctl.h>
#include <pspnet_resolver.h>
#include <psphttp.h>

#define printf pspDebugScreenPrintf

#endif

#include <string.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>

#include <curl/curl.h>

int netDialogActive = -1;

#define HTTP_SUCCESS 0
#define HTTP_FAILED -1

#ifdef __PSP__
extern "C"
{
  char *basename(const char *filename)
  {
    char *p = strrchr(filename, '/');
    return p ? p + 1 : (char *)filename;
  }
}
#endif

int curlDownload(std::string &full_url, std::string local_dst)
{
  CURL *curl;
  CURLcode res;
  FILE *fd;

  curl = curl_easy_init();
  if (!curl)
  {
    std::cout << "ERROR: CURL INIT\n"
              << std::endl;
    return HTTP_FAILED;
  }

  fd = fopen(local_dst.c_str(), "wb");
  if (!fd)
  {
    printf("fopen Error: File path '%s'\n", local_dst.c_str());
    return HTTP_FAILED;
  }

  printf("Download URL: %s >> %s\n", full_url.c_str(), local_dst.c_str());

  curl_easy_setopt(curl, CURLOPT_URL, full_url);
  // Set user agent string
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "cURL agent");
  // not sure how to use this when enabled
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
  // not sure how to use this when enabled
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
  // Set SSL VERSION to TLS 1.2
  curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
  // Set timeout for the connection to build
  curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
  // Follow redirects (?)
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  // The function that will be used to write the data
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fwrite);
  // The data filedescriptor which will be written to
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, fd);
  // maximum number of redirects allowed
  curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 20L);
  // Fail the request if the HTTP code returned is equal to or larger than 400
  curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
  // request using SSL for the FTP transfer if available
  curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_TRY);

  // Perform the request
  res = curl_easy_perform(curl);
  // close filedescriptor
  fclose(fd);
  // cleanup
  curl_easy_cleanup(curl);

  if (res != CURLE_OK)
  {
    printf("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    remove(local_dst.c_str());
    return HTTP_FAILED;
  }

  return HTTP_SUCCESS;
}

size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
  std::ofstream *out = (std::ofstream *)stream;
  out->write((char *)ptr, size * nmemb);
  return size * nmemb;
}

#ifdef __PSP__
void loadNetworkingLibs()
{
  int rc = sceUtilityLoadNetModule(PSP_NET_MODULE_COMMON);
  if (rc < 0)
    printf("net common didn't load.\n");
  rc = sceUtilityLoadNetModule(PSP_NET_MODULE_INET);
  if (rc < 0)
    printf("inet didn't load.\n");
}

void httpInit()
{
  pspDebugScreenPrintf("Loading module SCE_SYSMODULE_HTTP\n");
  sceUtilityLoadNetModule(PSP_NET_MODULE_HTTP);

  pspDebugScreenPrintf("Running sceHttpInit\n");
  sceHttpInit(4 * 1024 * 1024);
}

void httpTerm()
{
  pspDebugScreenPrintf("Running sceHttpTerm\n");
  sceHttpEnd();

  pspDebugScreenPrintf("Unloading module SCE_SYSMODULE_HTTP\n");
  sceUtilityUnloadNetModule(PSP_NET_MODULE_HTTP);
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
  httpInit();
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
  httpTerm();
  sceNetResolverTerm();
  sceNetApctlTerm();
  sceNetInetTerm();
  sceNetTerm();
}
#endif