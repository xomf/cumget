#include <chrono>
#include <cmath>
#include <curl/curl.h>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>

std::string filename;
double total_time;

size_t writeData(void *ptr, size_t size, size_t nmemb, FILE *stream) {
  return fwrite(ptr, size, nmemb, stream);
}

std::string getFileNameFromUrl(const std::string &url) {
  std::regex pattern("/([^/]+$)");
  std::smatch match;
  if (std::regex_search(url, match, pattern)) {
    return match[1];
  }
  return "file.out";
}

int progressCallback(void *clientp, curl_off_t dltotal, curl_off_t dlnow,
                     curl_off_t ultotal, curl_off_t ulnow) {
  static auto start_time = std::chrono::high_resolution_clock::now();
  auto current_time = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                      current_time - start_time)
                      .count();
  if (duration > 0) {
    double speed = (double)dlnow / (1024 * 1024) / (double)duration * 1000;
    double progress = 100.0 * dlnow / dltotal;
    std::cout << "Downloading file to \"" << filename << "\" at "
              << round(speed) << "Mbps (" << round(progress) << "%)   \r"
              << std::flush;
  }
  return 0;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " URL [FILENAME]" << std::endl;
    return 1;
  }

  CURL *curl = curl_easy_init();
  if (!curl)
    return 1;

  std::string url = argv[1];
  if (argc > 2) {
    filename = argv[2];
  } else {
    filename = getFileNameFromUrl(url);
  }

  FILE *fp = fopen(filename.c_str(), "wb");
  if (!fp)
    return 1;

  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeData);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
  curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
  curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progressCallback);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  CURLcode res = curl_easy_perform(curl);

  curl_easy_cleanup(curl);
  fclose(fp);
  std::cout << std::endl;
  return 0;
}
