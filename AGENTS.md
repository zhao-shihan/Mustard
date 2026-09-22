# AGENTS.md

Guidance for AI coding agents working in the **Mustard** repository. Humans should start from [`README.md`](README.md); contribution process (branches, PRs, review) is in [`CONTRIBUTING.md`](CONTRIBUTING.md).

Mustard is a C++20 offline software framework for HEP experiments, and the core library of MACESW. This checkout may sit inside another project (e.g. `MACESW/thirdparty/`), but treat this directory as the repository under work.

If your tool reads its own config file (e.g. `CLAUDE.md`), keep that file as a thin pointer and rely on this document for substance.

## Agent ground rules (read first)

1. **Ask about every decision branch — never assume.** Whenever the request admits more than one reasonable reading, target, behavior, scope, naming, API shape, dependency, or style outcome, stop and ask the human which one they want. This is the default, not a last resort: state the options you see, recommend one if you have a reason to, and wait for the answer. **Do not worry about asking too many questions** — a wrong assumption costs far more than an extra question, and the human would rather answer five questions than review the wrong implementation. Only proceed alone when the task is unambiguous, mechanical, and local.
2. **Never `git add`, `git commit`, `git push`, or create branches unless the human explicitly asks.** Finish the edit, report it, and stop. Do not stage files "helpfully".
3. **Stay in scope.** Fix what was asked. Do not reformat, rename, or modernize unrelated code while you are in a file.
4. **Ask before structural changes.** New module/public API, dependency or CMake option changes, file moves, or anything touching more than a couple of files: summarize the plan and get agreement first.
5. **When you discover an unstated choice mid-task, surface it instead of silently picking.** If implementing reveals a second reasonable approach, a needed design decision, or a behavior that only the human can arbitrate, pause and ask rather than committing to your own interpretation. "I assumed X" is not an acceptable substitute for having asked.
6. **Read [`STYLE_GUIDE.md`](STYLE_GUIDE.md) before writing code.** It is long, normative where it applies, and deliberately not sampled in full here. When in doubt, match the surrounding code.
7. **Run `clang-format` on every file you touch before finishing.** The repo has no format-enforcing CI, so an unformatted file will silently reach a PR. `clang-format` 21 is available locally.
8. **Never hand-edit generated files** (see [Generated files](#generated-files-do-not-edit)). Edit the source of truth, then regenerate.
9. **Every new file needs the license header** (see [License headers](#license-headers)) and, in headers, `#pragma once`.
10. **Prefer building over guessing.** The library is large; a full build is expensive, so build the narrowest target that proves your change, and do not reconfigure an existing build tree needlessly.

## Repository map

```
CMakeLists.txt          Top-level build; version, options, install/export rules
cmake/                  Build config modules (compile flags, package discovery)
src/Mustard/            The library — all .h++/.c++/.inl live here (GPL headers)
src/CMakeLists.txt      add_library(Mustard), GLOB_RECURSE over src/Mustard/*.c++
test/MustardTest.c++    Single test binary; registers every test subprogram
test/app/.../Testing/   App-level tests: one directory per subprogram
test/lib/.../Testing/   Reusable testing helpers (DataModel, TestEstimate, …)
test/{app,lib}/CMakeLists.txt  GLOB_RECURSE test sources into static libs
tooling/test-config.cfg Test sequences — the source of truth for CI + local runs
tooling/generate-test-scripts.py  Generates the CI workflow and local runner
document/doxygen/       Doxygen config (INPUT = ../../, EXCLUDE pattern excludes build dirs)
build/                  Local build tree (gitignored; already configured here)
```

Namespace root is `Mustard::`, mirroring `src/Mustard/` directory names: `Mustard::Math`, `Mustard::Geant4X`, `Mustard::Env`, `Mustard::Execution`, `Mustard::Data`, `Mustard::Detector`, `Mustard::Physics`, `Mustard::Utility`, `Mustard::Simulation`, `Mustard::Parallel`, `Mustard::ROOTX`, `Mustard::CLHEPX`, `Mustard::CLI`, `Mustard::Concept`, `Mustard::IO`, `Mustard::Memory`, `Mustard::Application`, `Mustard::Testing` (tests only).

Module rough roles:

| Module | Role |
| --- | --- |
| `Application` | `Subprogram` / `SubprogramLauncher` — multi-tool binary dispatch |
| `Env` | Runtime environment and singletons (`BasicEnv`, `MPIEnv`, `Geant4Env`, `MonteCarloEnv`, `MPIMonteCarloEnv`) |
| `Execution` | Executors and dispatchers (sequential, parallel, master/worker, shared memory) |
| `Data` | Data model, containers, RDF-based readers/processors |
| `Detector` | Detector description / construction / messenger layer |
| `Geant4X` | Geant4 extensions (largest module: geometry, generator, physics, stepping, scoring) |
| `Physics` | Event generators, matrix elements, phase space, decays |
| `Math` | Statistics, estimates, distributions, integration, POCA, linear algebra helpers |
| `Utility`, `Memory`, `IO`, `Parallel`, `Concept`, `CLI`, `CLHEPX`, `ROOTX`, `gslx`, `EigenX` | Support utilities and third-party adapters |

## Toolchain and dependencies

- **Language:** C++20, no compiler extensions (`CMAKE_CXX_EXTENSIONS OFF`).
- **Minimum compilers** (enforced in `CMakeLists.txt`): GCC 13, Clang 19, IntelLLVM 2025, MSVC 19.50. Local tree is built with GCC 16.2.
- **Build system:** CMake ≥ 3.24 + Ninja (or Make). Default build type is `Release` when unspecified.
- **System (must be installed, no fallback):** ROOT ≥ 6.36 (`Core Imt MathCore Hist Tree ROOTDataFrame ROOTNTuple RIO`), Geant4 ≥ 11.0 (`ui_all gdml`, plus `vis_all`/`static` per options), MPI ≥ 3.1.
- **Built from source via CPM if not found:** Eigen 5.0 (with `unsupported`), mplr, muc, fmt 12.2, yaml-cpp 0.9, argparse, envparse, gtl, Microsoft.GSL, backward-cpp, indicators, mimalloc, FFTW.
- CPM downloads land in `.cache/cpm` (`CPM_SOURCE_CACHE`), so a re-configure is usually cheap; the first configure on a fresh machine is not.

Common options (all defined in `CMakeLists.txt`, defaults shown):

| Option | Default | Note |
| --- | --- | --- |
| `MUSTARD_BUILD_TESTING` | `ON` if top-level | Builds `MustardTest` |
| `MUSTARD_WERROR` | `OFF` | **CI sets `ON`** — see below |
| `MUSTARD_FULL_UNITY_BUILD` | `OFF` | **CI sets `ON`** (unlimited batch) |
| `MUSTARD_UNITY_BUILD` | `OFF` | Batch size 8 when enabled instead |
| `MUSTARD_USE_FFTW` / `MUSTARD_USE_MIMALLOC` / `MUSTARD_USE_G4VIS` | `ON` | G4VIS toggles Geant4 visualization components |
| `MUSTARD_ENABLE_IPO` | `BUILD_SHARED_LIBS` | LTO for Release |
| `MUSTARD_SIGNAL_HANDLER` | `ON` | Registers handlers at environment construction |
| `BUILD_SHARED_LIBS` | `OFF` | Static library by default |
| `CMAKE_INSTALL_PREFIX` | `<repo>/install` | Override explicitly when needed |

## Building

A configured build tree already exists at `build/` (Ninja, Release, GCC 16.2, `MUSTARD_BUILD_TESTING=ON`). **Reuse it** — do not create a second tree or re-run the configure step unless you changed CMake inputs:

```bash
# Incremental rebuild using the existing tree (preferred)
ninja -C build                 # whole project
ninja -C build Mustard         # library only
ninja -C build MustardTesting MustardTestingApp MustardTest

# Fresh configuration, only if no tree exists or CMake files/options changed
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DMUSTARD_BUILD_TESTING=ON
ninja -C build
```

Notes:

- Sources are collected with `file(GLOB_RECURSE ...)` in `src/CMakeLists.txt`, `test/app/CMakeLists.txt`, `test/lib/CMakeLists.txt`. A brand-new `.c++` file therefore requires a **re-configure** (re-run `cmake -S . -B build`) to be picked up, not just `ninja`.
- Builds are heavy (ROOT/Geant4/Eigen in the include graph). Use `ninja -C build <target>` for narrow checks instead of a full rebuild, and avoid running two builds over the same tree concurrently.
- `MUSTARD_WERROR=OFF` and `MUSTARD_UNITY_BUILD=OFF` locally is the default for this tree, but that combination **hides the two most common CI failures**: new warnings, and unity-build symbol collisions (anonymous-namespace / internal-linkage clashes across translation units). See below.
- Never commit or hand-modify anything under `build/`, `install/`, or `.cache/` — all are gitignored.

### Reproducing CI before you hand work back

The CI workflow (`.github/workflows/build-and-test-amd64-gnu-linux.yml`) builds with:

```bash
cmake -S . -B build -G Ninja \
  -DCMAKE_C_COMPILER=gcc   -DCMAKE_CXX_COMPILER=g++ \
  -DMUSTARD_BUILD_TESTING=ON \
  -DMUSTARD_FULL_UNITY_BUILD=ON \
  -DMUSTARD_WERROR=ON
ninja -C build
```

It runs 4 build jobs (GCC/Clang × MPICH/OpenMPI) and one test job per sequence per configuration. To verify a change the way CI sees it, either configure a throwaway tree with those flags (`-B build-ci`) or reconfigure the existing tree temporarily and restore it afterwards — but do not leave the shared `build/` in an unexpected state without telling the human.

## Testing

Tests are **not** GoogleTest and there is **no CTest integration**. The pattern:

- One test per directory under `test/app/Mustard/Testing/<TestName>/<TestName>.h++` + `.c++`.
- Each test is a class deriving from `Mustard::Application::Subprogram`, implementing `auto Main(int argc, char* argv[]) const -> int override`. The constructor passes the CLI-visible name and a description:
  ```cpp
  TestSingleton::TestSingleton() :
      Subprogram{"TestSingleton", "Test Mustard::Env::Singleton."} {}
  ```
- Subprograms are registered manually in `test/MustardTest.c++` via `launcher.AddSubprogram<TestName>();`.
- A test that needs a runtime environment constructs it in `Main`, e.g. `Mustard::CLI::BasicCLI<> cli; Mustard::Env::MPIEnv env{argc, argv, cli};`.
- Run the binary from the directory it writes into; several tests produce ROOT files in the current directory:
  ```bash
  cd build/test && ./MustardTest TestSingleton
  ```
- MPI-parallel tests are launched through `tooling/parexec.sh` (`parexec [--use-hwthreads] ./MustardTest <TestName>`), which wraps `mpiexec` and picks physical cores by default.
- `tooling/run-tests.sh` runs the full suite serially; it is generated (see below) and expects `parexec.sh` in the working directory (`build/test/` gets both copied by the build).

### Adding a test end to end

1. Create `test/app/Mustard/Testing/<TestName>/<TestName>.h++` and `.c++` with license headers; derive from `Application::Subprogram`.
2. Register it in `test/MustardTest.c++`: add the include and `launcher.AddSubprogram<<TestName>>();`.
3. Append its execution sequence to `tooling/test-config.cfg`, one line per sequence, steps joined with `->` and prefixed by `seqexec` or `parexec`, e.g.
   ```
   seqexec TestFoo -> parexec TestFoo
   ```
4. Regenerate the derived artifacts: `python3 tooling/generate-test-scripts.py` (also re-run automatically by the `GenerateTestScripts` build target when Python 3 is available — `ninja -C build GenerateTestScripts`).
5. Re-configure so the new sources are globbed, then build `MustardTest` and run the new test both sequentially and, if MPI-capable, through `parexec`.

Reusable helpers for data-model / statistics tests live in `test/lib/Mustard/Testing/` (linked into `MustardTesting`); app-level tests go under `test/app/` (`MustardTestingApp`).

## Generated files (do not edit)

| File | Source of truth |
| --- | --- |
| `.github/workflows/build-and-test-amd64-gnu-linux.yml` | `tooling/test-config.cfg` → `tooling/generate-test-scripts.py` |
| `tooling/run-tests.sh` | same generator |
| `src/Mustard/Version.inl` | `src/Mustard/Version.inl.in` + version variables in `CMakeLists.txt` (`configure_file`) |
| `cmake/CPM.cmake`, `cmake/FindFFTW.cmake` | downloaded by `cmake/LookForPackages.cmake` (gitignored) |
| `build/**` | CMake/Ninja output |

Editing a generated file is always wrong: the next configure/build overwrites it, and the CI workflow would diverge from `test-config.cfg`. Change the generator or the config instead.

## Coding conventions (the parts agents get wrong)

Normative document: [`STYLE_GUIDE.md`](STYLE_GUIDE.md). Formatting is enforced by `.clang-format` (LLVM base, 4 spaces, `ColumnLimit: 0`, `BreakBeforeBraces: Attach`, `PointerAlignment: Left`, `PackConstructorInitializers: Never`, custom `IncludeCategories`). Run `clang-format -i <file>`.

Most-violated rules:

- **Extensions:** headers `.h++`, sources `.c++`, template/inline implementations `.inl`. Never `.hpp`/`.hh`/`.cpp`/`.cc`.
- **File layout:** copyright header → `#pragma once` (headers) → includes (`Mustard/...` first, third-party, standard library last, sorted case-sensitively per `.clang-format`) → namespace → forward declarations → code. **`.inl` files must not `#include` anything.**
- **Naming:** `PascalCase` for namespaces, types, and *all* functions and methods (including getters/setters: `Name()`, not `GetName()`). Local variables/constants `camelCase`; non-static members `f` + `PascalCase`; static members `fg` + `PascalCase`. No underscores anywhere except structured-binding placeholders (`_1`, `_2`). Booleans/flags use an adjective, not `is`/`has`/`can` prefixes. No `s`/`es` plurals (use `Momenta`, `HitSet`, …); `Ts`/`ADescriptions`-style `s` means a template parameter pack.
- **Functions:** always `auto` with a trailing return type (`auto F() -> void`, `auto F() const -> int`). Use `[[nodiscard]]`/`[[noreturn]]` where appropriate.
- **Namespaces:** one-line nested definitions (`namespace Mustard::Geant4X::inline Generator {`), always with the closing comment. `inline` namespaces for organization/versioning. Never `using namespace` in a header; restrict `using` to implementation or function scope. Anonymous namespaces only for internal linkage.
- **Classes:** declaration order is nested types → constructors → destructor → member functions → protected members → private members; all member variables/constants come last, in `private`/`protected`. Do not write special members you don't need. Use `final` when a class must not be derived from.
- **Members over globals:** environment access goes through `Mustard::Env` singletons (e.g. `MPIEnv::Instance()`), not ad-hoc statics.
- **Conversions and ownership:** C++-style casts only; prefer `static_cast`, avoid `reinterpret_cast` (`std::bit_cast` instead) and `const_cast`. Smart pointers (`std::unique_ptr`, `std::shared_ptr`, `Mustard::Arc`) for ownership, `gsl::not_null` for non-nullable pointers, references when ownership is not transferred.
- **Modern C++20:** use concepts/`requires` for template constraints, `auto&&` in range-based for loops, `std::as_const()` for read-only iteration, brace initialization, structured bindings.
- **Errors:** follow the environment/exception facilities already in the module you touch (`Env` and `Geant4X` have established error-handling paths) rather than introducing a new error style, and keep messages formatted consistently with nearby code (`fmt` is available).

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

## Common pitfalls

- **Deciding for the human.** Guessing a target, a scope, an API shape, a naming choice, or a "reasonable default" instead of asking. Every decision branch is a question for the human — ask, then implement.
- Editing `tooling/run-tests.sh` or the CI YAML instead of `test-config.cfg`.
- Adding a `.c++` file and expecting `ninja` alone to compile it (globs need a re-configure).
- Forgetting to register a new subprogram in `test/MustardTest.c++` — the test silently never runs.
- Assuming GoogleTest/CTest idioms; there is no `TEST()` macro and no `ctest` target.
- Writing `.hpp`/`.cpp` files, `Get`/`Set`/`Is`-prefixed names, or non-trailing return types.
- Adding `#include` lines to `.inl` files, or `using namespace` to headers.
- Leaving the shared `build/` reconfigured with CI flags, or leaving `build/test/*.root` artifacts behind.
- Committing `build/`, `install/`, `.cache/`, or `tmp/` contents.
