//
// Created by 19254 on 24-5-20.
//
#include "HttpClient.h"
#include "boost/algorithm/string.hpp"
#include "httplib.h"
#include "fmt/format.h"

namespace AC {

void HttpClient::throw_error(const httplib::Result& res) {
  if (res.error() == httplib::Error::Success) {
    return;
  }
  throw std::runtime_error(fmt::format("http request error: {}", httplib::to_string(res.error())));
}

HttpClient::HttpClient(std::string_view url) {
  auto pos = url.find("://");
  if (pos == std::string::npos) {
    auto pl = url.find('/');
    if (pl == std::string::npos) {
      m_uri = "/";
      m_host = "http://";
      m_host += url.substr(0, pl);
      return;
    }
    m_host = "http://";
    m_host += url.substr(0, pl);
    m_ssl = false;
    m_uri = url.substr(pl);
  } else {
    if (url.substr(0, pos) == "https") {
      m_ssl = true;
    } else if (url.substr(0, pos) == "http") {
      m_ssl = false;
    }
    auto pl = url.find('/', pos + 3);
    if (pl == std::string::npos) {
      m_host = url;
      m_uri = "/";
      return;
    }
    m_host = url.substr(0, pl);
    m_uri = url.substr(m_host.size());
  }

}

std::string HttpClient::Get() {
  std::string resp;
  httplib::Result res;
  httplib::Headers headers = {
    {"Accept-Encoding", "gzip, deflate"},
  };
  if (m_ssl) {
    httplib::SSLClient cli(m_host);
    res = cli.Get(m_uri, headers);
  } else {
    httplib::Client cli(m_host);
    res = cli.Get(m_uri, headers);
  }
  throw_error(res);
  return res->body;
}

std::string HttpClient::UpFile(const httplib::MultipartFormDataItems &items) {
httplib::Result res;
  if (m_ssl) {
    httplib::SSLClient cli(m_host);
    res = cli.Post(m_uri, items);
  } else {
    httplib::Client cli(m_host);
    res = cli.Post(m_uri, items);
  }
  throw_error(res);
  return res->body;
}

std::string HttpClient::PostJson(nlohmann::json &json) {
  httplib::Result res;
  if (m_ssl) {
    httplib::SSLClient cli(m_host);
    res = cli.Post(m_uri, json.dump(), "application/json");
  } else {
    httplib::Client cli(m_host);
    res = cli.Post(m_uri, json.dump(), "application/json");
  }
  throw_error(res);
  return res->body;
}

} // AC