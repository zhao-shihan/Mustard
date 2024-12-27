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

namespace Mustard::Env::Memory {

template<typename ADerived>
PassiveSingleton<ADerived>::PassiveSingleton(ADerived* self) :
    PassiveSingletonBase{},
    Base{self} {
    static_assert(PassiveSingletonified<ADerived>);
}

template<typename ADerived>
MUSTARD_ALWAYS_INLINE auto PassiveSingleton<ADerived>::Instance() -> ADerived& {
    switch (Base::UpdateInstance()) {
    [[unlikely]] case Base::Status::NotInstantiated:
        Throw<std::logic_error>(fmt::format("{} (passive singleton in environment) has not been instantiated",
                                            muc::try_demangle(typeid(ADerived).name())));
    [[likely]] case Base::Status::Available:
        return *static_cast<ADerived*>(*Base::fgInstance);
    [[unlikely]] case Base::Status::Expired:
        Throw<std::logic_error>(fmt::format("The instance of {} (passive singleton in environment) has been deleted",
                                            muc::try_demangle(typeid(ADerived).name())));
    }
    muc::unreachable();
}

} // namespace Mustard::Env::Memory
