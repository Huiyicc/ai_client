//
// Created by 19254 on 24-5-24.
//

#include "File.h"
#include <fstream>

namespace HYTools {

std::string ReadFile(std::string_view path) {
  std::ifstream ifs(path.data(), std::ios::binary | std::ios::in);
  if (!ifs) {
    throw std::runtime_error("file not found");
  }
  std::string content;
  ifs.seekg(0, std::ios::end);
  content.resize(ifs.tellg());
  ifs.seekg(0, std::ios::beg);
  ifs.read(content.data(), content.size());
  return content;
}


} // HYTools