/*
 * @Author: 景大虾(dengjingren@foxmail.com)
 * @Date: 2023-08-05
 * @Last Modified by: 景大虾(dengjingren@foxmail.com)
 * @Last Modified time: 2023-08-09
 */

#define LOG_TAG "SymbolLookup"
#include "symbol_lookup.h"

#include <elog.h>
#include <errno.h>
#include <fcntl.h>
#include <link.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

bool SymbolLookup::Init(pid_t pid) {
  do {
    Release();

    char name[32]{};
    if (pid < 0)
      snprintf(name, sizeof(name), "/proc/self/exe");
    else
      snprintf(name, sizeof(name), "/proc/%d/exe", pid);

    char path[1024]{};
    if (readlink(name, path, sizeof(path) - 1) <= 0) break;

    fd_ = TEMP_FAILURE_RETRY(open(path, O_RDONLY));
    if (fd_ < 0) break;

    pid_ = pid;
    base_ = GetModuleBase(pid);
    if (!base_) break;

    size_t size = lseek(fd_, 0, SEEK_END);
    if (size <= 0) break;
    auto elf =
        (const ElfW(Ehdr) *) mmap(0, size, PROT_READ, MAP_SHARED, fd_, 0);
    if (elf == MAP_FAILED) break;
    auto shoff = (uintptr_t) elf + elf->e_shoff;
    auto shstrtab =
        (const ElfW(Shdr) *) (shoff + elf->e_shstrndx * elf->e_shentsize);
    auto shstr = (const char *) ((uintptr_t) elf + shstrtab->sh_offset);
    for (int k = 0; k < elf->e_shnum; k++, shoff += elf->e_shentsize) {
      auto sh = (ElfW(Shdr) *) shoff;
      switch (sh->sh_type) {
        case SHT_DYNSYM:
          dynsym_ = (uintptr_t) elf + sh->sh_offset;
          dynsym_num_ = sh->sh_size / sizeof(ElfW(Sym));
          log_v("SHT_DYNSYM dynsym_num=%d", dynsym_num_);
          break;
        case SHT_SYMTAB:
          symtab_ = (uintptr_t) elf + sh->sh_offset;
          symtab_num_ = sh->sh_size / sizeof(ElfW(Sym));
          log_v("SHT_SYMTAB symtab_num=%u", symtab_num_);
          break;
        case SHT_STRTAB:
          if (!strcmp(shstr + sh->sh_name, ".dynstr")) {
            log_v(".dynstr sh->sh_info=%lx", sh->sh_info);
            dynstr_ = (const char *) ((uintptr_t) elf + sh->sh_offset);
          } else if (!strcmp(shstr + sh->sh_name, ".strtab")) {
            log_v(".strtab sh->sh_info=%lx", sh->sh_info);
            strtab_ = (const char *) ((uintptr_t) elf + sh->sh_offset);
          }
          break;
        case SHT_PROGBITS:
          if (!dynstr_ || !dynsym_ || bias_) break;
          bias_ = sh->sh_addr - sh->sh_offset;
          log_v("bias=%lx", bias_);
          break;
      }
    }
  } while (0);

  if (dynsym_ && dynstr_ || symtab_ && strtab_) {
    return true;
  } else {
    Release();
    return false;
  }
}

void SymbolLookup::Release() {
  if (fd_ >= 0) {
    close(fd_);
    fd_ = {-1};
    pid_ = {-1};
    base_ = {};
    dynsym_ = {};
    dynsym_num_ = {};
    symtab_ = {};
    symtab_num_ = {};
    dynstr_ = {};
    strtab_ = {};
    bias_ = {};
  }
}

void *SymbolLookup::Get(const char *symbol) const {
  auto dynsym = (const ElfW(Sym) *) (dynsym_);
  for (int i = 0; i < dynsym_num_; i++) {
    if (!strcmp(dynstr_ + dynsym[i].st_name, symbol)) {
      return (void *) (base_ + dynsym[i].st_value - bias_);
    }
  }
  auto symtab = (const ElfW(Sym) *) (symtab_);
  for (int i = 0; i < symtab_num_; i++) {
    if (!strcmp(strtab_ + symtab[i].st_name, symbol)) {
      return (void *) (base_ + symtab[i].st_value - bias_);
    }
  }
  return {};
}

uintptr_t SymbolLookup::GetModuleBase(pid_t pid, const char *libname) {
  char name[32]{};
  if (pid < 0)
    snprintf(name, sizeof(name), "/proc/self/maps");
  else
    snprintf(name, sizeof(name), "/proc/%d/maps", pid);
  auto const fp = fopen(name, "r");
  if (!fp) return {};

  char line[512]{};
  uintptr_t base{};
  uintptr_t end{};
  while (fgets(line, sizeof(line), fp)) {
    if (!libname || strstr(line, libname)) {
      sscanf(line, "%lx-%lx %*s %*s %*s %*d", &base, &end);
      break;
    }
  }
  fclose(fp);
  return base;
}
