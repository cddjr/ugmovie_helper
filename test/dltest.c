/*
 * @Author: 景大虾(dengjingren@foxmail.com)
 * @Date: 2023-08-06
 * @Last Modified by: 景大虾(dengjingren@foxmail.com)
 * @Last Modified time: 2023-08-06
 */
#include <dlfcn.h>
#include <stdio.h>

int main() {
  void *dlh;
  dlh = dlopen("./libugreen_tools.so", RTLD_NOW | RTLD_GLOBAL);
  if (dlh == NULL) {
    fprintf(stderr, "dlopen err:%s.\n", dlerror());
  } else {
    fprintf(stdout, "dlopen succ:%p.\n", dlh);
  }
  return 0;
}