/*
 * @Author: 景大虾(dengjingren@foxmail.com)
 * @Date: 2023-08-10
 * @Last Modified by: 景大虾(dengjingren@foxmail.com)
 * @Last Modified time: 2023-08-13
 */
#include "region.h"

#include <unordered_map>

// 豆瓣上的地区列表及对应的ISO 3166-1代码
static const std::unordered_map<std::string_view, std::string_view> s_regions{
    {"韩国", "KR"},     {"日本", "JP"},     {"中国大陆", "CN"},
    {"美国", "US"},     {"中国香港", "HK"}, {"中国台湾", "TW"},
    {"英国", "GB"},     {"法国", "FR"},     {"德国", "DE"},
    {"意大利", "IT"},   {"西班牙", "ES"},   {"印度", "IN"},
    {"泰国", "TH"},     {"俄罗斯", "RU"},   {"加拿大", "CA"},
    {"澳大利亚", "AU"}, {"爱尔兰", "IE"},   {"瑞典", "SE"},
    {"巴西", "BR"},     {"丹麦", "DK"},     {"新西兰", "NZ"},
    {"伊朗", "IR"},     {"黎巴嫩", "LB"},   {"阿根廷", "AR"},
    {"挪威", "NO"},     {"奥地利", "AT"},   {"荷兰", "NL"},
    {"比利时", "BE"},
};

bool scraper::region::IsRegion(std::string_view s) {
  return s_regions.find(s) != s_regions.end();
}

std::string_view scraper::region::GetChinese(std::string_view iso_3166_1) {
  for (auto &iter : s_regions) {
    if (iter.second == iso_3166_1) {
      return iter.first;
    }
  }
  return {};
}
