#include <pspdebug.h>
#include <pspiofilemgr.h>
#include <pspnet.h>
#include <pspnet_apctl.h>
#include <pspnet_inet.h>
#include <pspsysmem.h>
#include <pspkernel.h>
#include <psphttp.h>
#include <psputility.h>

#include <cstring>
#include <malloc.h>
#include <psphttp.h>
#include <pspthreadman.h>
#include <stdio.h>
#include <string>

#include <fcntl.h>

#include <curl/curl.h>

PSP_MODULE_INFO("curl_test", 0, 1, 1);

struct stringcurl
{
	char *ptr;
	size_t len;
};

void init_string(struct stringcurl *s)
{
	s->len = 0;
	s->ptr = (char *)malloc(s->len + 1);
	if (s->ptr == NULL)
	{
		fprintf(stderr, "malloc() failed\n");
		return;
		// exit(EXIT_FAILURE);
	}
	s->ptr[0] = '\0';
}

size_t writefunc(void *ptr, size_t size, size_t nmemb, struct stringcurl *s)
{
	size_t new_len = s->len + size * nmemb;
	s->ptr = (char *)realloc(s->ptr, new_len + 1);
	if (s->ptr == NULL)
	{
		fprintf(stderr, "realloc() failed\n");
		return 0;
		// exit(EXIT_FAILURE);
	}
	memcpy(s->ptr + s->len, ptr, size * nmemb);
	s->ptr[new_len] = '\0';
	s->len = new_len;

	return size * nmemb;
}

static size_t write_data_to_disk(void *ptr, size_t size, size_t nmemb,
																 void *stream)
{
	size_t written = sceIoWrite(*(int *)stream, ptr, size * nmemb);
	return written;
}

void curlDownloadFile(std::string url, std::string file)
{
	int imageFD = sceIoOpen(file.c_str(), O_WRONLY | O_CREAT, 0777);
	if (!imageFD)
	{
		return;
	}

	CURL *curl;
	CURLcode res;
	curl = curl_easy_init();

	if (curl)
	{
		struct stringcurl body;
		init_string(&body);
		struct stringcurl header;
		init_string(&header);
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		// Set useragent string
		curl_easy_setopt(
				curl, CURLOPT_USERAGENT,
				"Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, "
				"like Gecko) Chrome/58.0.3029.110 Safari/537.36");
		// not sure how to use this when enabled
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
		// not sure how to use this when enabled
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		// Set SSL VERSION to TLS 1.2
		curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1);
		// Set timeout for the connection to build
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
		// Follow redirects (?)
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		// Disable progress meter, set to 0L to enable and disable debug output
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
		// The function that will be used to write the data
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data_to_disk);
		// The data filedescriptor which will be written to
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &imageFD);
		// write function of response headers
		curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, writefunc);
		// the response header data where to write to
		curl_easy_setopt(curl, CURLOPT_HEADERDATA, &header);
		// Request Header :
		struct curl_slist *headerchunk = NULL;
		headerchunk = curl_slist_append(headerchunk, "Accept: */*");
		headerchunk =
				curl_slist_append(headerchunk, "Content-Type: application/json");
		headerchunk = curl_slist_append(
				headerchunk, "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) "
										 "AppleWebKit/537.36 (KHTML, like Gecko) "
										 "Chrome/58.0.3029.110 Safari/537.36");
		// headerchunk = curl_slist_append(headerchunk, "Host: discordapp.com");
		// Setting this will lead to errors when trying to download. should be set
		// depending on location : possible : cdn.discordapp.com or
		// images.discordapp.com
		headerchunk = curl_slist_append(headerchunk, "Content-Length: 0");
		res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerchunk);

		// Perform the request
		res = curl_easy_perform(curl);
		int httpresponsecode = 0;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpresponsecode);

		if (res != CURLE_OK)
		{
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
							curl_easy_strerror(res));
		}
		else
		{
		}
	}
	else
	{
	}

	// close filedescriptor
	sceIoClose(imageFD);

	// cleanup
	curl_easy_cleanup(curl);
}

void netInit(void)
{
	if (sceNetInit(128 * 1024, 42, 4 * 1024, 42, 4 * 1024) < 0)
		pspDebugScreenPrintf("sceNetInit failed\n");

	sceNetInetInit();

	sceNetApctlInit(0x8000, 48);
}

void netTerm(void)
{
	sceNetApctlTerm();

	sceNetInetTerm();

	sceNetTerm();
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

int main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	pspDebugScreenInit();
	pspDebugScreenPrintf("HTTP Sample v.1.0 by barooney\n\n");

	netInit();
	httpInit();

	struct SceIoStat *dirStat = (SceIoStat *)malloc(sizeof(SceIoStat));
	if (sceIoGetstat("host0:data/curltest", dirStat) < 0)
	{
		sceIoMkdir("host0:data/curltest", 0777);
	}
	pspDebugScreenPrintf("Downloading file psyq-4_7-converted-light.zip to host0:data/curltest/");
	curlDownloadFile("http://psx.arthus.net/sdk/Psy-Q/psyq-4_7-converted-light.zip",
									 "host0:data/curltest/vita_cord.vpk");

	httpTerm();
	netTerm();

	pspDebugScreenPrintf("This app will close in 10 seconds!\n");
	sceKernelDelayThread(10 * 1000 * 1000);

	sceKernelExitGame();
	return 0;
}
