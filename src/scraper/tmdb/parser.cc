/*
 * @Author: 景大虾(dengjingren@foxmail.com)
 * @Date: 2023-08-06
 * @Last Modified by: 景大虾(dengjingren@foxmail.com)
 * @Last Modified time: 2023-08-14
 */
#define LOG_TAG "TmdbParser"
#include "parser.h"

#include <cJSON.h>
#include <elog.h>

#include <cstring>

#include "scraper/region.h"
#include "tools/utils.h"

using namespace scraper;

static bool ParseTypeAndId(const cJSON *json, media::Type &type,
                           media::Id &id) {
  auto obj_id = cJSON_GetObjectItemCaseSensitive(json, "id");
  if (!cJSON_IsNumber(obj_id)) return false;
  id = obj_id->valueint;
  // 检测数据可能的类型
  if (auto obj_type = cJSON_GetObjectItemCaseSensitive(json, "media_type");
      cJSON_IsString(obj_type)) {
    if (!strcmp(obj_type->valuestring, "tv")) {
      type = media::Type::kTv;
    } else if (!strcmp(obj_type->valuestring, "movie")) {
      type = media::Type::kMovie;
    } else {
      // log_e("不支持的媒体类型 %s", type->valuestring);
      return false;
    }
  }
  return true;
}

static bool ParseGenres(const cJSON *json, std::vector<std::string> &genres) {
  /*if (auto ids = cJSON_GetObjectItemCaseSensitive(json, "genre_ids")) {
    // 来自搜索 只有标签ID
    const cJSON *item;
    cJSON_ArrayForEach(item, ids) {
      if (cJSON_IsNumber(item)) {
        const auto &str = tmdb::genres::Get(item->valueint);
        if (!str.empty()) {
          genres.emplace_back(str);
        }
      }
    }
    return true;
  } else*/
  if (auto ids = cJSON_GetObjectItemCaseSensitive(json, "genres")) {
    // 来自详情 有带名字的标签
    const cJSON *item;
    cJSON_ArrayForEach(item, ids) {
      auto genre = cJSON_GetObjectItemCaseSensitive(item, "name");
      if (cJSON_IsString(genre)) {
        genres.emplace_back(genre->valuestring);
      }
    }
    return true;
  }
  return false;
}

static media::InfoPtr ParseMovie(const cJSON *json, media::Info &info) {
  if (auto title = cJSON_GetObjectItemCaseSensitive(json, "title");
      cJSON_IsString(title)) {
    info.name = title->valuestring;
  }
  // 上映日期
  if (auto date = cJSON_GetObjectItemCaseSensitive(json, "release_date");
      cJSON_IsString(date)) {
    // 2016-09-23
    info.release_date = utils::StrToYmd(date->valuestring);
  }
  // 国家/地区
  if (auto regions =
          cJSON_GetObjectItemCaseSensitive(json, "production_countries")) {
    const cJSON *item;
    cJSON_ArrayForEach(item, regions) {
      auto region = cJSON_GetObjectItemCaseSensitive(item, "iso_3166_1");
      if (cJSON_IsString(region)) {
        auto name = region::GetChinese(region->valuestring);
        if (!name.empty()) info.regions.emplace_back(name);
      }
    }
  }
  return std::make_shared<media::Info>(std::move(info));
}

static std::vector<media::InfoPtr> ParseSeasons(const cJSON *json,
                                                media::Info &info) {
  // 电视剧详情才有seasons
  auto seasons = cJSON_GetObjectItemCaseSensitive(json, "seasons");
  if (!cJSON_IsArray(seasons)) return {};

  if (auto name = cJSON_GetObjectItemCaseSensitive(json, "name");
      cJSON_IsString(name)) {
    info.name = name->valuestring;
  }
  // 首播日期
  if (auto date = cJSON_GetObjectItemCaseSensitive(json, "first_air_date");
      cJSON_IsString(date)) {
    info.release_date = utils::StrToYmd(date->valuestring);
  }
  // 国家/地区
  if (auto regions = cJSON_GetObjectItemCaseSensitive(json, "origin_country")) {
    const cJSON *item;
    cJSON_ArrayForEach(item, regions) {
      if (cJSON_IsString(item)) {
        auto name = region::GetChinese(item->valuestring);
        if (!name.empty()) info.regions.emplace_back(name);
      }
    }
  }
  std::vector<media::InfoPtr> results;
  const cJSON *item;
  cJSON_ArrayForEach(item, seasons) {
    if (auto id = cJSON_GetObjectItemCaseSensitive(item, "id");
        cJSON_IsNumber(id)) {
      info.id = id->valueint;
      // 把每一季的信息拷贝构造到结果中去
      results.emplace_back(std::make_shared<media::Info>(info));
    }
  }
  return std::move(results);
}

bool tmdb::parser::ReadFromDetail(const char *response, media::Type hint,
                                  std::vector<media::InfoPtr> &results) {
  bool ret = false;
  auto root = cJSON_Parse(response);
  do {
    if (!root) break;
    // 所有媒体数据都应该有id
    media::Info info;
    info.source = ugreen::SourceType::kTmdb;
    info.type = hint;
    if (!ParseTypeAndId(root, info.type, info.id)) break;
    // 解析标签
    ParseGenres(root, info.genres);
    switch (info.type) {
      case media::Type::kMovie:
        if (auto ptr = ParseMovie(root, info)) {
          results.emplace_back(ptr);
          ret = true;
        }
        break;
      case media::Type::kTv:
        if (auto seasons = ParseSeasons(root, info); !seasons.empty()) {
          results.insert(results.end(), seasons.begin(), seasons.end());
          ret = true;
        }
        break;
      default:
        break;
    }

  } while (false);
  cJSON_Delete(root);
  return ret;
}