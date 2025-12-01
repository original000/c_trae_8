#pragma once
#include "IPlugin.h"
#include <windows.h>
#include <string>
#include <unordered_map>
#include <chrono>
#include <ctime>
#include <memory>

struct PluginInfo {
    HMODULE hModule = nullptr;
    IPlugin* pluginInstance = nullptr;
    uint32_t version = 0;
    CreatePluginFunc createFunc = nullptr;
    DestroyPluginFunc destroyFunc = nullptr;
    std::chrono::system_clock::time_point lastWriteTime;
    std::wstring dllPath;
};

class PluginReloader {
private:
    std::unordered_map<std::wstring, PluginInfo> plugins;
    std::wstring pluginDir;
    
    // 检查文件是否已修改
    bool IsFileModified(const PluginInfo& pluginInfo);
    
    // 获取文件最后写入时间
    std::chrono::system_clock::time_point GetLastWriteTime(const std::wstring& filePath);
    
    // 卸载插件
    void UnloadPlugin(PluginInfo& pluginInfo);
    
    // 加载插件
    bool LoadPlugin(const std::wstring& filePath);
    
    // 热补丁：修改当前函数的返回地址到新版本函数
    void HotPatchFunction(void* oldFunc, void* newFunc);
    
public:
    PluginReloader(const std::wstring& pluginDirectory = L"plugins/");
    ~PluginReloader();
    
    // 初始化插件目录
    bool Initialize();
    
    // 更新所有插件，检查文件变化并执行热重载
    void Update();
    
    // 获取所有加载的插件
    const std::unordered_map<std::wstring, PluginInfo>& GetPlugins() const;
    
    // 手动触发重载指定插件
    bool ReloadPlugin(const std::wstring& pluginName);
};
