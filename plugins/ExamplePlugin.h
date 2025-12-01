#pragma once
#include "../IPlugin.h"

class ExamplePlugin : public IPlugin {
private:
    uint32_t version;
    int counter;
    
public:
    ExamplePlugin(uint32_t pluginVersion = 1);
    ~ExamplePlugin() override = default;
    
    uint32_t GetVersion() const override;
    void Update(float deltaTime) override;
    const char* GetName() const override;
};
