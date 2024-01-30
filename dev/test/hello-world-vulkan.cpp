#define RAPID_VULKAN_IMPLEMENTATION
#include <rapid-vulkan/rapid-vulkan.h>

#define DETECTIVE_CONAN_IMPLEMENTATION
#include <detective-conan/detective-conan.h>

#include <iostream>
#include <fstream>

#include "shader/infinite.comp.spv.h"
#include "shader/noop.comp.spv.h"

using namespace RAPID_VULKAN_NAMESPACE;

int main() {
    auto instance  = Instance({.validation = Instance::Validation::LOG_ON_VK_ERROR});
    auto device    = Device({instance});
    auto gi        = device.gi();
    auto noop      = Shader(Shader::ConstructParameters {{"noop"}, gi}.setSpirv(noop_comp));
    auto infinite  = Shader(Shader::ConstructParameters {{"infinite"}, gi}.setSpirv(infinite_comp));
    auto pipeline1 = ComputePipeline({{"noop"}, &noop});
    auto pipeline2 = ComputePipeline({{"infinite"}, &infinite});

    detcon::vulkan::Contract contract;
    contract.allocator = gi->allocator;
    contract.instance  = gi->instance;
    contract.physical  = gi->physical;
    contract.device    = gi->device;
    auto detective     = detcon::vulkan::hire(contract);

    auto q = device.graphics();
    auto c = q->begin("main");
    detcon::vulkan::cmdInsertCheckpoint({detective, "checkpoint 1", c});
    pipeline1.cmdDispatch(c, {1, 1, 1});
    detcon::vulkan::cmdInsertCheckpoint({detective, "checkpoint 2", c});
    pipeline2.cmdDispatch(c, {1, 1, 1}); // this will hang GPU.
    detcon::vulkan::cmdInsertCheckpoint({detective, "checkpoint 3", c});
    printf("\nBe patient. We are generating an artificial hang on GPU. It could take a few seconds for the app to continue.\n\n");
    q->submit({c}).wait();

    auto report = investigate(detective);
    std::cout << report.content << std::endl;
    fire(detective);

    // done
    return 0;
}
