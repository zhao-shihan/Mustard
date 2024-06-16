# Mustard

Mustard is a modern offline software framework for HEP experiment.

- [Mustard](#mustard)
  - [Dependencies](#dependencies)
    - [Toolchain](#toolchain)
    - [Libraries](#libraries)
  - [Cite](#cite)

## Dependencies

### Toolchain

1. A C++ compiler (that supports ≥ C++20. [GCC](https://gcc.gnu.org/) ≥ 12, [LLVM Clang](https://clang.llvm.org/) ≥ 16, or MSVC ≥ 19.30 (i.e., [Visual Studio](https://visualstudio.microsoft.com/) 2022 ≥ 17.0))
2. A C++ library (that supports ≥ C++20. [libstdc++](https://gcc.gnu.org/onlinedocs/libstdc++/) ≥ 12, or [MSVC STL](https://github.com/microsoft/STL) with [Visual Studio](https://visualstudio.microsoft.com/) 2022 ≥ 17.0)
3. [CMake](https://cmake.org/) (≥ 3.16)
4. A build system compatible with CMake ([GNU Make](https://www.gnu.org/software/make/), [Ninja](https://ninja-build.org), or etc.)

### Libraries

Required:

1. [MPI (Message Passing Interface)](https://www.mpi-forum.org/) (≥ 3.0, [MPICH](https://www.mpich.org/), [OpenMPI](https://www.open-mpi.org/), [Intel MPI](https://www.intel.cn/content/www/cn/zh/developer/tools/oneapi/mpi-library.html), [Microsoft MPI](https://github.com/Microsoft/Microsoft-MPI), etc.)
2. [Eigen (A C++ template library for linear algebra)](https://eigen.tuxfamily.org/) (≥ 3.4.0)
3. [Geant4 (Toolkit for the simulation of the passage of particles through matter)](https://geant4.org/) (≥ 11.0.0, **with GDML**)
4. [ROOT (An open-source data analysis framework)](https://root.cern/) (≥ 6.30.00, **with GDML**)

Required, built-in if not found (network or pre-downloaded source is required):

1. [argparse (Argument Parser for Modern C++)](https://github.com/p-ranav/argparse) (master, built-in if not found)
2. [zhao-shihan/backward-cpp (A beautiful stack trace pretty printer for C++)](https://github.com/zhao-shihan/backward-cpp) (a backward-cpp fork, built-in if not found)
3. [EFM (A generic bilinear/trilinear interpolator)](https://github.com/zhao-shihan/EFM) (main, built-in if not found)
4. [envparse (A simple, header-only environment variable parser in C++)](https://github.com/zhao-shihan/envparse) (main, built-in if not found)
5. [fmt (A modern formatting library)](https://github.com/fmtlib/fmt) (≥ 10.0.0, built-in if not found)
6. [muc (A standard non-standard C++ library)](https://github.com/zhao-shihan/muc) (main, built-in if not found)
7. [Microsoft.GSL](https://github.com/Microsoft/GSL) ([ISO C++ guidelines support library](https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md#gsl-guidelines-support-library)) (≥ 4.0.0, built-in if not found)
8. [zhao-shihan/cpp-TimSort (A C++ implementation of timsort)](https://github.com/zhao-shihan/cpp-TimSort) (a cpp-TimSort fork, built-in if not found)
9.  [yaml-cpp (A YAML parser and emitter in C++)](https://github.com/jbeder/yaml-cpp) (≥ 0.8.0, built-in if not found)

## Cite

1. [Shihan Zhao and Jian Tang, Optimization of muonium yield in perforated silica aerogel, arXiv:2401.00222.](https://arxiv.org/abs/2401.00222)
