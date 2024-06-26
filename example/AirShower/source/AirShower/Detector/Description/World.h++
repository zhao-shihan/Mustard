#pragma once

#include "Mustard/Detector/Description/DescriptionBase.h++"

namespace AirShower::Detector::Description {

class World final : public Mustard::Detector::Description::DescriptionBase<World> {
    friend Mustard::Env::Memory::SingletonInstantiator;

private:
    World();
    ~World() = default;

public:
    auto Width() const -> auto { return fWidth; }
    auto MaxHeight() const -> auto { return fMaxHeight; }

    auto Width(double val) -> void { fWidth = val; }
    auto MaxHeight(double val) -> void { fMaxHeight = val; }

private:
    auto ImportAllValue(const YAML::Node& node) -> void override;
    auto ExportAllValue(YAML::Node& node) const -> void override;

private:
    double fWidth;
    double fMaxHeight;
};

} // namespace AirShower::Detector::Description
