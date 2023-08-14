/*
 * @Author: 景大虾(dengjingren@foxmail.com)
 * @Date: 2023-08-09
 * @Last Modified by: 景大虾(dengjingren@foxmail.com)
 * @Last Modified time: 2023-08-14
 */
#include <cJSON.h>

#include <cstring>

#include "hooks.h"
#include "scraper/douban/api.h"
#include "tools/utils.h"
using namespace utils::date;
using namespace scraper;

#ifdef ENHANCED_MANUAL_SEARCH
extern thread_local bool in_manual_matching;

void my_HttpResponseData(struct evhttp_request *req, char *s) {
  // 判断是否手动精准匹配
  if (__glibc_likely(!in_manual_matching)) {
    HttpResponseData(req, s);
    return;
  }
  // 改变下发给客户端的`date`字符串
  // 目前只有iOS客户端显示的是date(str)，其它终端在精准匹配时仅显示year(int)
  cJSON *root{};
  do {
    root = cJSON_Parse(s);
    if (__glibc_unlikely(!root)) break;
    auto list = cJSON_GetObjectItemCaseSensitive(root, "list");
    if (__glibc_unlikely(!list)) break;
    unsigned modified_count = 0;
    cJSON *item;
    cJSON_ArrayForEach(item, list) {
      // 只处理豆瓣源
      auto source = cJSON_GetObjectItemCaseSensitive(item, "source");
      if (__glibc_unlikely(!cJSON_IsNumber(source))) continue;
      if (__glibc_unlikely(source->valueint != ugreen::SourceType::kDouban))
        continue;
      // 如果豆瓣源提供了月份信息，我们跳过 🎉😄
      auto date = cJSON_GetObjectItemCaseSensitive(item, "date");
      if (__glibc_unlikely(!cJSON_IsString(date))) continue;
      if (strlen(date->valuestring) >= 6) continue;  // 2023-1-1
      // 判断影视类型
      auto type = cJSON_GetObjectItemCaseSensitive(item, "type");
      if (__glibc_unlikely(!cJSON_IsNumber(type))) continue;
      auto douban_id = cJSON_GetObjectItemCaseSensitive(item, "douban_id");
      if (__glibc_unlikely(!cJSON_IsNumber(douban_id))) continue;
      auto hint = media::Type::kMovie;
      if (type->valueint == ugreen::MediaType::kTv ||
          type->valueint == ugreen::MediaType::kShow) {
        // TV/Show
        hint = media::Type::kTv;
      } else if (type->valueint == ugreen::MediaType::kMovie) {
        hint = media::Type::kMovie;
      }
      // 二次请求豆瓣获得更详细的信息
      if (auto info = douban::api::Get(douban_id->valueint, hint)) {
        if (info->release_date != 0) {
          cJSON_SetValuestring(date, YmdToStr(info->release_date).c_str());
          modified_count++;
        }
      }
    }
    if (__glibc_unlikely(!modified_count)) break;
    auto replaced =
        cJSON_PrintBuffered(root, strlen(s) + 6 * modified_count, false);
    if (__glibc_unlikely(!replaced)) break;
    cJSON_Delete(root);
    HttpResponseData(req, replaced);
    cJSON_free(replaced);
    return;
  } while (false);
  cJSON_Delete(root);
  HttpResponseData(req, s);
}
#endif  // ENHANCED_MANUAL_SEARCH