#include "config.h"
#include "log.h"
#include <algorithm>
#include <list>
#include <string>
#include <utility>
#include <yaml-cpp/node/node.h>
#include <yaml-cpp/yaml.h>

namespace sylar {

auto Config::LookupBase(const std::string &name) -> ConfigVarBase::ptr {
  auto it = GetDatas().find(name);
  if (it == GetDatas().end()) {
    return nullptr;
  } else {
    return it->second;
  }
}

// 递归方式，遍历YAML格式的配置文件中的所有成员，将每个节点的名称和值存在list中
auto ListAllNode(const std::string &prefix, const YAML::Node &node,
                 std::list<std::pair<std::string, const YAML::Node>> &output) {

  if (prefix.find_first_not_of("qwertyuiopasdfghjklzxcvbnm._1234567890") !=
      std::string::npos) {
    SYLAR_LOG_ERROR(SYLAR_LOG_ROOT)
        << "Config invalid name: " << prefix << " ! " << node;
    return;
  }

  output.push_back(std::make_pair(prefix, node));

  if (node.IsMap()) {
    for (auto i = node.begin(); i != node.end(); ++i) {
      // 若前缀为空,说明为顶层，prefix为key的值，否则为子层，prefix为父层加上当前层。it->second为当前node
      ListAllNode(prefix.empty() ? i->first.Scalar()
                                 : prefix + "." + i->first.Scalar(),
                  i->second, output);
    }
  }
}

auto Config::LoadFromYaml(const YAML::Node &root) -> void {
  std::list<std::pair<std::string, const YAML::Node>> all_nodes;
  ListAllNode("", root, all_nodes);

  for (auto i : all_nodes) {
    std::string &key = i.first;
    if (key.empty()) {
      continue;
    }

    std::transform(key.begin(), key.end(), key.begin(), ::tolower);

    ConfigVarBase::ptr var = LookupBase(key);
    if (var) {
      if (i.second.IsScalar()) {
        var->fromString(i.second.Scalar());
      } else {
        std::stringstream ss;
        ss << i.second;
        var->fromString(ss.str());
      }
    }
  }
}

} // namespace sylar
