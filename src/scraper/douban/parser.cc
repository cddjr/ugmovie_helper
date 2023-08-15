/*
 * @Author: 景大虾(dengjingren@foxmail.com)
 * @Date: 2023-08-08
 * @Last Modified by: 景大虾(dengjingren@foxmail.com)
 * @Last Modified time: 2023-08-15
 */
#define LOG_TAG "DoubanParser"
#include "parser.h"

#include <elog.h>

#include <cstring>
#include <set>
#include <vector>

#include "scraper/region.h"
#include "tools/utils.h"

using namespace std::literals;
using namespace scraper;
using namespace utils;

#if defined(USE_DOUBAN_WEB_API)
static bool ParseMeta(std::string_view response,
                      std::vector<std::string> &regions, unsigned int &date,
                      std::vector<std::string> &genres) {
  /* https://m.douban.com/movie/subject/27619748/
  和27615441有多个地区 😕
  只有pc端网页才能区分地区与类型，移动端全合并在一起了
  <div class="sub-meta">
  美国 / 俄罗斯 / 剧情 / 悬疑 / 惊悚 / 2018-12-14(中国大陆)上映 / 片长102分钟
 中国大陆 / 中国香港 / 日本 / 喜剧 / 悬疑 / 2021-02-12(中国大陆)上映 /
片长136分钟
</div>
  */
  constexpr auto p = "<div class=\"sub-meta\">"sv;
  auto pos1 = response.find(p);
  if (__glibc_unlikely(pos1 == response.npos)) return false;
  pos1 += p.size();
  auto pos2 = response.find("</div>"sv, pos1);
  if (__glibc_unlikely(pos2 == response.npos)) return false;

  auto meta = Tokenize(response.substr(pos1, pos2 - pos1), "/"sv);
  auto meta_size = meta.size();

  if (__glibc_unlikely(meta_size < 1)) return false;
  if (meta[meta_size - 1].find("片长"sv) != std::string_view::npos) {
    meta_size--;
  }

  if (__glibc_unlikely(meta_size < 1)) return false;
  if (meta[meta_size - 1].find("上映"sv) != std::string_view::npos) {
    // 2023-07-20(中国大陆)上映
    // 2001-04-14上映
    date = StrToYmd(Trim(meta[meta_size - 1]));
    meta_size--;
  }
  // 地区在前、分类在后，只提取本地认识的元数据
  // 地区、不认识的、不认识的、地区、分类、分类、分类
  size_t genre_pos = 0;
  for (size_t i = 0; i < meta_size; i++) {
    auto item = Trim(meta[i]);
    if (region::IsRegion(item)) {
      regions.emplace_back(item);
      genre_pos = i + 1;
    }
  }
  // 剩下的都认为是分类，
  // FIXME 受移动端豆瓣网页的局限 可能会误把地区识别成分类
  for (size_t i = genre_pos; i < meta_size; i++) {
    auto item = Trim(meta[i]);
    genres.emplace_back(item);
  }
  return true;
}

/// @brief 解析出高清海报URL
/// @param response
/// @param result
/// @return
static bool ParseCover(std::string_view response, std::string &result) {
  // <meta itemprop="image" content="https://xxxxx.jpg?yyyy">
  constexpr auto p = "<meta itemprop=\"image\" content=\""sv;
  auto pos1 = response.find(p);
  if (__glibc_unlikely(pos1 == response.npos)) return false;
  pos1 += p.size();
  auto pos2 = response.find('?', pos1);  // 丢弃后面的参数，确保原图
  if (__glibc_unlikely(pos2 == response.npos))
    pos2 = response.find("\">", pos1);
  if (__glibc_unlikely(pos2 == response.npos)) return false;
  result = response.substr(pos1, pos2 - pos1);
  // 原图二次压缩，避免下载费劲
  // 这类地址支持 https://qnmob3.doubanio.com/view/photo/large/public/pXXXX.jpg
  result += "?imageView2/1/q/75/format/jpg"s;
  return true;
}

static bool ParseTypeAndId(std::string_view response, media::Type &type,
                           media::Id &id) {
  // <section class="subject-mark mark-xxx" data-type="xxx" data-id="yyy">
  constexpr auto p = "<section class=\"subject-mark mark-"sv;
  auto pos1 = response.find(p);
  if (__glibc_unlikely(pos1 == response.npos)) return false;
  // sscanf

  pos1 += p.size();
  auto pos2 = response.find('\"', pos1);
  if (__glibc_unlikely(pos2 == response.npos)) return false;
  auto str_type = response.substr(pos1, pos2 - pos1);
  if (str_type == "movie"sv)
    type = media::Type::kMovie;
  else if (str_type == "tv"sv)
    type = media::Type::kTv;
  else
    return false;

  constexpr auto p2 = "data-id=\""sv;
  pos1 = response.find(p2, pos2);
  if (__glibc_unlikely(pos1 == response.npos)) return false;
  pos1 += p2.size();
  id = atoi(response.substr(pos1).data());

  return true;
}

media::InfoPtr douban::parser::Read(std::string_view response) {
  media::Info info;
  info.source = ugreen::SourceType::kDouban;
  if (!ParseTypeAndId(response, info.type, info.id)) {
    log_e("id解析失败");
    return {};
  }
  if (!ParseCover(response, info.cover_url)) {
    log_w("海报URL解析失败");
  }
  if (!ParseMeta(response, info.regions, info.release_date, info.genres)) {
    log_w("元信息解析失败");
  }
  return std::make_shared<media::Info>(std::move(info));
}
#elif defined(USE_DOUBAN_WECHAT_API)
#include <cJSON.h>
static bool ParseMeta(const cJSON *root, std::vector<std::string> &regions,
                      unsigned int &date, std::vector<std::string> &genres) {

  const cJSON *item;
  auto obj_pubdate = cJSON_GetObjectItemCaseSensitive(root, "pubdate");
  if (__glibc_likely(cJSON_IsArray(obj_pubdate))) {
    cJSON_ArrayForEach(item, obj_pubdate) {
      if (__glibc_unlikely(!cJSON_IsString(item))) continue;
      date = StrToYmd(item->valuestring);
      // 只取第一个日期
      break;
    }
  }
  if (__glibc_unlikely(!date)) {
    auto obj_year = cJSON_GetObjectItemCaseSensitive(root, "year");
    if (__glibc_likely(cJSON_IsString(item))) {
      date = atoi(item->valuestring) * 10000;
    }
  }
  auto obj_countries = cJSON_GetObjectItemCaseSensitive(root, "countries");
  if (__glibc_likely(cJSON_IsArray(obj_countries))) {
    cJSON_ArrayForEach(item, obj_countries) {
      if (__glibc_unlikely(!cJSON_IsString(item))) continue;
      regions.emplace_back(item->valuestring);
    }
  }
  auto obj_genres = cJSON_GetObjectItemCaseSensitive(root, "genres");
  if (__glibc_likely(cJSON_IsArray(obj_genres) && obj_genres->child)) {
    // 2022 / 日本 / 剧情 喜剧 动作 爱情 动画 / 田中智也 / 齐藤壮马 会泽纱弥
    auto obj_card_subtitle =
        cJSON_GetObjectItemCaseSensitive(root, "card_subtitle");
    if (__glibc_likely(cJSON_IsString(obj_card_subtitle))) {
      auto meta = Tokenize(obj_card_subtitle->valuestring, "/"sv);
      int i = 0;
      if (__glibc_likely(date != 0)) i++;
      if (__glibc_likely(!regions.empty())) i++;
      if (__glibc_likely(i < meta.size())) {
        for (auto genre : Tokenize(meta[i], " "sv)) {
          genres.emplace_back(genre);
        }
      }
    }
  }
  return true;
}

static bool ParseCover(const cJSON *root, std::string &result) {
  /*
  "pic":{ "large":"XXX.jpg", "normal":"YYY.jpg" },
  "cover_url": "ZZZ.jpg"
  */

  /*
  FIXME 如果图片地址不含有效的company_token
  或者下载时没有豆瓣相关的referer，会被限速10kb
  */
  auto obj_cover_url = cJSON_GetObjectItemCaseSensitive(root, "cover_url");
  if (__glibc_unlikely(!cJSON_IsString(obj_cover_url))) return false;
  result.assign(obj_cover_url->valuestring);
  auto iter = result.find("/m_ratio_poster/");
  if (__glibc_unlikely(iter == result.npos))
    iter = result.find("/s_ratio_poster/");
  if (__glibc_likely(iter != result.npos)) {
    // 改为使用 l_ratio_poster
    result.data()[iter + 1] = 'l';
  }
  return true;
}

static bool ParseTypeAndId(const cJSON *root, media::Type &type,
                           media::Id &id) {
  auto obj_id = cJSON_GetObjectItemCaseSensitive(root, "id");
  if (__glibc_unlikely(!cJSON_IsString(obj_id))) return false;
  id = atoi(obj_id->valuestring);
  auto obj_type = cJSON_GetObjectItemCaseSensitive(root, "type");
  if (__glibc_unlikely(!cJSON_IsString(obj_type))) return false;
  if (!strcmp(obj_type->valuestring, "movie"))
    type = media::Type::kMovie;
  else if (!strcmp(obj_type->valuestring, "tv"))
    type = media::Type::kTv;
  else
    return false;
#if 0
  // TODO show
  auto obj_is_show = cJSON_GetObjectItemCaseSensitive(root, "is_show");
  if (cJSON_IsTrue(obj_is_show)) {
    type = media::Type::kShow;
  }
#endif
  return true;
}

media::InfoPtr douban::parser::Read(std::string_view response) {
  auto root = cJSON_ParseWithLength(response.data(), response.size());
  if (!root) return {};
  media::InfoPtr ret;
  do {
    media::Info info;
    info.source = ugreen::SourceType::kDouban;
    if (!ParseTypeAndId(root, info.type, info.id)) {
      log_e("id解析失败");
      break;
    }
    if (!ParseCover(root, info.cover_url)) {
      log_w("海报URL解析失败");
    }
    if (!ParseMeta(root, info.regions, info.release_date, info.genres)) {
      log_w("元信息解析失败");
    }
    ret = std::make_shared<media::Info>(std::move(info));
  } while (false);
  cJSON_Delete(root);

  return ret;
}
#endif