#include "ExamplePlugin.h"
#include <iostream>

ExamplePlugin::ExamplePlugin(uint32_t pluginVersion)
    : version(pluginVersion), counter(0) {
    std::cout << "ExamplePlugin v" << version << " created!" << std::endl;
}

uint32_t ExamplePlugin::GetVersion() const {
    return version;
}

void ExamplePlugin::Update(float deltaTime) {
    counter++;
    std::cout << "ExamplePlugin v" << version << ": Update called " << counter << " times (delta: " << deltaTime << "s)" << std::endl;
}

const char* ExamplePlugin::GetName() const {
    return "ExamplePlugin";
}

// 导出函数
extern "C" __declspec(dllexport) IPlugin* CreatePlugin() {
    return new ExamplePlugin(1);
}

extern "C" __declspec(dllexport) void DestroyPlugin(IPlugin* plugin) {
    delete plugin;
}
