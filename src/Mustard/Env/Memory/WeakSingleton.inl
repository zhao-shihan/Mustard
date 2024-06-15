namespace Mustard::Env::Memory {

template<typename ADerived>
std::shared_ptr<void*> WeakSingleton<ADerived>::fgInstance{};

template<typename ADerived>
WeakSingleton<ADerived>::WeakSingleton() :
    WeakSingletonBase{} {
    static_assert(WeakSingletonified<ADerived>);
    if (auto& weakSingletonPool = internal::WeakSingletonPool::Instance();
        not weakSingletonPool.Contains<ADerived>()) {
        fgInstance = weakSingletonPool.Insert<ADerived>(static_cast<ADerived*>(this));
    } else {
        throw std::logic_error{fmt::format("Mustard::Env::Memory::WeakSingleton::WeakSingleton(): "
                                           "Trying to construct {} (weak singleton in environment) twice",
                                           typeid(ADerived).name())};
    }
}

template<typename ADerived>
WeakSingleton<ADerived>::~WeakSingleton() {
    UpdateInstance();
    *fgInstance = nullptr;
}

template<typename ADerived>
MUSTARD_ALWAYS_INLINE auto WeakSingleton<ADerived>::UpdateInstance() -> Status {
    if (fgInstance == nullptr) [[unlikely]] {
        if (const auto sharedNode{internal::WeakSingletonPool::Instance().Find<ADerived>()};
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
