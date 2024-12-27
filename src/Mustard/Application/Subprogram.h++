#pragma once

#include <string>

namespace Mustard::Application {

class Subprogram {
public:
    Subprogram(std::string name, std::string description);
    virtual ~Subprogram() = default;

    auto Name() const -> const auto& { return fName; }
    auto Description() const -> const auto& { return fDescription; }

    virtual auto Main(int argc, char* argv[]) const -> int = 0;

private:
    std::string fName;
    std::string fDescription;
};

} // namespace Mustard::Application
