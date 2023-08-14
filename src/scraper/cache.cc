/*
 * @Author: 景大虾(dengjingren@foxmail.com)
 * @Date: 2023-08-08
 * @Last Modified by: 景大虾(dengjingren@foxmail.com)
 * @Last Modified time: 2023-08-14
 */
#define LOG_TAG "DoubanCache"
#include "cache.h"

#include <elog.h>

#include <fifo_map.hpp>
#include <unordered_map>

#include "tools/synchronized.h"

using namespace nlohmann;
using namespace scraper;

constexpr size_t kMaxCacheSize = 3 * 20 * 10;

using InfoMapping = fifo_map<media::Id, media::InfoPtr>;
using TypeMapping = std::unordered_map<media::Type, InfoMapping>;
using CacheMapping = std::unordered_map<ugreen::SourceType, TypeMapping>;

static std::shared_mutex s_mutex;
static CacheMapping s_caches;

void cache::Trim(ssize_t reserved, ssize_t threshold) {
  synchronized_write(s_mutex) {
    for (auto& source : s_caches) {
      for (auto& type : source.second) {
        auto& infos = type.second;
        ssize_t cached_size = infos.size();
        if (cached_size > threshold) {
          for (ssize_t i = 0; i < cached_size - reserved; i++) {
            // 移除最早添加进来的缓存
            infos.erase(infos.begin());
          }
        }
      }
    }
  }
}

void cache::Add(media::InfoPtr info) {
  synchronized_write(s_mutex) {
    auto& infos = s_caches[info->source][info->type];
    infos.erase(info->id);
    infos.emplace(info->id, info);
  }
  Trim(kMaxCacheSize / 2, kMaxCacheSize);
}

void cache::Add(std::vector<media::InfoPtr>&& arr) {
  synchronized_write(s_mutex) {
    for (auto& info : arr) {
      auto& infos = s_caches[info->source][info->type];
      infos.erase(info->id);
      infos.emplace(info->id, info);
    }
  }
  Trim(kMaxCacheSize / 2, kMaxCacheSize);
}

media::InfoPtr cache::Get(ugreen::SourceType source, media::Type type,
                          media::Id id) {
  synchronized_read(s_mutex) {
    auto src_iter = s_caches.find(source);
    if (src_iter == s_caches.end()) return {};
    auto& types = src_iter->second;
    auto type_iter = types.find(type);
    if (type_iter == types.end()) return {};
    auto& infos = type_iter->second;
    auto info_iter = infos.find(id);
    if (info_iter == infos.end()) return {};
    return info_iter->second;
  }
}

void cache::Dump() {
  synchronized_read(s_mutex) {
    for (auto& source : s_caches) {
      for (auto& type : source.second) {
        size_t i = 0;
        auto& infos = type.second;
        for (auto& pair : infos) {
          auto& info = *pair.second;
          log_i("[%d]来源:%d 类型:%s id:%d 地区数:%d 分类数:%d 日期:%d", i++,
                info.source, info.type == media::Type::kMovie ? "电影" : "剧集",
                pair.first, info.regions.size(), info.genres.size(),
                info.release_date);
        }
      }
    }
  }
}
