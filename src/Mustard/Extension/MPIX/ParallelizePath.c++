#include "Mustard/Env/MPIEnv.h++"
#include "Mustard/Extension/MPIX/ParallelizePath.h++"

#include "fmt/format.h"

namespace Mustard::inline Extension::MPIX {

auto ParallelizePath(const std::filesystem::path& path) -> std::filesystem::path {
    auto stem{path.stem()};
    if (stem.empty()) {
        throw std::invalid_argument{"Mustard::MPIX::ParallelizePath: Empty file name"};
    }
    if (stem == "." or stem == "..") {
        throw std::invalid_argument{fmt::format("Mustard::MPIX::ParallelizePath: Invalid file name '{}'", stem.c_str())};
    }

    if (const auto& mpiEnv{Env::MPIEnv::Instance()};
        mpiEnv.Parallel()) {
        // parent directory
        auto parent{std::filesystem::path{path}.replace_extension()};
        if (mpiEnv.OnCluster()) {
            parent /= mpiEnv.LocalNode().name;
        }
        // create parent directory
        if (mpiEnv.OnCommNodeMaster()) {
            std::filesystem::create_directories(parent);
        }
        // wait for create_directories
        MPI_Barrier(mpiEnv.CommNode());
        // construct full path
        return parent / stem.concat(fmt::format("_mpi{}.", mpiEnv.CommWorldRank())).replace_extension(path.extension());
    } else {
        return path;
    }
}

} // namespace Mustard::inline Extension::MPIX
