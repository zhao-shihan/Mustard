# AGENTS.md

Guidance for AI coding agents working in the **Mustard** repository — a C++20 offline software framework for HEP experiments and the core library of MACESW. Humans start from [`README.md`](README.md); the branch/PR/review process is in [`CONTRIBUTING.md`](CONTRIBUTING.md). This checkout may sit inside another project (e.g. `MACESW/thirdparty/`), but treat this directory as the repository under work. If your tool reads its own config file (e.g. `CLAUDE.md`), keep that file a thin pointer and rely on this document for substance.

## Ground rules (read first)

1. **Ask about every decision branch — never assume.** Whenever the request admits more than one reasonable reading, target, behavior, scope, naming, API shape, dependency, or style outcome, stop and ask the human which one they want. This is the default, not a last resort: state the options you see, recommend one if you have a reason to, and wait for the answer. **Do not worry about asking too many questions** — a wrong assumption costs far more than an extra question. The same holds mid-task: if implementing reveals a second reasonable approach or a decision only the human can arbitrate, pause instead of committing to your own interpretation ("I assumed X" is not an acceptable substitute for having asked). Proceed alone only when the task is unambiguous, mechanical, and local.
2. **Never `git add`, `git commit`, `git push`, or create branches unless the human explicitly asks.** Finish the edit, report it, and stop. Do not stage files "helpfully".
3. **Stay in scope.** Fix what was asked. Do not reformat, rename, or modernize unrelated code while you are in a file.
4. **Ask before structural changes.** New module/public API, dependency or CMake option changes, file moves, or anything touching more than a couple of files: summarize the plan and get agreement first.
5. **Read [`STYLE_GUIDE.md`](STYLE_GUIDE.md) before writing code; run `clang-format -i` on every file you touch before finishing.** The style guide is long, normative where it applies, and deliberately not sampled in full here. The repo has no format-enforcing CI, so an unformatted file will silently reach a PR. `clang-format` 21 is available locally.
6. **Never hand-edit generated files** (see [Generated files](#generated-files-do-not-edit)). Edit the source of truth, then regenerate.
7. **Every new file needs the license header** (see [License headers](#license-headers)) and, in headers, `#pragma once`.
8. **Prefer building over guessing.** The library is large; build the narrowest target that proves your change, and do not reconfigure an existing build tree needlessly.
9. **Do scratch work under `tmp/` — keep the workspace clean.** By default, put logs, temporary scripts, test artifacts, and throwaway build trees under the gitignored `tmp/` directory (e.g. `tmp/build-ci`), and clean them up when finished. Do not leave unrelated files anywhere else in the workspace; only intentional changes to tracked files should remain.

## Where things live

```
CMakeLists.txt, cmake/   Top-level build (version, options, install/export) + build modules
src/Mustard/             The library — all .h++/.c++/.inl live here (GPL headers)
test/                    MustardTest.c++ is the single binary and registers every subprogram;
                         app/ holds app-level tests, lib/ the reusable helpers (globbed static libs)
tooling/                 test-config.cfg = test sequences (source of truth); generate-test-scripts.py
document/doxygen/        Doxygen config (INPUT = ../../, EXCLUDE pattern skips build dirs)
build/                   Local build tree (Ninja, gitignored; already configured here)
```

Namespace root is `Mustard::`, mirroring `src/Mustard/` directory names: `Mustard::Math`, `Mustard::Geant4X`, `Mustard::Env`, `Mustard::Execution`, `Mustard::Data`, `Mustard::Detector`, `Mustard::Physics`, `Mustard::Utility`, `Mustard::Simulation`, `Mustard::Parallel`, `Mustard::ROOTX`, `Mustard::CLHEPX`, `Mustard::CLI`, `Mustard::Concept`, `Mustard::IO`, `Mustard::Memory`, `Mustard::Application`, `Mustard::Testing` (tests only).

| Module | Role |
| --- | --- |
| `Application` | `Subprogram` / `SubprogramLauncher` — multi-tool binary dispatch |
| `Env` | Runtime environment and singletons (`BasicEnv`, `MPIEnv`, `Geant4Env`, `MonteCarloEnv`, `MPIMonteCarloEnv`) |
| `Execution` | Executors and dispatchers (sequential, parallel, master/worker, shared memory) |
| `Data` | Data model, containers, RDF-based readers/processors |
| `Detector` | Detector description / construction / messenger layer |
| `Geant4X` | Geant4 extensions — largest module (geometry, generator, physics, stepping, scoring) |
| `Physics` | Event generators, matrix elements, phase space, decays |
| `Math` | Statistics, estimates, distributions, integration, POCA, linear algebra helpers |
| `Utility`, `Memory`, `IO`, `Parallel`, `Concept`, `CLI`, `CLHEPX`, `ROOTX`, `gslx`, `EigenX` | Support utilities and third-party adapters |

## Toolchain and dependencies

- **Language and compilers:** C++20, no compiler extensions (`CMAKE_CXX_EXTENSIONS OFF`); GCC 13, Clang 19, IntelLLVM 2025, MSVC 19.50 enforced in `CMakeLists.txt`. Local tree: GCC 16.2.
- **Build:** CMake ≥ 3.25 + Ninja (or Make); build type defaults to `Release`.
- **System, no fallback:** ROOT ≥ 6.36 (`Core Imt MathCore Hist Tree ROOTDataFrame ROOTNTuple RIO`), Geant4 ≥ 11.0 (`ui_all gdml`, plus `vis_all`/`static` per options), MPI ≥ 4.1.
- **Optional system libraries (auto-detected, silent fallback):** backward-cpp picks its stack-walking backend (libunwind when found and `MUSTARD_USE_LIBUNWIND` is `ON`, else the compiler unwind API) and its symbolization backend (libdw → libbfd → libdwarf → `backtrace_symbols`). Only libunwind gives exact stack traces from signal handlers; without libdw/libbfd/libdwarf traces lose file/line. The configure log prints both chosen backends (`Stack walking backend:` / `Stack symbolization backend:`).
- **CPM source fallback** (cached in `.cache/cpm`, so re-configures are usually cheap): Eigen 5.0 (+ `unsupported`), mp1, muc, fmt 12.2, yaml-cpp 0.9, argparse, envparse, gtl, Microsoft.GSL, backward-cpp, indicators, mimalloc, FFTW.

| Option | Default | Note |
| --- | --- | --- |
| `MUSTARD_BUILD_TESTING` | `ON` if top-level | Builds `MustardTest` |
| `MUSTARD_WERROR` | `OFF` | **CI sets `ON`** |
| `MUSTARD_FULL_UNITY_BUILD` / `MUSTARD_UNITY_BUILD` | `OFF` / `OFF` | CI sets `FULL` `ON` (unlimited batch); `UNITY` uses batch size 8 |
| `BUILD_SHARED_LIBS` / `MUSTARD_ENABLE_IPO` | `OFF` / `=BUILD_SHARED_LIBS` | Static by default; IPO is LTO for Release |
| `CMAKE_INSTALL_PREFIX` | `<repo>/install` | Override explicitly when needed |

Other options (`MUSTARD_USE_FFTW`, `MUSTARD_USE_MIMALLOC`, `MUSTARD_USE_G4VIS`, `MUSTARD_USE_LIBUNWIND`, `MUSTARD_USE_STATIC_G4`, `MUSTARD_SIGNAL_HANDLER`, warning-level and MSVC runtime flags) are declared with their defaults in `CMakeLists.txt`.

## Building

A configured tree already exists at `build/` (Ninja, Release, `MUSTARD_BUILD_TESTING=ON`). Reuse it:

```bash
ninja -C build                                     # whole project
ninja -C build Mustard                             # library only
ninja -C build MustardTesting MustardTestingApp MustardTest

# Only when no tree exists or a CMake input/glob changed:
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DMUSTARD_BUILD_TESTING=ON
```

- Sources are globbed (`file(GLOB_RECURSE ...)` in `src/CMakeLists.txt` and `test/{app,lib}/CMakeLists.txt`), so a **new `.c++` file needs a re-configure** before `ninja` sees it.
- Builds are heavy (ROOT/Geant4/Eigen in the include graph): build narrow targets rather than everything, and never run two builds on the same tree concurrently.
- Local `MUSTARD_WERROR=OFF` + `MUSTARD_UNITY_BUILD=OFF` hides the two most common CI failures: new warnings and unity-build collisions between anonymous-namespace symbols. CI adds `-DMUSTARD_FULL_UNITY_BUILD=ON -DMUSTARD_WERROR=ON`, runs 4 build jobs (GCC/Clang × MPICH/OpenMPI) plus one test job per sequence, and can be reproduced with a throwaway `tmp/build-ci` tree.
- Never modify or commit `build/`, `install/`, `.cache/`. `tmp/` is a gitignored scratch area for logs and drafts (see ground rule 9) — fine to write, never to commit.

## Testing

Tests are **not** GoogleTest, and there is **no CTest integration**. The pattern:

- One test per directory: `test/app/Mustard/Testing/<TestName>/<TestName>.h++` + `.c++`, deriving from `Mustard::Application::Subprogram` and implementing `auto Main(int argc, char* argv[]) const -> int override`. The constructor passes the CLI-visible name and a description:
  ```cpp
  TestSingleton::TestSingleton() :
      Subprogram{"TestSingleton", "Test Mustard::Env::Singleton."} {}
  ```
- Registration is manual in `test/MustardTest.c++`: `launcher.AddSubprogram<TestName>();`.
- A test needing a runtime environment constructs it in `Main`, e.g. `Mustard::CLI::BasicCLI<> cli; Mustard::Env::MPIEnv env{argc, argv, cli};`.
- Run from the directory the test writes into — several produce ROOT files in the cwd: `cd build/test && ./MustardTest TestSingleton`; for a whole sequence, `bash run-tests.sh` there.
- MPI-parallel tests go through `tooling/parexec.sh`: `parexec [--use-hwthreads] ./MustardTest <TestName>` (wraps `mpiexec`, physical cores by default).
- `tooling/run-tests.sh` is generated and expects `parexec.sh` in the working directory; the build copies both into `build/test/`.

### Adding a test end to end

1. Create `test/app/Mustard/Testing/<TestName>/<TestName>.h++` and `.c++` with license headers, deriving from `Application::Subprogram`.
2. Register it in `test/MustardTest.c++` (include + `launcher.AddSubprogram<<TestName>>();`).
3. Append the sequence to `tooling/test-config.cfg` — one line per sequence, steps joined with `->` and prefixed by `seqexec` or `parexec`, e.g. `seqexec TestFoo -> parexec TestFoo`.
4. Regenerate (`python3 tooling/generate-test-scripts.py`, or `ninja -C build GenerateTestScripts`), re-configure so the new sources are globbed, then build and run the test sequentially and, if MPI-capable, through `parexec`.

Reusable helpers for data-model/statistics tests live in `test/lib/Mustard/Testing/` (linked into `MustardTesting`); app-level tests live in `test/app/` (`MustardTestingApp`).

## Generated files (do not edit)

| File | Source of truth |
| --- | --- |
| `.github/workflows/build-and-test-amd64-gnu-linux.yml` | `tooling/test-config.cfg` → `tooling/generate-test-scripts.py` |
| `tooling/run-tests.sh` | same generator |
| `src/Mustard/Version.inl` | `src/Mustard/Version.inl.in` + version variables in `CMakeLists.txt` (`configure_file`) |
| `cmake/CPM.cmake`, `cmake/FindFFTW.cmake` | downloaded by `cmake/MustardLookForPackages.cmake` (gitignored) |

Editing a generated file is always wrong: the next configure/build overwrites it, and the CI workflow would diverge from `test-config.cfg`. Change the generator or the config instead.

## Coding conventions (the parts agents get wrong)

Normative document: [`STYLE_GUIDE.md`](STYLE_GUIDE.md). Formatting is enforced by `.clang-format` (LLVM base, 4 spaces, `ColumnLimit: 0`, `BreakBeforeBraces: Attach`, `PointerAlignment: Left`, `PackConstructorInitializers: Never`, custom `IncludeCategories`); run `clang-format -i <file>`.

- **This repository's style first:** never copy a third-party dependency's style into Mustard code; follow [`STYLE_GUIDE.md`](STYLE_GUIDE.md).
- **Extensions:** `.h++` headers, `.c++` sources, `.inl` template/inline implementations. Never `.hpp`/`.hh`/`.cpp`/`.cc`.
- **File layout:** copyright header → `#pragma once` (headers) → includes (project headers, third-party, standard library last, sorted case-sensitively per `.clang-format`) → namespace → forward declarations → code. **`.inl` files must not `#include` anything.**
- **Naming:** `PascalCase` for namespaces, types, and *all* functions and methods (getters/setters included: `Name()`, not `GetName()`). Locals/constants `camelCase`; non-static members `f` + `PascalCase`; static members `fg` + `PascalCase`. No underscores except structured-binding placeholders (`_1`, `_2`). Booleans/flags take an adjective, not `is`/`has`/`can`. No `s`/`es` plurals (`Momenta`, `HitSet`, …); `Ts`/`ADescriptions`-style `s` marks a template parameter pack.
- **Functions:** always `auto` with a trailing return type (`auto F() -> void`, `auto F() const -> int`); `[[nodiscard]]`/`[[noreturn]]` where appropriate.
- **Namespaces:** one-line nested definitions (`namespace Mustard::Geant4X::inline Generator {`) with the closing comment; `inline` namespaces for organization/versioning. Never `using namespace` in a header; keep `using` at implementation or function scope. Anonymous namespaces only for internal linkage.
- **Never qualify a name with an `inline` namespace** (`Utility`, `Math`, `IO`, `Execution`, `Memory`, `Physics`, the sub-namespaces of `Geant4X`, …): members belong to the enclosing namespace, so write `RandomEngineInstaller`, `Random::Uniform<double>`, not `Utility::RandomEngineInstaller`, `Math::Random::Uniform<double>`. Qualify only to disambiguate a nearer namespace that hides the name, e.g. `Math::Random::X` inside `Mustard::CLHEPX::Random`. See [`STYLE_GUIDE.md`](STYLE_GUIDE.md) → Namespace Conventions → Qualification.
- **Classes:** declaration order is nested types → constructors → destructor → member functions → protected members → private members; member variables/constants come last, in `private`/`protected`. Do not write special members you don't need. Mark `final` when a class must not be derived from.
- **Members over globals:** environment access goes through `Mustard::Env` singletons (e.g. `MPIEnv::Instance()`), not ad-hoc statics.
- **Conversions and ownership:** C++-style casts only; prefer `static_cast`, avoid `reinterpret_cast` (`std::bit_cast` instead) and `const_cast`. Smart pointers (`std::unique_ptr`, `std::shared_ptr`, `Mustard::Arc`) for ownership, `gsl::not_null` for non-nullable pointers, references when ownership is not transferred.
- **Modern C++20:** concepts/`requires` for template constraints, `auto&&` in range-based for, `std::as_const()` for read-only iteration, brace initialization, structured bindings.
- **Errors:** follow the environment/exception facilities already used in the module you touch (`Env` and `Geant4X` have established error paths) instead of inventing a new style, and keep messages consistent with nearby code (`fmt` is available).

## License headers

All source files — including tests and tooling's C++ — start with this exact header (no year change, no extra blank line before `#pragma once`):

```cpp
// -*- C++ -*-
//
// Copyright (C) 2020-2026  Shihan Zhao and contributors
//
// This file is part of Mustard, an offline software framework for HEP experiments.
//
// Mustard is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// Mustard is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// Mustard. If not, see <https://www.gnu.org/licenses/>.

```

The project is **GPL-3.0-or-later** (`COPYING`). Do not paste code from incompatibly licensed sources; if an algorithm follows a paper or another library, say so in a comment.

## Before you hand work back

- [ ] `clang-format -i` run on every file you touched.
- [ ] Narrowest relevant build passes (`ninja -C build Mustard`, or the `MustardTest` targets), with no new warnings.
- [ ] Affected test(s) run — sequentially and, for MPI-capable ones, through `parexec`.
- [ ] New test wired up end to end (registration + `test-config.cfg` + regenerated scripts), or a new `.c++` picked up by a re-configure.
- [ ] Generated files untouched; no stray `build/`, `install/`, `.cache/`, `tmp/` contents or `*.root` artifacts staged.
- [ ] Report says what changed, which commands ran, and what the results were — with no git operations performed.
