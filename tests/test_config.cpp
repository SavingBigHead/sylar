#include "config.h"
#include "log.h"
#include "yaml-cpp/yaml.h"
#include <vector>
#include <yaml-cpp/node/parse.h>

sylar::ConfigVar<int>::ptr g_init_value_config =
    sylar::Config::Lookup("system.port", 8080, "system prot");

sylar::ConfigVar<std::vector<int>>::ptr int_vec = sylar::Config::Lookup(
    "system.int_vec", std::vector<int>{1, 2, 3}, "system int_vec");

int main() {
  SYLAR_LOG_INFO(SYLAR_LOG_ROOT) << g_init_value_config->getVal();
  SYLAR_LOG_INFO(SYLAR_LOG_ROOT) << g_init_value_config->toString();

  auto &v = int_vec->getVal();
  for (auto &i : v) {
    SYLAR_LOG_INFO(SYLAR_LOG_ROOT) << "int_vec: " << i;
  }

  YAML::Node n = YAML::LoadFile("/home/meng/sylar/log.yml");

  sylar::Config::LoadFromYaml(n);

  SYLAR_LOG_INFO(SYLAR_LOG_ROOT) << g_init_value_config->getVal();
  SYLAR_LOG_INFO(SYLAR_LOG_ROOT) << g_init_value_config->toString();

  SYLAR_LOG_INFO(SYLAR_LOG_ROOT) << int_vec->toString();

  return 0;
}
