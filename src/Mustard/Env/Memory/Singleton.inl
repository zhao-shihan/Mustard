namespace Mustard::Env::Memory {

template<typename ADerived>
std::shared_ptr<void*> Singleton<ADerived>::fgInstance{};

template<typename ADerived>
Singleton<ADerived>::Singleton() :
    SingletonBase{} {
    static_assert(Singletonified<ADerived>);
}

template<typename ADerived>
Singleton<ADerived>::~Singleton() {
    UpdateInstance();
    *fgInstance = nullptr;
}

template<typename ADerived>
MUSTARD_ALWAYS_INLINE auto Singleton<ADerived>::Instance() -> ADerived& {
    switch (UpdateInstance()) {
    [[unlikely]] case Status::NotInstantiated: {
        auto& pool{internal::SingletonPool::Instance()};
        if (pool.Contains<ADerived>()) {
            throw std::logic_error{fmt::format("Mustard::Env::Memory::Singleton: "
                                               "Trying to construct {} (environmental singleton) twice",
                                               typeid(ADerived).name())};
        }
        fgInstance = pool.Insert<ADerived>(SingletonInstantiator::New<ADerived>());
        [[fallthrough]];
    }
    [[likely]] case Status::Available:
        return *static_cast<ADerived*>(*fgInstance);
    [[unlikely]] case Status::Expired:
        throw std::logic_error{fmt::format("Mustard::Env::Memory::Singleton::Instance(): "
                                           "The instance of {} has been deleted",
                                           typeid(ADerived).name())};
    }
    muc::unreachable();
}

template<typename ADerived>
MUSTARD_ALWAYS_INLINE auto Singleton<ADerived>::UpdateInstance() -> Status {
    if (fgInstance == nullptr) [[unlikely]] {
        if (const auto sharedNode{internal::SingletonPool::Instance().Find<ADerived>()};
            sharedNode == nullptr) {
            return Status::NotInstantiated;
        } else {
            fgInstance = sharedNode;
        }
    }
    if (*fgInstance == nullptr) {
        return Status::Expired;
    } else {
        return Status::Available;
    }
}

} // namespace Mustard::Env::Memory
