/*
 * @Author: 景大虾(dengjingren@foxmail.com)
 * @Date: 2023-08-10
 * @Last Modified by: 景大虾(dengjingren@foxmail.com)
 * @Last Modified time: 2023-08-14
 */
#include <elog.h>

#include "hooks.h"
#include "scraper/cache.h"
#include "scraper/label.h"
#include "ugreen/sqlite3_wraper_label.h"

thread_local int64_t current_ugreen_no = 0;

void my_refresh_movies_func(ugreen::movies::media_refresh_local_data& data) {
  log_d("正在刷新媒体库 ugreen_no:%d", data.ugreen_no);

  current_ugreen_no = data.ugreen_no;
  scraper::label::Init(data.ugreen_no);

  refresh_movies_func(data);

  current_ugreen_no = 0;

#ifdef DEBUG
  scraper::cache::Trim(10);
#else
  scraper::cache::Trim(0);
#endif  // DEBUG

  log_d("媒体库刷新结束 ugreen_no:%d", data.ugreen_no);
}