#include <config/config.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#define CHECK(cond)                                                                                \
    do {                                                                                           \
        if (!(cond)) {                                                                             \
            std::cerr << "FAIL: " #cond " at " << __FILE__ << ":" << __LINE__ << std::endl;        \
            return 1;                                                                              \
        }                                                                                          \
    } while (0)

int main() {
    const std::string yaml_text = R"(
server:
  port: 8080
  host: 127.0.0.1
  workers: 8
  ratio: 0.75
  enabled: true
databases:
  - { name: game, port: 3306 }
  - { name: log, port: 3307 }
)";

    auto cfg = pans::Config::LoadFromString(yaml_text);
    CHECK(cfg != nullptr);

    // 嵌套取值
    CHECK(cfg->getInt("server.port") == 8080);
    CHECK(cfg->getString("server.host") == "127.0.0.1");
    CHECK(cfg->getInt("server.workers") == 8);
    CHECK(cfg->getDouble("server.ratio") == 0.75);
    CHECK(cfg->getBool("server.enabled"));

    // 子节点
    auto server = cfg->getNode("server");
    CHECK(!server.isNull());
    CHECK(server.getInt("port") == 8080);
    CHECK(server.getString("host") == "127.0.0.1");

    // 列表
    auto dbs = cfg->getList("databases");
    CHECK(dbs.size() == 2);
    CHECK(dbs[0].getString("name") == "game");
    CHECK(dbs[0].getInt("port") == 3306);
    CHECK(dbs[1].getString("name") == "log");
    CHECK(dbs[1].getInt("port") == 3307);

    // 缺失 key 返回默认值
    CHECK(cfg->getInt("server.missing", 42) == 42);
    CHECK(cfg->getString("server.missing", "fallback") == "fallback");
    CHECK(cfg->getBool("server.missing", true));
    CHECK(cfg->getDouble("server.missing", 1.5) == 1.5);

    // has / isNull
    CHECK(cfg->has("server.port"));
    CHECK(!cfg->has("server.nope"));
    CHECK(cfg->getNode("server.nope").isNull());

    // 文件加载
    const auto path = std::filesystem::temp_directory_path() / "test_config.yaml";
    {
        std::ofstream out(path);
        out << "app:\n  name: pans\n  version: 1\n";
    }
    auto file_cfg = pans::Config::LoadFromFile(path);
    CHECK(file_cfg != nullptr);
    CHECK(file_cfg->getString("app.name") == "pans");
    CHECK(file_cfg->getInt("app.version") == 1);
    std::error_code ec;
    std::filesystem::remove(path, ec);

    std::cout << "test_config PASS" << std::endl;
    return 0;
}
