#include <string>
#include <set>
#include <mutex>
#include <cassert>

namespace DETECTIVE_CONAN_NAMESPACE {

struct DetectiveConan {
    virtual ~DetectiveConan() {}
    virtual void report(Report &) = 0;
};

struct TeamOnDuty {
    std::mutex                                mutex;
    std::set<std::shared_ptr<DetectiveConan>> detectives;

    std::shared_ptr<DetectiveConan> hire(std::shared_ptr<DetectiveConan> p) {
        try {
            auto lock = std::lock_guard {mutex};
            assert(detectives.find(p) == detectives.end());
            auto detective = std::shared_ptr<DetectiveConan>(p);
            detectives.insert(detective);
            return detective;
        } catch (...) { return nullptr; }
    }

    std::shared_ptr<DetectiveConan> validate(DetectiveConan * p) {
        auto lock = std::lock_guard {mutex};
        auto iter = std::find_if(detectives.begin(), detectives.end(), [p](auto c) { return c.get() == p; });
        if (iter == detectives.end()) return nullptr;
        return *iter;
    }

    Report investigate(DetectiveConan * p) {
        auto d = validate(p);
        if (!d) return {"Not the detective that I hired."};
        Report r;
        p->report(r);
        return r;
    }

    void fire(DetectiveConan * p) {
        auto lock = std::lock_guard {mutex};
        auto iter = std::find_if(detectives.begin(), detectives.end(), [p](auto c) { return c.get() == p; });
        if (iter != detectives.end()) detectives.erase(iter);
    }
};
static TeamOnDuty team;

Report investigate(DetectiveConan * p) { return team.investigate(p); }

void fire(DetectiveConan * p) { team.fire(p); }

} // namespace DETECTIVE_CONAN_NAMESPACE