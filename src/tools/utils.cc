/*
 * @Author: 景大虾(dengjingren@foxmail.com)
 * @Date: 2023-08-08
 * @Last Modified by: 景大虾(dengjingren@foxmail.com)
 * @Last Modified time: 2023-08-14
 */
#include "utils.h"

using namespace std::literals;

namespace utils {
namespace string {

  std::string_view Trim(std::string_view str) {
    if (str.empty()) return str;
    constexpr auto blanks = " \r\n\t\v\f"sv;
    auto s = str.find_first_not_of(blanks);
    if (s == str.npos) {
      return {};
    }
    auto e = str.find_last_not_of(blanks);
    return str.substr(s, e - s + 1);
  }

  std::vector<std::string_view> Tokenize(std::string_view str,
                                         std::string_view delimiters) {
    std::vector<std::string_view> tokens;
    // Skip delimiters at beginning.
    auto lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    auto pos = str.find_first_of(delimiters, lastPos);
    while (pos != str.npos || lastPos != str.npos) {
      // Found a token, add it to the vector.
      tokens.emplace_back(str.substr(lastPos, pos - lastPos));
      // Skip delimiters.  Note the "not_of"
      lastPos = str.find_first_not_of(delimiters, pos);
      // Find next "non-delimiter"
      pos = str.find_first_of(delimiters, lastPos);
    }
    return tokens;
  }
}  // namespace string

namespace date {
  std::string YmdToStr(unsigned yyyyMMdd) {
    return YmdToStr(yyyyMMdd / 10000, (yyyyMMdd / 100) % 100, yyyyMMdd % 100);
  }

  std::string YmdToStr(unsigned year, unsigned month, unsigned day) {
    char date[32]{};
    if (year) {
      if (month)
        if (day)
          snprintf(date, sizeof(date), "%d-%d-%d", year, month, day);
        else
          snprintf(date, sizeof(date), "%d-%d", year, month);
      else
        snprintf(date, sizeof(date), "%d", year);
    }
    return date;
  }

  unsigned StrToYmd(std::string_view date_str) {
    unsigned year = 0, month = 0, day = 0;
    sscanf(date_str.data(), "%04d-%02d-%02d", &year, &month, &day);
    return year * 10000 + month * 100 + day;
  }
}  // namespace date

}  // namespace utils
