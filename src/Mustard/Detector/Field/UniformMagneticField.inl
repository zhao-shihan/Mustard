namespace Mustard::Detector::Field {

constexpr UniformMagneticField::UniformMagneticField(double bx, double by, double bz) :
    MagneticFieldBase<UniformMagneticField>{},
    fBx{bx},
    fBy{by},
    fBz{bz} {}

template<Concept::InputVector3D T>
constexpr UniformMagneticField::UniformMagneticField(T b) :
    UniformMagneticField{b[0], b[1], b[2]} {}

} // namespace Mustard::Detector::Field
