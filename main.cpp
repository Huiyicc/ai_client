#include "audio/AudioClient.h"
#include "utils/extend/Logger.h"
#include <iostream>
#include <thread>

int main() {
  auto cli = AC::AudioClient::GetInstance();
  auto dev = AC::AudioClient::GetDefaultInputDevice();
  PrintDebug("default input device: {}", dev.Name.c_str());
  auto s = cli->OpenStream(dev);
  s.SetCallback([](AC::AudioStream *stream, void *userData) {
    stream->SaveData("re.wav", true);
    PrintInfo("saved");
  });
  std::thread a([&]() { s.Start(); });
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
