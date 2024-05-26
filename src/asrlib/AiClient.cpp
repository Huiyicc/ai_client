//
// Created by 19254 on 24-5-26.
//

#include "AiClient.h"
#include "extend/File.h"
#include "extend/Logger.h"

namespace AC {

AiClient::AiClient(std::string_view host) : m_host(host), m_cli(host) {

}

std::string AiClient::SplicPath(const std::string &path) {
  return "/v1" + path;
}

nlohmann::json AiClient::checkResp(std::string_view resp) {
  auto j = nlohmann::json::parse(resp);
  if (j["errcode"] != 200) {
    throw std::runtime_error(j.value("msg", "unknown error"));
  }
  return j;
};

std::string AiClient::UploadWav(std::string_view file) {
  auto resp = m_cli.UpFile(SplicPath("/api/up_file"), {
    {
      "file",
      HYTools::ReadFile(file),
      "audio.wav",
      "audio/wav",
    }
  });
  auto j = checkResp(resp);
  return j.value("name", "");
};

std::string AiClient::Asr(std::string_view name, std::string_view lang) {
  auto data = nlohmann::json::object();
  data["file_name"] = name;
  data["out_type"] = "obj";
  data["align_text"] = "";
  data["lang"] = lang;
  auto resp = m_cli.PostJson(SplicPath("/api/asr"), data);
  auto j = checkResp(resp);
  return j["raw"].value("text", "");
};

} // AC