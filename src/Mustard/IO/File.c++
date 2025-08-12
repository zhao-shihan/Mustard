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

#include "Mustard/IO/File.h++"
#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/Parallel/ProcessSpecificPath.h++"

#include "TFile.h"

#include "muc/algorithm"
#include "muc/ctype"
#include "muc/utility"

#include "fmt/std.h"

#include <stdexcept>
#include <string_view>
#include <utility>

namespace Mustard::inline IO {

template<>
File<>::~File() = default;

template<>
auto File<>::UpdatePathByOption(Option option, std::filesystem::path& filePath) -> void {
    switch (option) {
    case Option::Homogeneous:
        break;
    case Option::Heterogeneous:
        filePath = Parallel::ProcessSpecificPath(filePath);
        break;
    default:
        Throw<std::invalid_argument>("Invalid option");
    }
}

File<std::FILE>::File(std::filesystem::path filePath, gsl::czstring mode) :
    File{std::string_view{mode}.find_first_of("wa") == std::string_view::npos ?
             Option::Homogeneous :
             Option::Heterogeneous,
         std::move(filePath), mode} {}

File<std::FILE>::File(Option option, std::filesystem::path filePath, gsl::czstring mode) :
    File<>{},
    fFile{} {
    UpdatePathByOption(option, filePath);
    fFile = {std::fopen(filePath.generic_string().c_str(), mode), CloseFile{}};
    if (not fFile) {
        Throw<std::runtime_error>(fmt::format("Cannot open file '{}' in '{}' mode", filePath, mode));
    }
}

namespace internal {

template<std::derived_from<std::ios_base> F>
FStream<F>::FStream(std::filesystem::path filePath, F::openmode mode) :
    FStream{mode & F::out ?
                Option::Heterogeneous :
                Option::Homogeneous,
            std::move(filePath), mode} {}

template<std::derived_from<std::ios_base> F>
FStream<F>::FStream(Option option, std::filesystem::path filePath, F::openmode mode) :
    File<>{},
    fFile{} {
    UpdatePathByOption(option, filePath);
    fFile = std::make_unique<F>(filePath, mode);
    if (not fFile->is_open()) {
        Throw<std::runtime_error>(fmt::format("Cannot open file '{}' in {:#b} mode", filePath, muc::to_underlying(mode)));
    }
}

} // namespace internal

template<muc::character C>
File<std::basic_ifstream<C>>::File(std::filesystem::path filePath, Type::openmode mode) :
    internal::FStream<std::basic_ifstream<C>>{std::move(filePath), mode} {}

template<muc::character C>
File<std::basic_ifstream<C>>::File(Option option, std::filesystem::path filePath, Type::openmode mode) :
    internal::FStream<std::basic_ifstream<C>>{option, std::move(filePath), mode} {}

template class File<std::ifstream>;
template class File<std::wifstream>;

template<muc::character C>
File<std::basic_ofstream<C>>::File(std::filesystem::path filePath, Type::openmode mode) :
    internal::FStream<std::basic_ofstream<C>>{std::move(filePath), mode} {}

template<muc::character C>
File<std::basic_ofstream<C>>::File(Option option, std::filesystem::path filePath, Type::openmode mode) :
    internal::FStream<std::basic_ofstream<C>>{option, std::move(filePath), mode} {}

template class File<std::ofstream>;
template class File<std::wofstream>;

template<muc::character C>
File<std::basic_fstream<C>>::File(std::filesystem::path filePath, Type::openmode mode) :
    internal::FStream<std::basic_fstream<C>>{std::move(filePath), mode} {}

template<muc::character C>
File<std::basic_fstream<C>>::File(Option option, std::filesystem::path filePath, Type::openmode mode) :
    internal::FStream<std::basic_fstream<C>>{option, std::move(filePath), mode} {}

template class File<std::fstream>;
template class File<std::wfstream>;

File<TFile>::File(std::filesystem::path filePath, std::string mode, int compress, int netOpt) :
    File<>{} {
    std::ranges::transform(mode, mode.begin(), muc::toupper);
    const auto option{muc::ranges::contains_subrange(mode, "READ") ?
                          Option::Homogeneous :
                          Option::Heterogeneous};
    Open(option, std::move(filePath), std::move(mode), compress, netOpt);
}

File<TFile>::File(Option option, std::filesystem::path filePath, std::string mode, int compress, int netOpt) :
    File<>{},
    fFile{} {
    Open(option, std::move(filePath), std::move(mode), compress, netOpt);
}

File<TFile>::~File() = default;

auto File<TFile>::Open(Option option, std::filesystem::path filePath, std::string mode, int compress, int netOpt) -> void {
    UpdatePathByOption(option, filePath);
    fFile.reset(TFile::Open(filePath.generic_string().c_str(), mode.c_str(), "", compress, netOpt));
    if (not fFile) {
        Throw<std::runtime_error>(fmt::format("Cannot open file '{}' in '{}' mode", filePath, mode));
    }
}

} // namespace Mustard::inline IO
