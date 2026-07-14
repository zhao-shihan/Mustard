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

#pragma once

#include "Mustard/EigenX/AlignedVector3.h++"

namespace Mustard::Geant4X::inline Process {

/// @brief Abstract interface describing a target's fine structure (tiny sub-Geant4-geometry detail)
///        for atom thermal scattering process.
/// @note  Positions/directions are transformed between the global frame and a fine-structure-local
///        frame; the containment predicates are queried in the local frame. A concrete target derives
///        from this interface and, typically, keeps it consistent with its Geant4 geometry.
class FineStructureForAtomThermalScattering {
public:
    virtual ~FineStructureForAtomThermalScattering() = default;

    /// @brief In-place transform of a position from the global frame to the fine-structure-local frame.
    /// @param x Position to transform in place.
    /// @note  Default implementation is the identity transform.
    virtual auto TransformPositionFromGlobalFrameToFineStructureLocalFrame(EigenX::AlignedVector3d&) const -> void {}
    /// @brief In-place transform of a direction from the global frame to the fine-structure-local frame.
    /// @param d Direction to transform in place.
    /// @note  Default implementation is the identity transform.
    virtual auto TransformDirectionFromGlobalFrameToFineStructureLocalFrame(EigenX::AlignedVector3d&) const -> void {}
    /// @brief In-place transform of a position from the fine-structure-local frame to the global frame.
    /// @param x Position to transform in place.
    /// @note  Default implementation is the identity transform.
    virtual auto TransformPositionFromFineStructureLocalFrameToGlobalFrame(EigenX::AlignedVector3d&) const -> void {}
    /// @brief In-place transform of a direction from the fine-structure-local frame to the global frame.
    /// @param d Direction to transform in place.
    /// @note  Default implementation is the identity transform.
    virtual auto TransformDirectionFromFineStructureLocalFrameToGlobalFrame(EigenX::AlignedVector3d&) const -> void {}

    /// @brief Whether a point lies inside the region that carries the fine structure.
    /// @param p Position expressed in the fine-structure-local frame.
    /// @return  true if the point is inside the fine-structure-bearing region.
    /// @note    Must be consistent with the Geant4 geometry.
    virtual auto FineStructureVolumeContains(const EigenX::AlignedVector3d& p) const -> bool = 0;
    /// @brief Whether a point lies inside the tiny fine structure that cannot be described by Geant4 geometry.
    /// @param p Position expressed in the fine-structure-local frame.
    /// @return  true if the point is inside the sub-geometry fine structure.
    virtual auto FineStructureSubterm(const EigenX::AlignedVector3d& p) const -> bool = 0;
    /// @brief Whether a point lies inside the fine structure (both in the fine-structure volume and its subterm).
    /// @param p Position expressed in the fine-structure-local frame.
    /// @return  FineStructureVolumeContains(p) and FineStructureSubterm(p).
    /// @note User can override this to implement a more efficient predicate that does not require two separate checks.
    virtual auto FineStructureContains(const EigenX::AlignedVector3d& p) const -> bool { return FineStructureVolumeContains(p) and FineStructureSubterm(p); }
};

} // namespace Mustard::Geant4X::inline Process
