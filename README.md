# Mustard

[![GitHub License](https://img.shields.io/github/license/zhao-shihan/Mustard?color=red)](COPYING)
![GitHub top language](https://img.shields.io/github/languages/top/zhao-shihan/Mustard?color=f34b7d)
[![GitHub activity](https://img.shields.io/github/commit-activity/m/zhao-shihan/Mustard)](https://github.com/zhao-shihan/Mustard/pulse)
![GitHub last commit](https://img.shields.io/github/last-commit/zhao-shihan/Mustard)
[![GitHub release](https://badgen.net/github/release/zhao-shihan/Mustard)](https://github.com/zhao-shihan/Mustard/releases)
[![GitHub contributors](https://img.shields.io/github/contributors/zhao-shihan/Mustard?style=flat)](https://github.com/zhao-shihan/Mustard/graphs/contributors)

**Mustard** is a modern, high-performance offline software framework designed for particle physics experiments. It provides a generic architecture with core features including:
- **Distributed computing:** Scalable parallel processing capabilities.
- **Unified geometry:** A consistent interface for handling experimental geometries.
- **Data model framework:** High-level data models abstractions to simplify simulation and analysis workflows.
- **Physics:** An event generator framework and a set of Geant4 physics and processes.
- **Many utilities:** Useful functions, classes, wrappers and etc. to speed up development and improve runtime performance.

This framework serves as the core component of the [**MACE offline software (MACESW)**](https://github.com/zhao-shihan/MACESW) and was originally developed as part of that project.

- [Mustard](#mustard)
  - [Building from Source](#building-from-source)
    - [Prerequisites](#prerequisites)
      - [Toolchain](#toolchain)
      - [Dependencies](#dependencies)
      - [Optional Dependencies](#optional-dependencies)
  - [Projects using this library](#projects-using-this-library)
  - [Citation](#citation)

## Building from Source

This section guides you through building Mustard from source.

### Prerequisites

#### Toolchain
*   **C++ compiler:** GCC ≥ 13 or LLVM Clang ≥ 16 or equivalent
*   **C++ standard library:** libstdc++ ≥ 13 or equivalent
*   **Build system:** CMake ≥ 3.21, and GNU Make or Ninja or equivalent

#### Dependencies

Mustard requires the following external libraries to be installed on your system:

| Library                                   | Min version | Notes                         |
| :---------------------------------------- | :---------- | :---------------------------- |
| [**Eigen**](https://eigen.tuxfamily.org/) | 3.4.0       |                               |
| [**Geant4**](https://geant4.org/)         | 11.0.0      | Requires GDML support         |
| [**ROOT**](https://root.cern/)            | 6.30.00     | Requires `gdml` component     |
| [**MPI**](https://www.mpi-forum.org/)     | 3.1         | MPICH, OpenMPI, or equivalent |

#### Optional Dependencies

The following dependencies are optional. If they are not found on your system during configuration, CMake will automatically download and build them.

| Library                                                                     | Min version | Description                               |
| :-------------------------------------------------------------------------- | :---------- | :---------------------------------------- |
| [**argparse**](https://github.com/p-ranav/argparse)                         | 3.2         | Argument parsing for modern C++           |
| [**zhao-shihan/backward-cpp**](https://github.com/zhao-shihan/backward-cpp) | 1.6.1       | Beautiful stack trace printing            |
| [**EFM**](https://github.com/zhao-shihan/EFM)                               | 0.1.0       | Generic bi/trilinear interpolation        |
| [**envparse**](https://github.com/zhao-shihan/envparse)                     | 1.0.0       | Header-only environment variable parser   |
| [**fmt**](https://github.com/fmtlib/fmt)                                    | 11.2.0      | Modern formatting library                 |
| [**indicators**](https://github.com/p-ranav/indicators)                     | 2.3         | Activity indicators for C++               |
| [**Microsoft.GSL**](https://github.com/Microsoft/GSL)                       | 4.2.0       | C++ Core Guidelines Support Library       |
| [**MPLR**](https://github.com/zhao-shihan/mplr)                             | 0.25.1003   | C++17 message passing library (MPI-based) |
| [**muc**](https://github.com/zhao-shihan/muc)                               | 0.25.1011   | Standard non-standard C++ utilities       |
| [**zhao-shihan/yaml-cpp**](https://github.com/zhao-shihan/yaml-cpp)         | 0.8.2       | YAML parser and emitter for C++           |

## Projects using this library

Mustard has been used as the framework for the following projects:

- [**MACESW**](https://github.com/zhao-shihan/MACESW): Muonium-to-Antimuonium Conversion Experiment (MACE) offline software
- [**MusAirS**](https://github.com/zhao-shihan/MACESW): An air shower simulation tool

They are also examples demonstrating the usage of Mustard.

If you are aware of other projects using this library, please let me know by email or by submitting an issue.

## Citation

If you use Mustard in your research, please cite the following paper:

> **BAI Ai-Yu, CAI Hanjie, CHEN Chang-Lin, et al (MACE working group).**  
> **Conceptual Design of the Muonium-to-Antimuonium Conversion Experiment (MACE) [DB/OL].**  
> *arXiv preprint*, 2024: 2410.18817 [hep-ex].  
> https://arxiv.org/abs/2410.18817.
