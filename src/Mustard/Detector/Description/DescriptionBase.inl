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

template<typename AValue, typename AReadAs, std::convertible_to<std::string>... AStrings>
    requires std::assignable_from<AValue&, AReadAs>
auto DescriptionBase<>::ImportValue(const YAML::Node& node, AValue& value, AStrings&&... nodeNames) -> void {
    if (const auto leaf = UnpackToLeafNodeForImporting(node, nodeNames...);
        leaf.has_value()) {
        value = leaf->template as<AReadAs>();
    } else {
        PrintNodeNotFoundNotice(nodeNames...);
    }
}

template<typename AReadAs, std::convertible_to<std::string>... AStrings>
auto DescriptionBase<>::ImportValue(const YAML::Node& node, const std::regular_invocable<AReadAs> auto& ImportAction, AStrings&&... nodeNames) -> void {
    if (const auto leaf = UnpackToLeafNodeForImporting(node, nodeNames...);
        leaf.has_value()) {
        ImportAction(leaf->template as<AReadAs>());
    } else {
        PrintNodeNotFoundNotice(nodeNames...);
    }
}

template<typename AValue, typename AWriteAs, std::convertible_to<std::string>... AStrings>
    requires std::convertible_to<const AValue&, AWriteAs>
auto DescriptionBase<>::ExportValue(YAML::Node& node, const AValue& value, AStrings&&... nodeNames) const -> void {
    UnpackToLeafNodeForExporting(node, nodeNames...) = static_cast<AWriteAs>(value);
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

template<std::convertible_to<std::string>... AStrings>
auto DescriptionBase<>::UnpackToLeafNodeForImporting(const YAML::Node& node, AStrings&&... nodeNames) -> std::optional<const YAML::Node> {
    try {
        std::array<YAML::Node, sizeof...(nodeNames)> leafNodes;
        gsl::index i{};
        internal::TupleForEach(std::tie(std::forward<AStrings>(nodeNames)...),
                               [&node, &leafNodes, &i](auto&& name) {
                                   leafNodes[i] = (i == 0 ? node : leafNodes[i - 1])[name];
                                   ++i;
                               });
        return leafNodes.back();
    } catch (const YAML::InvalidNode&) {
        return std::nullopt;
    }
}

template<std::convertible_to<std::string>... AStrings>
auto DescriptionBase<>::UnpackToLeafNodeForExporting(YAML::Node& node, AStrings&&... nodeNames) const -> YAML::Node {
    std::array<YAML::Node, sizeof...(nodeNames)> leafNodes;
    gsl::index i{};
    internal::TupleForEach(std::tie(std::forward<AStrings>(nodeNames)...),
                           [&node, &leafNodes, &i](auto&& name) {
                               leafNodes[i] = (i == 0 ? node : leafNodes[i - 1])[name];
                               ++i;
                           });
    return leafNodes.back();
}

template<std::convertible_to<std::string>... AStrings>
auto DescriptionBase<>::PrintNodeNotFoundNotice(AStrings&&... nodeNames) const -> void {
    Env::PrintInfo("Notice: YAML node '{}", fName);
    internal::TupleForEach(std::tie(std::forward<AStrings>(nodeNames)...),
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
