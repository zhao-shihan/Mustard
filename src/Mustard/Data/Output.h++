// -*- C++ -*-
//
// Copyright 2020-2024  The Mustard development team
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

#include "Mustard/Data/Tuple.h++"
#include "Mustard/Data/TupleModel.h++"
#include "Mustard/Data/internal/BranchHelper.h++"
#include "Mustard/Env/Print.h++"
#include "Mustard/Utility/NonMoveableBase.h++"

#include "TDirectory.h"
#include "TLeaf.h"
#include "TTree.h"

#include "muc/utility"

#include "fmt/format.h"

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <filesystem>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <ranges>
#include <string>
#include <type_traits>

class TFile;

namespace Mustard::Data {

template<TupleModelizable... Ts>
class Output : public NonMoveableBase {
public:
    using Model = TupleModel<Ts...>;

public:
    explicit Output(const std::string& name, const std::string& title = {});

    template<typename T = Tuple<Ts...>>
        requires std::assignable_from<Tuple<Ts...>&, T&&>
    auto Fill(T&& tuple) -> std::size_t;
    template<typename T>
        requires ProperSubTuple<Tuple<Ts...>, std::decay_t<T>>
    auto Fill(T&& tuple) -> std::size_t;

    template<std::ranges::input_range R = std::initializer_list<Tuple<Ts...>>>
        requires std::assignable_from<Tuple<Ts...>&, std::ranges::range_reference_t<R>> or
                     ProperSubTuple<Tuple<Ts...>, std::ranges::range_value_t<R>>
    auto Fill(R&& data) -> std::size_t;

    template<std::ranges::input_range R>
        requires std::indirectly_readable<std::ranges::range_reference_t<R>> and
                     (std::assignable_from<Tuple<Ts...>&, std::iter_reference_t<std::ranges::range_value_t<R>>> or
                      ProperSubTuple<Tuple<Ts...>, std::iter_value_t<std::ranges::range_value_t<R>>>)
    auto Fill(R&& data) -> std::size_t;

    auto Entry() -> auto { return OutputIterator{this}; }

    auto Write(int option = 0, int bufferSize = 0) const -> auto;

private:
    class OutputIterator final {
    public:
        using difference_type = std::ptrdiff_t;
        using value_type = void;
        using pointer = void;
        using reference = void;
        using iterator_category = std::output_iterator_tag;

    public:
        explicit OutputIterator(Output* output);

        template<typename T = Tuple<Ts...>>
        auto operator=(T&& data) const -> auto& { return *fOutput << std::forward<T>(data), *this; }

        auto operator*() const -> auto& { return *this; }
        auto operator++() const -> auto& { return *this; }
        auto operator++(int) const -> auto& { return *this; }

    private:
        Output* fOutput;
    };

private:
    Tuple<Ts...> fEntry;
    std::string fROOTPath;
    std::unique_ptr<TTree> fTree;
    internal::BranchHelper<Tuple<Ts...>> fBranchHelper;
};

} // namespace Mustard::Data

#include "Mustard/Data/Output.inl"
