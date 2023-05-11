#ifndef NDEBUG
    #define RAPID_VULKAN_ENABLE_DEBUG_BUILD 1
#endif
#define RAPID_VULKAN_IMPLEMENTATION
#define RAPID_VULKAN_ENABLE_LOADER 1
#include <rapid-vulkan/rapid-vulkan.h>

#define DETECTIVE_CONAN_IMPLEMENTATION
#define DETECTIVE_CONAN_SUPPORT_VULKAN 1
#include <detective-conan/detective-conan.h>

#include <iostream>
#include <fstream>

#include "shader/infinite.comp.spv.h"
#include "shader/noop.comp.spv.h"

using namespace RAPID_VULKAN_NAMESPACE;

int main() {
    auto instance  = Instance({});
    auto device    = Device({instance});
    auto gi        = device.gi();
    auto noop      = Shader({"noop", gi, {sizeof(noop_comp) / sizeof(uint32_t), (const uint32_t *) noop_comp}});
    auto infinite  = Shader({"infinite", gi, {sizeof(infinite_comp) / sizeof(uint32_t), (const uint32_t *) infinite_comp}});
    auto pipeline1 = ComputePipeline({"noop", noop});
    auto pipeline2 = ComputePipeline({"infinite", infinite});

    dc::VulkanContract contract;
    contract.allocator = gi->allocator;
    contract.instance  = gi->instance;
    contract.physical  = gi->physical;
    contract.device    = gi->device;
    auto detective     = dc::hireVulkan(contract);

    auto q = rapid_vulkan::CommandQueue({"main", gi, device.graphics()->family(), device.graphics()->index()});
    auto c = q.begin("main");
    dc::cmdInsertVulkanCheckpoint({detective, "checkpoint 1", c->handle()});
    pipeline1.cmdDispatch(c->handle(), {1, 1, 1});
    dc::cmdInsertVulkanCheckpoint({detective, "checkpoint 2", c->handle()});
    pipeline2.cmdDispatch(c->handle(), {1, 1, 1}); // this will hang GPU.
    dc::cmdInsertVulkanCheckpoint({detective, "checkpoint 3", c->handle()});
    q.submit(c);
    q.wait();

    auto report = investigate(detective);
    std::cout << report.content << std::endl;
    fire(detective);

    // done
    return 0;
}
