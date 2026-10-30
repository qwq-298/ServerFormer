#include <config/config.h>

#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <yaml-cpp/yaml.h>

namespace pans {

class Config::Impl {
public:
    explicit Impl(YAML::Node node) : m_node(std::move(node)) {}

    YAML::Node m_node;
};

namespace {

std::vector<std::string> SplitKey(std::string_view key) {
    std::vector<std::string> parts;
    std::size_t start = 0;
    while (start <= key.size()) {
        const std::size_t dot = key.find('.', start);
        if (dot == std::string_view::npos) {
            parts.emplace_back(key.substr(start));
            break;
        }
        parts.emplace_back(key.substr(start, dot - start));
        start = dot + 1;
    }
    return parts;
}

// 沿 "a.b.c" 逐级查找；任一段缺失/非 map 时返回未定义节点。
// 注意：必须用 const 引用递归下钻，绝不能对「与树共享节点的 YAML::Node」
// 做拷贝赋值 —— YAML::Node 的 operator= 会调用 set_ref 改写被赋值节点，
// 从而破坏整棵树（首个查找成功、后续查找失效）。
YAML::Node NavigateImpl(const YAML::Node& node, const std::vector<std::string>& parts,
                        std::size_t i) {
    if (i >= parts.size()) {
        return node;
    }
    if (parts[i].empty() || !node.IsMap()) {
        // 注意：不能返回默认构造的 YAML::Node() —— 它的 IsDefined() 为 true。
        return YAML::Node(YAML::NodeType::Undefined);
    }
    const YAML::Node child = node[parts[i]]; // const operator[] 不会插入缺失键
    if (!child.IsDefined()) {
        return YAML::Node(YAML::NodeType::Undefined);
    }
    return NavigateImpl(child, parts, i + 1);
}

YAML::Node Navigate(const YAML::Node& root, std::string_view key) {
    return NavigateImpl(root, SplitKey(key), 0);
}

} // namespace

Config::Config(std::shared_ptr<Impl> impl) noexcept : m_impl(std::move(impl)) {}

std::shared_ptr<Config> Config::LoadFromFile(const std::filesystem::path& path) {
    try {
        YAML::Node root = YAML::LoadFile(path.string());
        return std::shared_ptr<Config>(new Config(std::make_shared<Impl>(std::move(root))));
    } catch (const YAML::Exception& e) {
        throw std::runtime_error(std::string("Config::LoadFromFile '") + path.string() +
                                 "' failed: " + e.what());
    }
}

std::shared_ptr<Config> Config::LoadFromString(std::string_view text) {
    try {
        YAML::Node root = YAML::Load(std::string(text));
        return std::shared_ptr<Config>(new Config(std::make_shared<Impl>(std::move(root))));
    } catch (const YAML::Exception& e) {
        throw std::runtime_error(std::string("Config::LoadFromString failed: ") + e.what());
    }
}

std::string Config::getString(std::string_view key, std::string_view def) const {
    const YAML::Node node = Navigate(m_impl->m_node, key);
    if (!node.IsDefined() || !node.IsScalar()) {
        return std::string(def);
    }
    return node.Scalar();
}

s64 Config::getInt(std::string_view key, s64 def) const {
    const YAML::Node node = Navigate(m_impl->m_node, key);
    if (!node.IsDefined() || !node.IsScalar()) {
        return def;
    }
    try {
        return node.as<s64>();
    } catch (const YAML::Exception&) {
        return def;
    }
}

double Config::getDouble(std::string_view key, double def) const {
    const YAML::Node node = Navigate(m_impl->m_node, key);
    if (!node.IsDefined() || !node.IsScalar()) {
        return def;
    }
    try {
        return node.as<double>();
    } catch (const YAML::Exception&) {
        return def;
    }
}

bool Config::getBool(std::string_view key, bool def) const {
    const YAML::Node node = Navigate(m_impl->m_node, key);
    if (!node.IsDefined() || !node.IsScalar()) {
        return def;
    }
    try {
        return node.as<bool>();
    } catch (const YAML::Exception&) {
        return def;
    }
}

Config Config::getNode(std::string_view key) const {
    YAML::Node node = Navigate(m_impl->m_node, key);
    return Config(std::make_shared<Impl>(std::move(node)));
}

std::vector<Config> Config::getList(std::string_view key) const {
    std::vector<Config> result;
    YAML::Node node = Navigate(m_impl->m_node, key);
    if (!node.IsDefined() || !node.IsSequence()) {
        return result;
    }
    for (const YAML::Node& item : node) {
        result.push_back(Config(std::make_shared<Impl>(item)));
    }
    return result;
}

bool Config::has(std::string_view key) const {
    return Navigate(m_impl->m_node, key).IsDefined();
}

bool Config::isNull() const noexcept {
    return !m_impl || !m_impl->m_node.IsDefined() || m_impl->m_node.IsNull();
}

} // namespace pans
