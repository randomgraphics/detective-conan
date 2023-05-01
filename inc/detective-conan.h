#pragma once

#define DETECTIVE_CONAN_VERSION_MAJOR 0
#define DETECTIVE_CONAN_VERSION_MINOR 1
#define DETECTIVE_CONAN_VERSION_MICRO 0
#define DETECTIVE_CONAN_VERSION       (DETECTIVE_CONAN_VERSION_MAJOR * 1000000 + DETECTIVE_CONAN_VERSION_MINOR * 1000 + DETECTIVE_CONAN_VERSION_MICRO)

#ifndef DETECTIVE_CONAN_GRAPHICS_API
    #error "DETECTIVE_CONAN_GRAPHICS_API must be defined before including detective-conan.h. Supported APIs are: Vulkan, D3D12."
#endif

#include <stdint.h>

#ifdef _MSC_VER
    #define DC_API extern "C" __declspec(dllexport)
#elif __GNUC__
    #define DC_API extern "C" __attribute__((visibility("default")))
#endif

#define DC_IMPL_API_VULKAN 1
#define DC_IMPL_API_D3D12  2
#define DC_IMPL_IS_VULKAN  (DC_IMPL_API##DETECTIVE_CONAN_GRAPHICS_API == DC_IMPL_API_VULKAN)
#define DC_IMPL_IS_D3D12   (DC_IMPL_API##DETECTIVE_CONAN_GRAPHICS_API == DC_IMPL_API_D3D12)

struct DetectiveConan;

struct CheckPointInfo {
    DetectiveConan * who;
    const char *     why;
};

struct Report {
    const char * details;
    size_t       size; ///< length of the details in bytes. Not including the null terminator.
};

#if DC_IMPL_IS_VULKAN

struct VulkanContract {
    const VkAllocationCallbacks * allocator;
    VkInstance                    instance;
    VkPhysicalDevice              physical;
    VkDevice                      device;
    VmaAllocator                  vma;
};

DC_API DetectiveConan * hireVulkanDetective(const VulkanContract *);

#elif DC_IMPL_IS_D3D12

struct D3D12Contract {
    void * placeholder;
};

DC_API DetectiveConan * hireD3D12Detective(const D3D12Contract *);

#endif

/// End the case. Fire the detective. Also close any open reports.
DC_API void fire(DetectiveConan *);

DC_API void checkpoint(const CheckPointInfo *);

DC_API Report * investigate(const DetectiveConan *);

DC_API void closeReport(Report *);