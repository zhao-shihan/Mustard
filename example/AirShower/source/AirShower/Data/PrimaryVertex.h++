#pragma once

#include "Mustard/Data/TupleModel.h++"
#include "Mustard/Data/Value.h++"

#include "muc/array"

namespace AirShower::Data {

using PrimaryVertex = Mustard::Data::TupleModel<
    Mustard::Data::Value<int, "EvtID", "Event ID">,
    Mustard::Data::Value<int, "PDGID", "PDG ID">,
    Mustard::Data::Value<double, "t0", "Primary time">,
    Mustard::Data::Value<muc::array3f, "x0", "Primary position">,
    Mustard::Data::Value<float, "Ek0", "Primary kinetic energy">,
    Mustard::Data::Value<muc::array3f, "p0", "Primary momentum">>;

} // namespace MACE::Data
