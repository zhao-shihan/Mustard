#include "AirShower/Detector/Description/Field.h++"

#include "Mustard/Utility/LiteralUnit.h++"

namespace AirShower::Detector::Description {

using namespace Mustard::LiteralUnit::MagneticFluxDensity;

Field::Field() :
    DescriptionBase{"Field"},
    fMagneticField{0, 50_uT, 0} {}

void Field::ImportAllValue(const YAML::Node& node) {
    ImportValue(node, fMagneticField, "MagneticField");
}

void Field::ExportAllValue(YAML::Node& node) const {
    ExportValue(node, fMagneticField, "MagneticField");
}

} // namespace AirShower::Detector::Description
