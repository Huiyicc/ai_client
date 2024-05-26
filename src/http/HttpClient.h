//
// Created by 19254 on 24-5-20.
//

#ifndef AUDIO_CLIENT_HTTPCLIENT_H
#define AUDIO_CLIENT_HTTPCLIENT_H

#include <string>
#include <vector>
#include <map>
#include "httplib.h"
#include "nlohmann/json.hpp"

namespace AC {

class HttpClient {
public:

public:
  explicit HttpClient(std::string_view url);

  std::string Get();
  std::string Get(std::string_view path);

  std::string UpFile(const httplib::MultipartFormDataItems &items);

  std::string UpFile(std::string_view path, const httplib::MultipartFormDataItems &items);

  std::string PostJson(nlohmann::json &json);

  std::string PostJson(std::string_view path, nlohmann::json &json);

private:
  std::string m_host;
  std::string m_uri;
  bool m_ssl = false;
  std::map<std::string, std::string> m_headers;

  void throw_error(const httplib::Result &res);
};

} // AC

#endif //AUDIO_CLIENT_HTTPCLIENT_H
