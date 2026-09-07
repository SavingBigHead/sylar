#pragma once

#include <memory>
namespace sylar {

template <class T, class X = void, int N = 0> class Singleton {
public:
  static auto GetInstance() -> T * {
    static T v;
    return &v;
  }
};

template <class T, class X = void, int N = 0> class SingletonPtr {
public:
  static auto GetInstance() -> std::shared_ptr<T> {
    static std::shared_ptr<T> v(new T);
    return v;
  }
};

} // namespace sylar
