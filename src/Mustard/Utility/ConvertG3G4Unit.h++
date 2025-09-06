#pragma once

#include "CLHEP/Units/SystemOfUnits.h"

#include "TGeoSystemOfUnits.h"

#include "muc/ceta_string"
#include "muc/concepts"
#include "muc/math"
#include "muc/utility"

#include <type_traits>

namespace Mustard::inline Utility {

/// @brief Converts Geant4 unit system to Geant3 unit system.
/// @tparam U Type of value to be converted.
/// @tparam ACategory The unit category.
/// @param x The value
/// @return Converted value.
template<muc::ceta_string ACategory, muc::general_arithmetic U>
constexpr auto ToG3(const U& x) -> U {
    constexpr auto L{TGeoUnit::mm};
    constexpr auto T{TGeoUnit::ns};
    constexpr auto E{TGeoUnit::MeV};
    static_assert(CLHEP::eplus == 1);
    static_assert(TGeoUnit::eplus == 1);
    constexpr auto Q{TGeoUnit::eplus};
    using muc::pow;
    if constexpr (ACategory == "Length") {
        return x * L;
    } else if constexpr (ACategory == "Angle") {
        return x * (TGeoUnit::radian / CLHEP::radian);
    } else if constexpr (ACategory == "Time") {
        return x * T;
    } else if constexpr (ACategory == "Energy") {
        return x * E;
    } else if constexpr (ACategory == "Mass") {
        return x * (E * pow(T, 2) * pow(L, -2));
    } else if constexpr (ACategory == "Power") {
        return x * (E * pow(T, -1));
    } else if constexpr (ACategory == "Force") {
        return x * (E * pow(L, -1));
    } else if constexpr (ACategory == "Pressure") {
        return x * (E * pow(L, -3));
    } else if constexpr (ACategory == "Electric current") {
        return x * (Q * pow(T, -1));
    } else if constexpr (ACategory == "Electric potential") {
        return x * (E * pow(Q, -1));
    } else if constexpr (ACategory == "Electric resistance") {
        return x * (E * T * pow(Q, -2));
    } else if constexpr (ACategory == "Electric capacitance") {
        return x * (pow(Q, 2) * pow(E, -1));
    } else if constexpr (ACategory == "Magnetic flux") {
        return x * (T * E * pow(Q, -1));
    } else if constexpr (ACategory == "Magnetic field") {
        return x * (T * E * pow(Q, -1) * pow(L, -2));
    } else if constexpr (ACategory == "Inductance") {
        return x * (pow(T, 2) * E * pow(Q, -2));
    } else if constexpr (ACategory == "Temperature") {
        return x * (TGeoUnit::kelvin / CLHEP::kelvin);
    } else if constexpr (ACategory == "Amount of substance") {
        return x * (TGeoUnit::mole / CLHEP::mole);
    } else if constexpr (ACategory == "Activity") {
        return x * pow(T, -1);
    } else if constexpr (ACategory == "Absorbed dose") {
        return x * (pow(L, 2) * pow(T, -2));
    } else if constexpr (ACategory == "Luminous intensity") {
        return x * (TGeoUnit::candela / CLHEP::candela);
    } else if constexpr (ACategory == "Luminous flux") {
        return x * (TGeoUnit::lumen / CLHEP::lumen);
    } else if constexpr (ACategory == "Illuminance") {
        return x * (TGeoUnit::lux / CLHEP::lux);
    } else {
        static_assert(muc::dependent_false<U>, "No such unit category");
    }
}

/// @brief Converts Geant3 unit system to Geant4 unit system.
/// @tparam U Type of value to be converted.
/// @tparam ACategory The unit category.
/// @param x The value
/// @return Converted value.
template<muc::ceta_string ACategory, muc::general_arithmetic U>
constexpr auto ToG4(const U& x) -> U {
    constexpr auto L{CLHEP::cm};
    constexpr auto T{CLHEP::s};
    constexpr auto E{CLHEP::GeV};
    static_assert(CLHEP::eplus == 1);
    static_assert(TGeoUnit::eplus == 1);
    constexpr auto Q{CLHEP::eplus};
    using muc::pow;
    if constexpr (ACategory == "Length") {
        return x * L;
    } else if constexpr (ACategory == "Angle") {
        return x * (CLHEP::radian / TGeoUnit::radian);
    } else if constexpr (ACategory == "Time") {
        return x * T;
    } else if constexpr (ACategory == "Energy") {
        return x * E;
    } else if constexpr (ACategory == "Mass") {
        return x * (E * pow(T, 2) * pow(L, -2));
    } else if constexpr (ACategory == "Power") {
        return x * (E * pow(T, -1));
    } else if constexpr (ACategory == "Force") {
        return x * (E * pow(L, -1));
    } else if constexpr (ACategory == "Pressure") {
        return x * (E * pow(L, -3));
    } else if constexpr (ACategory == "Electric current") {
        return x * (Q * pow(T, -1));
    } else if constexpr (ACategory == "Electric potential") {
        return x * (E * pow(Q, -1));
    } else if constexpr (ACategory == "Electric resistance") {
        return x * (E * T * pow(Q, -2));
    } else if constexpr (ACategory == "Electric capacitance") {
        return x * (pow(Q, 2) * pow(E, -1));
    } else if constexpr (ACategory == "Magnetic Flux") {
        return x * (T * E * pow(Q, -1));
    } else if constexpr (ACategory == "Magnetic Field") {
        return x * (T * E * pow(Q, -1) * pow(L, -2));
    } else if constexpr (ACategory == "Inductance") {
        return x * (pow(T, 2) * E * pow(Q, -2));
    } else if constexpr (ACategory == "Temperature") {
        return x * (CLHEP::kelvin / TGeoUnit::kelvin);
    } else if constexpr (ACategory == "Amount of substance") {
        return x * (CLHEP::mole / TGeoUnit::mole);
    } else if constexpr (ACategory == "Activity") {
        return x * pow(T, -1);
    } else if constexpr (ACategory == "Absorbed dose") {
        return x * (pow(L, 2) * pow(T, -2));
    } else if constexpr (ACategory == "Luminous intensity") {
        return x * (CLHEP::candela / TGeoUnit::candela);
    } else if constexpr (ACategory == "Luminous flux") {
        return x * (CLHEP::lumen / TGeoUnit::lumen);
    } else if constexpr (ACategory == "Illuminance") {
        return x * (CLHEP::lux / TGeoUnit::lux);
    } else {
        static_assert(muc::dependent_false<U>, "No such unit category");
    }
}

} // namespace Mustard::inline Utility
