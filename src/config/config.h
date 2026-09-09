#pragma once

#include "log.h"
#include <boost/lexical_cast.hpp>
#include <exception>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>

namespace sylar {
class ConfigVarBase {
public:
  using ptr = std::shared_ptr<ConfigVarBase>;

  ConfigVarBase(const std::string &name, const std::string &description = "")
      : name_(name), description_(description) {}

  virtual ~ConfigVarBase() {}

  auto getName() { return name_; }
  auto getDescription() { return description_; }

  virtual auto toString() -> std::string = 0;
  virtual auto fromString(const std::string &) -> bool = 0;

protected:
  std::string name_;
  std::string description_;
};

template <typename T> class ConfigVar : public ConfigVarBase {
public:
  using ptr = std::shared_ptr<ConfigVar>;

  ConfigVar(const std::string &name, const T &default_value,
            const std::string description)
      : ConfigVarBase(name, description), val_(default_value) {}

  auto toString() -> std::string override {
    try {
      return boost::lexical_cast<std::string>(val_);
    } catch (std::exception &e) {
      SYLAR_LOG_ERROR(SYLAR_LOG_ROOT)
          << "ConfigVar::tostring exception" << e.what()
          << " convert: " << typeid(val_).name() << " to string";
    }
    return "";
  }

  auto fromString(const std::string &val) -> bool override {
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

  auto getVal() const { return val_; }
  auto setVal(T &t) { val_ = t; }

private:
  T val_;
};

class Config {
public:
  using ConfigVarMap = std::map<std::string, ConfigVarBase::ptr>;

  template <typename T>
  static auto Lookup(const std::string &name) -> typename ConfigVar<T>::ptr {
    auto it = datas_.find(name);
    if (it == datas_.end()) {
      return nullptr;
    } else {
      return std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
    }
  }

  template <typename T>
  static auto Lookup(const std::string &name, const T &default_value,
                     const std::string &description) {
    auto temp = Lookup<T>(name);
    if (temp) {
      SYLAR_LOG_INFO(SYLAR_LOG_ROOT) << "Lookup name=" << name << " exists";
      return temp;
    }

    if (name.find_first_not_of("qwertyuiopasdfghjklzxvbnm._1234567890") !=
        std::string::npos) {
      SYLAR_LOG_ERROR(SYLAR_LOG_ROOT) << "Lookup name invalid " << name;
      throw std::invalid_argument(name);
    }

    typename ConfigVar<T>::ptr v(
        new ConfigVar<T>(name, default_value, description));
    datas_[name] = v;

    return v;
  }

private:
  static ConfigVarMap datas_;
};
} // namespace sylar
