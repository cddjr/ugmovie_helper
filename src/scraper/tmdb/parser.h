/*
 * @Author: 景大虾(dengjingren@foxmail.com)
 * @Date: 2023-08-06
 * @Last Modified by: 景大虾(dengjingren@foxmail.com)
 * @Last Modified time: 2023-08-14
 */
#pragma once

#include <memory>
#include <vector>

#include "scraper/media.h"

namespace scraper::tmdb::parser {

/// @brief 从电影、TV详情中提取出所有结果
/// @param response
/// @param hint 媒体类型
/// @param results
/// @return
bool ReadFromDetail(const char *response, scraper::media::Type hint,
                    std::vector<scraper::media::InfoPtr> &results);
}  // namespace scraper::tmdb::parser