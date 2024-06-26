#pragma once

#include "Mustard/Data/TupleModel.h++"
#include "Mustard/Data/Value.h++"

#include "muc/array"

namespace AirShower::Data {

using DecayVertex = Mustard::Data::TupleModel<
    Mustard::Data::Value<int, "EvtID", "Event ID">,
    Mustard::Data::Value<int, "TrkID", "Parent track ID">,
    Mustard::Data::Value<int, "PDGID", "Parent PDG ID">,
    Mustard::Data::Value<std::vector<int>, "SecPDGID", "PDG ID of decay products">,
    Mustard::Data::Value<double, "t", "Decay time">,
    Mustard::Data::Value<muc::array3f, "x", "Decay position">,
    Mustard::Data::Value<float, "Ek", "Parent kinetic energy at decay">,
    Mustard::Data::Value<muc::array3f, "p", "Parent momentum at decay">>;

} // namespace MACE::Data
