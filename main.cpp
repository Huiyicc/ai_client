#include "audio/AudioClient.h"
#include "utils/extend/Logger.h"
#include "http/HttpClient.h"
#include <iostream>
#include <thread>

#include "extend/File.h"
#include "extend/Coding.h"

void pasr(std::string_view file) {
  AC::HttpClient cli("http://127.0.0.1:11435/v1/api/asr");
  auto data = nlohmann::json::object();
  /*
  {
    "file_name":"2df8a8bab77b49e78c25f85a91831794_1716559569", // 上传文件上传返回的文件名
    "out_type": "obj",
    "align_text": "",
    "lang": "zh"
  }
   */
  data["file_name"] = file;
  data["out_type"] = "obj";
  data["align_text"] = "";
  data["lang"] = "zh";
  auto res = cli.PostJson(data);
  auto res_obj = nlohmann::json::parse(res);
  auto te = HYTools::Utf8ToAnsi(res_obj["raw"].value("text", ""));
  PrintDebug("resp:{}", te);
}

int main() {
//  pasr("2df8a8bab77b49e78c25f85a91831794_1716559569");
//  exit(1);
  auto cli = AC::AudioClient::GetInstance();
  auto dev = AC::AudioClient::GetDefaultInputDevice();
  PrintDebug("default input device: {}", dev.Name.c_str());
  auto s = cli->OpenStream(dev);
  s.SetMuteCallback([](AC::AudioStream *stream, void *userData) -> bool {
    stream->SaveData("re.wav", true);

    return true;
  });
  s.SetStartCallback([](AC::AudioStream *stream, void *userData) -> bool {
    PrintInfo("start");
    return true;
  });
  std::thread a([&]() { s.Start(0.01, 0.03, 3000); });
  std::this_thread::sleep_for(std::chrono::seconds(5));

  while (true) {
    std::string cmd;
    std::cin >> cmd;
    if (cmd == "exit") {
      s.Stop();
      a.join();
      break;
    }

  }

  return 0;
}
