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

#pragma once

#include "Compression.h"

#include "muc/concepts"
#include "muc/utility"

#include "gsl/gsl"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>

class TFile;

namespace Mustard::inline IO {

/// @brief File path options
enum struct FilePathOption {
    Direct,         ///< Open file according to passed path unchanged
    ProcessSpecific ///< Open file according to path modified by Mustard::Parallel::ProcessSpecificPath
};

/// @brief Useful and MPI-aware file wrapper
///
/// Provides unified interface for various file types with automatic
/// MPI process-specific path handling if necessary.
///
/// @tparam F File type to wrap (default (void) for base, specific types for specializations)
///
/// @note Key features:
///   - Direct mode: Open file according to passed path unchanged
///   - ProcessSpecific mode: Open file according to path
///     modified by Mustard::Parallel::ProcessSpecificPath
///
/// @warning Open file with FilePathOption::ProcessSpecific is
/// an MPI collective operation (must be called by all processes)
template<typename F = void>
class File {
protected:
    /// @brief Construct base class
    /// @param pathOption Direct/ProcessSpecific
    /// @param filePath File path
    File(FilePathOption pathOption, std::filesystem::path filePath);

public:
    /// @brief Default d'tor
    virtual ~File() = default;

    /// @brief Get the file path
    auto Path() const -> const auto& { return fPath; }

private:
    std::filesystem::path fPath; ///< Exact file path (may different from c'tor parameter)
};

/// @brief MPI-aware process-specific file.
/// Just a wrapper of `File` with FilePathOption::ProcessSpecific
/// @warning Open process-specific file is an MPI collective operation
template<typename>
class ProcessSpecificFile;

/// @brief Specialization for `std::FILE`
template<>
class File<std::FILE> : public File<> {
public:
    using Type = std::FILE; ///< Wrapped file type

public:
    /// @brief Open file with FilePathOption::Direct
    /// @param filePath File path
    /// @param mode fopen-style mode string
    File(std::filesystem::path filePath, gsl::czstring mode);
    /// @brief Open file with explicit path option
    /// @param pathOption Direct/ProcessSpecific
    /// @param filePath File path
    /// @param mode fopen-style mode string
    /// @warning Open file with FilePathOption::ProcessSpecific is an MPI collective operation
    File(FilePathOption pathOption, std::filesystem::path filePath, gsl::czstring mode);

    /// @brief Check the file is opened or not
    /// @return true if file opened, false if not
    auto Opened() const -> bool { return fFile.get(); }

    auto operator*() -> auto* { return Get(); }
    auto operator*() const -> const auto* { return Get(); }

    operator std::FILE*() { return Get(); }
    operator const std::FILE*() const { return Get(); }

private:
    auto Get() const -> std::FILE*;

private:
    struct CloseFile {
        auto operator()(std::FILE* file) const -> void { std::fclose(file); }
    };

private:
    std::unique_ptr<std::FILE, CloseFile> fFile;
};

/// @brief Specialization for `std::FILE`
/// @warning Open process-specific file is an MPI collective operation
template<>
class ProcessSpecificFile<std::FILE> : public File<std::FILE> {
public:
    /// @brief Open file with FilePathOption::ProcessSpecific
    /// @param filePath File path
    /// @param mode fopen-style mode string
    /// @warning Open process-specific file is an MPI collective operation
    ProcessSpecificFile(std::filesystem::path filePath, gsl::czstring mode);
};

namespace internal {

/// @internal
/// @brief Internal base for C++ fstream wrappers
/// @tparam F fstream type (ifstream, ofstream, fstream)
template<std::derived_from<std::ios_base> F>
class FStream : public File<> {
protected:
    /// @brief Open file with FilePathOption::Direct
    /// @param filePath File path
    /// @param mode file stream open mode
    FStream(std::filesystem::path filePath, F::openmode mode);
    /// @brief Open file with explicit path option
    /// @param pathOption Direct/ProcessSpecific
    /// @param filePath File path
    /// @param mode file stream open mode
    /// @warning Open file with FilePathOption::ProcessSpecific is an MPI collective operation
    FStream(FilePathOption pathOption, std::filesystem::path filePath, F::openmode mode);

public:
    /// @brief Check the file is opened or not
    /// @return true if file opened, false if not
    auto Opened() const -> bool { return fFile.get(); }

    auto operator*() -> auto& { return Get(); }
    auto operator*() const -> const auto& { return Get(); }

    auto operator->() -> auto* { return &Get(); }
    auto operator->() const -> const auto* { return &Get(); }

    operator F&() { return Get(); }
    operator const F&() const { return Get(); }

private:
    auto Get() const -> F&;

private:
    std::unique_ptr<F> fFile;
};

} // namespace internal

/// @brief Specialization for `std::basic_ifstream`
/// @tparam C Character type (char, wchar_t)
template<muc::character C>
class File<std::basic_ifstream<C>> : public internal::FStream<std::basic_ifstream<C>> {
public:
    using Type = std::basic_ifstream<C>; ///< Wrapped file type

public:
    /// @brief Open input file with FilePathOption::Direct
    /// @param filePath File path
    /// @param mode file stream open mode
    File(std::filesystem::path filePath, Type::openmode mode = Type::in);
    /// @brief Open file with explicit path option
    /// @param pathOption Direct/ProcessSpecific
    /// @param filePath File path
    /// @param mode file stream open mode
    /// @warning Open file with FilePathOption::ProcessSpecific is an MPI collective operation
    File(FilePathOption pathOption, std::filesystem::path filePath, Type::openmode mode = Type::in);
};

/// @brief Specialization for `std::basic_ifstream`
/// @tparam C Character type (char, wchar_t)
/// @warning Open process-specific file is an MPI collective operation
template<muc::character C>
class ProcessSpecificFile<std::basic_ifstream<C>> : public File<std::basic_ifstream<C>> {
public:
    using typename File<std::basic_ifstream<C>>::Type;

public:
    /// @brief Open file with FilePathOption::ProcessSpecific
    /// @param filePath File path
    /// @param mode file stream open mode
    /// @warning Open process-specific file is an MPI collective operation
    ProcessSpecificFile(std::filesystem::path filePath, Type::openmode mode = Type::in);
};

/// @brief Specialization for `std::basic_ofstream`
/// @tparam C Character type (char, wchar_t)
template<muc::character C>
class File<std::basic_ofstream<C>> : public internal::FStream<std::basic_ofstream<C>> {
public:
    using Type = std::basic_ofstream<C>; ///< Wrapped file type

public:
    /// @brief Open output file with FilePathOption::Direct
    /// @param filePath File path
    /// @param mode file stream open mode
    File(std::filesystem::path filePath, Type::openmode mode = Type::out);
    /// @brief Open file with explicit path option
    /// @param pathOption Direct/ProcessSpecific
    /// @param filePath File path
    /// @param mode file stream open mode
    /// @warning Open file with FilePathOption::ProcessSpecific is an MPI collective operation
    File(FilePathOption pathOption, std::filesystem::path filePath, Type::openmode mode = Type::out);
};

/// @brief Specialization for `std::basic_ofstream`
/// @tparam C Character type (char, wchar_t)
/// @warning Open process-specific file is an MPI collective operation
template<muc::character C>
class ProcessSpecificFile<std::basic_ofstream<C>> : public File<std::basic_ofstream<C>> {
public:
    using typename File<std::basic_ofstream<C>>::Type;

public:
    /// @brief Open file with FilePathOption::ProcessSpecific
    /// @param filePath File path
    /// @param mode file stream open mode
    /// @warning Open process-specific file is an MPI collective operation
    ProcessSpecificFile(std::filesystem::path filePath, Type::openmode mode = Type::out);
};

/// @brief Specialization for `std::basic_fstream`
/// @tparam C Character type (char, wchar_t)
template<muc::character C>
class File<std::basic_fstream<C>> : public internal::FStream<std::basic_fstream<C>> {
public:
    using Type = std::basic_fstream<C>; ///< Wrapped file type

public:
    /// @brief Open file with FilePathOption::Direct
    /// @param filePath File path
    /// @param mode file stream open mode
    File(std::filesystem::path filePath, Type::openmode mode = Type::in | Type::out);
    /// @brief Open file with explicit path option
    /// @param pathOption Direct/ProcessSpecific
    /// @param filePath File path
    /// @param mode file stream open mode
    /// @warning Open file with FilePathOption::ProcessSpecific is an MPI collective operation
    File(FilePathOption pathOption, std::filesystem::path filePath, Type::openmode mode = Type::in | Type::out);
};

/// @brief Specialization for `std::basic_fstream`
/// @tparam C Character type (char, wchar_t)
/// @warning Open process-specific file is an MPI collective operation
template<muc::character C>
class ProcessSpecificFile<std::basic_fstream<C>> : public File<std::basic_fstream<C>> {
public:
    using typename File<std::basic_fstream<C>>::Type;

public:
    /// @brief Open file with FilePathOption::ProcessSpecific
    /// @param filePath File path
    /// @param mode file stream open mode
    /// @warning Open process-specific file is an MPI collective operation
    ProcessSpecificFile(std::filesystem::path filePath, Type::openmode mode = Type::in | Type::out);
};

/// @brief Specialization for `TFile`
template<>
class File<TFile> : public File<> {
public:
    /// @brief Open ROOT file with FilePathOption::Direct
    File(std::filesystem::path filePath, std::string mode = "READ",
         int compress = muc::to_underlying(ROOT::RCompressionSetting::EDefaults::kUseGeneralPurpose),
         int netOpt = 0);
    /// @brief Open file with explicit path option
    /// @warning Open file with FilePathOption::ProcessSpecific is an MPI collective operation
    File(FilePathOption pathOption, std::filesystem::path filePath, std::string mode = "READ",
         int compress = muc::to_underlying(ROOT::RCompressionSetting::EDefaults::kUseGeneralPurpose),
         int netOpt = 0);
    ~File();

    auto operator*() -> auto& { return *fFile; }
    auto operator*() const -> const auto& { return *fFile; }

    auto operator->() -> auto* { return fFile.get(); }
    auto operator->() const -> const auto* { return fFile.get(); }

    operator TFile&() { return **this; }
    operator const TFile&() const { return **this; }

private:
    std::unique_ptr<TFile> fFile;
};

/// @brief Specialization for `TFile`
/// @warning Open process-specific file is an MPI collective operation
template<>
class ProcessSpecificFile<TFile> : public File<TFile> {
public:
    /// @brief Open ROOT file with FilePathOption::ProcessSpecific
    /// @warning Open process-specific file is an MPI collective operation
    ProcessSpecificFile(std::filesystem::path filePath, std::string mode = "READ",
                        int compress = muc::to_underlying(ROOT::RCompressionSetting::EDefaults::kUseGeneralPurpose),
                        int netOpt = 0);
};

} // namespace Mustard::inline IO
