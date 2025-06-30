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

template<typename T>
DescriptionWithCacheBase<>::Simple<T>::Simple(const DescriptionWithCacheBase<>* description, const T& value) :
    NonCopyableBase{},
    fValue{value},
    fDescription{description} {}

template<typename T>
DescriptionWithCacheBase<>::Simple<T>::Simple(const DescriptionWithCacheBase<>* description, T&& value) :
    NonCopyableBase{},
    fValue{std::move(value)},
    fDescription{description} {}

template<typename T>
DescriptionWithCacheBase<>::Simple<T>::Simple(const DescriptionWithCacheBase<>* description, auto&&... args) :
    NonCopyableBase{},
    fValue{std::forward<decltype(args)>(args)...},
    fDescription{description} {}

template<typename T>
template<typename U>
    requires std::assignable_from<T&, U&&>
auto DescriptionWithCacheBase<>::Simple<T>::operator=(U&& other) -> auto& {
    fDescription->ExpireCache();
    fValue = std::forward<U>(other);
    return *this;
}

template<typename T>
DescriptionWithCacheBase<>::Cached<T>::Cached(DescriptionWithCacheBase<>* description, std::function<auto()->T> CalculateValue) :
    CacheBase{description},
    fValue{},
    fCalculateValue{std::move(CalculateValue)} {}

template<typename T>
DescriptionWithCacheBase<>::Cached<T>::operator const T&() const {
    if (fUpToDate) [[likely]] {
        return fValue;
    }
    fValue = fCalculateValue();
    fUpToDate = true;
    return fValue;
}

template<typename AValue, typename AReadAs>
    requires std::assignable_from<AValue&, AReadAs>
auto DescriptionWithCacheBase<>::ImportValue(const YAML::Node& node, Simple<AValue>& value, std::convertible_to<std::string> auto&&... names) -> void {
    DescriptionBase::ImportValue<Simple<AValue>, AReadAs>(node, value, std::forward<decltype(names)>(names)...);
}

template<typename AValue, typename AWriteAs>
    requires std::convertible_to<const AValue&, AWriteAs>
auto DescriptionWithCacheBase<>::ExportValue(YAML::Node& node, const Simple<AValue>& value, std::convertible_to<std::string> auto&&... names) const -> void {
    DescriptionBase::ExportValue<AValue, AWriteAs>(node, *value, std::forward<decltype(names)>(names)...);
}

} // namespace Mustard::Detector::Description
