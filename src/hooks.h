/*
 * @Author: 景大虾(dengjingren@foxmail.com)
 * @Date: 2023-08-05
 * @Last Modified by: 景大虾(dengjingren@foxmail.com)
 * @Last Modified time: 2023-08-14
 */

#pragma once
#include "ugreen/types.h"

void InitFunctions();
void DetourInitilization();

#define DO_APP_FUNC(r, n, p, s) extern r(*n) p
#define DO_API(r, n, p, s) DO_APP_FUNC(r, n, p, s)
#include "functions.inc"
#undef DO_APP_FUNC
#undef DO_API

#define DO_APP_FUNC(r, n, p, s) r my_##n p
#define DO_API(r, n, p, s)
#include "functions.inc"
#undef DO_APP_FUNC
#undef DO_API
