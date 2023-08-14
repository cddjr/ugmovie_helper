/*
 * @Author: 景大虾(dengjingren@foxmail.com)
 * @Date: 2023-08-08
 * @Last Modified by: 景大虾(dengjingren@foxmail.com)
 * @Last Modified time: 2023-08-14
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