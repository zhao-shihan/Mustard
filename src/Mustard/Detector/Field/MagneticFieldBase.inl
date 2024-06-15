namespace Mustard::Detector::Field {

template<typename ADerived>
constexpr MagneticFieldBase<ADerived>::MagneticFieldBase() :
    ElectromagneticFieldBase<ADerived>{} {
    static_assert(std::derived_from<ADerived, MagneticFieldBase<ADerived>>);
    static_assert(not std::derived_from<ADerived, ElectricFieldBase<ADerived>>);
    static_assert(MagneticField<ADerived>);
}

} // namespace Mustard::Detector::Field
