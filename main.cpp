#include "audio/AudioClient.h"
#include "utils/extend/Logger.h"
#include <iostream>
#include <thread>

int main() {
  auto cli = AC::AudioClient::GetInstance();
  auto dev = AC::AudioClient::GetDefaultInputDevice();
  PrintDebug("default input device: {}", dev.Name.c_str());
  auto s = cli->OpenStream(dev);
  s.SetMuteCallback([](AC::AudioStream *stream, void *userData)->bool {
    stream->SaveData("re.wav", true);
    PrintInfo("saved");
    return true;
  });
  s.SetStartCallback([](AC::AudioStream *stream, void *userData)->bool {
    PrintInfo("start");
    return true;
  });
  std::thread a([&]() { s.Start(0.02,0.05,3000); });
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
