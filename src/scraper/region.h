/*
 * @Author: 景大虾(dengjingren@foxmail.com)
 * @Date: 2023-08-10
 * @Last Modified by: 景大虾(dengjingren@foxmail.com)
 * @Last Modified time: 2023-08-10
 */
#pragma once
#include <string>

namespace scraper::region {
bool IsRegion(std::string_view s);

std::string_view GetChinese(std::string_view iso_3166_1);
}  // namespace scraper::region
