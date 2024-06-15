namespace Mustard::Data::internal {

template<typename ADerived, internal::UniqueStdTuple AStdTuple>
template<muc::ceta_string AName, gsl::index I>
consteval auto ModelBase<ADerived, AStdTuple>::IndexImpl() -> gsl::index {
    if constexpr (I == Size()) {
        static_assert(I < Size(), "no such value of this name within this data model");
        return StopConsteval();
    } else if constexpr (std::tuple_element_t<I, AStdTuple>::Name() == AName) {
        return I;
    } else {
        return IndexImpl<AName, I + 1>();
    }
}

} // namespace Mustard::Data::internal
