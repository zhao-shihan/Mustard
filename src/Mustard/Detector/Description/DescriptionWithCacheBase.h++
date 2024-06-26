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

#include "Mustard/Detector/Description/DescriptionBase.h++"
#include "Mustard/Env/Memory/Singleton.h++"
#include "Mustard/Utility/NonMoveableBase.h++"

#include <concepts>
#include <functional>
#include <string>
#include <utility>
#include <vector>

namespace Mustard::Detector::Description {

template<typename... Ts>
    requires(sizeof...(Ts) <= 1)
class DescriptionWithCacheBase;

template<>
class DescriptionWithCacheBase<> : public DescriptionBase<> {
protected:
    DescriptionWithCacheBase(std::string name);
    ~DescriptionWithCacheBase() = default;

protected:
    template<typename T>
    friend class Simple;
    template<typename T>
    class Simple : public NonMoveableBase {
    public:
        Simple(const DescriptionWithCacheBase<>* description, const T& value);
        Simple(const DescriptionWithCacheBase<>* description, T&& value);
        Simple(const DescriptionWithCacheBase<>* description, auto&&... args);

        auto operator*() const -> const auto& { return fValue; }
        auto operator->() const -> auto { return &fValue; }
        operator const T&() const { return fValue; }

        template<typename U>
            requires std::assignable_from<T&, U&&>
        auto operator=(U&& other) -> auto&;

    private:
        T fValue;
        const DescriptionWithCacheBase<>* fDescription;
    };

    template<typename T>
    friend class Cached;
    template<typename T>
    class Cached : public NonMoveableBase {
    public:
        Cached(const DescriptionWithCacheBase<>* description, std::function<T()> CalculateValue);

        auto operator*() const -> const T& { return static_cast<const T&>(*this); }
        auto operator->() const -> auto { return &static_cast<const T&>(*this); }
        operator const T&() const;

    private:
        mutable T fValue;
        const DescriptionWithCacheBase<>* fDescription;
        std::function<T()> fCalculateValue;
    };

protected:
    template<typename AValue, typename AReadAs = AValue>
        requires std::assignable_from<AValue&, AReadAs>
    auto ImportValue(const YAML::Node& node, Simple<AValue>& value, std::convertible_to<std::string> auto&&... names) -> void;
    template<typename AValue, typename AWriteAs = AValue>
        requires std::convertible_to<const AValue&, AWriteAs>
    auto ExportValue(YAML::Node& node, const Simple<AValue>& value, std::convertible_to<std::string> auto&&... names) const -> void;

private:
    mutable bool fCacheUpToDate;
};

template<typename ADerived>
class DescriptionWithCacheBase<ADerived> : public Env::Memory::Singleton<ADerived>,
                                           public DescriptionWithCacheBase<> {
protected:
    using DescriptionWithCacheBase<>::DescriptionWithCacheBase;
    ~DescriptionWithCacheBase() = default;
};

} // namespace Mustard::Detector::Description

#include "Mustard/Detector/Description/DescriptionWithCacheBase.inl"
