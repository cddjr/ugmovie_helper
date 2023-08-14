/*
 * @Author: 景大虾(dengjingren@foxmail.com)
 * @Date: 2023-08-05
 * @Last Modified by: 景大虾(dengjingren@foxmail.com)
 * @Last Modified time: 2023-08-14
 * @Description: sqlite3_wraper_film 和 sqlite3_wraper_tv 相关的hook实现
 */
#define LOG_TAG "MovieDB"
#include <elog.h>

#include "hooks.h"
#include "scraper/cache.h"
#include "scraper/douban/api.h"
#include "scraper/label.h"

using namespace std::literals;
using namespace scraper;

extern thread_local int64_t current_ugreen_no;

class TableHelper {
 public:
  TableHelper(void *db, void *t_xx_info, media::Type hint) {
    db_ = db;
    t_info_ = t_xx_info;
    type_ = hint;
  }
  ~TableHelper() {}

#define SET_XXX_IMPL(type, name) \
  void set_##name(type value) {  \
    switch (type_) {             \
      case media::Type::kMovie:  \
        get_film().name = value; \
      case media::Type::kTv:     \
        get_tv().name = value;   \
    }                            \
  }

#define GET_XXX_IMPL(type, name) \
  type get_##name() const {      \
    switch (type_) {             \
      case media::Type::kMovie:  \
        return get_film().name;  \
      case media::Type::kTv:     \
        return get_tv().name;    \
      default:                   \
        return {};               \
    }                            \
  }

#define GET_SET_IMPLS(type, name) \
  GET_XXX_IMPL(type, name)        \
  SET_XXX_IMPL(type, name)

  GET_SET_IMPLS(ugreen::SourceType, source)
  GET_SET_IMPLS(int, douban_id)
  GET_SET_IMPLS(int, date)
  GET_SET_IMPLS(std::string_view, name)

  GET_XXX_IMPL(int64_t, id)
  // GET_XXX_IMPL(int64_t, ugreen_no)
  GET_XXX_IMPL(int64_t, labels)

  media::Type get_type() const { return type_; }

  int64_t get_ugreen_no() const {
    int64_t uid;
    switch (type_) {
      case media::Type::kMovie:
        uid = get_film().ugreen_no;
        break;
      case media::Type::kTv:
        uid = get_tv().ugreen_no;
        break;
      default:
        return current_ugreen_no;
    }
    if (uid) return uid;
    uid = current_ugreen_no;
    if (!uid) {
      // 当前不在刮削线程内
      switch (type_) {
        case media::Type::kMovie: {
          std::vector<ugreen::t_film_info> results;
          sqlite3_wraper_film_select_record_by_id(db_, get_film().id, results);
          if (!results.empty()) {
            uid = results[0].ugreen_no;
            // get_film().ugreen_no = uid;
          }
          break;
        }
        case media::Type::kTv: {
          std::vector<ugreen::t_tv_info> results;
          sqlite3_wraper_tv_select_record_by_id(db_, get_tv().id, results);
          if (!results.empty()) {
            uid = results[0].ugreen_no;
            // get_tv().ugreen_no = uid;
          }
          break;
        }
        default:
          return uid;
      }
    }
    return uid;
  }

  // protected:
  ugreen::t_film_info &get_film() const {
    return *reinterpret_cast<ugreen::t_film_info *>(t_info_);
  }
  ugreen::t_tv_info &get_tv() const {
    return *reinterpret_cast<ugreen::t_tv_info *>(t_info_);
  }

 private:
  void *db_;
  void *t_info_;
  media::Type type_;
};

static void Handle(TableHelper &&table, int &labels) {
  media::InfoPtr info;
  if (table.get_source() == ugreen::SourceType::kDouban) {
    if (true /*强制更新标签*/ || table.get_date() % 10000 == 0) {
      // 访问豆瓣获得更详细的信息
      info = douban::api::Get(table.get_douban_id(), table.get_type());
      if (info) {
        // 完善豆瓣刮削的日期
        table.set_date(info->release_date);
      }
    }
  } else if (table.get_source() == ugreen::SourceType::kTmdb) {
    info =
        cache::Get(table.get_source(), table.get_type(), table.get_douban_id());
  } else {
    // TODO 比如nfo 暂不处理
    return;
  }
  if (!info) {
    log_w("无法获得信息。来源:%d id:%d name:%s labels:%x", table.get_source(),
          table.get_douban_id(), table.get_name().data(), table.get_labels());
    return;
  }
  // FIXME 电影的刮削流程取不到uid和label
  if (table.get_type() == media::Type::kMovie)
    labels = ugreen::MediaType::kMovie;
  // table.get_labels() & (~ugreen::MediaType::kUnrecognized);
  auto uid = table.get_ugreen_no();
  label::Init(uid, false);
  for (const auto &genre : info->genres) {
    labels |= label::GetLabelId(uid, genre);
  }
  if (!info->regions.empty()) {
    // 地区只取第一个
    labels |= label::GetLabelId(uid, info->regions[0]);
  }
#if LOG_LVL >= ELOG_LVL_VERBOSE
  std::string genres;
  for (const auto &genre : info->regions) {
    genres.append(genre + ","s);
  }
  for (const auto &genre : info->genres) {
    genres.append(genre + ","s);
  }
  log_v("视频:%d,%s 标签:%s(%x)", info->id, info->name.c_str(), genres.c_str(),
        labels);
#endif
}

/// @brief 精准匹配电影更新
/// @param __this
/// @param record
/// @return
int my_update_film_match_info(void *__this, ugreen::t_film_info &record) {
  // 目前绿联的精准匹配只有豆瓣源
  int labels = 0;
  Handle(TableHelper{__this, &record, media::Type::kMovie}, labels);
  int ret = update_film_match_info(__this, record);
  if (!ret && labels) {
    labels |= ugreen::MediaType::kMovie;
    sqlite3_wraper_film_update_label_by_id(__this, record.id, labels);
  }
  return ret;
}

/// @brief 精准匹配剧集拆分为电影
/// @param __this
/// @param record
/// @return
int my_sqlite3_wraper_film_insert_record_2(void *__this,
                                           ugreen::t_film_info &record,
                                           long long &record_id) {
  int labels = 0;
  Handle(TableHelper{__this, &record, media::Type::kMovie}, labels);
  int ret = sqlite3_wraper_film_insert_record_2(__this, record, record_id);
  if (!ret && labels) {
    labels |= ugreen::MediaType::kMovie;
    sqlite3_wraper_film_update_label_by_id(__this, record_id, labels);
  }
  return ret;
}

/// @brief 精准匹配剧集入库
/// @param __this
/// @param record
/// @return
int my_sqlite3_wraper_tv_update_match_info_by_id(void *__this,
                                                 ugreen::t_tv_info &record) {
  int labels = 0;
  Handle(TableHelper{__this, &record, media::Type::kTv}, labels);
  int ret = sqlite3_wraper_tv_update_match_info_by_id(__this, record);
  if (!ret && labels) {
    sqlite3_wraper_tv_update_label_by_id(__this, record.id, labels);
  }
  return ret;
}

/// @brief 精准匹配剧集更新
/// @param __this
/// @param record
/// @return
int my_sqlite3_wraper_tv_update_douban_info_record(void *__this,
                                                   ugreen::t_tv_info &record) {
  int labels = 0;
  Handle(TableHelper{__this, &record, media::Type::kTv}, labels);
  int ret = sqlite3_wraper_tv_update_douban_info_record(__this, record);
  if (!ret && labels) {
    sqlite3_wraper_tv_update_label_by_id(__this, record.id, labels);
  }
  return ret;
}

/// @brief 电影刮削入库
/// @param __this
/// @param record
/// @return
int my_refresh_update_film_match_info(void *__this,
                                      ugreen::t_film_info &record) {
  int labels = 0;
  Handle(TableHelper{__this, &record, media::Type::kMovie}, labels);
  int ret = refresh_update_film_match_info(__this, record);
  if (!ret && labels) {
    labels |= ugreen::MediaType::kMovie;
    sqlite3_wraper_film_update_label_by_id(__this, record.id, labels);
  }
  return ret;
}

/// @brief 剧集刮削入库
/// @param __this
/// @param record
/// @param __unused1
/// @param __unused2
/// @return
int my_sqlite3_wraper_tv_insert_record(void *__this, ugreen::t_tv_info &record,
                                       long long &record_id,
                                       long long &__unused2) {
  int labels = 0;
  Handle(TableHelper{__this, &record, media::Type::kTv}, labels);
  int ret =
      sqlite3_wraper_tv_insert_record(__this, record, record_id, __unused2);
  if (!ret && labels) {
    sqlite3_wraper_tv_update_label_by_id(__this, record_id, labels);
  }
  return ret;
}