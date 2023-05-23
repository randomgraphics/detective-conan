/*
MIT License

Copyright (c) 2023 randomgraphics

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#ifndef DETECTIVE_CONAN_H__
#define DETECTIVE_CONAN_H__

/// The revision number that uniquely identifies the version of Detective Conan.
#define DETECTIVE_CONAN_HEADER_REVISION 1

#ifndef DETECTIVE_CONAN_NAMESPACE
#define DETECTIVE_CONAN_NAMESPACE dc
#endif

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

void cmdInsertVulkanCheckpoint(const VulkanCheckPointInfo &);

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

#endif // DETECTIVE_CONAN_H__

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
