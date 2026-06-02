// -*- C++ -*-
//
// Copyright (C) 2020-2026  Shihan Zhao and contributors
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
Executor<T>::Executor(std::string_view dispatcher) :
    Executor{MakeCodedDispatcher<T>(dispatcher)} {}

template<std::integral T>
Executor<T>::Executor(std::string executionName, std::string opName, std::string taskName, std::string_view dispatcher) :
    Executor{std::move(executionName), std::move(opName), std::move(taskName), MakeCodedDispatcher<T>(dispatcher)} {}

template<std::integral T>
Executor<T>::Executor(std::unique_ptr<Dispatcher<T>> dispatcher) :
    Executor{"Run", "Execution", "task", std::move(dispatcher)} {}

template<std::integral T>
Executor<T>::Executor(std::string executionName, std::string opName, std::string taskName, std::unique_ptr<Dispatcher<T>> dispatcher) :
    fImpl{[&] {
        if (not mplr::available() or mplr::comm_world().size() == 1) {
            return std::make_unique<Impl>(
                std::in_place_type<impl::SequentialExecutorImpl<T>>,
                std::move(executionName), std::move(opName), std::move(taskName), std::move(dispatcher));
        }
        return std::make_unique<Impl>(
            std::in_place_type<impl::ParallelExecutorImpl<T>>,
            std::move(executionName), std::move(opName), std::move(taskName), std::move(dispatcher));
    }()} {}

template<std::integral T>
auto Executor<T>::SwitchDispatcher(std::string_view dispatcher) -> void {
    VisitImpl([&](auto&& impl) -> decltype(auto) {
        impl.SwitchDispatcher(dispatcher);
    });
}

template<std::integral T>
auto Executor<T>::SwitchDispatcher(std::unique_ptr<Dispatcher<T>> dispatcher) -> void {
    VisitImpl([&](auto&& impl) -> decltype(auto) {
        impl.SwitchDispatcher(std::move(dispatcher));
    });
}

template<std::integral T>
auto Executor<T>::NProcess() const -> int {
    return VisitImpl([&](auto&& impl) -> decltype(auto) {
        return impl.NProcess();
    });
}

template<std::integral T>
auto Executor<T>::Task() const -> struct Dispatcher<T>::Task {
    return VisitImpl([&](auto&& impl) -> decltype(auto) {
        return impl.Task();
    });
}
//
template<std::integral T>
auto Executor<T>::NTask() const -> T {
    return VisitImpl([&](auto&& impl) -> decltype(auto) {
        return impl.NTask();
    });
}

template<std::integral T>
auto Executor<T>::ExecutingTask() const -> T {
    return VisitImpl([&](auto&& impl) -> decltype(auto) {
        return impl.ExecutingTask();
    });
}

template<std::integral T>
auto Executor<T>::NLocalExecutedTask() const -> T {
    return VisitImpl([&](auto&& impl) -> decltype(auto) {
        return impl.NLocalExecutedTask();
    });
}

template<std::integral T>
auto Executor<T>::Executing() const -> bool {
    return VisitImpl([&](auto&& impl) -> decltype(auto) {
        return impl.Executing();
    });
}

template<std::integral T>
auto Executor<T>::PrintProgress() const -> bool {
    return VisitImpl([&](auto&& impl) -> decltype(auto) {
        return impl.PrintProgress();
    });
}

template<std::integral T>
auto Executor<T>::PrintProgressInterval() const -> muc::chrono::seconds<double> {
    return VisitImpl([&](auto&& impl) -> decltype(auto) {
        return impl.PrintProgressInterval();
    });
}

template<std::integral T>
auto Executor<T>::PrintProgress(bool print) -> void {
    VisitImpl([&](auto&& impl) -> decltype(auto) {
        impl.PrintProgress(print);
    });
}

template<std::integral T>
auto Executor<T>::PrintProgressInterval(muc::chrono::seconds<double> t) -> void {
    VisitImpl([&](auto&& impl) -> decltype(auto) {
        impl.PrintProgressInterval(t);
    });
}

template<std::integral T>
auto Executor<T>::ExecutionName() const -> const std::string& {
    return VisitImpl([&](auto&& impl) -> decltype(auto) {
        return impl.ExecutionName();
    });
}

template<std::integral T>
auto Executor<T>::ExecutionName(std::string name) -> void {
    VisitImpl([&](auto&& impl) -> decltype(auto) {
        impl.ExecutionName(std::move(name));
    });
}

template<std::integral T>
auto Executor<T>::OperationName() const -> const std::string& {
    return VisitImpl([&](auto&& impl) -> decltype(auto) {
        return impl.OperationName();
    });
}

template<std::integral T>
auto Executor<T>::OperationName(std::string name) -> void {
    VisitImpl([&](auto&& impl) -> decltype(auto) {
        impl.OperationName(std::move(name));
    });
}

template<std::integral T>
auto Executor<T>::TaskName() const -> const std::string& {
    return VisitImpl([&](auto&& impl) -> decltype(auto) {
        return impl.TaskName();
    });
}

template<std::integral T>
auto Executor<T>::TaskName(std::string name) -> void {
    VisitImpl([&](auto&& impl) -> decltype(auto) {
        impl.TaskName(std::move(name));
    });
}

template<std::integral T>
auto Executor<T>::Run(struct Dispatcher<T>::Task task, std::invocable<T> auto&& F) -> T {
    return VisitImpl([&](auto&& impl) -> decltype(auto) {
        return impl.Run(std::move(task), std::forward<decltype(F)>(F));
    });
}

template<std::integral T>
auto Executor<T>::Run(T size, std::invocable<T> auto&& F) -> T {
    return Run({0, size}, std::forward<decltype(F)>(F));
}

template<std::integral T>
auto Executor<T>::ExecutionInfo() const -> const ExecutionInfoType& {
    return VisitImpl([&](auto&& impl) -> decltype(auto) {
        return impl.ExecutionInfo();
    });
}

template<std::integral T>
auto Executor<T>::PrintExecutionSummary() const -> void {
    VisitImpl([&](auto&& impl) -> decltype(auto) {
        impl.PrintExecutionSummary();
    });
}

template<std::integral T>
template<typename F>
auto Executor<T>::VisitImpl(F&& visitor) const -> decltype(auto) {
    return std::visit(std::forward<F>(visitor), *fImpl);
}

template<std::integral T>
template<typename F>
auto Executor<T>::VisitImpl(F&& visitor) -> decltype(auto) {
    return std::visit(std::forward<F>(visitor), *fImpl);
}

} // namespace Mustard::inline Execution
