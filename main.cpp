#include "json_parser.h"
#include "string_literal.h"
#include <iostream>
#include <string>
#include <array>

// 定义配置结构体
struct Config {
    Json::string_view name;
    int port;
    bool debug;
    std::array<Json::string_view, 2> tags;
    
    static constexpr auto fields = std::make_tuple(
        Json::detail::FieldDesc<decltype("name"_sl), decltype(&Config::name), &Config::name>{},
        Json::detail::FieldDesc<decltype("port"_sl), decltype(&Config::port), &Config::port>{},
        Json::detail::FieldDesc<decltype("debug"_sl), decltype(&Config::debug), &Config::debug>{},
        Json::detail::FieldDesc<decltype("tags"_sl), decltype(&Config::tags), &Config::tags>{}
    );
};

// JSON字符串常量
constexpr const char config_json[] = R"({
    "name": "server",
    "port": 8080,
    "debug": true,
    "tags": ["web", "api"]
})";

int main() {
    // 编译期解析JSON
    constexpr auto config = Json::parse<Config, sizeof(config_json)>(config_json);
    
    // 编译期断言验证
    static_assert(config.name == "server"_sl, "Name mismatch");
    static_assert(config.port == 8080, "Port mismatch");
    static_assert(config.debug == true, "Debug flag mismatch");
    static_assert(config.tags[0] == "web"_sl, "First tag mismatch");
    static_assert(config.tags[1] == "api"_sl, "Second tag mismatch");
    
    // 运行时输出验证结果
    ::std::cout << "=== Compile-time JSON Parser Demo ===\n";
    ::std::cout << "Name: " << config.name.to_string() << "\n";
    ::std::cout << "Port: " << config.port << "\n";
    ::std::cout << "Debug: " << std::boolalpha << config.debug << "\n";
    ::std::cout << "Tags: [\"" << config.tags[0].to_string() << "\", \"" << config.tags[1].to_string() << "\"]\n";
    ::std::cout << "\nAll static assertions passed successfully!\n";
    
    return 0;
}