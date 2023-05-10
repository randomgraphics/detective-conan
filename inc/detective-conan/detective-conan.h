#pragma once

#define DETECTIVE_CONAN_VERSION_MAJOR 0
#define DETECTIVE_CONAN_VERSION_MINOR 1
#define DETECTIVE_CONAN_VERSION_MICRO 0
#define DETECTIVE_CONAN_VERSION       (DETECTIVE_CONAN_VERSION_MAJOR * 1000000 + DETECTIVE_CONAN_VERSION_MINOR * 1000 + DETECTIVE_CONAN_VERSION_MICRO)

#ifndef DETECTIVE_CONAN_NAMESPACE
    #define DETECTIVE_CONAN_NAMESPACE dc
#endif

#define DETECTIVE_CONAN_IMPLEMENTATION

#include <stdint.h>

#ifdef DETECTIVE_CONAN_SUPPORT_VULKAN
    #include <vulkan/vulkan.hpp>
#endif

namespace DETECTIVE_CONAN_NAMESPACE {

struct DetectiveConan;

struct CheckPointInfo {
    /// The detective who is responsible for this check point.
    DetectiveConan * detective;

    /// Name of the check point. This name will be used in the report.
    const char * name;
};

struct Report {
    std::string content;
};

#ifdef DETECTIVE_CONAN_SUPPORT_VULKAN

struct VulkanContract {
    const vk::AllocationCallbacks * allocator {};
    vk::Instance                    instance {};
    vk::PhysicalDevice              physical {};
    vk::Device                      device {};
};

struct VulkanCheckPointInfo : public CheckPointInfo {
    vk::CommandBuffer commandBuffer {};
};

DetectiveConan * hireVulkan(const VulkanContract &);

void insertVulkanCheckpoint(const VulkanCheckPointInfo &);

#endif

#ifdef DETECTIVE_CONAN_SUPPORT_D3D12

struct DCD3D12Contract {
    void * placeholder;
};

DetectiveConan * hireInsertD3D12(const D3D12Contract *);

#endif

Report investigate(DetectiveConan *);

/// End the case. Fire the detective.
void fire(DetectiveConan *);

} // namespace DETECTIVE_CONAN_NAMESPACE

#ifdef DETECTIVE_CONAN_IMPLEMENTATION
    #undef DETECTIVE_CONAN_IMPLEMENTATION
    #include "base/base.cpp"

    #ifdef DETECTIVE_CONAN_SUPPORT_VULKAN
        #include "vulkan/detective-vulkan.cpp"
    #endif

    #ifdef DETECTIVE_CONAN_SUPPORT_D3D12
        #include "d3d12/detective-d3d12.cpp"
    #endif
#endif
