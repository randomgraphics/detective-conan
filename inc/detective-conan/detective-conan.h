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

/// \def DETECTIVE_CONAN_NAMESPACE
/// \brief The namespace that all the symbols of Detective Conan are defined in. Default value is `detcon`.
#ifndef DETECTIVE_CONAN_NAMESPACE
#define DETECTIVE_CONAN_NAMESPACE detcon
#endif

#include <stdint.h>

#ifdef DETECTIVE_CONAN_SUPPORT_VULKAN
#include <vulkan/vulkan.hpp>
#endif

namespace DETECTIVE_CONAN_NAMESPACE {

struct DetectiveConan;

struct Report {
    std::string content;
};

#ifdef DETECTIVE_CONAN_SUPPORT_VULKAN

namespace vulkan {

struct Contract {
    const vk::AllocationCallbacks * allocator {};
    vk::Instance                    instance {};
    vk::PhysicalDevice              physical {};
    vk::Device                      device {};
};

struct CheckPointInfo {
    /// @brief The detective who is responsible for this check point.
    DetectiveConan * detective;

    /// @brief Name of the check point. This name will be used in the report.
    const char * name;

    /// @brief The command buffer that the check point is inserted into.
    vk::CommandBuffer commandBuffer {};
};

DetectiveConan * hire(const Contract &);

void cmdInsertCheckpoint(const CheckPointInfo &);

} // namespace vulkan

#endif

#ifdef DETECTIVE_CONAN_SUPPORT_D3D12

namespace d3d12 {

struct Contract {
    /// @brief The detective who is responsible for this check point.
    DetectiveConan * detective;

    /// @brief Name of the check point. This name will be used in the report.
    const char * name;
};

DetectiveConan * hire(const Contract *);

} // namespace d3d12

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
