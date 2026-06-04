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
auto MakeCodedDispatcher(std::string_view dispatcher) -> std::unique_ptr<Dispatcher<T>> {
    static const gtl::flat_hash_map<std::string_view, std::function<auto()->std::unique_ptr<Dispatcher<T>>>> dispatcherMap{
        {"cmw",  [] { return std::make_unique<ClusterAwareMasterWorkerDispatcher<T>>(); }},
        {"mw",   [] { return std::make_unique<MasterWorkerDispatcher<T>>(); }            },
        {"seq",  [] { return std::make_unique<SequentialDispatcher<T>>(); }              },
        {"shm",  [] { return std::make_unique<SharedMemoryDispatcher<T>>(); }            },
        {"stat", [] { return std::make_unique<StaticDispatcher<T>>(); }                  }
    };
    try {
        return dispatcherMap.at(dispatcher)();
    } catch (const std::out_of_range&) {
        std::vector<std::string_view> available(dispatcherMap.size());
        std::ranges::transform(dispatcherMap, available.begin(), [](auto&& s) { return s.first; });
        muc::timsort(available);
        Throw<std::out_of_range>(fmt::format("Dispatcher '{}' not found, available are {}.", dispatcher, available));
    }
}

template<std::integral T>
auto MakeDefaultDispatcher() -> std::unique_ptr<Dispatcher<T>> {
    return MakeCodedDispatcher<T>(DefaultDispatcherCode());
}

} // namespace Mustard::inline Execution
