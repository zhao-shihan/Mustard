#pragma once

#include "mpi.h"

#include <complex>
#include <concepts>
#include <cstdint>

namespace Mustard::Concept {

template<typename T>
concept MPIPredefined =
    std::same_as<T, char> or
    std::same_as<T, signed short int> or
    std::same_as<T, signed int> or
    std::same_as<T, signed long> or
    std::same_as<T, signed long long> or
    std::same_as<T, signed long long> or
    std::same_as<T, signed char> or
    std::same_as<T, unsigned char> or
    std::same_as<T, unsigned short> or
    std::same_as<T, unsigned int> or
    std::same_as<T, unsigned long> or
    std::same_as<T, unsigned long long> or
    std::same_as<T, float> or
    std::same_as<T, double> or
    std::same_as<T, long double> or
    std::same_as<T, wchar_t> or
    std::same_as<T, std::int8_t> or
    std::same_as<T, std::int16_t> or
    std::same_as<T, std::int32_t> or
    std::same_as<T, std::int64_t> or
    std::same_as<T, std::uint8_t> or
    std::same_as<T, std::uint16_t> or
    std::same_as<T, std::uint32_t> or
    std::same_as<T, std::uint64_t> or
    std::same_as<T, MPI_Aint> or
    std::same_as<T, MPI_Count> or
    std::same_as<T, MPI_Offset> or
    std::same_as<T, bool> or
    std::same_as<T, std::complex<float>> or
    std::same_as<T, std::complex<double>> or
    std::same_as<T, std::complex<long double>>;

} // namespace Mustard::Concept
