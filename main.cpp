#include "PluginReloader.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <conio.h>

int main() {
    std::cout << "=== C++ Function-Level Hot Reload Demo ===" << std::endl;
    std::cout << "1. Edit plugins/example.cpp and recompile to see hot reload" << std::endl;
    std::cout << "2. Press any key to exit" << std::endl << std::endl;
    
    // 创建插件重载器
    PluginReloader reloader(L"plugins/");
    
    if (!reloader.Initialize()) {
        std::cerr << "Failed to initialize plugin reloader" << std::endl;
        return 1;
    }
    
    // 主循环
    while (!_kbhit()) {
        reloader.Update();
        
        // 打印插件状态
        auto& plugins = reloader.GetPlugins();
        for (const auto& pair : plugins) {
            if (pair.second.pluginInstance) {
                std::cout << "[" << pair.second.pluginInstance->GetName() << "] Version: " << pair.second.version << std::endl;
            }
        }
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
        system("cls");
    }
    
    return 0;
}
