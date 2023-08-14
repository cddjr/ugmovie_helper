/*
 * @Author: 景大虾(dengjingren@foxmail.com)
 * @Date: 2023-08-11
 * @Last Modified by: 景大虾(dengjingren@foxmail.com)
 * @Last Modified time: 2023-08-14
 */
#define LOG_TAG "Label"
#include "label.h"

#include <elog.h>

#include <map>

#include "hooks.h"
#include "label_mapping.inc"
#include "tools/synchronized.h"
#include "ugreen/sqlite3_wraper_label.h"

static std::shared_mutex s_mutex;

using LabelIdMapping = std::map<std::string, int64_t, std::less<void>>;
static std::map<int64_t, LabelIdMapping> s_label_name_mapping;

bool scraper::label::RefreshLabels(void* label_db, int64_t uid) {
  // 同步影院已有标签
  std::vector<ugreen::t_label_info> labels;
  int ret = sqlite3_wraper_label_select_all_records(label_db, uid, &labels);
  if (ret != 0) {
    return false;
  }
  synchronized_write(s_mutex) {
    auto& mapping = s_label_name_mapping[uid];
    mapping.clear();
    for (auto& curr_label : labels) {
      // 复用标签id
      mapping[curr_label.label] = curr_label.labelId;
      log_v("复用标签 %s,%x", curr_label.label.c_str(), curr_label.labelId);
    }
  }
  return true;
}

bool scraper::label::Init(int64_t uid, bool force) {
  if (!force) {
    synchronized_read(s_mutex) {
      if (s_label_name_mapping.find(uid) != s_label_name_mapping.end()) {
        return true;
      }
    }
  }
  auto db = ugreen::sqlite3_wraper_label::get();
  if (!db) return false;
  return RefreshLabels(db->get_handle(), uid);
}

static int64_t GetFromCache(int64_t uid, std::string_view name) {
  synchronized_read(s_mutex) {
    const auto iter = s_label_name_mapping.find(uid);
    if (__glibc_likely(iter != s_label_name_mapping.end())) {
      auto iter2 = iter->second.find(name);
      if (__glibc_likely(iter2 != iter->second.end())) {
        return iter2->second;
      }
    }
    return 0;
  }
}

/// @brief 创建影院标签 如果重复则返回已有标签id
/// @param uid
/// @param name
/// @return 标签id 非零代表成功
static int64_t GetOrCreate(int64_t uid, std::string_view name) {
  int64_t label_id = GetFromCache(uid, name);
  if (label_id) return label_id;
  // FIXME 这里修改标签数据库暂时不会与影视库的更新产生死锁
  auto db = ugreen::sqlite3_wraper_label::get();
  if (!db) return 0;
  // 首先获得影院已有标签
  std::vector<ugreen::t_label_info> labels;
  if (db->select_all_records(uid, labels)) {
    log_e("无法查询影院标签");
    return 0;
  }
  db->getValidLabelId(uid, label_id);
  if (__glibc_unlikely(!label_id)) {
    // 满了
    log_e("标签已满无法新增");
    return 0;
  }
  ugreen::t_label_info info{};
  info.ugreen_no = uid;
  info.labelId = label_id;
  info.label = name;
  if (db->insert_record_v2(info)) {
    log_e("插入标签失败:%s", name.data());
    return 0;
  }
  synchronized_write(s_mutex) {
    s_label_name_mapping[uid][std::string{name}] = label_id;
  }
  log_d("创建标签 %s,%x", name.data(), label_id);
  return label_id;
}

/// @brief 获得标签属于哪些分类
/// @param name
/// @return true说明标签自身就是分类
static std::pair<bool, std::vector<std::string_view>> GetCategoryNames(
    std::string_view name) {
  std::vector<std::string_view> ret;
  bool is_cat = label_mapping.find(name) != label_mapping.end();
  for (const auto& pair : label_mapping) {
    if (pair.second.find(name) != pair.second.end()) {
      ret.emplace_back(pair.first);
    }
  }
  return std::make_pair(is_cat, std::move(ret));
}

int64_t scraper::label::GetLabelId(int64_t uid, std::string_view name) {
  if (!uid) {
    log_e("uid为零");
    return 0;
  }
  int64_t label_id = 0;
  const auto& result = GetCategoryNames(name);
  if (!result.first && result.second.empty()) {
    // 这个标签没有配置 不主动创建
    label_id = GetFromCache(uid, name);
    if (label_id) {
      log_d("没有配置 但已分配 %s,%x", name.data(), label_id);
    } else {
      log_v("没有配置 %s,%x", name.data(), label_id);
    }
  } else {
    if (result.first) {
      // 标签自身就是分类
      label_id |= GetOrCreate(uid, name);
      log_v("独立的分类 %s,%x", name.data(), label_id);
    }
    // 标签对应多个分类（一对多） 创建所有分类
    for (const auto& cat : result.second) {
      label_id |= GetOrCreate(uid, cat);
      log_v("对应子分类 %s->%s,%x", name.data(), cat.data(), label_id);
    }
  }
  return label_id;
}
