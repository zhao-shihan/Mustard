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

/// @brief MPI-aware file wrapper with heterogeneous/homogeneous access modes
///
/// Provides unified interface for various file types with automatic
/// MPI process-specific path handling if necessary.
///
/// @tparam F File type to wrap (default (void) for base, specific types for specializations)
///
/// @note Key features:
///   - Homogeneous mode: All processes access same file
///   - Heterogeneous mode: Per-process accesess its unique file
///
/// @warning Construct with Option::Heterogeneous is an collective operation (must be called by all processes)
template<typename F = void>
class File {
public:
    /// @brief File access mode options
    enum struct Option {
        Homogeneous,  ///< All processes access same file
        Heterogeneous ///< Per-process accesses its unique file
    };

public:
    virtual ~File() = 0;

protected:
    /// @brief Adjust file path based on access mode
    /// @param option Homogeneous/Heterogeneous
    /// @param filePath File path to modify
    static auto UpdatePathByOption(Option option, std::filesystem::path& filePath) -> void;
};

/// @brief Specialization for `std::FILE`
template<>
class File<std::FILE> : public File<> {
public:
    using Type = std::FILE; ///< Wrapped file type

public:
    /// @brief Open file with automatic access option
    /// @param filePath File path
    /// @param mode fopen-style mode string
    /// @note Access option:
    ///   - With 'w' or 'a': Heterogeneous
    ///   - Otherwise: Homogeneous
    /// @warning Construct with heterogeneous option is an MPI collective operation
    File(std::filesystem::path filePath, gsl::czstring mode);
    /// @brief Open file with explicit access option
    /// @param option Homogeneous/Heterogeneous selection
    /// @param filePath File path
    /// @param mode fopen-style mode string
    /// @warning Construct with heterogeneous option is an MPI collective operation
    File(Option option, std::filesystem::path filePath, gsl::czstring mode);

    auto operator*() -> auto& { return *fFile; }
    auto operator*() const -> const auto& { return *fFile; }

    auto operator->() -> auto* { return fFile.get(); }
    auto operator->() const -> const auto* { return fFile.get(); }

    operator std::FILE&() { return **this; }
    operator const std::FILE&() const { return **this; }

private:
    struct CloseFile {
        auto operator()(std::FILE* file) const -> void { std::fclose(file); }
    };

private:
    std::unique_ptr<std::FILE, CloseFile> fFile;
};

namespace internal {

/// @internal
/// @brief Internal base for C++ fstream wrappers
/// @tparam F fstream type (ifstream, ofstream, fstream)
template<std::derived_from<std::ios_base> F>
class FStream : public File<> {
protected:
    /// @brief Open file with automatic access option
    /// @note Writing mode -> Heterogeneous
    /// @warning Construct with heterogeneous option is an MPI collective operation
    FStream(std::filesystem::path filePath, F::openmode mode);
    /// @brief Construct with explicit access mode
    /// @warning Construct with heterogeneous option is an MPI collective operation
    FStream(Option option, std::filesystem::path filePath, F::openmode mode);

public:
    auto operator*() -> F& { return *fFile; }
    auto operator*() const -> const F& { return *fFile; }

    auto operator->() -> F* { return fFile.get(); }
    auto operator->() const -> const F* { return fFile.get(); }

    operator F&() { return **this; }
    operator const F&() const { return **this; }

private:
    std::unique_ptr<F> fFile;
};

} // namespace internal

/// @brief Specialization for `std::basic_ifstream`
/// @tparam C Character type (char, wchar_t)
template<muc::character C>
class File<std::basic_ifstream<C>> : public internal::FStream<std::basic_ifstream<C>> {
public:
    using typename File<>::Option;
    using Type = std::basic_ifstream<C>; ///< Wrapped file type

public:
    /// @brief Open input file (homogeneous by default)
    /// @warning Construct with heterogeneous option is an MPI collective operation
    File(std::filesystem::path filePath, Type::openmode mode = Type::in);
    /// @brief Construct with explicit access mode
    /// @warning Construct with heterogeneous option is an MPI collective operation
    File(Option option, std::filesystem::path filePath, Type::openmode mode = Type::in);
};

/// @brief Specialization for `std::basic_ofstream`
/// @tparam C Character type (char, wchar_t)
template<muc::character C>
class File<std::basic_ofstream<C>> : public internal::FStream<std::basic_ofstream<C>> {
public:
    using typename File<>::Option;
    using Type = std::basic_ofstream<C>; ///< Wrapped file type

public:
    /// @brief Open output file (heterogeneous by default)
    /// @warning Construct with heterogeneous option is an MPI collective operation
    File(std::filesystem::path filePath, Type::openmode mode = Type::out);
    /// @brief Construct with explicit access mode
    /// @warning Construct with heterogeneous option is an MPI collective operation
    File(Option option, std::filesystem::path filePath, Type::openmode mode = Type::out);
};

/// @brief Specialization for `std::basic_fstream`
/// @tparam C Character type (char, wchar_t)
template<muc::character C>
class File<std::basic_fstream<C>> : public internal::FStream<std::basic_fstream<C>> {
public:
    using typename File<>::Option;
    using Type = std::basic_fstream<C>; ///< Wrapped file type

public:
    /// @brief Open file with automatic access option
    /// @note Writing mode -> Heterogeneous
    /// @warning Construct with heterogeneous option is an MPI collective operation
    File(std::filesystem::path filePath, Type::openmode mode = Type::in | Type::out);
    /// @brief Construct with explicit access mode
    /// @warning Construct with heterogeneous option is an MPI collective operation
    File(Option option, std::filesystem::path filePath, Type::openmode mode = Type::in | Type::out);
};

/// @brief Specialization for TFile
template<>
class File<TFile> : public File<> {
public:
    /// @brief Open ROOT file with automatic mode detection
    /// @note Mode contains "READ": homogeneous, otherwise Heterogeneous
    File(std::filesystem::path filePath, std::string mode = "READ",
         int compress = muc::to_underlying(ROOT::RCompressionSetting::EDefaults::kUseGeneralPurpose),
         int netOpt = 0);
    /// @brief Construct with explicit access mode
    /// @warning Construct with heterogeneous option is an MPI collective operation
    File(Option option, std::filesystem::path filePath, std::string mode = "READ",
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
    auto Open(Option option, std::filesystem::path filePath, std::string mode, int compress, int netOpt) -> void;

private:
    std::unique_ptr<TFile> fFile;
};

} // namespace Mustard::inline IO
