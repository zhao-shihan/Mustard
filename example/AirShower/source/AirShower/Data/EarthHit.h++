#pragma once

#include "Mustard/Data/Tuple.h++"
#include "Mustard/Data/Value.h++"

#include "muc/array"

namespace AirShower::Data {

using EarthHit = Mustard::Data::TupleModel<
    Mustard::Data::Value<int, "EvtID", "Event ID">,
    Mustard::Data::Value<int, "TrkID", "Track ID">,
    Mustard::Data::Value<int, "PDGID", "Particle PDG ID">,
    Mustard::Data::Value<double, "t", "Hit time">,
    Mustard::Data::Value<double, "t0", "Vertex time">,
    Mustard::Data::Value<muc::array3f, "x", "Hit position">,
    Mustard::Data::Value<muc::array3f, "x0", "Vertex position">,
    Mustard::Data::Value<float, "Ek", "Hit kinetic energy">,
    Mustard::Data::Value<float, "Ek0", "Vertex kinetic energy">,
    Mustard::Data::Value<muc::array3f, "p", "Hit momentum">,
    Mustard::Data::Value<muc::array3f, "p0", "Vertex momentum">,
    Mustard::Data::Value<float, "theta", "Direction zenith angle">,
    Mustard::Data::Value<float, "phi", "Direction azimuth angle">,
    Mustard::Data::Value<float, "len", "Track total length">,
    Mustard::Data::Value<std::string, "CreatProc", "Track creator process">>;

} // namespace AirShower::Data
