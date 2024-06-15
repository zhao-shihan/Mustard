namespace Mustard::Env::Memory::internal {

template<Singletonified ASingleton>
[[nodiscard]] auto SingletonPool::Find() -> std::shared_ptr<void*> {
    if (const auto existed{std::as_const(fInstanceMap).find(typeid(ASingleton))};
        existed == fInstanceMap.cend()) {
        return {};
    } else {
        auto&& [type, instanceInfo]{*existed};
        auto&& instance{get<0>(instanceInfo)};
        if (instance.expired()) {
            throw std::logic_error{fmt::format("Mustard::Env::Memory::internal::SingletonPool::Find(): "
                                               "Instance pointer of {} expired",
                                               type.name())};
        }
        return instance.lock();
    }
}

template<Singletonified ASingleton>
[[nodiscard]] auto SingletonPool::Insert(gsl::not_null<ASingleton*> instance) -> std::shared_ptr<void*> {
    const auto sharedInstance{std::make_shared<void*>(instance)};
    const auto [_, inserted]{fInstanceMap.try_emplace(typeid(ASingleton),
                                                      std::tuple{sharedInstance,
                                                                 fInstanceMap.size(),
                                                                 static_cast<SingletonBase*>(instance)})};
    if (not inserted) {
        throw std::logic_error{fmt::format("Mustard::Env::Memory::internal::SingletonPool::Insert: "
                                           "Instance of type {} already exists",
                                           typeid(ASingleton).name())};
    }
    return sharedInstance;
}

} // namespace Mustard::Env::Memory::internal
