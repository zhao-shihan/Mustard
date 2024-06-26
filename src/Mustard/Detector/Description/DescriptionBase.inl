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

namespace Mustard::Detector::Description {

template<typename AValue, typename AReadAs>
    requires std::assignable_from<AValue&, AReadAs>
auto DescriptionBase<>::ImportValue(const YAML::Node& node, AValue& value, std::convertible_to<std::string> auto&&... names) -> void {
    if (const auto leaf{UnpackToLeafNodeForImporting(node, std::forward<decltype(names)>(names)...)};
        leaf.has_value()) {
        value = leaf->template as<AReadAs>();
    } else {
        PrintNodeNotFoundNotice(std::forward<decltype(names)>(names)...);
    }
}

template<typename AReadAs>
auto DescriptionBase<>::ImportValue(const YAML::Node& node, std::invocable<AReadAs> auto&& ImportAction, std::convertible_to<std::string> auto&&... names) -> void {
    if (const auto leaf{UnpackToLeafNodeForImporting(node, std::forward<decltype(names)>(names)...)};
        leaf.has_value()) {
        ImportAction(leaf->template as<AReadAs>());
    } else {
        PrintNodeNotFoundNotice(std::forward<decltype(names)>(names)...);
    }
}

template<typename AValue, typename AWriteAs>
    requires std::convertible_to<const AValue&, AWriteAs>
auto DescriptionBase<>::ExportValue(YAML::Node& node, const AValue& value, std::convertible_to<std::string> auto&&... names) const -> void {
    UnpackToLeafNodeForExporting(node, std::forward<decltype(names)>(names)...) = static_cast<AWriteAs>(value);
}

namespace internal {
namespace {

constexpr void TupleForEach(auto&& tuple, auto&& func) {
    std::apply(
        [&func](auto&&... args) {
            (..., func(std::forward<decltype(args)>(args)));
        },
        std::forward<decltype(tuple)>(tuple));
}

} // namespace
} // namespace internal

auto DescriptionBase<>::UnpackToLeafNodeForImporting(const YAML::Node& node, std::convertible_to<std::string> auto&&... names) -> std::optional<const YAML::Node> {
    try {
        std::array<YAML::Node, sizeof...(names)> leafNodes;
        gsl::index i{};
        internal::TupleForEach(std::tie(std::forward<decltype(names)>(names)...),
                               [&node, &leafNodes, &i](auto&& name) {
                                   leafNodes[i] = (i == 0 ? node : leafNodes[i - 1])[name];
                                   ++i;
                               });
        return leafNodes.back();
    } catch (const YAML::InvalidNode&) {
        return std::nullopt;
    }
}

auto DescriptionBase<>::UnpackToLeafNodeForExporting(YAML::Node& node, std::convertible_to<std::string> auto&&... names) const -> YAML::Node {
    std::array<YAML::Node, sizeof...(names)> leafNodes;
    gsl::index i{};
    internal::TupleForEach(std::tie(std::forward<decltype(names)>(names)...),
                           [&node, &leafNodes, &i](auto&& name) {
                               leafNodes[i] = (i == 0 ? node : leafNodes[i - 1])[name];
                               ++i;
                           });
    return leafNodes.back();
}

auto DescriptionBase<>::PrintNodeNotFoundNotice(std::convertible_to<std::string> auto&&... names) const -> void {
    Env::PrintInfo("Notice: YAML node '{}", fName);
    internal::TupleForEach(std::tie(std::forward<decltype(names)>(names)...),
                           [](auto&& name) {
                               Env::PrintInfo(".{}", name);
                           });
    Env::PrintLnInfo("' not defined, skipping");
}

template<typename ADerived>
DescriptionBase<ADerived>::DescriptionBase(std::string name) :
    Env::Memory::Singleton<ADerived>{},
    DescriptionBase<>{std::move(name)} {
    static_assert(Description<ADerived>);
}

} // namespace Mustard::Detector::Description
