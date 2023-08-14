/*
 * @Author: 景大虾(dengjingren@foxmail.com)
 * @Date: 2023-08-05
 * @Last Modified by: 景大虾(dengjingren@foxmail.com)
 * @Last Modified time: 2023-08-14
 */

#pragma once
#include <memory>
#include <vector>

#include "ugreen/types.h"

namespace ugreen {
class sqlite3_wraper_label {
 public:
  static std::shared_ptr<sqlite3_wraper_label> get(int max_size = 2);

  sqlite3_wraper_label(std::shared_ptr<void> handle) : handle_(handle){};
  virtual ~sqlite3_wraper_label();

  int getCounts(int ugreen_no, int& count, int is_default = -1) const;
  int select_all_records(int ugreen_no,
                         std::vector<ugreen::t_label_info>& records) const;
  /// @brief 获得未被使用的标签id
  /// @param ugreen_no
  /// @param id 返回标签id
  /// @return
  int getValidLabelId(int ugreen_no, int64_t& id) const;
  /// @brief 新增标签
  /// @param record 标签id和名称不能重复
  /// @return
  int insert_record_v2(const ugreen::t_label_info& record);

  void* get_handle() const { return handle_.get(); }

 private:
  std::shared_ptr<void> handle_;
};
}  // namespace ugreen
