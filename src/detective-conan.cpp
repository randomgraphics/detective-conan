#include "base.h"
#include <set>
#include <mutex>
#include <cassert>

struct TeamOnDuty {
    std::mutex                                mutex;
    std::set<std::shared_ptr<DetectiveConan>> detectives;
    std::set<std::shared_ptr<ReportImpl>>     reports;

    std::shared_ptr<DetectiveConan> hire(std::shared_ptr<DetectiveConan> p) {
        try {
            auto lock = std::lock_guard(mutex);
            assert(std::find_if(detectives.begin(), detectives.end(), [p](auto c) { return c.get() == p }) == detectives.end());
            auto detective = std::shared_ptr<DetectiveConan>(p);
            detectives.insert(detective);
            return detective;
        } catch (...) { return nullptr; }
    }

    std::shared_ptr<DetectiveConan> validate(DetectiveConan * p) {
        auto lock = std::lock_guard(mutex);
        auto iter = std::find_if(detectives.begin(), detectives.end(), [p](auto c) { return c.get() == p });
        if (iter == detectives.end()) return nullptr;
        return *iter;
    }

    void fire(DetectiveConan * p) {
        auto lock = std::lock_guard(mutex);
        auto iter = std::find_if(detectives.begin(), detectives.end(), [p](auto c) { return c.get() == p });
        if (iter != detectives.end()) detectives.erase(iter);
    }

    Report * openReport(DetectiveConan * p) {
        auto lock = std::lock_guard(mutex);
        auto d    = team.validate(p);
        if (d) return;
        auto report = std::make_shared<ReportImpl>();
        p->fillReport(*report);
        report->details = report->content.c_str();
        report->size    = report->content.size();
        reports.insert(report);
        return report.get();
    }

    void closeReport(ReportImpl * p) {
        auto lock = std::lock_guard(mutex);
        auto iter = std::find_if(reports.begin(), reports.end(), [p](auto c) { return c.get() == p });
        if (iter != reports.end()) reports.erase(iter);
    }
};
static TeamOnDuty team;

#if DC_IMPL_IS_VULKAN

    #include "detective-vulkan.h"

DetectiveConan * hireVulkanDetective(const VulkanContract * c) { return team.hire(std::make_shared<DetectiveVulkan>(c)); }

#elif DC_IMPL_IS_D3D12

    #include "detective-d3d12.h"

DetectiveConan * hireD3D12Detective(const D3D12Contract *) {
    // not implemented.
    return nullptr;
}

#endif

void fire(DetectiveConan * p) { team.fire(p); }

void checkpoint(const CheckPointInfo * cpi) {
    if (!cpi) return;
    auto p = team.validate(cpi->who);
    if (!p) return;
}

Report * investigate(DetectiveConan * p) { return team.openReport(p); }

void closeReport(Report * p) { team.closeReport((ReportImpl *) p); }