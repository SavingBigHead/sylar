#include "config.h"
#include "log.h"
#include "yaml-cpp/yaml.h"
#include <yaml-cpp/node/parse.h>

sylar::ConfigVar<int>::ptr g_init_value_config =
    sylar::Config::Lookup("system.port", 8080, "system prot");

int main() {
  SYLAR_LOG_INFO(SYLAR_LOG_ROOT) << g_init_value_config->getVal();
  SYLAR_LOG_INFO(SYLAR_LOG_ROOT) << g_init_value_config->toString();

  YAML::Node n = YAML::LoadFile("/home/meng/sylar/log.yml");

  sylar::Config::LoadFromYaml(n);

  SYLAR_LOG_INFO(SYLAR_LOG_ROOT) << g_init_value_config->getVal();
  SYLAR_LOG_INFO(SYLAR_LOG_ROOT) << g_init_value_config->toString();

  return 0;
}
