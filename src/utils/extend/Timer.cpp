//
// Created by 19254 on 24-4-16.
//

#include "Timer.h"
#include <thread>

namespace AC::Timer {

void SleepMilliseconds(int ms) {
  std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

uint64_t GetCurrentTimeMillis() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
    std::chrono::system_clock::now().time_since_epoch()).count();
}

} // HYDUI