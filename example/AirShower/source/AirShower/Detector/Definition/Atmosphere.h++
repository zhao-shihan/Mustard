#pragma once

#include "Mustard/Detector/Definition/DefinitionBase.h++"

namespace AirShower::Detector::Definition {

class Atmosphere final : public Mustard::Detector::Definition::DefinitionBase {
private:
    auto Construct(bool checkOverlaps) -> void override;
};

} // namespace AirShower::Detector::Definition
