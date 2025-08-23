// -*- C++ -*-
//
// Copyright 2020-2024  The Mustard development team
//
// This file is part of Mustard, an offline software framework for HEP experiments.
//
// Mustard is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// Mustard is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// Mustard. If not, see <https://www.gnu.org/licenses/>.

namespace Mustard::inline Execution {

template<std::integral T>
Executor<T>::Executor(std::string_view scheduler) :
    Executor{MakeCodedScheduler<T>(scheduler)} {}

template<std::integral T>
Executor<T>::Executor(std::string executionName, std::string taskName, std::string_view scheduler) :
    Executor{std::move(executionName), std::move(taskName), MakeCodedScheduler<T>(scheduler)} {}

template<std::integral T>
Executor<T>::Executor(std::unique_ptr<Scheduler<T>> scheduler) :
    Executor{"Execution", "Task", std::move(scheduler)} {}

template<std::integral T>
Executor<T>::Executor(std::string executionName, std::string taskName, std::unique_ptr<Scheduler<T>> scheduler) :
    fImpl{[&] {
        if (not mplr::available() or mplr::comm_world().size() == 1) {
            return std::make_unique<Impl>(
                std::in_place_type<internal::SequentialExecutorImpl<T>>,
                std::move(executionName), std::move(taskName), std::move(scheduler));
        }
        return std::make_unique<Impl>(
            std::in_place_type<internal::ParallelExecutorImpl<T>>,
            std::move(executionName), std::move(taskName), std::move(scheduler));
    }()} {}

template<std::integral T>
auto Executor<T>::SwitchScheduler(std::string_view scheduler) -> void {
    std::visit([&](auto&& impl) {
        impl.SwitchScheduler(scheduler);
    },
               *fImpl);
}

template<std::integral T>
auto Executor<T>::SwitchScheduler(std::unique_ptr<Scheduler<T>> scheduler) -> void {
    std::visit([&](auto&& impl) {
        impl.SwitchScheduler(std::move(scheduler));
    },
               *fImpl);
}

template<std::integral T>
auto Executor<T>::NProcess() const -> int {
    return std::visit([&](auto&& impl) {
        return impl.NProcess();
    },
                      *fImpl);
}

template<std::integral T>
auto Executor<T>::Task() const -> struct Scheduler<T>::Task {
    return std::visit([&](auto&& impl) {
        return impl.Task();
    },
                      *fImpl);
}
//
template<std::integral T>
auto Executor<T>::NTask() const -> T {
    return std::visit([&](auto&& impl) {
        return impl.NTask();
    },
                      *fImpl);
}

template<std::integral T>
auto Executor<T>::ExecutingTask() const -> T {
    return std::visit([&](auto&& impl) {
        return impl.ExecutingTask();
    },
                      *fImpl);
}

template<std::integral T>
auto Executor<T>::NLocalExecutedTask() const -> T {
    return std::visit([&](auto&& impl) {
        return impl.NLocalExecutedTask();
    },
                      *fImpl);
}

template<std::integral T>
auto Executor<T>::Executing() const -> bool {
    return std::visit([&](auto&& impl) {
        return impl.Executing();
    },
                      *fImpl);
}

template<std::integral T>
auto Executor<T>::PrintProgress(bool print) -> void {
    std::visit([&](auto&& impl) {
        impl.PrintProgress(print);
    },
               *fImpl);
}

template<std::integral T>
auto Executor<T>::PrintProgressInterval(muc::chrono::seconds<double> t) -> void {
    std::visit([&](auto&& impl) {
        impl.PrintProgressInterval(t);
    },
               *fImpl);
}

template<std::integral T>
auto Executor<T>::ExecutionName() const -> const std::string& {
    return std::visit([&](auto&& impl) -> const auto& {
        return impl.ExecutionName();
    },
                      *fImpl);
}

template<std::integral T>
auto Executor<T>::TaskName() const -> const std::string& {
    return std::visit([&](auto&& impl) -> const auto& {
        return impl.TaskName();
    },
                      *fImpl);
}

template<std::integral T>
auto Executor<T>::ExecutionName(std::string name) -> void {
    std::visit([&](auto&& impl) {
        impl.ExecutionName(std::move(name));
    },
               *fImpl);
}

template<std::integral T>
auto Executor<T>::TaskName(std::string name) -> void {
    std::visit([&](auto&& impl) {
        impl.TaskName(std::move(name));
    },
               *fImpl);
}

template<std::integral T>
auto Executor<T>::operator()(struct Scheduler<T>::Task task, std::invocable<T> auto&& F) -> T {
    return std::visit([&](auto&& impl) {
        return impl(std::move(task), std::forward<decltype(F)>(F));
    },
                      *fImpl);
}

template<std::integral T>
auto Executor<T>::operator()(T size, std::invocable<T> auto&& F) -> T {
    return (*this)({0, size}, std::forward<decltype(F)>(F));
}

template<std::integral T>
auto Executor<T>::PrintExecutionSummary() const -> void {
    std::visit([&](auto&& impl) {
        impl.PrintExecutionSummary();
    },
               *fImpl);
}

} // namespace Mustard::inline Execution
