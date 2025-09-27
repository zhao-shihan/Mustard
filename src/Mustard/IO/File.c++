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
File<>::File(FilePathOption pathOption, std::filesystem::path filePath) :
    fPath{std::move(filePath)} {
    switch (pathOption) {
    case FilePathOption::Direct:
        break;
    case FilePathOption::ProcessSpecific:
        fPath = Parallel::ProcessSpecificPath(fPath);
        break;
    default:
        Throw<std::invalid_argument>("Invalid path option");
    }
}

File<std::FILE>::File(std::filesystem::path filePath, gsl::czstring mode) :
    File{FilePathOption::Direct, std::move(filePath), mode} {}

File<std::FILE>::File(FilePathOption pathOption, std::filesystem::path filePath, gsl::czstring mode) :
    File<>{pathOption, std::move(filePath)},
    fFile{} {
    fFile = {std::fopen(Path().generic_string().c_str(), mode), CloseFile{}};
}

auto File<std::FILE>::Get() const -> std::FILE* {
    if (not Opened()) {
        Throw<std::runtime_error>(fmt::format("Cannot open file '{}'", Path()));
    }
    return fFile.get();
}

ProcessSpecificFile<std::FILE>::ProcessSpecificFile(std::filesystem::path filePath, gsl::czstring mode) :
    File{FilePathOption::ProcessSpecific, std::move(filePath), mode} {}

namespace internal {

template<std::derived_from<std::ios_base> F>
FStream<F>::FStream(std::filesystem::path filePath, F::openmode mode) :
    FStream{FilePathOption::Direct, std::move(filePath), mode} {}

template<std::derived_from<std::ios_base> F>
FStream<F>::FStream(FilePathOption pathOption, std::filesystem::path filePath, F::openmode mode) :
    File<>{pathOption, std::move(filePath)},
    fFile{} {
    fFile = std::make_unique<F>(Path(), mode);
    if (fFile->fail()) {
        fFile.reset();
    }
}

template<std::derived_from<std::ios_base> F>
auto FStream<F>::Get() const -> F& {
    if (not Opened()) {
        Throw<std::runtime_error>(fmt::format("Cannot open file '{}'", Path()));
    }
    return *fFile;
}

template class FStream<std::ifstream>;
template class FStream<std::wifstream>;
template class FStream<std::ofstream>;
template class FStream<std::wofstream>;
template class FStream<std::fstream>;
template class FStream<std::wfstream>;

} // namespace internal

template<muc::character C>
File<std::basic_ifstream<C>>::File(std::filesystem::path filePath, Type::openmode mode) :
    internal::FStream<std::basic_ifstream<C>>{std::move(filePath), mode} {}

template<muc::character C>
File<std::basic_ifstream<C>>::File(FilePathOption pathOption, std::filesystem::path filePath, Type::openmode mode) :
    internal::FStream<std::basic_ifstream<C>>{pathOption, std::move(filePath), mode} {}

template class File<std::ifstream>;
template class File<std::wifstream>;

template<muc::character C>
ProcessSpecificFile<std::basic_ifstream<C>>::ProcessSpecificFile(std::filesystem::path filePath, Type::openmode mode) :
    File<std::basic_ifstream<C>>{FilePathOption::ProcessSpecific, std::move(filePath), mode} {}

template class ProcessSpecificFile<std::ifstream>;
template class ProcessSpecificFile<std::wifstream>;

template<muc::character C>
File<std::basic_ofstream<C>>::File(std::filesystem::path filePath, Type::openmode mode) :
    internal::FStream<std::basic_ofstream<C>>{std::move(filePath), mode} {}

template<muc::character C>
File<std::basic_ofstream<C>>::File(FilePathOption pathOption, std::filesystem::path filePath, Type::openmode mode) :
    internal::FStream<std::basic_ofstream<C>>{pathOption, std::move(filePath), mode} {}

template class File<std::ofstream>;
template class File<std::wofstream>;

template<muc::character C>
ProcessSpecificFile<std::basic_ofstream<C>>::ProcessSpecificFile(std::filesystem::path filePath, Type::openmode mode) :
    File<std::basic_ofstream<C>>{FilePathOption::ProcessSpecific, std::move(filePath), mode} {}

template class ProcessSpecificFile<std::ofstream>;
template class ProcessSpecificFile<std::wofstream>;

template<muc::character C>
File<std::basic_fstream<C>>::File(std::filesystem::path filePath, Type::openmode mode) :
    internal::FStream<std::basic_fstream<C>>{std::move(filePath), mode} {}

template<muc::character C>
File<std::basic_fstream<C>>::File(FilePathOption pathOption, std::filesystem::path filePath, Type::openmode mode) :
    internal::FStream<std::basic_fstream<C>>{pathOption, std::move(filePath), mode} {}

template class File<std::fstream>;
template class File<std::wfstream>;

template<muc::character C>
ProcessSpecificFile<std::basic_fstream<C>>::ProcessSpecificFile(std::filesystem::path filePath, Type::openmode mode) :
    File<std::basic_fstream<C>>{FilePathOption::ProcessSpecific, std::move(filePath), mode} {}

template class ProcessSpecificFile<std::fstream>;
template class ProcessSpecificFile<std::wfstream>;

File<TFile>::File(std::filesystem::path filePath, std::string mode, int compress, int netOpt) :
    File{FilePathOption::Direct, std::move(filePath), mode, compress, netOpt} {}

File<TFile>::File(FilePathOption pathOption, std::filesystem::path filePath, std::string mode, int compress, int netOpt) :
    File<>{pathOption, std::move(filePath)},
    fFile{TFile::Open(Path().generic_string().c_str(), mode.c_str(), "", compress, netOpt)} {
    if (not fFile) {
        Throw<std::runtime_error>(fmt::format("Cannot open file '{}' in '{}' mode", Path(), mode));
    }
}

File<TFile>::~File() = default;

ProcessSpecificFile<TFile>::ProcessSpecificFile(std::filesystem::path filePath, std::string mode, int compress, int netOpt) :
    File{FilePathOption::ProcessSpecific, std::move(filePath), mode, compress, netOpt} {}

} // namespace Mustard::inline IO
