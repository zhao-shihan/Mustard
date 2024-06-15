namespace Mustard::Detector::Field {

template<typename ADerived>
constexpr ElectricFieldBase<ADerived>::ElectricFieldBase() :
    ElectromagneticFieldBase<ADerived>{} {
    static_assert(std::derived_from<ADerived, ElectricFieldBase<ADerived>>);
    static_assert(not std::derived_from<ADerived, MagneticFieldBase<ADerived>>);
    static_assert(ElectricField<ADerived>);
}

} // namespace Mustard::Detector::Field
