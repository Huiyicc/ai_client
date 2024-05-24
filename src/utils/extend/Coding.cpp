//
// Created by 19254 on 24-5-24.
//

#include "Coding.h"

#ifdef _HOST_WINDOWS_

#include <Windows.h>

#endif

#include <memory>

namespace HYTools {

std::string Utf8ToAnsi(const std::string &utf8Str) {
  #ifdef _HOST_WINDOWS_
  int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, nullptr, 0);
  if (wlen <= 0) return "";
  std::shared_ptr<WCHAR> wszGBK(new WCHAR[wlen], std::default_delete<WCHAR[]>());

  if (!wszGBK) return "";

  // 将UTF-8字符串转换为宽字符（Unicode）
  MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, wszGBK.get(), wlen);

  // 获取ANSI编码需要的字节数
  int alen = WideCharToMultiByte(CP_ACP, 0, wszGBK.get(), wlen, nullptr, 0, nullptr, nullptr);
  if (alen <= 0) {
    // delete[] wszGBK;
    return "";
  }

  // char *szGBK = new char[alen];
  std::shared_ptr<char> szGBK(new char[alen], std::default_delete<char[]>());
  if (!szGBK) {
    // delete[] wszGBK;
    return "";
  }

  // 将宽字符转换为ANSI编码
  WideCharToMultiByte(CP_ACP, 0, wszGBK.get(), wlen, szGBK.get(), alen, nullptr, nullptr);

  std::string result(szGBK.get(), alen - 1); // 注意：alen包含结束符'\0'，所以减1

  return result;
  #else
  #error "Unsupported platform"
  #endif

}

} // HYTOOL