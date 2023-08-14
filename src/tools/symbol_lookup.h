/*
 * @Author: 景大虾(dengjingren@foxmail.com)
 * @Date: 2023-08-05
 * @Last Modified by: 景大虾(dengjingren@foxmail.com)
 * @Last Modified time: 2023-08-05
 */

#pragma once
#include <stdint.h>
#include <unistd.h>

class SymbolLookup {
 public:
  SymbolLookup() = default;
  SymbolLookup(pid_t pid) : SymbolLookup() { Init(pid); }
  virtual ~SymbolLookup() { Release(); }

 public:
  bool Init(pid_t pid);
  void Release();
  bool Good() const { return fd_ >= 0; };
  void* Get(const char* symbol) const;

 public:
  static uintptr_t GetModuleBase(pid_t pid = -1, const char* libname = {});

 protected:
  int fd_ = {-1};
  pid_t pid_ = {-1};
  uintptr_t base_ = {};
  uintptr_t dynsym_ = {};
  int dynsym_num_ = {};
  uintptr_t symtab_ = {};
  int symtab_num_ = {};
  const char* dynstr_ = {};
  const char* strtab_ = {};
  uintptr_t bias_ = {};
};