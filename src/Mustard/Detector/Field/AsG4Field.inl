namespace Mustard::Detector::Field {

template<ElectromagneticField AField, bool AEMFieldChangeEnergy>
auto AsG4Field<AField, AEMFieldChangeEnergy>::GetFieldValue(const G4double* x, G4double* f) const -> void {
    std::ranges::copy(std::bit_cast<std::array<G4double, 6>>(
                          this->BE(
                              VectorCast<muc::array3d>(x))),
                      f);
}

} // namespace Mustard::Detector::Field
