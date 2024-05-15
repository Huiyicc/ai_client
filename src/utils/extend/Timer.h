//
// Created by 19254 on 24-4-16.
//

#ifndef HYDUI_TIMER_H
#define HYDUI_TIMER_H

#include <cstdint>

namespace AC::Timer {

void SleepMilliseconds(int ms);

uint64_t GetCurrentTimeMillis();

} // HYDUI

#endif //HYDUI_TIMER_H
