namespace Mustard::Env::Memory {

template<typename ADerived>
PassiveSingleton<ADerived>::PassiveSingleton() :
    PassiveSingletonBase{},
    Base{} {
    static_assert(PassiveSingletonified<ADerived>);
}

template<typename ADerived>
MUSTARD_ALWAYS_INLINE auto PassiveSingleton<ADerived>::Instance() -> ADerived& {
    switch (Base::UpdateInstance()) {
    [[unlikely]] case Base::Status::NotInstantiated:
        throw std::logic_error{fmt::format("Mustard::Env::Memory::PassiveSingleton::Instance(): "
                                           "{} (passive singleton in environment) has not been instantiated",
                                           typeid(ADerived).name())};
    [[likely]] case Base::Status::Available:
        return *static_cast<ADerived*>(*Base::fgInstance);
    [[unlikely]] case Base::Status::Expired:
        throw std::logic_error{fmt::format("Mustard::Env::Memory::PassiveSingleton::Instance(): "
                                           "The instance of {} (passive singleton in environment) has been deleted",
                                           typeid(ADerived).name())};
    }
    muc::unreachable();
}

} // namespace Mustard::Env::Memory
