#pragma once

#include "log.h"
#include <boost/lexical_cast.hpp>
#include <exception>
#include <memory>
#include <string>

namespace sylar {
class ConfigVarBase {
public:
  using ptr = std::shared_ptr<ConfigVarBase>;

  ConfigVarBase(const std::string &name, const std::string &description = "")
      : name_(name), description_(description) {}

  virtual ~ConfigVarBase();

  auto getName() { return name_; }
  auto getDescription() { return description_; }

  virtual auto toString() -> std::string = 0;
  virtual auto fromString(const std::string &) -> bool = 0;

protected:
  std::string name_;
  std::string description_;
};

template <class T> class ConfigVar : public ConfigVarBase {
public:
  using ptr = std::shared_ptr<ConfigVar>;

  ConfigVar(const std::string &name, const T &default_value,
            const std::string description)
      : ConfigVarBase(name, description), val_(default_value) {}

  auto toString() override {
    try {
      return boost::lexical_cast<std::string>(val_);
    } catch (std::exception &e) {
      SYLAR_LOG_ERROR(SYLAR_LOG_ROOT)
          << "ConfigVar::tostring exception" << e.what()
          << " convert: " << typeid(val_).name() << " to string";
    }
    return "";
  }

  auto fromString(const std::string &val) override {
    try {
      val_ = boost::lexical_cast<T>(val);
      return true;
    } catch (std::exception &e) {
      SYLAR_LOG_ERROR(SYLAR_LOG_ROOT)
          << "ConfigVar::fromstring exception" << e.what()
          << " convert: string to " << typeid(val_).name();
    }
    return false;
  }

private:
  T val_;
};
} // namespace sylar
