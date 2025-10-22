// -*- C++ -*-
//
// Copyright (C) 2020-2025  Mustard developers
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
auto MakeCodedScheduler(std::string_view scheduler) -> std::unique_ptr<Scheduler<T>> {
    static const muc::flat_hash_map<std::string_view, std::function<auto()->std::unique_ptr<Scheduler<T>>>> schedulerMap{
        {"clmw", [] { return std::make_unique<ClusterAwareMasterWorkerScheduler<T>>(); }},
        {"mw",   [] { return std::make_unique<MasterWorkerScheduler<T>>(); }            },
        {"seq",  [] { return std::make_unique<SequentialScheduler<T>>(); }              },
        {"shm",  [] { return std::make_unique<SharedMemoryScheduler<T>>(); }            },
        {"stat", [] { return std::make_unique<StaticScheduler<T>>(); }                  }
    };
    try {
        return schedulerMap.at(scheduler)();
    } catch (const std::out_of_range&) {
        std::vector<std::string_view> available(schedulerMap.size());
        std::ranges::transform(schedulerMap, available.begin(), [](auto&& s) { return s.first; });
        muc::timsort(available);
        Throw<std::out_of_range>(fmt::format("Scheduler '{}' not found, available are {}", scheduler, available));
    }
}

template<std::integral T>
auto MakeDefaultScheduler() -> std::unique_ptr<Scheduler<T>> {
    return MakeCodedScheduler<T>(DefaultSchedulerCode());
}

} // namespace Mustard::inline Execution
