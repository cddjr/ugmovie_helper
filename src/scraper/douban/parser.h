/*
 * @Author: 景大虾(dengjingren@foxmail.com)
 * @Date: 2023-08-08
 * @Last Modified by: 景大虾(dengjingren@foxmail.com)
 * @Last Modified time: 2023-08-14
 */
#pragma once

#include "scraper/media.h"

namespace scraper::douban::parser {
/// @brief 从Movie、TV详情中提取出标签id
/// @param response
/// @return
scraper::media::InfoPtr Read(std::string_view response);
}  // namespace scraper::douban::parser