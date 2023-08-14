/*
 * @Author: 景大虾(dengjingren@foxmail.com)
 * @Date: 2023-08-08
 * @Last Modified by: 景大虾(dengjingren@foxmail.com)
 * @Last Modified time: 2023-08-14
 */
#pragma once

#include "media.h"

namespace scraper::cache {
/// @brief 添加进缓存
/// @param info
void Add(scraper::media::InfoPtr info);

/// @brief 批量添加进缓存
/// @param arr
void Add(std::vector<scraper::media::InfoPtr>&& arr);

/// @brief 根据id获取缓存
/// @param source 获取豆瓣或Tmdb的缓存
/// @param type 电影、TV等类型
/// @param id 豆瓣、Tmdb id
/// @return 缓存的信息
scraper::media::InfoPtr Get(ugreen::SourceType source,
                            scraper::media::Type type, scraper::media::Id id);

void Trim(ssize_t reserved, ssize_t threshold = 0);

/// @brief 缓存内容输出到log
void Dump();
}  // namespace scraper::cache
