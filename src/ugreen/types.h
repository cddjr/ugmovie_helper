/*
 * @Author: 景大虾(dengjingren@foxmail.com)
 * @Date: 2023-08-05
 * @Last Modified by: 景大虾(dengjingren@foxmail.com)
 * @Last Modified time: 2023-08-15
 */
#pragma once
#include <curl/curl.h>
#include <event2/buffer.h>
#include <event2/event.h>
#include <event2/http.h>
#include <event2/http_struct.h>
#include <stdint.h>

#include <string>
#include <vector>

#include "tools/enum_class_helper.h"

// 绿联目前用的旧ABI编译
static_assert(_GLIBCXX_USE_CXX11_ABI == 0);
static_assert(LIBEVENT_VERSION_NUMBER >= 0x2010c00,
              "更新 libevent2-dev 需要2.1.12-stable以上");

static_assert(sizeof(std::string) == sizeof(uintptr_t));

//  定义可参考 http_request_get_token()
struct NASUserNode {
  void* db;
  int32_t id;
  int32_t status;
  int32_t version;
  int32_t ugreen_no;  // 绿联UID
  int32_t role;       // 1 管理员 0 子用户
  // ...
};

using SingletonLazySystemSetting = void*;

namespace ugreen {

namespace MediaType {
  constexpr int32_t kMovie = 1;
  constexpr int32_t kTv = 2;
  constexpr int32_t kShow = 4;
  constexpr int32_t kCollection = 8;
  constexpr int32_t kUnrecognized = 1 << 30;
}  // namespace MediaType
using MediaTypeValues = int32_t;

// 定义可参考ugreen::sqlite3_wraper_label::insert_record()
struct t_label_info {
  int64_t id;
  int64_t labelId;
  int64_t ugreen_no;
  int64_t sort;
  int64_t is_default;
  std::string label;
  bool operator==(std::string_view name) const { return this->label == name; }
  bool operator==(const std::string& name) const { return this->label == name; }
  bool operator<(const t_label_info& rhs) const { return sort < rhs.sort; }
};
// 大小可参考std::vector<ugreen::t_label_info>::~vector()
static_assert(sizeof(t_label_info) == 48);

// 用到它的结构体需确保8字节对齐
enum class SourceType : int32_t {
  kNone = 0,
  kNfo = 1,
  kDouban = 2,
  kTmdb = 3,
};

// 定义可参考ugreen::sqlite3_wraper_tv::insert_record()
struct t_tv_info {
  int64_t id;
  int64_t media_id;
  int64_t ugreen_no;
  int64_t tv_type;
  SourceType source;
  int64_t definition;
  int64_t ep_num;
  int64_t douban_ep_num;
  int64_t public_ep_num;  // 230804 新增
  int64_t play_ep;
  int64_t play_time;
  int64_t score;
  int64_t star_count;
  int64_t douban_id;
  int64_t date;
  int64_t favorite_time;
  int64_t remove_status;
  int64_t info_lock;
  int32_t match_mode;
  std::string uuid;
  std::string name;
  std::string douban_title;
  int64_t labels;
  int64_t play_ep_index;
  std::string cover_fanart;
  std::string cover_poster;
};
static_assert(sizeof(t_tv_info) == 208);

// 定义可参考ugreen::sqlite3_wraper_tv_file::insert_record()
struct t_tv_file_info {
  int64_t id;
  int64_t media_id;
  int64_t tv_id;
  int64_t tv_type;
  int64_t ep;
  int64_t ep_update_time;
  SourceType source;
  int64_t ugreen_no;
  int64_t path_id;
  int64_t size;
  int64_t duration;
  int64_t definition;
  int64_t score;
  int64_t star_count;
  int64_t douban_id;
  int64_t date;
  int64_t favorite_time;
  int64_t remove_status;
  int64_t play_time;
  int64_t play_progress;
  int64_t play_device;
  int64_t info_lock;
  std::string uuid;
  std::string parent_hash;
  std::string parent_path;
  std::string path_hash;
  std::string file_name;
  std::string name;
  std::string douban_title;
  int64_t labels;
  std::string cover_fanart;
  std::string cover_poster;
  std::string film_title;
};
static_assert(sizeof(t_tv_file_info) == 0x108);

// 定义可参考ugreen::sqlite3_wraper_film::insert_record()
struct t_film_info {
  int64_t id;
  int64_t media_id;
  int64_t ugreen_no;
  int64_t path_id;
  SourceType source;
  int64_t size;
  int64_t duration;
  int64_t definition;
  int64_t score;
  int64_t star_count;
  int64_t douban_id;
  int64_t date;
  int64_t favorite_time;
  int64_t remove_status;
  int64_t play_time;
  int64_t play_progress;
  int64_t play_device;
  int64_t info_lock;
  int32_t match_mode;
  int32_t not_coll;  // 230804 新增
  std::string uuid;
  std::string parent_hash;
  std::string parent_path;
  std::string path_hash;
  std::string file_name;
  std::string name;
  std::string douban_title;
  int64_t labels;
  std::string cover_fanart;
  std::string cover_poster;
};
static_assert(sizeof(t_film_info) == 0xe8);
static_assert(__builtin_offsetof(t_film_info, size) == 0x28);

namespace movies {
  struct media_refresh_local_data {
    int64_t unused1;
    int64_t ugreen_no;
    // ...
  };
}  // namespace movies
}  // namespace ugreen

// 定义可参考DouBanApi::GetRatingForMoviesDataParse()
struct DoubanMoviesInfo {
  int32_t rating;      // 分数
  int32_t star_count;  // 星星数
  int32_t ep_num;
  int32_t year;      // 上映年份(如1998， 没有月日)
  char subtype[32];  // 类型 movie tv show
  char cannot_show_reason[256];
  char intro[1024];      // 简介
  char title[1024];      // 影视名称
  char cover_url[1024];  // 海报url
  uint64_t id;           // 豆瓣ID
  std::vector<void*> directors;
  std::vector<void*> actors;
};
static_assert(sizeof(DoubanMoviesInfo) == 429 * 8);

// 定义可参考parseMatchJsonListData、updateMoviesInfo
struct _MatchListStr {
  uint64_t id;
  ugreen::SourceType source;
  ugreen::MediaTypeValues old_type;
  ugreen::MediaTypeValues new_type;
  std::string uuid;
  std::string cover_path;
  std::string date;
  DoubanMoviesInfo douban;
  bool chg_file_name;
  int ver;
};
static_assert(sizeof(_MatchListStr) == 0xDA0);