//
// Created by 19254 on 24-5-26.
//

#ifndef AUDIO_CLIENT_AICLIENT_H
#define AUDIO_CLIENT_AICLIENT_H

#include <string>
#include <memory>
#include "http/HttpClient.h"
#include "nlohmann/json.hpp"

namespace AC {

class AiClient {
public:
  explicit AiClient(std::string_view host);

  std::string UploadWav(std::string_view file);

  std::string Asr(std::string_view name, std::string_view lang = "zh");

private:
  static std::string SplicPath(const std::string &path);

  static nlohmann::json checkResp(std::string_view resp);

  std::string m_host;
  HttpClient m_cli;
};

} // AC

#endif //AUDIO_CLIENT_AICLIENT_H
