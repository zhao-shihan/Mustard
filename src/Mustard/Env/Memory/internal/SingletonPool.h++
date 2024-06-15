#pragma once

#include "Mustard/Env/Memory/PassiveSingleton.h++"
#include "Mustard/Env/Memory/Singletonified.h++"
#include "Mustard/Utility/NonMoveableBase.h++"

#include "gsl/gsl"

#include "fmt/format.h"

#include <memory>
#include <stdexcept>
#include <tuple>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <utility>
#include <vector>

namespace Mustard::Env::Memory::internal {

class SingletonBase;

/// @brief Implementation detail of Mustard::Env::Memory::Singleton.
/// Not API.
class SingletonPool final : public PassiveSingleton<SingletonPool> {
public:
    ~SingletonPool();

    template<Singletonified ASingleton>
    [[nodiscard]] auto Find() -> std::shared_ptr<void*>;
    template<Singletonified ASingleton>
    [[nodiscard]] auto Contains() const -> auto { return fInstanceMap.contains(typeid(ASingleton)); }
    template<Singletonified ASingleton>
    [[nodiscard]] auto Insert(gsl::not_null<ASingleton*> instance) -> std::shared_ptr<void*>;
    [[nodiscard]] auto GetUndeletedInReverseInsertionOrder() const -> std::vector<gsl::owner<const SingletonBase*>>;

private:
    std::unordered_map<std::type_index, const std::tuple<std::weak_ptr<void*>, gsl::index, gsl::owner<const SingletonBase*>>> fInstanceMap;
};

} // namespace Mustard::Env::Memory::internal

#include "Mustard/Env/Memory/internal/SingletonPool.inl"
