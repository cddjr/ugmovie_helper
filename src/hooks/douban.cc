/*
 * @Author: 景大虾(dengjingren@foxmail.com)
 * @Date: 2023-08-05
 * @Last Modified by: 景大虾(dengjingren@foxmail.com)
 * @Last Modified time: 2023-08-14
 */
#define LOG_TAG "Douban"
#include <elog.h>
#include <string.h>

#include "hooks.h"
#include "scraper/douban/api.h"

using namespace scraper;

#if defined(HIGH_RES_DOUBAN) || defined(ENHANCED_MANUAL_SEARCH)
static bool FetchDoubanHighResolutionURL(DoubanMoviesInfo &douban) {
#ifdef HIGH_RES_DOUBAN
  if (__glibc_unlikely(!douban.id)) {
    // 没有豆瓣信息
    return false;
  }
  if (__glibc_unlikely(douban.cover_url[0] &&
                       !strstr(douban.cover_url, "/s_ratio_poster/"))) {
    // 豆瓣给绿联大图了？ 我们放弃处理 🎉
    return false;
  }
  if (__glibc_unlikely(!douban.subtype[0])) {
    log_w("缺少subtype信息");
  }
  // 简单方案是把s替换为m或l，但这里用其它方案
  log_d("尝试获取豆瓣高清图 %d,%s", douban.id, douban.title);
  auto type = media::Type::kMovie;
  if (!strcmp(douban.subtype, "tv") || !strcmp(douban.subtype, "show")) {
    type = media::Type::kTv;
  }
  if (auto info = douban::api::Get(douban.id, type)) {
    if (!info->cover_url.empty()) {
      snprintf(douban.cover_url, sizeof(douban.cover_url), "%s",
               info->cover_url.c_str());
      return true;
    }
  }
  return false;
#else
  // 没有启用高清图功能，只缓存信息，不覆盖绿联的url
  auto type = media::Type::kMovie;
  if (!strcmp(douban.subtype, "tv") || !strcmp(douban.subtype, "show")) {
    type = media::Type::kTv;
  }
  return !!douban::api::Get(douban.id, type);
#endif  // HIGH_RES_DOUBAN
}
#endif  // defined(HIGH_RES_DOUBAN) || defined(ENHANCED_MANUAL_SEARCH)

/*
开启这个宏可实现精准匹配搜索出来的结果也是高清图且带完整日期
副作用是每一个无关结果都会去二次请求豆瓣，导致搜索偏慢

关闭它将只在用户最终确定了匹配项目后再去拉详情，这种效率高一些
*/
#ifdef ENHANCED_MANUAL_SEARCH

thread_local bool in_manual_matching = false;

/// @brief 解析豆瓣数据
/// @param __this
/// @param resp 豆瓣json响应体
/// @param result 解析结果
/// @return
int my_DouBanApi_GetRatingForMoviesDataParse(void *__this, const char *resp,
                                             DoubanMoviesInfo *result) {
  /*
  刮削时会模糊匹配出一堆视频 为了避免无关视频也去请求高清图
  我们需要多处理几个函数：
    match_douban_result_to_db
    match_result_to_db
    handleMoviesMatchList
  如果后期这几个变动太大，为了省事还是直接全部都请求一遍吧
  */
  int ret = DouBanApi_GetRatingForMoviesDataParse(__this, resp, result);
  if (!ret && in_manual_matching) {
    // 我们应仅当手动精准匹配的时候才去获取高清图
    FetchDoubanHighResolutionURL(*result);
  }
  return ret;
}

/// @brief 响应用户发起的精准匹配请求
/// @param req
/// @param __unused1
void my_handleMoviesMatchList(struct evhttp_request *req, void *__unused1) {
  in_manual_matching = true;
  handleMoviesMatchList(req, __unused1);
  in_manual_matching = false;
}

/// @brief 手动创建剧集的精准匹配
/// @param req
/// @param __unused1
void my_handleMoviesMatchEps(struct evhttp_request *req, void *__unused1) {
  in_manual_matching = true;
  handleMoviesMatchEps(req, __unused1);
  in_manual_matching = false;
}
#else
#ifdef HIGH_RES_DOUBAN
/// @brief 解析手动精准匹配的结果
/// @param __unused1
/// @param __unused2
/// @param result
/// @return
int my_parseMatchJsonListData(const char *__unused1, int __unused2,
                              _MatchListStr &result) {
  int ret = parseMatchJsonListData(__unused1, __unused2, result);
  if (__glibc_likely(!ret && result.cover_path.empty()  // 非本地上传封面
                     )) {
    if (!result.douban.subtype[0]) {
      // FIXME 绿联没设置subtype
      switch (result.new_type) {
        case ugreen::MediaType::kMovie:
          strcpy(result.douban.subtype, "movie");
          break;
        case ugreen::MediaType::kTv:
          strcpy(result.douban.subtype, "tv");
          break;
        case ugreen::MediaType::kShow:
          strcpy(result.douban.subtype, "show");
          break;
        default:
          break;
      }
    }
    FetchDoubanHighResolutionURL(result.douban);
  }
  return ret;
}
#endif  // HIGH_RES_DOUBAN
#endif  // ENHANCED_MANUAL_SEARCH

#ifdef HIGH_RES_DOUBAN
/// @brief 豆瓣刮削结果入库
/// @param uid
/// @param uuid
/// @param douban
/// @param path
/// @param record
/// @param type
void my_match_douban_result_to_db(long long ugreen_no, const std::string &uuid,
                                  const DoubanMoviesInfo &douban, void *path,
                                  ugreen::t_film_info &record, int type) {
  FetchDoubanHighResolutionURL(const_cast<DoubanMoviesInfo &>(douban));
  match_douban_result_to_db(ugreen_no, uuid, douban, path, record, type);
}

/// @brief NFO和豆瓣刮削结果入库
/// @param ugreen_no
/// @param uuid
/// @param douban
/// @param nfo
/// @param path
/// @param record
/// @param type
void my_match_douban_nfo_result_to_db(long long ugreen_no,
                                      const std::string &uuid,
                                      const DoubanMoviesInfo &douban, void *nfo,
                                      void *path, ugreen::t_film_info &record,
                                      int type) {
  // 防止nfo的海报不正常导致回退豆瓣小图
  FetchDoubanHighResolutionURL(const_cast<DoubanMoviesInfo &>(douban));
  match_douban_nfo_result_to_db(ugreen_no, uuid, douban, nfo, path, record,
                                type);
}
#endif  // HIGH_RES_DOUBAN