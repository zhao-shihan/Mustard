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

namespace Mustard::inline Memory {

template<typename T>
[[nodiscard]] auto Allocator<T>::allocate(std::size_t n) -> value_type* {
    if (n > std::numeric_limits<std::size_t>::max() / sizeof(value_type)) {
        throw std::bad_array_new_length{};
    }
    const auto nByte{n * sizeof(value_type)};
    if constexpr (alignof(value_type) > alignof(std::max_align_t)) {
        return static_cast<value_type*>(Allocate(nByte, std::align_val_t{alignof(value_type)}));
    } else {
        return static_cast<value_type*>(Allocate(nByte));
    }
}

template<typename T>
auto Allocator<T>::deallocate(value_type* p, std::size_t n) noexcept -> void {
    const auto nByte{n * sizeof(value_type)};
    if constexpr (alignof(value_type) > alignof(std::max_align_t)) {
        Deallocate(p, nByte, std::align_val_t{alignof(value_type)});
    } else {
        Deallocate(p, nByte);
    }
}

} // namespace Mustard::inline Memory
