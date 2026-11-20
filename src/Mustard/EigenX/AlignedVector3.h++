// -*- C++ -*-
//
// Copyright (C) 2020-2026  Shihan Zhao and contributors
//
// This file is part of Mustard, an offline software framework for HEP experiments.
//
// Mustard is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// Mustard is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// Mustard. If not, see <https://www.gnu.org/licenses/>.

#include "unsupported/Eigen/AlignedVector3"

#include <complex>

namespace Mustard::EigenX {

using AlignedVector3f = Eigen::AlignedVector3<float>;
using AlignedVector3d = Eigen::AlignedVector3<double>;
using AlignedVector3cf = Eigen::AlignedVector3<std::complex<float>>;
using AlignedVector3cd = Eigen::AlignedVector3<std::complex<double>>;

} // namespace Mustard::EigenX
