// -*- C++ -*-
//
// Copyright (C) 2020-2025  The Mustard development team
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

namespace Mustard::Geant4X::inline Memory {

template<typename ADerived>
[[nodiscard]] MUSTARD_ALWAYS_INLINE auto UseG4Allocator<ADerived>::operator new(std::size_t) -> void* {
    static_assert(std::derived_from<ADerived, UseG4Allocator<ADerived>>);
    static_assert(std::is_final_v<ADerived>);
    return SingletonG4Allocator<ADerived>::Instance().MallocSingle();
}

template<typename ADerived>
MUSTARD_ALWAYS_INLINE auto UseG4Allocator<ADerived>::operator delete(void* ptr) -> void {
    SingletonG4Allocator<ADerived>::Instance().FreeSingle(static_cast<ADerived*>(ptr));
}

} // namespace Mustard::Geant4X::inline Memory
