#pragma once

#include "CLHEP/Vector/TwoVector.h"

namespace Mustard::inline Compatibility::inline CLHEPDefectFix::inline Hep2VectorDivisionAssignment {

inline auto& operator/=(CLHEP::Hep2Vector& v, double c) {
    v[0] /= c;
    v[1] /= c;
    return v;
}

} // namespace Mustard::inline Compatibility::inline CLHEPDefectFix::inline Hep2VectorDivisionAssignment
