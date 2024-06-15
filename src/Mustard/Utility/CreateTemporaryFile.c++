#include "Mustard/Env/BasicEnv.h++"
#include "Mustard/Env/MPIEnv.h++"
#include "Mustard/Utility/CreateTemporaryFile.h++"

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
    for (int i{}; i < 100'000; ++i) {
        path = fs::temp_directory_path() / fmt::format("{}{}{:x}_", programName, signature, random());
        if (Env::MPIEnv::Available()) {
            path.concat(fmt::format("mpi{}_", Env::MPIEnv::Instance().CommWorldRank()));
        }
        path.concat("tmp.").replace_extension(extension);
        file = std::fopen(path.generic_string().c_str(), "wx");
        if (file) { break; }
    }
    if (file == nullptr) { throw std::runtime_error{"failed to create a temporary file"}; }
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
