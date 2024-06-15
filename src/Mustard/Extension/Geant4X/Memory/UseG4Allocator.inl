namespace Mustard::inline Extension::Geant4X::inline Memory {

template<typename ADerived>
[[nodiscard]] MUSTARD_ALWAYS_INLINE auto UseG4Allocator<ADerived>::operator new(std::size_t) -> void* {
    static_assert(std::derived_from<ADerived, UseG4Allocator<ADerived>>);
    static_assert(std::is_final_v<ADerived>);
    return SingletonG4Allocator<ADerived>::Instance().MallocSingle();
}

template<typename ADerived>
MUSTARD_ALWAYS_INLINE auto UseG4Allocator<ADerived>::operator delete(void* ptr) -> void {
    SingletonG4Allocator<ADerived>::Instance().FreeSingle(static_cast<ADerived*>(ptr));
}

} // namespace Mustard::inline Extension::Geant4X::inline Memory
