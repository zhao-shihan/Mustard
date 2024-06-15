namespace Mustard::Detector::Field {

constexpr UniformElectricField::UniformElectricField(double ex, double ey, double ez) :
    ElectricFieldBase<UniformElectricField>{},
    fEx{ex},
    fEy{ey},
    fEz{ez} {}

template<Concept::InputVector3D T>
constexpr UniformElectricField::UniformElectricField(T e) :
    UniformElectricField{e[0], e[1], e[2]} {}

} // namespace Mustard::Detector::Field
