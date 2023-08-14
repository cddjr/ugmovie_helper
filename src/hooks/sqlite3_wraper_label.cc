/*
 * @Author: 景大虾(dengjingren@foxmail.com)
 * @Date: 2023-08-14
 * @Last Modified by: 景大虾(dengjingren@foxmail.com)
 * @Last Modified time: 2023-08-14
 */
#include <elog.h>

#include <map>

#include "hooks.h"
#include "scraper/label.h"
#include "tools/synchronized.h"

using namespace scraper::label;

int my_sqlite3_wraper_label_renameLabel(void* __this, long long ugreen_no,
                                        int label, std::string name) {
  int ret = sqlite3_wraper_label_renameLabel(__this, ugreen_no, label, name);
  if (!ret) {
    // 同步
    RefreshLabels(__this, ugreen_no);
  }
  return ret;
}

int my_sqlite3_wraper_label_deleteLabel(void* __this, long long ugreen_no,
                                        int label) {
  int ret = sqlite3_wraper_label_deleteLabel(__this, ugreen_no, label);
  if (!ret) {
    // 同步
    RefreshLabels(__this, ugreen_no);
  }
  return ret;
}

/// @brief 用户创建标签会在这里hook，我们内部用后缀v2的不会进来
/// @param __this
/// @param label
/// @return
int my_sqlite3_wraper_label_insert_record(void* __this,
                                          const ugreen::t_label_info& label) {
  int ret = sqlite3_wraper_label_insert_record(__this, label);
  if (!ret) {
    // 同步
    RefreshLabels(__this, label.ugreen_no);
  }
  return ret;
}