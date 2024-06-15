namespace Mustard::Detector::Field {

template<Concept::InputVector3D T1, Concept::InputVector3D T2>
ToroidField::ToroidField(double b, double r0, T1 x0, T2 n) :
    MagneticFieldBase<ToroidField>{},
    fB{b},
    fR0{r0},
    fTransform{} {
    const Eigen::Vector3d normal{VectorCast<Eigen::Vector3d>(n).normalized()}; // clang-format off
    const Eigen::Vector3d axis{Eigen::Vector3d{0, 0, 1}.cross(normal)};
    const double angle{std::acos(Eigen::Vector3d{0, 0, 1}.dot(normal))}; // clang-format on
    fTransform = (Eigen::Translation3d{VectorCast<Eigen::Vector3d>(x0)} * Eigen::AngleAxisd{angle, axis}).inverse();
}

template<Concept::NumericVector3D T>
auto ToroidField::B(T x) const -> T {
    const auto x0{fTransform * VectorCast<Eigen::Vector3d>(x)};
    const auto alpha{fB * fR0 / x0.squaredNorm()};
    return {alpha * x0.z(), 0, -alpha * x0.x()};
}

} // namespace Mustard::Detector::Field
