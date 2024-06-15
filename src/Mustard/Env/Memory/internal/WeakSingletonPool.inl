namespace Mustard::Env::Memory::internal {

template<WeakSingletonified AWeakSingleton>
[[nodiscard]] auto WeakSingletonPool::Find() -> std::shared_ptr<void*> {
    if (const auto existed = fInstanceMap.find(typeid(AWeakSingleton));
        existed == fInstanceMap.cend()) {
        return {};
    } else {
        const auto& [type, instance]{*existed};
        if (instance.expired()) {
            throw std::logic_error{fmt::format("Mustard::Env::Memory::internal::WeakSingletonPool::Find(): "
                                               "Instance pointer of {} expired",
                                               type.name())};
        }
        return instance.lock();
    }
}

template<WeakSingletonified AWeakSingleton>
[[nodiscard]] auto WeakSingletonPool::Insert(gsl::not_null<AWeakSingleton*> instance) -> std::shared_ptr<void*> {
    const auto sharedInstance{std::make_shared<void*>(instance)};
    const auto [_, inserted]{fInstanceMap.try_emplace(typeid(AWeakSingleton), sharedInstance)};
    if (not inserted) {
        throw std::logic_error{fmt::format("Mustard::Env::Memory::internal::WeakSingletonPool::Insert: "
                                           "Instance of type {} already exists",
                                           typeid(AWeakSingleton).name())};
    }
    return sharedInstance;
}

} // namespace Mustard::Env::Memory::internal
