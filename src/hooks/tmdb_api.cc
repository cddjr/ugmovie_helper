/*
 * @Author: 景大虾(dengjingren@foxmail.com)
 * @Date: 2023-08-05
 * @Last Modified by: 景大虾(dengjingren@foxmail.com)
 * @Last Modified time: 2023-08-14
 */
#define LOG_TAG "Tmdb"
#include <cJSON.h>
#include <elog.h>
#include <string.h>

#include "hooks.h"
#include "scraper/cache.h"
#include "scraper/tmdb/parser.h"
using namespace scraper;

int my_TmdbApi_GetMovieDetailDataParse(void *__this, const char *resp,
                                       void *__unused1) {
  // 缓存电影信息
  std::vector<media::InfoPtr> results;
  if (tmdb::parser::ReadFromDetail(resp, media::Type::kMovie, results)) {
    cache::Add(std::move(results));
#ifndef NDEBUG
    cache::Dump();
#endif
  } else {
    log_w(resp);
  }
  return TmdbApi_GetMovieDetailDataParse(__this, resp, __unused1);
}

int my_TmdbApi_GetTvDetailDataParse(void *__this, const char *resp,
                                    void *__unused1) {
  // 解析剧集详情里面季列表
  std::vector<media::InfoPtr> results;
  if (tmdb::parser::ReadFromDetail(resp, media::Type::kTv, results)) {
    cache::Add(std::move(results));
#ifndef NDEBUG
    cache::Dump();
#endif
  } else {
    log_w(resp);
  }
  return TmdbApi_GetTvDetailDataParse(__this, resp, __unused1);
}