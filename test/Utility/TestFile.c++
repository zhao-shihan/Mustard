#include "Mustard/Env/MPIEnv.h++"
#include "Mustard/Utility/File.h++"

#include <memory>
#include <vector>

auto main(int argc, char* argv[]) -> int {
    Mustard::Env::MPIEnv env{argc, argv, {}};

    std::vector<std::unique_ptr<Mustard::File<>>> file;
    file.emplace_back(std::make_unique<Mustard::File<std::FILE>>(argv[1], "w"));
    file.emplace_back(std::make_unique<Mustard::File<std::fstream>>(argv[1]));
    file.emplace_back(std::make_unique<Mustard::File<std::ifstream>>(argv[1]));
    file.emplace_back(std::make_unique<Mustard::File<std::ofstream>>(argv[1]));
    file.emplace_back(std::make_unique<Mustard::File<TFile>>(argv[1], "RECREATE"));

    return EXIT_SUCCESS;
}
