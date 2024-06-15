#pragma once

#include "Mustard/Env/MPIEnv.h++"
#include "Mustard/Extension/MPIX/Execution/Scheduler.h++"

#include <concepts>
#include <utility>

namespace Mustard::inline Extension::MPIX::inline Execution {

template<std::integral T>
class StaticScheduler : public Scheduler<T> {
    virtual auto PreLoopAction() -> void override;
    virtual auto PreTaskAction() -> void override {}
    virtual auto PostTaskAction() -> void override;
    virtual auto PostLoopAction() -> void override {}

    virtual auto NExecutedTask() const -> std::pair<bool, T> override;
};

} // namespace Mustard::inline Extension::MPIX::inline Execution

#include "Mustard/Extension/MPIX/Execution/StaticScheduler.inl"
