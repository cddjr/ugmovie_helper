/*
 * @Author: 景大虾(dengjingren@foxmail.com)
 * @Date: 2023-08-08
 * @Last Modified by: 景大虾(dengjingren@foxmail.com)
 * @Last Modified time: 2023-08-14
 */
#define LOG_TAG "DoubanApi"
#include "api.h"

#include <elog.h>

#include "network/http_client.h"
#include "parser.h"
#include "scraper/cache.h"

using namespace std::literals;
using namespace scraper;

static HttpClient douban_http;

media::InfoPtr douban::api::Get(media::Id id, media::Type hint, bool force) {
  if (!force) {
    if (auto info = cache::Get(ugreen::SourceType::kDouban, hint, id)) {
      // 重新加入缓存以增加其权重
      cache::Add(info);
      return info;
    }
  }
  char url[1024];
  const char* fmt;
  std::string referer;
  switch (hint) {
    case media::Type::kMovie:
      fmt = "https://m.douban.com/movie/subject/%d/?event_source=movie_showing";
      referer = "https://m.douban.com/movie/"s;
      break;
    case media::Type::kTv:
      fmt = "https://m.douban.com/movie/subject/%d/?refer=home";
      referer = "https://m.douban.com/tv/"s;
      break;
    default:
      return {};
      break;
  }
  if (snprintf(url, sizeof(url), fmt, id) < 0) {
    log_e("");
    return {};
  }

#if 0  // def DEBUG
  douban_http.set_proxy("http://192.168.1.3:8888");
#endif

  std::string resp;
  auto code = douban_http.Get(url, &resp, {{"Referer"s, std::move(referer)}});
  if (code != HttpClient::kOk) {
    log_e("无法访问豆瓣网 code:%d", code);
    return {};
  }

  if (auto info = parser::Read(resp)) {
    cache::Add(info);
    return info;
  }
  return {};
}
