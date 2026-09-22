#pragma once

#include "cast.h"
#include "log.h"
#include <algorithm>
#include <boost/lexical_cast.hpp>
#include <cctype>
#include <cstdint>
#include <exception>
#include <functional>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <yaml-cpp/node/node.h>
#include <yaml-cpp/node/parse.h>

namespace sylar {
class ConfigVarBase {
public:
  using ptr = std::shared_ptr<ConfigVarBase>;

  ConfigVarBase(const std::string &name, const std::string &description = "")
      : name_(name), description_(description) {
    std::transform(name_.begin(), name_.end(), name_.begin(), ::tolower);
  }

  virtual ~ConfigVarBase() {}

  auto getName() { return name_; }
  auto getDescription() { return description_; }

  virtual auto toString() -> std::string = 0;
  virtual auto fromString(const std::string &) -> bool = 0;
  virtual auto getTypeName() const -> std::string = 0;

protected:
  std::string name_;
  std::string description_;
};

template <typename T, typename FromStr = LexicalCast<std::string, T>,
          typename ToStr = LexicalCast<T, std::string>>
class ConfigVar : public ConfigVarBase {
public:
  using ptr = std::shared_ptr<ConfigVar>;
  using on_change_cb =
      std::function<void(const T &old_value, const T &new_value)>;

  ConfigVar(const std::string &name, const T &default_value,
            const std::string description)
      : ConfigVarBase(name, description), val_(default_value) {}

  auto toString() -> std::string override {
    try {
      return ToStr()(val_);
    } catch (std::exception &e) {
      SYLAR_LOG_ERROR(SYLAR_LOG_ROOT)
          << "ConfigVar::tostring exception" << e.what()
          << " convert: " << typeid(val_).name() << " to string";
    }
    return "";
  }

  auto fromString(const std::string &val) -> bool override {
    try {
      val_ = FromStr()(val);
      return true;
    } catch (std::exception &e) {
      SYLAR_LOG_ERROR(SYLAR_LOG_ROOT)
          << "ConfigVar::fromstring exception" << e.what()
          << " convert: string to " << typeid(val_).name();
    }
    return false;
  }

  auto getVal() -> T & { return val_; }
  auto setVal(T &t) {
    if (t == val_) {
      return;
    }
    for (auto &i : cbs_) {
      i.second(val_, t);
    }
    val_ = t;
  }
  auto getTypeName() const -> std::string override {
    return typeid(val_).name();
  }

  auto addListener(uint64_t key, on_change_cb cb) { cbs_[key] = cb; }

  auto delListener(uint64_t key) { cbs_.erase(key); }

  auto getListener(uint64_t key) -> on_change_cb {
    auto it = cbs_.find(key);
    if (it != cbs_.end()) {
      return it->second;
    } else {
      return nullptr;
    }
  }

private:
  T val_;
  std::map<uint64_t, on_change_cb> cbs_;
};

class Config {
public:
  using ConfigVarMap = std::map<std::string, ConfigVarBase::ptr>;

  template <typename T>
  static auto Lookup(const std::string &name) -> typename ConfigVar<T>::ptr {
    auto it = GetDatas().find(name);
    if (it == GetDatas().end()) {
      return nullptr;
    } else {
      return std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
    }
  }

  template <typename T>
  static auto Lookup(const std::string &name, const T &default_value,
                     const std::string &description) ->
      typename ConfigVar<T>::ptr {
    auto it = GetDatas().find(name);
    if (it != GetDatas().end()) {
      auto temp = std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
      if (temp) {
        SYLAR_LOG_INFO(SYLAR_LOG_ROOT) << "Lookup name =" << name << " exists";
        return temp;
      } else {
        SYLAR_LOG_ERROR(SYLAR_LOG_ROOT)
            << "Lookup name = " << name << " exists but type not "
            << typeid(T).name()
            << "real type name: " << it->second->getTypeName();
        return nullptr;
      }
    }

    if (name.find_first_not_of("qwertyuiopasdfghjklzxcvbnm._1234567890") !=
        std::string::npos) {
      SYLAR_LOG_ERROR(SYLAR_LOG_ROOT) << "Lookup name invalid " << name;
      throw std::invalid_argument(name);
    }

    typename ConfigVar<T>::ptr v(
        new ConfigVar<T>(name, default_value, description));
    GetDatas()[name] = v;

    return v;
  }

  static auto LookupBase(const std::string &name) -> ConfigVarBase::ptr;
  static auto LoadFromYaml(const YAML::Node &root) -> void;

private:
  static auto GetDatas() -> ConfigVarMap & {
    static ConfigVarMap datas_;
    return datas_;
  }
};
} // namespace sylar
