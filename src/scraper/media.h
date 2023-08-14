/*
 * @Author: 景大虾(dengjingren@foxmail.com)
 * @Date: 2023-08-10
 * @Last Modified by: 景大虾(dengjingren@foxmail.com)
 * @Last Modified time: 2023-08-14
 */
#pragma once
#include <memory>
#include <string>
#include <vector>

#include "ugreen/types.h"

namespace scraper::media {
using Id = int;

enum class Type {
  kMovie,
  kTv,
};

struct Info {
  ugreen::SourceType source = {};  // 刮削源
  scraper::media::Type type = {};  // 类型
  Id id = {};

  unsigned release_date = {};  // 上映日期
  std::string name;            // 影视名称
  std::string cover_url;       // 可能需要有效的Referer才能成功下载
  std::vector<std::string> regions;  // 地区
  std::vector<std::string> genres;   // 分类标签
};

using InfoPtr = std::shared_ptr<scraper::media::Info>;
}  // namespace scraper::media
