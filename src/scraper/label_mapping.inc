/*
 * @Author: 景大虾(dengjingren@foxmail.com)
 * @Date: 2023-08-10
 * @Last Modified by: 景大虾(dengjingren@foxmail.com)
 * @Last Modified time: 2023-08-15
 */

/*
TODO 目前暂时只考虑最简单的对应关系
即 展示标签=标签1 或 标签2 或 标签n
则 只要是以上任意标签均归类为展示标签

因为绿联没有地区标签，如果把地区和分类都作为标签自动添加
就会导致界面上显示得太长太多了
为此只能暂时用白名单的方式，即没有配置的标签都会被忽略（除非这个标签已手动添加进影院）
*/
#include <fifo_map.hpp>
#include <set>
#include <string>

/*
豆瓣
   "喜剧", "爱情", "动作", "科幻", "动画",   "悬疑", "犯罪", "惊悚",
    "冒险", "音乐", "历史", "奇幻", "恐怖",   "战争", "传记", "歌舞",
    "武侠", "情色", "灾难", "西部", "纪录片", "短片", "家庭", "真人秀",
    "儿童", "脱口秀", "古装"

TMDB
     {28, "动作"},        {12, "冒险"},        {16, "动画"},
    {35, "喜剧"},        {80, "犯罪"},        {99, "纪录"},
    {18, "剧情"},        {10751, "家庭"},     {14, "奇幻"},
    {36, "历史"},        {27, "恐怖"},        {10402, "音乐"},
    {9648, "悬疑"},      {10749, "爱情"},     {878, "科幻"},
    {10770, "电视电影"}, {53, "惊悚"},        {10752, "战争"},
    {37, "西部"},        {10759, "动作冒险"}, {10762, "儿童"},
    {10763, "新闻"},     {10764, "真人秀"},   {10765, "Sci-Fi & Fantasy"},
    {10766, "肥皂剧"},   {10767, "脱口秀"},   {10768, "War & Politics"},
*/

inline const nlohmann::fifo_map<std::string_view, std::set<std::string_view>>
    label_mapping{
        {"华语", {"中国大陆", "中国香港", "中国台湾"}},
        {"欧美", {"美国", "英国", "法国", "德国", "意大利"}},
        {"日韩", {"日本", "韩国"}},
        {"综艺", {"真人秀", "脱口秀"}},
        {"喜剧", {}},
        {"爱情", {}},
        {"动作", {}},
        {"科幻", {"Sci-Fi & Fantasy"}},
        {"儿童", {}},
        {"动画", {}},
        {"悬疑", {}},
        {"犯罪", {}},
        {"恐怖", {}},
        {"冒险", {"动作冒险"}},
        {"历史", {}},
        {"奇幻", {"Sci-Fi & Fantasy"}},
        {"战争", {"War & Politics"}},
        {"灾难", {"这是豆瓣特有的"}},
        {"纪录片", {"纪录"}},
        {"音乐", {}},
        {"西部", {}},
        {"短片", {}},
        {"家庭", {}},
        {"传记", {"这是豆瓣特有的"}},
        {"歌舞", {"这是豆瓣特有的"}},
        {"武侠", {"这是豆瓣特有的"}},
        {"古装", {"这是豆瓣特有的"}},
    };