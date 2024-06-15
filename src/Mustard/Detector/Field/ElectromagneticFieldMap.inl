namespace Mustard::Detector::Field {

template<typename AFieldMap>
template<Concept::NumericVector3D T>
auto ElectromagneticFieldMap<"WithCache", AFieldMap>::B(T x) const -> T {
    const auto xEigen{VectorCast<Eigen::Vector3d>(x)};
    if (xEigen != fCachedX or fCache == std::nullopt) {
        fCachedX = xEigen;
        fCache = (*this)(x[0], x[1], x[2]);
    }
    const auto& f{*fCache};
    return {f[0], f[1], f[2]};
}

template<typename AFieldMap>
template<Concept::NumericVector3D T>
auto ElectromagneticFieldMap<"WithCache", AFieldMap>::E(T x) const -> T {
    const auto xEigen{VectorCast<Eigen::Vector3d>(x)};
    if (xEigen != fCachedX or fCache == std::nullopt) {
        fCachedX = xEigen;
        fCache = (*this)(x[0], x[1], x[2]);
    }
    const auto& f{*fCache};
    return {f[3], f[4], f[5]};
}

template<typename AFieldMap>
template<Concept::NumericVector3D T>
auto ElectromagneticFieldMap<"WithCache", AFieldMap>::BE(T x) const -> F<T> {
    fCachedX = VectorCast<Eigen::Vector3d>(x);
    fCache = (*this)(x[0], x[1], x[2]);
    const auto& f{*fCache};
    return {VectorCast<T>(f[0], f[1], f[2]),
            VectorCast<T>(f[3], f[4], f[5])};
}

template<typename AFieldMap>
template<Concept::NumericVector3D T>
auto ElectromagneticFieldMap<"NoCache", AFieldMap>::B(T x) const -> T {
    const auto f{(*this)(x[0], x[1], x[2])};
    return {f[0], f[1], f[2]};
}

template<typename AFieldMap>
template<Concept::NumericVector3D T>
auto ElectromagneticFieldMap<"NoCache", AFieldMap>::E(T x) const -> T {
    const auto f{(*this)(x[0], x[1], x[2])};
    return {f[3], f[4], f[5]};
}

template<typename AFieldMap>
template<Concept::NumericVector3D T>
auto ElectromagneticFieldMap<"NoCache", AFieldMap>::BE(T x) const -> F<T> {
    const auto v{(*this)(x[0], x[1], x[2])}; // clang-format off
    return {{v[0], v[1], v[2]}, {v[3], v[4], v[5]}}; // clang-format on
}

} // namespace Mustard::Detector::Field
