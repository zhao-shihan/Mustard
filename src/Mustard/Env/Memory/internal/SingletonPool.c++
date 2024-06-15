#include "Mustard/Env/Memory/internal/SingletonPool.h++"

#include <algorithm>
#include <cassert>

namespace Mustard::Env::Memory::internal {

SingletonPool::~SingletonPool() {
    for ([[maybe_unused]] auto&& [_, instanceInfo] : std::as_const(fInstanceMap)) {
        [[maybe_unused]] auto&& [instance, __, ___]{instanceInfo};
        assert(not instance.expired());
        assert(*instance.lock() == nullptr);
    }
}

[[nodiscard]] auto SingletonPool::GetUndeletedInReverseInsertionOrder() const -> std::vector<gsl::owner<const SingletonBase*>> {
    std::vector<std::pair<gsl::index, gsl::owner<const SingletonBase*>>> undeletedListWithID;
    undeletedListWithID.reserve(fInstanceMap.size());
    for (auto&& [type, instanceInfo] : fInstanceMap) {
        auto&& [instance, index, base]{instanceInfo};
        if (instance.expired()) {
            throw std::logic_error{fmt::format("Mustard::Env::Memory::internal::SingletonPool::GetUndeletedInReverseInsertionOrder(): "
                                               "Instance pointer of {} expired",
                                               type.name())};
        }
        if (*instance.lock() != nullptr) {
            undeletedListWithID.emplace_back(index, base);
        }
    }

    std::ranges::sort(undeletedListWithID,
                      [](const auto& lhs, const auto& rhs) {
                          return lhs.first > rhs.first;
                      });

    std::vector<gsl::owner<const SingletonBase*>> undeletedList;
    undeletedList.reserve(undeletedListWithID.size());
    for (auto&& [id, base] : std::as_const(undeletedListWithID)) {
        undeletedList.emplace_back(base);
    }
    return undeletedList;
}

} // namespace Mustard::Env::Memory::internal
