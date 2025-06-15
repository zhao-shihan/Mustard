# Mustard

Mustard is a generic offline software framework for particle physics experiments, aimed at providing a modern, high-performace architecture for distributed computing capabilities, unified geometric interfaces, and high-level abstraction of data models out of the box. This framework is a core component of the MACE offline software, and was formerly developed within the MACE offline software.

- [Mustard](#mustard)
  - [Pre-built binaries](#pre-built-binaries)
  - [Dependencies](#dependencies)
    - [Toolchain](#toolchain)
    - [Libraries](#libraries)

## Pre-built binaries

Pre-built, optimized binaries are available through [Apptainer](https://apptainer.org/) (formerly known as Singularity).

For more information, please check out [the image repository homepage](https://github.com/zhao-shihan/Mustard-apptainer).

## Dependencies

### Toolchain

1. A C++ compiler (that supports ≥ C++20. [GCC](https://gcc.gnu.org/) ≥ 12, [LLVM Clang](https://clang.llvm.org/) ≥ 16. MSVC not tested.)
2. A C++ library (that supports ≥ C++20. [libstdc++](https://gcc.gnu.org/onlinedocs/libstdc++/) ≥ 12. [MSVC STL](https://github.com/microsoft/STL) or [libc++](https://libcxx.llvm.org) not tested)
3. [CMake](https://cmake.org/) (≥ 3.16)
4. A build system compatible with CMake ([GNU Make](https://www.gnu.org/software/make/), [Ninja](https://ninja-build.org), or etc.)

### Libraries

Required:

1. [Eigen (A C++ template library for linear algebra)](https://eigen.tuxfamily.org/) (≥ 3.4.0)
2. [Geant4 (Toolkit for the simulation of the passage of particles through matter)](https://geant4.org/) (≥ 11.0.0, **with GDML**)
3. [ROOT (An open-source data analysis framework)](https://root.cern/) (≥ 6.30.00, **with GDML**)
4. [MPI (Message Passing Interface)](https://www.mpi-forum.org/) (≥ 3.1, [MPICH](https://www.mpich.org/), [OpenMPI](https://www.open-mpi.org/), [Intel MPI](https://www.intel.cn/content/www/cn/zh/developer/tools/oneapi/mpi-library.html), etc.)

Required, built-in if not found (network or pre-downloaded source is required):

1. [argparse (Argument Parser for Modern C++)](https://github.com/p-ranav/argparse) (≥ 3.2, built-in if not found)
2. [zhao-shihan/backward-cpp (A beautiful stack trace pretty printer for C++)](https://github.com/zhao-shihan/backward-cpp) (≥ 1.6.1, built-in if not found)
3. [EFM (A generic bilinear/trilinear interpolator)](https://github.com/zhao-shihan/EFM) (≥ 0.1.0, built-in if not found)
4. [envparse (A simple, header-only environment variable parser in C++)](https://github.com/zhao-shihan/envparse) (≥ 1.0.0, built-in if not found)
5. [fmt (A modern formatting library)](https://github.com/fmtlib/fmt) (≥ 11.2.0, built-in if not found)
6. [indicators (Activity Indicators for Modern C++)](https://github.com/p-ranav/indicators) (≥ 2.3, built-in if not found)
7. [Microsoft.GSL](https://github.com/Microsoft/GSL) ([ISO C++ guidelines support library](https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md#gsl-guidelines-support-library)) (≥ 4.2.0, built-in if not found)
8. [MPL (A C++17 message passing library based on MPI)](https://github.com/rabauke/mpl) (≥ 0.5.0, built-in if not found)
9. [muc (A standard non-standard C++ library)](https://github.com/zhao-shihan/muc) (≥ 0.25.522, built-in if not found)
10. [yaml-cpp (A YAML parser and emitter in C++)](https://github.com/jbeder/yaml-cpp) (≥ 0.8.0, built-in if not found)
