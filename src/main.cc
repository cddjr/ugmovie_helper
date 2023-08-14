/*
 * @Author: 景大虾(dengjingren@foxmail.com)
 * @Date: 2023-08-05
 * @Last Modified by: 景大虾(dengjingren@foxmail.com)
 * @Last Modified time: 2023-08-11
 */
#include <elog.h>  // log,assert...

#include "hooks.h"

extern "C" const char *__asan_default_options() {
  return "log_path=/tmp/ugtools.asan:debug=1:detect_stack_use_after_return=1";
}

#ifdef ELOG_OUTPUT_ENABLE
static void InitLog() {
  elog_init();
#ifndef DEBUG
  elog_set_filter_lvl(ELOG_LVL_INFO);
#endif
  elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_ALL & ~(ELOG_FMT_DIR));
  elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_ALL & ~(ELOG_FMT_DIR));
  elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_ALL & ~(ELOG_FMT_DIR));
  elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_ALL & ~(ELOG_FMT_DIR));
  elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_ALL & ~(ELOG_FMT_DIR));
  elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_ALL & ~(ELOG_FMT_DIR));
  elog_start();
}
#endif

__attribute__((constructor(65535))) void Entry() {
#ifdef ELOG_OUTPUT_ENABLE
  InitLog();
#endif
  InitFunctions();
  DetourInitilization();
}