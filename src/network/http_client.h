/*
 * @Author: 景大虾(dengjingren@foxmail.com)
 * @Date: 2023-08-08
 * @Last Modified by: 景大虾(dengjingren@foxmail.com)
 * @Last Modified time: 2023-08-14
 */
#pragma once
#include <stack>
#include <string>
#include <vector>

#include "tools/synchronized.h"

class HttpClient {
 public:
  using StatusCode = long;

  static constexpr StatusCode kOk = 200;

  virtual ~HttpClient() noexcept;

  void set_proxy(std::string_view proxy) { proxy_ = proxy; }

  StatusCode Get(
      std::string_view url, std::string* response,
      const std::vector<std::pair<std::string, std::string>>& header = {});

 protected:
  void* GetHandle();
  void ReleaseHandle(void* handle);

 private:
  std::mutex mutex_;
  std::stack<void*> handles_;
  std::string proxy_;
};