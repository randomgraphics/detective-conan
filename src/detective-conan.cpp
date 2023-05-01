#include "detective-conan.h"

#if DC_IMPL_IS_VULKAN

DetectiveConan * hireVulkanDetective(const VulkanContract *) { return nullptr; }

#elif DC_IMPL_IS_D3D12

DetectiveConan * hireD3D12Detective(const D3D12Contract *) { return nullptr; }

#endif

void fire(DetectiveConan *) {}

void checkpoint(const CheckPointInfo *) {}

Report * investigate(const DetectiveConan *) { return nullptr; }

void closeReport(Report *) {}