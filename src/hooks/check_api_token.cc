/*
 * @Author: 景大虾(dengjingren@foxmail.com)
 * @Date: 2023-08-05
 * @Last Modified by: 景大虾(dengjingren@foxmail.com)
 * @Last Modified time: 2023-08-05
 */

#include <elog.h>

#include "hooks.h"

int my_check_api_token(void *req, NASUserNode *user) {
  int ret = check_api_token(req, user);
  if (ret == 0) {
    // auto instance = SystemSettingGetInstance();
    // auto temp = SystemSettingGetCpuTemperature(instance);
    // log_v("CpuTemperature=%d", temp);
  }
  return ret;
}