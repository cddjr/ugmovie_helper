/*
 * @Author: 景大虾(dengjingren@foxmail.com)
 * @Date: 2023-08-06
 * @Last Modified by: 景大虾(dengjingren@foxmail.com)
 * @Last Modified time: 2023-08-06
 */

#include <sys/cdefs.h>

#include <mutex>
#include <shared_mutex>

#ifndef CONCATENATE
#define CONCATENATE(a, b) __CONCAT(a, b)
#endif

#define UNIQUE_NAME(prefix) \
  CONCATENATE(CONCATENATE(prefix##_, __COUNTER__), CONCATENATE(at, __LINE__))

#define _synchronized(lock, name, mtx) if (lock name(mtx); true)
#define synchronized_read(smtx) \
  _synchronized(std::shared_lock<decltype(smtx)>, UNIQUE_NAME(rdLock), smtx)
#define synchronized_write(mtx) \
  _synchronized(std::unique_lock<decltype(mtx)>, UNIQUE_NAME(lock), mtx)
/*
        it mimics the behaviour of the Java construct
        "synchronized(this) { }"
*/
#define synchronized(mtx) synchronized_write(mtx)