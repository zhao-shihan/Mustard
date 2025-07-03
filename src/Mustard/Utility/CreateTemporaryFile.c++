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

#include "Mustard/Env/BasicEnv.h++"
#include "Mustard/Utility/CreateTemporaryFile.h++"
#include "Mustard/Utility/PrettyLog.h++"

#include "mplr/mplr.hpp"

#include <chrono>
#include <cstdio>
#include <random>
#include <stack>
#include <stdexcept>
#include <system_error>

namespace Mustard::inline Utility {

auto CreateTemporaryFile(std::string_view signature, std::filesystem::path extension) -> std::filesystem::path {
    std::minstd_rand random;
    if (std::random_device randomDevice;
        randomDevice.entropy() > 0) {
        random.seed(randomDevice());
    } else {
        random.seed(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    }

    namespace fs = std::filesystem;
    fs::path path;
    std::FILE* file;
    const auto programName{fs::path{Env::BasicEnv::Instance().Argv()[0]}.filename().generic_string()};
    for (int i{}; i < 100000; ++i) {
        path = fs::temp_directory_path() /
               fmt::format("{}_{}_{:x}_{:x}tmp.",
                           programName, signature, random(),
                           mplr::available() ? mplr::comm_world().rank() : 0);
        path.replace_extension(extension);
        file = std::fopen(path.generic_string().c_str(), "wx");
        if (file) {
            break;
        }
    }
    if (file == nullptr) {
        Throw<std::runtime_error>("Failed to create a temporary file");
    }
    std::fclose(file);

    static class RemoveTemporaryFileAtExitHelper {
    public:
        ~RemoveTemporaryFileAtExitHelper() {
            while (not fTemporaryFile.empty()) {
                std::error_code muteRemoveError;
                std::filesystem::remove(fTemporaryFile.top(), muteRemoveError);
                fTemporaryFile.pop();
            }
        }

        auto Push(const std::filesystem::path& path) {
            fTemporaryFile.push(path);
        }

    private:
        std::stack<std::filesystem::path> fTemporaryFile;
    } removeHelper;
    removeHelper.Push(path);

    return path;
}

} // namespace Mustard::inline Utility
