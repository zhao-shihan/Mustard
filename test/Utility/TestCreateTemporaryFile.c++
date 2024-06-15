#include "Mustard/Env/MPIEnv.h++"
#include "Mustard/Utility/CreateTemporaryFile.h++"

#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>

using namespace Mustard;

auto main(int argc, char* argv[]) -> int {
    Mustard::Env::MPIEnv env{argc, argv, {}};

    const auto path{CreateTemporaryFile(argv[1], argv[2])};
    std::cout << path << std::endl;
    if (not std::filesystem::exists(path)) { throw std::runtime_error{"bug"}; }

    std::getchar();

    return EXIT_SUCCESS;
}
