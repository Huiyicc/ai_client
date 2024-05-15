//
// Created by 19254 on 24-4-18.
//

#ifndef HYDUI_LOGGER_H
#define HYDUI_LOGGER_H

#include "fmt/format.h"
#include <iostream>
#include <chrono>

namespace HYDUI {

#ifdef _HOST_WINDOWS_
#define LOCALTIME(__time_str__)                                                          \
  auto __now__ = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()); \
  std::tm __tm_info__ = {0};                                                             \
  localtime_s(&__tm_info__, &__now__);                                                       \
  strftime(__time_str__, sizeof(__time_str__), "%Y-%m-%d %H:%M:%S", &__tm_info__);

#else
#define LOCALTIME(time_str)    \
  auto now = time(nullptr);    \
  struct tm tm_info = {0};     \
  localtime_r(&now, &tm_info); \
  strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", &tm_info);

#endif

#define PrintInfo(fstr, ...)                                                          \
  do {                                                                                 \
    char __time_str__F__[32];                                                                 \
    LOCALTIME(__time_str__F__);                                                               \
    std::string __lstr__ = "<I> <{}> [{}:{}] ";                                            \
    std::string_view __fPath__ = __FILE__;                                                 \
    __fPath__ = __fPath__.substr(strlen(PRIOJECT_PATH), __fPath__.size() - strlen(PRIOJECT_PATH)); \
    std::cout << fmt::format(__lstr__ + std::string(fstr), __time_str__F__, __fPath__, __LINE__, ##__VA_ARGS__)<< std::endl; \
  } while (0)

#define PrintDebug(fstr, ...)                                                          \
  do {                                                                                 \
    char __time_str__F__[32];                                                                 \
    LOCALTIME(__time_str__F__);                                                               \
    std::string __lstr__ = "<D> <{}> [{}:{}] ";                                            \
    std::string_view __fPath__ = __FILE__;                                                 \
    __fPath__ = __fPath__.substr(strlen(PRIOJECT_PATH), __fPath__.size() - strlen(PRIOJECT_PATH)); \
    std::cout << fmt::format(__lstr__ + std::string(fstr), __time_str__F__, __fPath__, __LINE__, ##__VA_ARGS__)<< std::endl; \
  } while (0)

#define PrintError(fstr, ...)                                                          \
  do {                                                                                 \
    char __time_str__F__[32];                                                                 \
    LOCALTIME(__time_str__F__);                                                               \
    std::string __lstr__ = "<E> <{}> [{}:{}] ";                                            \
    std::string_view __fPath__ = __FILE__;                                                 \
    __fPath__ = __fPath__.substr(strlen(PRIOJECT_PATH), __fPath__.size() - strlen(PRIOJECT_PATH)); \
    std::cerr << fmt::format(__lstr__ + std::string(fstr), __time_str__F__, __fPath__, __LINE__, ##__VA_ARGS__)<< std::endl; \
  } while (0)


} // HYDUI

#endif //HYDUI_LOGGER_H
