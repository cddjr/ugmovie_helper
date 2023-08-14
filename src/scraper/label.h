/*
 * @Author: 景大虾(dengjingren@foxmail.com)
 * @Date: 2023-08-11
 * @Last Modified by: 景大虾(dengjingren@foxmail.com)
 * @Last Modified time: 2023-08-14
 */
#include <string>

namespace scraper::label {
bool Init(int64_t uid, bool force = true);
bool RefreshLabels(void* label_db, int64_t uid);
int64_t GetLabelId(int64_t uid, std::string_view tag);
}  // namespace scraper::label