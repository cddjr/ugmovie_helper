/*
 * @Author: 景大虾(dengjingren@foxmail.com)
 * @Date: 2023-08-08
 * @Last Modified by: 景大虾(dengjingren@foxmail.com)
 * @Last Modified time: 2023-08-14
 */
#pragma once

#include <string>
#include <vector>

namespace utils {
inline namespace string {

  /// @brief 移除字符串两端空白
  /// @param str
  /// @return
  std::string_view Trim(std::string_view str);

  /// @brief 分割字符串
  /// @param str
  /// @param delimiters 分割符号
  /// @return 分割得到的数组
  std::vector<std::string_view> Tokenize(std::string_view str,
                                         std::string_view delimiters);
}  // namespace string
inline namespace date {
  /// @brief yyyyMMdd转成字符串日期
  /// @param yyyyMMdd
  /// @return 返回如 "2023-01-01"
  std::string YmdToStr(unsigned yyyyMMdd);
  std::string YmdToStr(unsigned year, unsigned month, unsigned day);

  /// @brief 日期转 yyyyMMdd
  /// @param date_str 如 "2023-01-01"
  /// @return
  unsigned StrToYmd(std::string_view date_str);
}  // namespace date
}  // namespace utils
