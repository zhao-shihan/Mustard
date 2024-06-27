#pragma once

#include "Mustard/Detector/Description/DescriptionBase.h++"

#include "muc/array"

namespace AirShower::Detector::Description {

class Field final : public Mustard::Detector::Description::DescriptionBase<Field> {
    friend Mustard::Env::Memory::SingletonInstantiator;

private:
    Field();
    ~Field() = default;

public:
    auto MagneticField() const -> auto { return fMagneticField; }

    auto MagneticField(muc::array3d val) -> void { fMagneticField = val; }

private:
    auto ImportAllValue(const YAML::Node& node) -> void override;
    auto ExportAllValue(YAML::Node& node) const -> void override;

private:
    muc::array3d fMagneticField;
};

} // namespace AirShower::Detector::Description
