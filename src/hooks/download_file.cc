/*
 * @Author: 景大虾(dengjingren@foxmail.com)
 * @Date: 2023-08-15
 * @Last Modified by: 景大虾(dengjingren@foxmail.com)
 * @Last Modified time: 2023-08-15
 */
#define LOG_TAG "Download"
#include <elog.h>

#include "hooks.h"
using namespace std::literals;

#if defined(HIGH_RES_DOUBAN) && defined(USE_DOUBAN_WECHAT_API)
thread_local bool is_restricted_by_douban = false;

CURL* my_ugreen_curl_easy_init(void) {
  auto handle = ugreen_curl_easy_init();
  if (handle && is_restricted_by_douban) {
    curl_easy_setopt(handle, CURLOPT_REFERER, "https://www.douban.com/");
  }
  return handle;
}

int my_movies_svr_common_download_file(const std::string& url,
                                       const std::string& __unused1,
                                       bool __unused2, size_t __unused3) {
  if (url.find(".doubanio.com/"sv) != url.npos) {
    if (url.find("company_token=") == url.npos) {
      is_restricted_by_douban = true;
      log_d("尝试模拟豆瓣下载 %s", url.c_str());
    }
  }
  int ret =
      movies_svr_common_download_file(url, __unused1, __unused2, __unused3);
  is_restricted_by_douban = false;
  return ret;
}
#endif  // HIGH_RES_DOUBAN && USE_DOUBAN_WECHAT_API