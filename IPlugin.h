#pragma once
#include <cstdint>

// 插件接口版本
#define PLUGIN_INTERFACE_VERSION 1

class IPlugin {
public:
    virtual ~IPlugin() = default;
    
    // 获取插件版本号
    virtual uint32_t GetVersion() const = 0;
    
    // 插件主更新函数
    virtual void Update(float deltaTime) = 0;
    
    // 获取插件名称
    virtual const char* GetName() const = 0;
};

// 插件导出函数签名
extern "C" {
    typedef IPlugin* (*CreatePluginFunc)();
    typedef void (*DestroyPluginFunc)(IPlugin*);
}
