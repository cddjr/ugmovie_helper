/*
 * @Author: 景大虾(dengjingren@foxmail.com)
 * @Date: 2023-08-09
 * @Last Modified by: 景大虾(dengjingren@foxmail.com)
 * @Last Modified time: 2023-08-14
 */
#define LOG_TAG "Douban"
#include <cJSON.h>
#include <elog.h>

#include <cstring>

#include "hooks.h"
#include "scraper/douban/api.h"
using namespace scraper;

#ifndef ENHANCED_MANUAL_SEARCH
#ifdef HIGH_RES_DOUBAN

thread_local bool in_eps_create_func = false;
thread_local void *temp_buffer = {};

unsigned char *my_ugreen_evbuffer_pullup(struct evbuffer *buf,
                                         ev_ssize_t size) {
  auto data = ugreen_evbuffer_pullup(buf, size);
  if (__glibc_unlikely(data && in_eps_create_func)) {
    // 在这里修改创建剧集提交的原始数据，把豆瓣高清URL替换进去
    auto ori_len = evbuffer_get_length(buf);
    auto root =
        cJSON_ParseWithLength(reinterpret_cast<const char *>(data), ori_len);
    do {
      if (__glibc_unlikely(!root)) break;
      auto item = cJSON_GetObjectItemCaseSensitive(root, "info");
      if (__glibc_unlikely(!item)) break;
      // 只处理豆瓣源
      auto source = cJSON_GetObjectItemCaseSensitive(item, "source");
      if (__glibc_unlikely(!cJSON_IsNumber(source))) break;
      if (__glibc_unlikely(source->valueint != ugreen::SourceType::kDouban))
        break;
      auto cover_fanart =
          cJSON_GetObjectItemCaseSensitive(item, "cover_fanart");
      if (__glibc_unlikely(!cJSON_IsString(cover_fanart))) break;
      // 判断影视类型
      auto type = cJSON_GetObjectItemCaseSensitive(item, "type");
      if (__glibc_unlikely(!cJSON_IsNumber(type))) break;
      auto douban_id = cJSON_GetObjectItemCaseSensitive(item, "douban_id");
      if (__glibc_unlikely(!cJSON_IsNumber(douban_id))) break;
      auto hint = media::Type::kTv;
      if (type->valueint == ugreen::MediaType::kTv ||
          type->valueint == ugreen::MediaType::kShow) {
        // TV/Show
        hint = media::Type::kTv;
      } else if (type->valueint == ugreen::MediaType::kMovie) {
        hint = media::Type::kMovie;
      }
      // 二次请求豆瓣获得高清海报的URL
      if (auto info = douban::api::Get(douban_id->valueint, hint)) {
        if (!info->cover_url.empty()) {
          cJSON_SetValuestring(cover_fanart, info->cover_url.c_str());
          auto replaced = cJSON_PrintBuffered(
              root, ori_len + info->cover_url.length(), false);
          if (__glibc_unlikely(!replaced)) break;
          temp_buffer = data = reinterpret_cast<unsigned char *>(replaced);
          log_d("成功修改了请求数据 豆瓣id:%d", douban_id->valueint);
        }
      }
    } while (false);
    cJSON_Delete(root);
  }
  return data;
}

/// @brief 响应手动创建剧集
/// @param req
/// @param __unused1
void my_handleMoviesEpsCreate(struct evhttp_request *req, void *__unused1) {
  in_eps_create_func = true;
  handleMoviesEpsCreate(req, __unused1);
  if (temp_buffer) {
    cJSON_free(temp_buffer);
    temp_buffer = {};
  }
  in_eps_create_func = false;
}
#endif  // HIGH_RES_DOUBAN
#endif  // ENHANCED_MANUAL_SEARCH