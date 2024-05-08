//
// Created by 19254 on 24-4-16.
//

#include "Timer.h"
#include <thread>

namespace HYDUI::Timer  {

void SleepMilliseconds(int ms) {
  std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

} // HYDUI