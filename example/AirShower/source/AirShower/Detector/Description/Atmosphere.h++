#pragma once

#include "Mustard/Detector/Description/DescriptionWithCacheBase.h++"

#include <vector>

namespace AirShower::Detector::Description {

class Atmosphere final : public Mustard::Detector::Description::DescriptionWithCacheBase<Atmosphere> {
    friend Mustard::Env::Memory::SingletonInstantiator;

private:
    Atmosphere();
    ~Atmosphere() = default;

public:
    auto MaxAltitude() const -> auto { return *fMaxAltitude; }
    auto NPressureSlice() const -> auto { return *fNPressureSlice; }
    auto AltitudeSlice() const -> const auto& { return *fAltitudeSlice; }
    auto StateSlice() const -> const auto& { return *fStateSlice; }

    auto MaxAltitude(double val) -> void { fMaxAltitude = val; }
    auto NPressureSlice(int val) -> void { fNPressureSlice = val; }

private:
    struct AtmoState {
        double pressure;
        double density;
        double temperature;
    };

private:
    auto CalculateAltitudeSlice() const -> std::vector<double>;
    auto CalculateStateSlice() const -> std::vector<AtmoState>;

    auto ImportAllValue(const YAML::Node& node) -> void override;
    auto ExportAllValue(YAML::Node& node) const -> void override;

private:
    Simple<double> fMaxAltitude;
    Simple<int> fNPressureSlice;
    Cached<std::vector<double>> fAltitudeSlice;
    Cached<std::vector<AtmoState>> fStateSlice;
};

} // namespace AirShower::Detector::Description
