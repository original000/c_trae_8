#include "PluginReloader.h"
#include <iostream>
#include <stdexcept>
#include <cstring>

PluginReloader::PluginReloader(const std::wstring& pluginDirectory)
    : pluginDir(pluginDirectory) {
}

PluginReloader::~PluginReloader() {
    // 卸载所有插件
    for (auto& pair : plugins) {
        UnloadPlugin(pair.second);
    }
    plugins.clear();
}

std::chrono::system_clock::time_point PluginReloader::GetLastWriteTime(const std::wstring& filePath) {
    HANDLE hFile = CreateFileW(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                              nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    
    if (hFile == INVALID_HANDLE_VALUE) {
        return std::chrono::system_clock::time_point();
    }
    
    FILETIME ftCreate, ftAccess, ftWrite;
    if (!GetFileTime(hFile, &ftCreate, &ftAccess, &ftWrite)) {
        CloseHandle(hFile);
        return std::chrono::system_clock::time_point();
    }
    
    CloseHandle(hFile);
    
    // 转换为time_point
    ULARGE_INTEGER ull;
    ull.LowPart = ftWrite.dwLowDateTime;
    ull.HighPart = ftWrite.dwHighDateTime;
    
    std::time_t timestamp = static_cast<std::time_t>(ull.QuadPart / 10000000ULL - 11644473600ULL);
    return std::chrono::system_clock::from_time_t(timestamp);
}

bool PluginReloader::IsFileModified(const PluginInfo& pluginInfo) {
    auto currentTime = GetLastWriteTime(pluginInfo.dllPath);
    return currentTime > pluginInfo.lastWriteTime;
}

void PluginReloader::UnloadPlugin(PluginInfo& pluginInfo) {
    if (pluginInfo.pluginInstance) {
        if (pluginInfo.destroyFunc) {
            pluginInfo.destroyFunc(pluginInfo.pluginInstance);
        }
        pluginInfo.pluginInstance = nullptr;
    }
    
    if (pluginInfo.hModule) {
        FreeLibrary(pluginInfo.hModule);
        pluginInfo.hModule = nullptr;
    }
    
    pluginInfo.createFunc = nullptr;
    pluginInfo.destroyFunc = nullptr;
    pluginInfo.version = 0;
}

void PluginReloader::HotPatchFunction(void* oldFunc, void* newFunc) {
    // 修改内存保护属性，允许写入
    DWORD oldProtect;
    if (!VirtualProtect(oldFunc, 5, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        return;
    }
    
    // 构造跳转指令 (JMP rel32)
    uint8_t jmpInstruction = 0xE9;
    uintptr_t relativeAddr = reinterpret_cast<uintptr_t>(newFunc) - reinterpret_cast<uintptr_t>(oldFunc) - 5;
    
    std::memcpy(oldFunc, &jmpInstruction, 1);
    std::memcpy(reinterpret_cast<uint8_t*>(oldFunc) + 1, &relativeAddr, 4);
    
    // 恢复内存保护
    VirtualProtect(oldFunc, 5, oldProtect, &oldProtect);
    
    // 刷新指令缓存
    FlushInstructionCache(GetCurrentProcess(), oldFunc, 5);
}

bool PluginReloader::LoadPlugin(const std::wstring& filePath) {
    // 先卸载旧版本（如果存在）
    auto it = plugins.find(filePath);
    if (it != plugins.end()) {
        UnloadPlugin(it->second);
    }
    
    PluginInfo pluginInfo;
    pluginInfo.dllPath = filePath;
    
    // 加载DLL
    pluginInfo.hModule = LoadLibraryExW(filePath.c_str(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
    if (!pluginInfo.hModule) {
            std::string filePathStr(filePath.begin(), filePath.end());
        std::cerr << "Failed to load plugin: " << filePathStr << std::endl;
        return false;
    }
    
    // 获取导出函数
    pluginInfo.createFunc = reinterpret_cast<CreatePluginFunc>(GetProcAddress(pluginInfo.hModule, "CreatePlugin"));
    pluginInfo.destroyFunc = reinterpret_cast<DestroyPluginFunc>(GetProcAddress(pluginInfo.hModule, "DestroyPlugin"));
    
    if (!pluginInfo.createFunc || !pluginInfo.destroyFunc) {
            std::string filePathStr(filePath.begin(), filePath.end());
        std::cerr << "Failed to find plugin entry points: " << filePathStr << std::endl;
        UnloadPlugin(pluginInfo);
        return false;
    }
    
    // 创建插件实例
    pluginInfo.pluginInstance = pluginInfo.createFunc();
    if (!pluginInfo.pluginInstance) {
            std::string filePathStr(filePath.begin(), filePath.end());
        std::cerr << "Failed to create plugin instance: " << filePathStr << std::endl;
        UnloadPlugin(pluginInfo);
        return false;
    }
    
    // 获取插件版本
    pluginInfo.version = pluginInfo.pluginInstance->GetVersion();
    pluginInfo.lastWriteTime = GetLastWriteTime(filePath);
    
    std::string filePathStr(filePath.begin(), filePath.end());
    std::cout << "Loaded plugin: " << filePathStr << ", version: " << pluginInfo.version << std::endl;
    
    // 如果之前有旧版本，自动完成热重载
    if (it != plugins.end()) {
        std::cout << "Hot reload completed successfully!" << std::endl;
    }
    
    plugins[filePath] = pluginInfo;
    return true;
}

bool PluginReloader::Initialize() {
    std::wstring searchPath = pluginDir + L"*.dll";
    
    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(searchPath.c_str(), &findData);
    
    if (hFind == INVALID_HANDLE_VALUE) {
            std::string pluginDirStr(pluginDir.begin(), pluginDir.end());
        std::cerr << "No plugins found in directory: " << pluginDirStr << std::endl;
        return true; // 没有插件也算初始化成功
    }
    
    do {
        if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            std::wstring filePath = pluginDir + findData.cFileName;
            LoadPlugin(filePath);
        }
    } while (FindNextFileW(hFind, &findData));
    
    FindClose(hFind);
    return true;
}

void PluginReloader::Update() {
    // 检查所有插件是否需要重载
    for (auto& pair : plugins) {
        if (IsFileModified(pair.second)) {
                std::string filePathStr(pair.first.begin(), pair.first.end());
            std::cout << "Plugin modified, reloading: " << filePathStr << std::endl;
            LoadPlugin(pair.first);
        }
        
        // 调用插件Update函数
        if (pair.second.pluginInstance) {
            try {
                pair.second.pluginInstance->Update(0.016f); // 模拟60帧每秒
            } catch (const std::exception& e) {
                std::cerr << "Plugin update error: " << e.what() << std::endl;
            }
        }
    }
    
    // 检查是否有新插件加入
    std::wstring searchPath = pluginDir + L"*.dll";
    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(searchPath.c_str(), &findData);
    
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                std::wstring filePath = pluginDir + findData.cFileName;
                if (plugins.find(filePath) == plugins.end()) {
                    LoadPlugin(filePath);
                }
            }
        } while (FindNextFileW(hFind, &findData));
        
        FindClose(hFind);
    }
}

const std::unordered_map<std::wstring, PluginInfo>& PluginReloader::GetPlugins() const {
    return plugins;
}

bool PluginReloader::ReloadPlugin(const std::wstring& pluginName) {
    std::wstring filePath = pluginDir + pluginName;
    return LoadPlugin(filePath);
}
