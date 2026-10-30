#ifndef PANS_INCLUDE_CONFIG_CONFIG_H
#define PANS_INCLUDE_CONFIG_CONFIG_H

#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include <export.h>
#include <maroco.h>

namespace pans {

// 配置模块：从 YAML 文件/字符串加载配置，提供类型化的取值接口。
// 所有 getter 在 key 缺失或类型不匹配时返回默认值，不会抛出异常。
class PANS_API Config final {
public:
    // 从文件加载；失败时抛出 std::runtime_error。
    static std::shared_ptr<Config> LoadFromFile(const std::filesystem::path& path);
    // 从 YAML 字符串加载；失败时抛出 std::runtime_error。
    static std::shared_ptr<Config> LoadFromString(std::string_view text);

    // 类型化取值，key 支持点号分隔的嵌套路径，如 "server.port"。
    std::string getString(std::string_view key, std::string_view def = "") const;
    s64        getInt(std::string_view key, s64 def = 0) const;
    double     getDouble(std::string_view key, double def = 0.0) const;
    bool       getBool(std::string_view key, bool def = false) const;

    // 返回子节点；key 缺失时返回 isNull() == true 的空节点。
    Config getNode(std::string_view key) const;
    // 返回序列节点对应的元素列表；非序列或缺失时返回空 vector。
    std::vector<Config> getList(std::string_view key) const;

    bool has(std::string_view key) const;
    bool isNull() const noexcept;

private:
    class Impl;
    explicit Config(std::shared_ptr<Impl> impl) noexcept;
    std::shared_ptr<Impl> m_impl;
};

} // namespace pans

#endif // PANS_INCLUDE_CONFIG_CONFIG_H
