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

namespace Mustard::inline Utility {

template<typename T>
    requires std::is_trivially_copyable_v<T>
auto ToBase64(const T& pod) -> std::string {
    const auto byteArray{std::bit_cast<std::array<char, sizeof(T)>>(pod)};
    return std::string{TBase64::Encode(byteArray.data(), byteArray.size())};
}

template<typename T>
    requires std::is_trivially_copyable_v<T>
auto FromBase64(const std::string& base64) -> T {
    const auto byteString{TBase64::Decode(base64.c_str())};
    if (byteString.Length() != sizeof(T)) {
        Throw<std::runtime_error>(fmt::format("Decoded base64 string is invalid. Expected {} bytes, got {} bytes.",
                                              sizeof(T), byteString.Length()));
    }
    if constexpr (std::is_default_constructible_v<T>) {
        T pod;
        std::memcpy(&pod, byteString.Data(), sizeof(T));
        return pod;
    } else {
        std::array<char, sizeof(T)> byteArray;
        std::memcpy(byteArray.data(), byteString.Data(), sizeof(T));
        return std::bit_cast<T>(byteArray);
    }
}

} // namespace Mustard::inline Utility
