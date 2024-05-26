#include "audio/AudioClient.h"
#include "utils/extend/Logger.h"
#include "http/HttpClient.h"
#include <iostream>
#include <thread>

#include "extend/File.h"
#include "extend/Coding.h"
#include "Algorithm/AsyncQueueProcessor.h"
#include "asrlib/AiClient.h"

int main() {
  auto cli = AC::AudioClient::GetInstance();
  auto dev = AC::AudioClient::GetDefaultInputDevice();
  PrintDebug("default input device: {}", dev.Name.c_str());
  auto s = cli->OpenStream(dev);
  AC::AiClient asr("http://127.0.0.1:11435");

  s.SetMuteCallback([&asr](AC::AudioStream *stream, void *userData) -> bool {
    try{
      stream->SaveData("out.wav", true);
      PrintInfo("stop and asr");
      auto fname = asr.UploadWav("out.wav");
      auto txt = asr.Asr(fname);
      PrintInfo("asr result: {}", txt);
    } catch (const std::exception& err) {
      PrintError("err: {}", err.what() );
    } catch (...) {
      PrintError("err" );
    }
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
