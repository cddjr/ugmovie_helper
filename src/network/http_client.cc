/*
 * @Author: 景大虾(dengjingren@foxmail.com)
 * @Date: 2023-08-08
 * @Last Modified by: 景大虾(dengjingren@foxmail.com)
 * @Last Modified time: 2023-08-15
 */
#include "http_client.h"

#include <curl/curl.h>
static_assert(CURL_AT_LEAST_VERSION(7, 81, 0), "更新 libcurl4-openssl-dev");

using namespace std::literals;

#if 0  // 绿联已经初始化了
__attribute__((constructor)) void init() {
  curl_global_init(CURL_GLOBAL_ALL);
}
#endif

HttpClient::~HttpClient() noexcept {
  while (!handles_.empty()) {
    curl_easy_cleanup(handles_.top());
    handles_.pop();
  }
}

static curl_slist* BuildHeaderList(
    const std::vector<std::pair<std::string, std::string>>& headers) {
  char header[1024]{};
  curl_slist* slist{};
  for (const auto& pair : headers) {
    snprintf(header, sizeof(header), "%s: %s", pair.first.data(),
             pair.second.data());
    slist = curl_slist_append(slist, header);
    if (__glibc_unlikely(!slist)) {
      curl_slist_free_all(slist);
      break;
    }
  }
  return slist;
}

static void AddHeader(std::vector<std::pair<std::string, std::string>>& headers,
                      std::string_view key, std::string_view value,
                      bool replace = false) {
  if (replace) {
    for (auto& p : headers) {
      if (std::equal(p.first.begin(), p.first.end(), key.begin(), key.end(),
                     [](char a, char b) { return tolower(a) == tolower(b); })) {
        p.second = value;
        return;
      }
    }
  }
  headers.emplace_back(key, value);
  return;
}

static size_t WriteCallback(char* data, size_t size, size_t nmemb,
                            void* userdata) {
  auto response = static_cast<std::string*>(userdata);
  if (__glibc_unlikely(!response)) return 0;
  size_t realsize = size * nmemb;
  response->append(data, realsize);
  return realsize;
}

void HttpClient::InitDefaultHeader(
    std::vector<std::pair<std::string, std::string>>& headers) {
  AddHeader(
      headers, "User-Agent"sv,
      "Mozilla/5.0 (iPhone; CPU iPhone OS 15_7_1 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) Mobile/15E148 ugtools/1.0.0"sv);
  AddHeader(headers, "Accept-Language"sv, "zh-CN,zh-Hans;q=0.9"sv);
  AddHeader(headers, "Accept"sv,
            "text/html,application/json,image/jpeg;q=0.9,*/*;q=0.8"sv);
}

HttpClient::StatusCode HttpClient::Get(
    std::string_view url, std::string* response,
    const std::vector<std::pair<std::string, std::string>>& headers) {
  CURL* curl = static_cast<CURL*>(GetHandle());
  if (__glibc_unlikely(!curl)) return -1;
  if (__glibc_unlikely(!response)) return -1;

  curl_easy_setopt(curl, CURLOPT_URL, url.data());

  auto headers_copy = headers;
  InitDefaultHeader(headers_copy);
  auto header = BuildHeaderList(headers_copy);
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);

  response->clear();
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);

  if (!proxy_.empty()) {
    curl_easy_setopt(curl, CURLOPT_PROXY, proxy_.c_str());
  }

  StatusCode code{-1};
  CURLcode res = curl_easy_perform(curl);
  if (__glibc_likely(res == CURLE_OK)) {
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
  }

  curl_slist_free_all(header);
  ReleaseHandle(curl);
  return code;
}

HttpClient::StatusCode HttpClient::Post(
    std::string_view url, std::string* response, std::string_view data,
    const std::vector<std::pair<std::string, std::string>>& headers) {
  CURL* curl = static_cast<CURL*>(GetHandle());
  if (__glibc_unlikely(!curl)) return -1;
  if (__glibc_unlikely(!response)) return -1;

  curl_easy_setopt(curl, CURLOPT_URL, url.data());

  auto headers_copy = headers;
  InitDefaultHeader(headers_copy);
  AddHeader(headers_copy, "Content-Type"sv,
            "application/x-www-form-urlencoded"sv);
  auto header = BuildHeaderList(headers_copy);
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);

  response->clear();
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);

  if (!proxy_.empty()) {
    curl_easy_setopt(curl, CURLOPT_PROXY, proxy_.c_str());
  }

  curl_easy_setopt(curl, CURLOPT_POST, 1L);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.data());
  curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data.size());

  StatusCode code{-1};
  CURLcode res = curl_easy_perform(curl);
  if (__glibc_likely(res == CURLE_OK)) {
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
  }

  curl_slist_free_all(header);
  ReleaseHandle(curl);
  return code;
}

void* HttpClient::GetHandle() {
  synchronized(mutex_) {
    if (__glibc_likely(handles_.empty())) {
      auto curl = curl_easy_init();
      if (__glibc_unlikely(!curl)) return {};

      // 初始化
      curl_easy_setopt(curl, CURLOPT_COOKIEJAR, "/tmp/.ugreen_tools_cookies");
      curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "/tmp/.ugreen_tools_cookies");
      curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

      curl_easy_setopt(curl, CURLOPT_HTTPPROXYTUNNEL, 1L);
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);

      // 不验证SSL
      curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
      curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

      // 由curl决定编码
      curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");

      // 长连接
      curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
      curl_easy_setopt(curl, CURLOPT_TCP_KEEPIDLE, 30L);
      curl_easy_setopt(curl, CURLOPT_TCP_KEEPINTVL, 20L);

      // 超时设置
      curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
      curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
      curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);

      return curl;
    } else {
      auto handle = handles_.top();
      handles_.pop();
      return handle;
    }
  }
}

void HttpClient::ReleaseHandle(void* handle) {
  CURL* curl = static_cast<CURL*>(handle);
  if (__glibc_unlikely(!curl)) return;

  curl_easy_setopt(curl, CURLOPT_URL, nullptr);
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, nullptr);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, nullptr);
  curl_easy_setopt(curl, CURLOPT_PROXY, nullptr);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, nullptr);
  curl_easy_setopt(curl, CURLOPT_POST, 0L);

  synchronized(mutex_) { handles_.push(curl); }
}
