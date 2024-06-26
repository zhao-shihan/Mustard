#include "AirShower/Detector/Description/World.h++"

#include "Mustard/Utility/LiteralUnit.h++"

namespace AirShower::Detector::Description {

using namespace Mustard::LiteralUnit::Length;

World::World() :
    DescriptionBase{"World"},
    fWidth{1000_km},
    fMaxHeight{110_km} {}

void World::ImportAllValue(const YAML::Node& node) {
    ImportValue(node, fWidth, "Width");
    ImportValue(node, fMaxHeight, "MaxHeight");
}

void World::ExportAllValue(YAML::Node& node) const {
    ExportValue(node, fWidth, "Width");
    ExportValue(node, fMaxHeight, "MaxHeight");
}

} // namespace AirShower::Detector::Description
