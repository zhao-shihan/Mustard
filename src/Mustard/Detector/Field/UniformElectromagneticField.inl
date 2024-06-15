namespace Mustard::Detector::Field {

constexpr UniformElectromagneticField::UniformElectromagneticField(double bx, double by, double bz,
                                                                   double ex, double ey, double ez) :
    ElectromagneticFieldBase<UniformElectromagneticField>{},
    fBx{bx},
    fBy{by},
    fBz{bz},
    fEx{ex},
    fEy{ey},
    fEz{ez} {}

template<Concept::InputVector3D T1, Concept::InputVector3D T2>
constexpr UniformElectromagneticField::UniformElectromagneticField(T1 b, T2 e) :
    UniformElectromagneticField{b[0], b[1], b[2],
                                e[0], e[1], e[2]} {}

} // namespace Mustard::Detector::Field
