/*
 * @Author: https://blog.csdn.net/witton/article/details/110873292
 * @Date: 2020-12-08
 * @Last Modified by: 景大虾(dengjingren@foxmail.com)
 * @Last Modified time: 2023-08-11
 * @Description: 实现枚举类（enum class）与整形的直接比较
 */
#pragma once

template <typename A, typename B,
          std::enable_if_t<std::is_integral<A>::value ||
                           std::is_enum<A>::value>* = nullptr,
          std::enable_if_t<std::is_enum<B>::value ||
                           std::is_integral<B>::value>* = nullptr>
bool operator==(A a, B b) {
  return a == (A) b;
}

template <typename A, typename B,
          std::enable_if_t<std::is_integral<A>::value ||
                           std::is_enum<A>::value>* = nullptr,
          std::enable_if_t<std::is_enum<B>::value ||
                           std::is_integral<B>::value>* = nullptr>
bool operator>(A a, B b) {
  return a > (A) b;
}

template <typename A, typename B,
          std::enable_if_t<std::is_integral<A>::value ||
                           std::is_enum<A>::value>* = nullptr,
          std::enable_if_t<std::is_enum<B>::value ||
                           std::is_integral<B>::value>* = nullptr>
bool operator>=(A a, B b) {
  return a >= (A) b;
}

template <typename A, typename B,
          std::enable_if_t<std::is_integral<A>::value ||
                           std::is_enum<A>::value>* = nullptr,
          std::enable_if_t<std::is_enum<B>::value ||
                           std::is_integral<B>::value>* = nullptr>
bool operator<(A a, B b) {
  return a < (A) b;
}

template <typename A, typename B,
          std::enable_if_t<std::is_integral<A>::value ||
                           std::is_enum<A>::value>* = nullptr,
          std::enable_if_t<std::is_enum<B>::value ||
                           std::is_integral<B>::value>* = nullptr>
bool operator<=(A a, B b) {
  return a <= (A) b;
}

template <typename A, typename B,
          std::enable_if_t<std::is_integral<A>::value ||
                           std::is_enum<A>::value>* = nullptr,
          std::enable_if_t<std::is_enum<B>::value ||
                           std::is_integral<B>::value>* = nullptr>
bool operator!=(A a, B b) {
  return a != (A) b;
}

template <typename A, typename B,
          std::enable_if_t<std::is_integral<A>::value ||
                           std::is_enum<A>::value>* = nullptr,
          std::enable_if_t<std::is_enum<B>::value ||
                           std::is_integral<B>::value>* = nullptr>
bool operator|=(A a, B b) {
  return a |= (A) b;
}