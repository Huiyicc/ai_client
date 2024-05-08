#include "audio/AudioClient.h"
#include <iostream>

int main() {
  auto cli =  AC::AudioClient::GetInstance();
  auto dev = AC::AudioClient::GetDefaultInputDevice();
  auto s = cli->OpenStream(dev);
  s.Start();

  return 0;
}
