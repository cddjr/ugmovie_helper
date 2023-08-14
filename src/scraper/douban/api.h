/*
 * @Author: 景大虾(dengjingren@foxmail.com)
 * @Date: 2023-08-08
 * @Last Modified by: 景大虾(dengjingren@foxmail.com)
 * @Last Modified time: 2023-08-14
 */
#pragma once

#include <string>
#include <vector>

#include "scraper/media.h"

namespace scraper::douban::api {
/// @brief 获取豆瓣更多的信息
/// @param id 豆瓣id
/// @param hint 影视类型
/// @param force 是否无视缓存强制联网
/// @return
scraper::media::InfoPtr Get(scraper::media::Id id, media::Type hint,
                            bool force = false);
}  // namespace scraper::douban::api
