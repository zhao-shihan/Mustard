#include "Mustard/Application/Subprogram.h++"

#include <utility>

namespace Mustard::Application {

Subprogram::Subprogram(std::string name, std::string description) :
    fName{std::move(name)},
    fDescription{std::move(description)} {}

} // namespace Mustard::Application
