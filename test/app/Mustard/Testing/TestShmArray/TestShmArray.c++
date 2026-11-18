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

#include "Mustard/CLI/BasicCLI.h++"
#include "Mustard/Env/MPIEnv.h++"
#include "Mustard/IO/PrettyLog.h++"
#include "Mustard/Parallel/ShmArray.h++"
#include "Mustard/Testing/TestShmArray/TestShmArray.h++"

#include "mp1/mp1.hpp"

#include <array>
#include <cstdlib>
#include <numeric>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

namespace Mustard::Testing {

namespace {

constexpr auto nElement{1024};

auto MakeSource() -> std::vector<int> {
    std::vector<int> source(nElement);
    std::iota(source.begin(), source.end(), 0);
    return source;
}

auto CheckIntArray(const Parallel::ShmArray<int>& array) -> void {
    if (array.size() != static_cast<std::size_t>(nElement)) {
        Throw<std::runtime_error>("ShmArray<int> has the wrong element count.");
    }
    if (array.size_bytes() != static_cast<std::size_t>(nElement) * sizeof(int)) {
        Throw<std::runtime_error>("ShmArray<int> has the wrong byte size.");
    }
    if (array.empty() or array.front() != 0 or array.back() != nElement - 1) {
        Throw<std::runtime_error>("ShmArray<int> has invalid boundary elements.");
    }
    for (std::size_t i{}; i < array.size(); ++i) {
        if (array[i] != static_cast<int>(i)) {
            Throw<std::runtime_error>("ShmArray<int> does not expose the expected shared data.");
        }
    }
}

} // namespace

TestShmArray::TestShmArray() :
    Subprogram{"TestShmArray", "Test Mustard::Parallel::ShmArray."} {}

auto TestShmArray::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::MPIEnv env{argc, argv, cli};

    const auto& worldComm{mp1::comm_world()};
    // Two kinds of roots are involved. The inter-node root is world rank 0, while
    // the intra-node constructors allocate the shared window on rank 0 of the
    // local node's intra-node communicator.
    const auto isRoot{worldComm.rank() == 0};
    const auto intraRoot{env.IntraNodeComm().rank() == 0};

    // Local path (one process) and intra-node shared-memory path (several processes).
    // The intra-node constructor reads the source only on the intra-node root, so
    // each node supplies it from its own intra-node root.
    {
        auto source{intraRoot ? MakeSource() : std::vector<int>{}};
        Parallel::ShmArray<int> array{source};
        CheckIntArray(array);

        Parallel::ShmArray<int> moved{std::move(array)};
        CheckIntArray(moved);

        Parallel::ShmArray<int> assigned;
        assigned = std::move(moved);
        CheckIntArray(assigned);
    }

    // Inter-node broadcast path. The source is read only on the inter-node root
    // (world rank 0). This reduces to the intra-node path on a single node.
    {
        auto source{isRoot ? MakeSource() : std::vector<int>{}};
        Parallel::ShmArray<int> array{Parallel::BcastTag{}, source};
        CheckIntArray(array);
    }

    // Non-root attachment path; the intra-node root still supplies the source data.
    // This block is restricted to a single node, where world rank 0 is also the
    // intra-node root, so isRoot is the correct source rank here.
    if (worldComm.size() > 1 and env.OnSingleNode()) {
        std::optional<Parallel::ShmArray<int>> array;
        if (isRoot) {
            array.emplace(MakeSource());
        } else {
            array.emplace(Parallel::NonRootTag{});
        }
        CheckIntArray(*array);
    }

    // Zero-length construction must not allocate a shared window.
    {
        Parallel::ShmArray<int> empty{std::vector<int>{}};
        if (not empty.empty() or empty.size() != 0 or empty.size_bytes() != 0) {
            Throw<std::runtime_error>("An empty ShmArray is not empty.");
        }
    }

    // A non-scalar trivially copyable element type. As in the first intra-node
    // block, the source is read only on the intra-node root.
    {
        using Element = std::array<int, 3>;
        std::vector<Element> source;
        if (intraRoot) {
            source.resize(nElement);
            for (std::size_t i{}; i < source.size(); ++i) {
                source[i] = {static_cast<int>(i), static_cast<int>(i + 1), static_cast<int>(i + 2)};
            }
        }
        Parallel::ShmArray<Element> array{source};
        if (array.size() != static_cast<std::size_t>(nElement)) {
            Throw<std::runtime_error>("ShmArray<std::array<int, 3>> has the wrong element count.");
        }
        for (std::size_t i{}; i < array.size(); ++i) {
            if (array[i] != Element{static_cast<int>(i), static_cast<int>(i + 1), static_cast<int>(i + 2)}) {
                Throw<std::runtime_error>("ShmArray<std::array<int, 3>> does not expose the expected shared data.");
            }
        }
    }

    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
