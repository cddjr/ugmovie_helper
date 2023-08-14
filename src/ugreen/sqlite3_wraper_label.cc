/*
 * @Author: 景大虾(dengjingren@foxmail.com)
 * @Date: 2023-08-05
 * @Last Modified by: 景大虾(dengjingren@foxmail.com)
 * @Last Modified time: 2023-08-11
 */
#define LOG_TAG "Sqlite3Label"
#include "sqlite3_wraper_label.h"

#include <elog.h>
#include <string.h>

#include "hooks.h"

std::shared_ptr<ugreen::sqlite3_wraper_label> ugreen::sqlite3_wraper_label::get(
    int max_size) {
  std::shared_ptr<void> handle;
  sqlite3_wraper_label_get((void**) &handle, max_size);
  if (!handle) {
    return {};
  }
  return std::make_shared<ugreen::sqlite3_wraper_label>(handle);
}

int ugreen::sqlite3_wraper_label::getCounts(int ugreen_no, int& count,
                                            int is_default) const {
  return sqlite3_wraper_label_getCounts(handle_.get(), ugreen_no, count,
                                        is_default);
}

int ugreen::sqlite3_wraper_label::select_all_records(
    int ugreen_no, std::vector<ugreen::t_label_info>& records) const {
  return sqlite3_wraper_label_select_all_records(
      handle_.get(), ugreen_no,
      // FIXME 只有STL与绿联的一致才能这样操作
      &records);
}

int ugreen::sqlite3_wraper_label::getValidLabelId(int ugreen_no,
                                                  int64_t& id) const {
  id = {};
  return sqlite3_wraper_label_getValidLabelId(handle_.get(), ugreen_no,
                                              (int&) id);
}

int ugreen::sqlite3_wraper_label::insert_record_v2(
    const ugreen::t_label_info& record) {
  return sqlite3_wraper_label_insert_record_v2(handle_.get(), record);
}

ugreen::sqlite3_wraper_label::~sqlite3_wraper_label() {
  if (handle_.unique()) {
    // FIXME 特例 由我们来释放db 会不会有问题？
    log_i("release handle_(%p)...", handle_.get());
  }
}
