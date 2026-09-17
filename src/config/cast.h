#include <boost/lexical_cast.hpp>
#include <cstddef>
#include <sstream>
#include <vector>
#include <yaml-cpp/node/node.h>
#include <yaml-cpp/node/parse.h>
#include <yaml-cpp/yaml.h>

namespace sylar {
template <typename F, typename T> class LexicalCast {
public:
  auto operator()(const F &v) -> T { return boost::lexical_cast<T>(v); }
};

template <typename T> class LexicalCast<std::string, std::vector<T>> {
public:
  auto operator()(const std::string &str) -> std::vector<T> {
    std::vector<T> out;
    YAML::Node n = YAML::Load(str);
    std::stringstream ss;
    for (std::size_t i = 0; i < n.size(); ++i) {
      ss.str("");
      ss << n[i];
      out.push_back(LexicalCast<std::string, T>()(ss.str()));
    }

    return out;
  }
};

template <typename T> class LexicalCast<std::vector<T>, std::string> {
public:
  auto operator()(std::vector<T> &vec) -> std::string {
    YAML::Node n;
    for (auto &i : vec) {
      n.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
    }
    std::stringstream ss;
    ss << n;
    return ss.str();
  }
};

} // namespace sylar
