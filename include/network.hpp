#pragma once

#include <string>

#define SAVE_FILE "./save.zip"

#ifdef __PSP__
int drawNetDialog();
int netDialog();
void stopNetworking();
void startNetworking();
#endif

int curlDownload(std::string &url, std::string file = SAVE_FILE);

size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream);