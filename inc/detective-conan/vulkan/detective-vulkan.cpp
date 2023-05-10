#ifndef DETECTIVE_CONAN_SUPPORT_VULKAN
    #define DETECTIVE_CONAN_SUPPORT_VULKAN
#endif
#include "../detective-conan.h"
#include "asset/atomic-counter.comp.spv.h"
#include <sstream>
#include <iomanip>

namespace DETECTIVE_CONAN_NAMESPACE {

struct PushConstant {
    uint32_t checkpoint;
};

class DetectiveVulkan : public DetectiveConan {
public:
    DetectiveVulkan(const VulkanContract &);
    ~DetectiveVulkan() override {}

    void insertCheckpoint(const CheckPointInfo &) override;
    void fillReport(Report &) override;

private:
    inline constexpr static uint64_t BUFFER_SIZE = 4; // 32-bit

    vk::Result                    _error = vk::Result::eSuccess;
    VulkanContract                _contract;
    vk::UniqueDescriptorSetLayout _descriptorSetLayout;
    vk::UniquePipelineLayout      _pipelineLayout;
    vk::UniquePipeline            _pipeline;
    vk::UniqueBuffer              _buffer;
    vk::UniqueDeviceMemory        _memory;
    vk::UniqueDescriptorPool      _pool;
    vk::DescriptorSet             _set;
    uint32_t                      _nextCheckpoint = 1;
    uint32_t                      _finished       = 0;

    void allocatePipeline();
    void allocateBuffer();
    void allocateDescriptorPool();
    void allocateDescriptorSet();
};

// ---------------------------------------------------------------------------------------------------------------------
//
DetectiveVulkan::DetectiveVulkan(const VulkanContract & contract): _contract(contract) {
    allocatePipeline();
    allocateBuffer();
    allocateDescriptorPool();
    allocateDescriptorSet();
}

// ---------------------------------------------------------------------------------------------------------------------
//
void DetectiveVulkan::fillReport(Report & r) {
    if (_error != vk::Result::eSuccess) {
        r.content = std::string("DetectiveVulkan internal error: ") + std::to_string((int) _error);
    } else {
        _finished = *(const uint32_t *) _contract.device.mapMemory(_memory.get(), 0, VK_WHOLE_SIZE, vk::MemoryMapFlags {});
        _contract.device.unmapMemory(_memory.get());

        auto submitted = _nextCheckpoint - 1;
        auto ss        = std::stringstream();
        ss << "Last finished  checkpoint = 0x" << std::hex << _finished << std::endl;
        ss << "Last submitted checkpoint = 0x" << std::hex << submitted << std::endl;
        if (submitted == _finished) {
            ss << "Looks all good!" << std::endl;
        } else {
            ss << "[ERROR] GPU hang found between checkpoint #" << _finished << " and "
               << "#" << submitted << std::endl;
        }
        r.content = ss.str();
    }
}

// ---------------------------------------------------------------------------------------------------------------------
//
void DetectiveVulkan::insertCheckpoint(const CheckPointInfo & cpi) {
    if (_error != vk::Result::eSuccess) return;
    try {
        auto info = (const VulkanCheckPointInfo *) &cpi;
        auto cb   = info->commandBuffer;

        {
            auto bmb = vk::BufferMemoryBarrier()
                           .setSrcAccessMask(vk::AccessFlagBits::eShaderWrite)
                           .setDstAccessMask(vk::AccessFlagBits::eShaderRead)
                           .setBuffer(_buffer.get())
                           .setOffset(0)
                           .setSize(BUFFER_SIZE);
            cb.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eComputeShader, {}, {}, {bmb}, {});
        }

        // ensure previous compute shader works are complete done.
        {
            auto access =
                vk::AccessFlagBits::eShaderWrite | vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eMemoryRead | vk::AccessFlagBits::eMemoryWrite;
            auto mb = vk::MemoryBarrier().setSrcAccessMask(access).setDstAccessMask(access);
            cb.pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlagBits::eAllCommands, {}, {mb}, {}, {});
        }

        auto ps = PushConstant {_nextCheckpoint++};
        cb.bindPipeline(vk::PipelineBindPoint::eCompute, _pipeline.get());
        cb.bindDescriptorSets(vk::PipelineBindPoint::eCompute, _pipelineLayout.get(), 0, {_set}, {});
        cb.pushConstants(_pipelineLayout.get(), vk::ShaderStageFlagBits::eCompute, 0, sizeof(ps), &ps);
        cb.dispatch(1, 1, 1);
    } catch (...) {
        // do nothing.
    }
}

// ---------------------------------------------------------------------------------------------------------------------
//
void DetectiveVulkan::allocatePipeline() {
    // load compute shader module
    using namespace std::string_literals;
    auto shader = _contract.device.createShaderModuleUnique(
        vk::ShaderModuleCreateInfo().setCodeSize(sizeof(atomic_counter_comp)).setPCode((const uint32_t *) atomic_counter_comp));
    auto stage = vk::PipelineShaderStageCreateInfo().setStage(vk::ShaderStageFlagBits::eCompute).setModule(shader.get()).setPName("main");

    // create a pipeline layout that binds a storage buffer to compute shader
    auto descriptorSetLayoutBinding = vk::DescriptorSetLayoutBinding()
                                          .setBinding(0)
                                          .setDescriptorType(vk::DescriptorType::eStorageBuffer)
                                          .setDescriptorCount(1)
                                          .setStageFlags(vk::ShaderStageFlagBits::eCompute);
    _descriptorSetLayout =
        _contract.device.createDescriptorSetLayoutUnique(vk::DescriptorSetLayoutCreateInfo().setBindingCount(1).setPBindings(&descriptorSetLayoutBinding));
    _pipelineLayout = _contract.device.createPipelineLayoutUnique(
        vk::PipelineLayoutCreateInfo()
            .setSetLayouts({_descriptorSetLayout.get()})
            .setPushConstantRangeCount(1)
            .setPPushConstantRanges(&(vk::PushConstantRange().setStageFlags(vk::ShaderStageFlagBits::eCompute).setOffset(0).setSize(sizeof(PushConstant)))));

    // create the compute pipeline object
    _pipeline =
        _contract.device.createComputePipelineUnique(vk::PipelineCache(), vk::ComputePipelineCreateInfo().setLayout(_pipelineLayout.get()).setStage(stage))
            .value;
}

// ---------------------------------------------------------------------------------------------------------------------
//
void DetectiveVulkan::allocateDescriptorPool() {
    auto poolSize = vk::DescriptorPoolSize().setType(vk::DescriptorType::eStorageBuffer).setDescriptorCount(1);
    _pool         = _contract.device.createDescriptorPoolUnique(vk::DescriptorPoolCreateInfo().setMaxSets(1024).setPoolSizeCount(1).setPPoolSizes(&poolSize));
}

// ---------------------------------------------------------------------------------------------------------------------
//
void DetectiveVulkan::allocateDescriptorSet() {
    auto allocInfo = vk::DescriptorSetAllocateInfo().setDescriptorPool(_pool.get()).setSetLayouts({_descriptorSetLayout.get()});
    _set           = _contract.device.allocateDescriptorSets(allocInfo)[0];
    auto bi        = vk::DescriptorBufferInfo().setBuffer(_buffer.get()).setOffset(0).setRange(BUFFER_SIZE);
    _contract.device.updateDescriptorSets({vk::WriteDescriptorSet()
                                               .setDstSet(_set)
                                               .setDstBinding(0)
                                               .setDescriptorCount(1)
                                               .setDescriptorType(vk::DescriptorType::eStorageBuffer)
                                               .setBufferInfo({bi})},
                                          {});
}

// ---------------------------------------------------------------------------------------------------------------------
/// TODO: Defer to VMA for memory allocations if it's enabled
vk::UniqueDeviceMemory allocateDeviceMemory(const VulkanContract & c, const vk::MemoryRequirements & memRequirements, vk::MemoryPropertyFlags memoryProperties,
                                            const vk::MemoryAllocateFlags allocFlags) {
    vk::PhysicalDeviceMemoryProperties memProperties;
    c.physical.getMemoryProperties(&memProperties);

    uint32_t memoryIndex = UINT_MAX;
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if (0 == (memRequirements.memoryTypeBits & (1 << i))) continue;
        auto propFlags = memProperties.memoryTypes[i].propertyFlags;
        if (memoryProperties != (propFlags & memoryProperties)) continue;
        // found the memory type we need
        memoryIndex = i;
        break;
    }
    if (memoryIndex > memProperties.memoryTypeCount) {
        // ERROR("Can't find a memory type that supports the required memory usage.");
        return {};
    }

    auto afi           = vk::MemoryAllocateFlagsInfo {allocFlags};
    auto ai            = vk::MemoryAllocateInfo();
    ai.allocationSize  = memRequirements.size;
    ai.memoryTypeIndex = memoryIndex;
    ai.pNext           = allocFlags ? &afi : nullptr;

    return c.device.allocateMemoryUnique(ai);
}

// ---------------------------------------------------------------------------------------------------------------------
//
void DetectiveVulkan::allocateBuffer() {
    // allocate a new buffer
    _buffer = _contract.device.createBufferUnique(vk::BufferCreateInfo().setSize(BUFFER_SIZE).setUsage(vk::BufferUsageFlagBits::eStorageBuffer));
    _memory = allocateDeviceMemory(_contract, _contract.device.getBufferMemoryRequirements(_buffer.get()),
                                   vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, {});
    _contract.device.bindBufferMemory(_buffer.get(), _memory.get(), 0);

    // Initialize the data in the storage buffer
    uint32_t initialData = UINT32_MAX;
    auto     mappedData  = _contract.device.mapMemory(_memory.get(), 0, VK_WHOLE_SIZE, vk::MemoryMapFlags {});
    memcpy(mappedData, &initialData, sizeof(initialData));
    _contract.device.unmapMemory(_memory.get());
}

// ---------------------------------------------------------------------------------------------------------------------
//
DetectiveConan * hireVulkan(const VulkanContract & c) { return team.hire(std::make_shared<dc::DetectiveVulkan>(c)).get(); }

// ---------------------------------------------------------------------------------------------------------------------
//
void insertVulkanCheckpoint(const VulkanCheckPointInfo & cpi) {
    auto p = team.validate(cpi.detective);
    if (!p) return;
    return p->insertCheckpoint(cpi);
}

} // namespace DETECTIVE_CONAN_NAMESPACE