//
// Created by 19254 on 24-4-18.
//

#ifndef HYDUI_DEFRT_H
#define HYDUI_DEFRT_H
#include <functional>

namespace HYTools {

struct ExitCaller {
  ~ExitCaller() { functor_(); }
  ExitCaller(std::function<void()>&& functor)
    : functor_(std::move(functor)) {}
private:
  std::function<void()> functor_;
};


#define DEFER_PASTE_(x, y) x##y
#define DEFER_CAT_(x, y) DEFER_PASTE_(x, y)

#define DEFER(code) \
  HYTools::ExitCaller DEFER_CAT_(defer_, __LINE__)([&](){code;}) \

} // HYTools

#endif //HYDUI_DEFRT_H
