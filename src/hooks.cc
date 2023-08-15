/*
 * @Author: 景大虾(dengjingren@foxmail.com)
 * @Date: 2023-08-05
 * @Last Modified by: 景大虾(dengjingren@foxmail.com)
 * @Last Modified time: 2023-08-15
 */

#define LOG_TAG "Hooks"
#include "hooks.h"

#include <detours.h>
#include <elog.h>

#include "tools/symbol_lookup.h"

#define DO_APP_FUNC(r, n, p, s) r(*n) p = {}
#define DO_API(r, n, p, s) DO_APP_FUNC(r, n, p, s)
#include "functions.inc"
#undef DO_APP_FUNC
#undef DO_API

void InitFunctions() {
  log_d("begin");

  // 可直接拿到地址
  ugreen_evbuffer_pullup = evbuffer_pullup;
  ugreen_curl_easy_init = curl_easy_init;

  SymbolLookup symbol{-1};
  if (!symbol.Good()) {
    log_e("SymbolLookup");
    return;
  }
#define DO_APP_FUNC(r, n, p, s)                             \
  n = n ? n : reinterpret_cast<decltype(n)>(symbol.Get(s)); \
  if (!n)                                                   \
    log_w("Unable to locate %s", s);                        \
  else                                                      \
    log_v("%s=>%p", s, n);
#define DO_API(r, n, p, s) DO_APP_FUNC(r, n, p, s)
#include "functions.inc"
#undef DO_APP_FUNC
#undef DO_API

  log_d("end");
}

static bool HookFunction(void *pPointer, void *pDetour, const char *name) {
  HOOK_TRACE_INFO handle{};
  if (DetourInstallHook(pPointer, pDetour, {}, &handle) ||
      DetourSetExclusiveACL({}, 0, &handle)) {
    log_e("Failed to hook %s", name);
    return false;
  } else {
    log_v("%s -> %p", name, pDetour);
  }
  return true;
}

#define HOOKFUNC(n) \
  if (!HookFunction((void *) n, (void *) my_##n, #n)) exit(1);

void DetourInitilization() {
  log_d("begin");

#ifdef ENHANCED_MANUAL_SEARCH
  HOOKFUNC(handleMoviesMatchList);
  HOOKFUNC(handleMoviesMatchEps);
  HOOKFUNC(HttpResponseData);
  HOOKFUNC(DouBanApi_GetRatingForMoviesDataParse);
#else
#ifdef HIGH_RES_DOUBAN
  HOOKFUNC(parseMatchJsonListData);
  HOOKFUNC(handleMoviesEpsCreate);
  HOOKFUNC(ugreen_evbuffer_pullup);
#endif  // HIGH_RES_DOUBAN
#endif  // ENHANCED_MANUAL_SEARCH
#ifdef HIGH_RES_DOUBAN
  HOOKFUNC(match_douban_result_to_db);
  HOOKFUNC(match_douban_nfo_result_to_db);
#ifdef USE_DOUBAN_WECHAT_API
  // 小程序接口拿到的高清图需要我们伪造Referer
  // 否则会被限速10kb
  HOOKFUNC(movies_svr_common_download_file);
  HOOKFUNC(ugreen_curl_easy_init);
#endif  // USE_DOUBAN_WECHAT_API
#endif  // HIGH_RES_DOUBAN
  HOOKFUNC(refresh_update_film_match_info);
  HOOKFUNC(sqlite3_wraper_film_insert_record_2);
  HOOKFUNC(sqlite3_wraper_tv_insert_record);
  HOOKFUNC(sqlite3_wraper_tv_update_match_info_by_id);
  HOOKFUNC(sqlite3_wraper_tv_update_douban_info_record);
  HOOKFUNC(update_film_match_info);
  HOOKFUNC(TmdbApi_GetMovieDetailDataParse);
  HOOKFUNC(TmdbApi_GetTvDetailDataParse);
  // 响应刷新媒体库
  HOOKFUNC(refresh_movies_func);
  HOOKFUNC(sqlite3_wraper_label_deleteLabel);
  HOOKFUNC(sqlite3_wraper_label_insert_record);

  log_d("end");
}
