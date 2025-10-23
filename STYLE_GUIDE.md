# Mustard C++ Coding Style Guide

This guide establishes consistent C++ coding standards for Mustard, emphasizing best practices, readability, and maintainability across the codebase.

This guide focuses on aspects that `.clang-format` doesn't or cannot handle, including structural and organizational aspects.

## Table of Contents
- [Mustard C++ Coding Style Guide](#mustard-c-coding-style-guide)
  - [Table of Contents](#table-of-contents)
  - [Important Notice](#important-notice)
  - [Common Principles](#common-principles)
  - [File Organization and Structure](#file-organization-and-structure)
    - [File Extensions](#file-extensions)
    - [File Headers and Copyright](#file-headers-and-copyright)
    - [Header Guards and Structure](#header-guards-and-structure)
  - [Formatting Conventions](#formatting-conventions)
    - [Indentation](#indentation)
    - [Empty Lines](#empty-lines)
    - [Spaces](#spaces)
    - [Initializations](#initializations)
    - [Concept](#concept)
  - [Naming Conventions](#naming-conventions)
    - [General Rules](#general-rules)
    - [Constants and Variables](#constants-and-variables)
    - [Classes and Types](#classes-and-types)
    - [Functions](#functions)
    - [Namespaces](#namespaces)
  - [Namespace Conventions](#namespace-conventions)
    - [Declaration](#declaration)
    - [Using Directives](#using-directives)
    - [Anonymous Namespaces](#anonymous-namespaces)
  - [Class Structure](#class-structure)
    - [General rules](#general-rules-1)
    - [Typical declaration order](#typical-declaration-order)
    - [Example Class Structure](#example-class-structure)
    - [Inheritance](#inheritance)
  - [Constant and Variable Declaration](#constant-and-variable-declaration)
    - [Const Correctness](#const-correctness)
    - [Auto Keyword](#auto-keyword)
    - [Reference Qualifiers](#reference-qualifiers)
    - [Pointer](#pointer)
    - [Attribute and Specifier Ordering](#attribute-and-specifier-ordering)
  - [Member Constant and Variable Declaration](#member-constant-and-variable-declaration)
    - [Declaration Order](#declaration-order)
  - [Function Declaration](#function-declaration)
    - [Parameters](#parameters)
    - [Template](#template)
    - [Return Type Syntax](#return-type-syntax)
    - [Attribute and Specifier Ordering](#attribute-and-specifier-ordering-1)
    - [Trailing Keywords Ordering](#trailing-keywords-ordering)
  - [The `main` Function](#the-main-function)
    - [Function Signature](#function-signature)
    - [Return Value](#return-value)
  - [Member Function Declaration](#member-function-declaration)
    - [Return Type Syntax](#return-type-syntax-1)
    - [Const Correctness](#const-correctness-1)
    - [Virtual Functions and Override](#virtual-functions-and-override)
  - [Enumeration](#enumeration)
    - [Use scoped enumeration](#use-scoped-enumeration)
  - [Initialization](#initialization)
    - [Brace Initialization](#brace-initialization)
    - [Base Class Initialization](#base-class-initialization)
    - [Member Initialization](#member-initialization)
    - [Delegating Constructors](#delegating-constructors)
    - [Move Semantics in Initialization](#move-semantics-in-initialization)
    - [Local Variable Initialization](#local-variable-initialization)
  - [Modern C++ Features](#modern-c-features)
    - [Concepts and Requires Clauses](#concepts-and-requires-clauses)
    - [Auto Return Types](#auto-return-types)
    - [Range-based For Loops and Structured Bindings](#range-based-for-loops-and-structured-bindings)
    - [Smart Pointers and Memory Management](#smart-pointers-and-memory-management)
  - [Constructor/Destructor Patterns](#constructordestructor-patterns)
    - [Default/Delete Special Members](#defaultdelete-special-members)
    - [Singleton Pattern](#singleton-pattern)
  - [Template Programming](#template-programming)
    - [Template Declaration](#template-declaration)
    - [Template Specialization](#template-specialization)
    - [Explicit Template Instantiation](#explicit-template-instantiation)
  - [Type Aliases and Nested Classes](#type-aliases-and-nested-classes)
    - [Type Aliases Placement](#type-aliases-placement)
    - [Nested Classes](#nested-classes)
  - [Operator Overloading](#operator-overloading)
    - [Comparison Operators](#comparison-operators)
    - [Conversion Operators](#conversion-operators)
    - [Dereference and Arrow Operators](#dereference-and-arrow-operators)
    - [Subscript Operators](#subscript-operators)
  - [Comments and Documentation](#comments-and-documentation)
    - [Documentation](#documentation)
    - [Implementation Comments](#implementation-comments)
  - [Error Handling](#error-handling)
    - [Exceptions](#exceptions)
    - [Assertions](#assertions)
  - [Constexpr](#constexpr)
    - [Constexpr Functions](#constexpr-functions)
    - [Constexpr Variables](#constexpr-variables)
  - [Miscellaneous Conventions](#miscellaneous-conventions)
    - [Lambda Expressions](#lambda-expressions)
    - [Literals](#literals)
    - [String Formatting](#string-formatting)
  - [Macros](#macros)
  - [Forward Declarations](#forward-declarations)
  - [Friend Declarations](#friend-declarations)
  - [Summary Checklist](#summary-checklist)
    - [Tooling](#tooling)
    - [File Organization](#file-organization)
    - [Formatting](#formatting)
    - [Naming Conventions](#naming-conventions-1)
    - [Class Design](#class-design)
    - [Initialization](#initialization-1)
    - [Functions and Methods](#functions-and-methods)
    - [Modern C++ Features](#modern-c-features-1)
    - [Templates](#templates)
    - [Namespaces](#namespaces-1)
    - [Documentation and Comments](#documentation-and-comments)
    - [Error Handling](#error-handling-1)
    - [Best Practices](#best-practices)
    - [Code Review Focus Areas](#code-review-focus-areas)
  - [Maintainer](#maintainer)

## Important Notice
- **Not Comprehensive**: This document captures most of — but not all coding styles and conventions, and should not be considered complete. This is not a technical specification. Even if a code fully complies with this guideline, its quality may still be poor. The hope of this written guidelines is that all developers are aware of, and adhere to the principles implied by these rules.
- **Reality Over Rules**: Practical considerations and technical constraints may override style guidelines. Always prioritize readability, maintainability, and team consensus over rigid adherence.
- **Living Document**: This guide evolves with the codebase — expect exceptions and context-dependent variations.
- **Contribute to It**: We encourage developers to contribute to this document.

## Common Principles
- Follow the principle of least surprise.
- When in doubt, match the surrounding style.
- Run `clang-format` every time before saving or committing code.
- Code reviews are the final arbiter of style disputes.

## File Organization and Structure

### File Extensions
- **Header Files**: Use `.h++` extension for all C++ headers (not `.hpp`, `.hh`, or `.h`)
- **Source Files**: Use `.c++` extension for implementation files (not `.cpp`, `.cc`, or `.cxx`)
- **Template Implementation**: Use `.inl` extension for template and inline function implementations

### File Headers and Copyright
All files must begin with a standard copyright header:

```cpp
// -*- C++ -*-
//
// Copyright (C) 2020-2025  Mustard developers
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

### Header Guards and Structure
- Use `#pragma once` instead of traditional include guards
- File structure order:
  1. Copyright header
  2. `#pragma once` (for headers)
  3. System includes
  4. Third-party library includes
  5. Project includes (using full paths relative to source root)
  6. Namespace declarations
  7. Forward declarations (if needed)
  8. Code

```cpp
// -*- C++ -*-
//
// Copyright (C) ...
// ...

#pragma once

#include "Mustard/Geant4X/Generator/DataReader.h++"     // Project headers first
#include "External/YAML/include/yaml-cpp/yaml.h"        // External library headers

#include <vector>        // Standard library last
#include <string>
#include <memory>

// Forward declarations (if needed)
class ForwardDeclaration;

// Forward declarations (if needed)
namespace SomeNamespace {
class FwdDeclaration;
} // namespace SomeNamespace

namespace MACE::Detector::Definition {

// Forward declarations (if needed)
class ForwardDecl;

// class or function definitions

} // namespace MACE::Detector::Definition
```

## Formatting Conventions

### Indentation
- Use **4 spaces** for indentation (no tabs)
- Use **Allman style** braces (braces on new lines)
- No hard limit on maximum line length, but recommend 240 characters or less

### Empty Lines
- Add an empty line:
  - After namespace opening and before closing braces
  - Between function bodies, class bodies, and namespaces
  - Between different member access control group in class
  - Between non-static member group and static member group
  - Before the end of file
- **No** empty line:
  - After class, function, and branch opening braces
  - Before class, function, and branch closing braces

```cpp
namespace MyNamespace1 {

struct MyStruct {
    int fMyVariable;
};

class MyClass {
public:
    void MyFunction(bool condition) {
        if (condition) {
            /* ... */
        } else {
            /* ... */
        }
    }

private:
    int fMyVariable;
};

} // namespace MyNamespace1

namespace MyNamespace2 {

auto MyFunction1(int a, int b) -> auto {
    return a + b;
}

auto MyFunction2(double x) -> auto {
    return x * x;
}

} // namespace MyNamespace2

```

### Spaces
- Use spaces around binary operators (`+`, `-`, `*`, `/`, `=`, `==`, `!=`, `&&`, etc.)
- No space between function name and opening parenthesis
- Space after control statement keywords (`if`, `for`, `while`)

### Initializations
- Variable and initializer in the same line if possible
- Always place Lambda name and initializer in the same line, wrap Lambda body
```cpp
const std::string detectorName{"detector"};
const auto eta{std::sqrt(x + epsilon)};

// √ Good
constexpr auto CompareTrackID{[](const auto& hit1, const auto& hit2) {
    return Get<"TrkID">(*hit1) < Get<"TrkID">(*hit2);
}};
// × Bad
constexpr auto CompareTrackID{
    [](const auto& hit1, const auto& hit2) {
        return Get<"TrkID">(*hit1) < Get<"TrkID">(*hit2);
    }};
```

### Concept
- No wrapping after `=`
```cpp
template<typename M1, typename M2>
concept SubTupleModel = requires {
    /* ... */
};

template<typename T1, typename T2>
concept SubTuple = TupleLike<T1> and TupleLike<T2> and
                   SubTupleModel<typename T1::Model, typename T2::Model>;
```

## Naming Conventions

### General Rules
- Use `camelCase` or `PascalCase` and add prefix depending on context
- Plural form
  - Never use `s` or `es` suffixed plural form; except for template parameter pack
  - Special plural forms like `Momenta` is allowed
  - `s` or `es` in template parameter means a template parameter pack
- Never use underscore `_`, excepts for placeholders in structural bindings e.g. `_1`, `_2`

### Constants and Variables
- **Local constants or variables**: `camelCase`: e.g. `eventCount`, `detectorName`
- **Non-static Member constants or variables**: Prefix with `f` + `PascalCase`: e.g. `fEventCount`, `fDetectorName`
- **Static member constants or variables**: Prefix with `fg` + `PascalCase`: e.g. `fgInstanceSet`
- Boolean flags directly use adjective whenever possible (should **not** begin with `is`, `has`, `can`, or similar)

```cpp
constexpr auto defaultMacro{/* ... */};
const auto outputPath{/* ... */};

class Example {
private:
    std::string fName;
    double fPressure;

    static std::set<DescriptionBase*> fgInstanceSet;
    static constexpr auto fgLearningRatePower{-0.3};
};
```

### Classes and Types
- **`PascalCase` for classes, structs, enums, type aliases, and concepts**
- **Template Parameters**: 
  - **Single uppercase letter**: e.g. `typename A`, or
  - **Prefix with `A` + `PascalCase`**: `typename ADerived`, `int ABitWidth`
- **Template Parameter Pack**: 
  - **Single uppercase letter suffix in plural form**: e.g. `typename Ts`, or
  - **Prefix with `A` + `PascalCase` in plural form**: `typename ADescriptions`, `muc::ceta_string AOptions`

```cpp
class DescriptionBase;

struct VertexInfo;

enum class Status { 
    Ok,
    Error,
    Pending 
};

using EventID = int;

template<typename ADerived>
class DescriptionBase {
    /* ... */ 
};

template<int ABitWidth>
class Statistic;
```

### Functions
- **`PascalCase` for all functions**
- Prefer descriptive verb-based names: `Calculate`, `Process`, `Initialize`, `Update`
- For getters/setters, use property name **without** `Get`/`Set` prefix
- Boolean queries directly use adjective whenever possible (should **not** start with `Is`, `Has`, `Can`, or similar)

```cpp
auto CalculateMomentum() -> double;

auto Name() const -> auto { return fName; }           // getter
auto Name(std::string val) -> void { fName = val; }   // setter
```

### Namespaces
- **`PascalCase` for all namespaces**, excepts
- `namespace internal` for implementation detail

```cpp
namespace MACE::Detector::Definition {

/* ... */

} // namespace MACE::Detector::Definition

namespace Mustard::inline Reconstruction::MMSTracking::inline Finder {

namespace internal {

/* ... */

} // namespace internal

/* ... */

} // namespace Mustard::inline Reconstruction::MMSTracking::inline Finder
```

## Namespace Conventions

### Declaration
- Multi-level namespaces use nested namespace definition or nested inline namespace definition, i.e. definition on one line with `::` syntax
- Use `inline` namespaces for organizing or versioning
- Always add closing comments

```cpp
namespace MACE::Detector::Definition {

/* ... */

} // namespace MACE::Detector::Definition

namespace Mustard::inline Reconstruction::MMSTracking::inline Finder {

/* ... */

} // namespace Mustard::inline Reconstruction::MMSTracking::inline Finder
```

### Using Directives
- **Never use `using namespace` in header files**
- Limit `using` declarations to implementation files or function scope

### Anonymous Namespaces
- Only use for introducing **internal linkage** (**do not use** when there should be or possibly should be external linkage):
- When used in conjunction with `namespace internal`, place `namespace internal` outside the anonymous namespace

```cpp
namespace Mustard::Geant4X::inline Generator {

class Class2 {
    /* ... */
};

namespace {

class Class1 {
    /* ... */
};

} // namespace

namespace internal {
namespace {

class Class1 {
    /* ... */
};

} // namespace
} // namespace internal

} // namespace Mustard::Geant4X::inline Generator
```

## Class Structure

### General rules
- Declare all member functions before any member variables
- Declare all member variables before any member constants
- Declare all member constants and variables in the `protected` or `private` section at the end of the class
- Group related functions together
- Group related constants or variables together
- Order constants or variables by logical relationship, not by type
- Consider cache locality for frequently accessed members variables
- Use `final` when a class should not be further derived

### Typical declaration order
1. Type aliases and nested types
2. Constructors (can be private or protected)
3. Destructor (can be private or protected)
4. Member functions
5. Protected type aliases nested types
6. Protected member functions
7. Protected member variables
8. Private type aliases nested types
9.  Private member functions
10. Private member variables

### Example Class Structure
```cpp
class [[some_attribute]] FinderBase {
public:
    using Hit = AHit;
    using Track = ATrack;

public:
    FinderBase();
    FinderBase(std::string name);

protected:
    ~FinderBase();

public:
    auto MinNHit() const -> auto { return fMinNHit; }
    auto MinNHit(int n) -> void { fMinNHit = n; }

protected:
    template<std::indirectly_readable AHitPointer>
    struct Result {
        /* ... */
    };

protected:
    template<std::indirectly_readable AHitPointer>
    static auto GoodHitData(const std::vector<AHitPointer>& hitData) -> bool;

private:
    int fMinNHit;
    std::string fName;
};
```

### Inheritance
- Place single base class on the same line
```cpp
class Accelerator final : public Mustard::Detector::Definition::DefinitionBase {
    /* ... */
};
```
- Place multiple base classes on the different line successively
```cpp
class Geant4Env : virtual public MPIEnv,
                  virtual public MonteCarloEnv<512>,
                  public Memory::PassiveSingleton<Geant4Env> {
    /* ... */
};
```

## Constant and Variable Declaration

### Const Correctness
- **Use `const` for all local variables that will not be modified after initialization**
- Use `const T&` for read-only, heavy function parameters (heavy objects, containers, etc.)
- Use `std::as_const()` for read-only range-based for loops

### Auto Keyword
- **Use `auto` extensively for type deduction to improve maintainability**
- Use `auto` for complex template types and iterators
- Use explicit types when clarity is essential (e.g., numeric literals that need specific types)

### Reference Qualifiers
- Use `const T&` or `T&` to "alias" a constant or variable 
- Use `const T&` to capture singleton instances
- Use `auto&&` in range-based for loops for universal references; use `std::as_const()` for read-only loops

### Pointer
- Always use `auto` as the type of local pointer whenever possible; do not use `auto*` or `const auto*`
- Use `T*` in function parameter only when it is nullable or for API-friendliness

### Attribute and Specifier Ordering
Sort constant or variable attribute and specifiers by:
1. Attributes (e.g. `[[no_unique_address]]`)
1. `static`
2. `constexpr`
3. `inline`
4. `const`
5. `volatile`
6. `T` (the unqualified type)

## Member Constant and Variable Declaration

### Declaration Order
- Declare all member variables after any member functions
- Declare all member constants after any member variables
- Declare all member constants and variables in the `protected` or `private` section at the end of the class
- Group related constants or variables together
- Order constants or variables by logical relationship, not by type
- Consider cache locality for frequently accessed members variables

## Function Declaration

### Parameters
- Use `T` for 
  - Light parameters (fundamental types, trivially copyable small type, etc.), or
  - Parameters that should be copied and modified
- Use `T&` for passing parameters to be modified in-place
- Use `const T&` for read-only, heavy parameters (heavy objects, containers, etc.)
- Use `T&&` with `std::move()` for transferring ownership
- Use pointer only when really necessary

### Template
- **Use `template<typename T>` instead of `template<class T>`**
- Use normal template syntax if template parameters are explicitly used or there are specializations
- Use abbreviated function template (i.e. `auto`, `const auto&`, etc.) only when template parameters are not essential and no specializations
- Use `concept auto` to constrain abbreviated function templates' template parameters
- Reduce the use of trailing requires-clause
- Use `auto&&` or `T&&` (`T` is a template parameter) and `std::forward` to perfectly forward function parameters

### Return Type Syntax
- **Always use trailing return type** (`auto ... -> Type`) for functions
- Use `auto` or its reference without explicit type for inline function definition (e.g. simple getters that return member variables)
- Use explicit return types when function declaration and definition are separated
- Place return type on the same line whenever appropriate

### Attribute and Specifier Ordering
Sort function specifiers by:
1. Attributes (e.g. `[[noreturn]]`)
2. `static`
3. `constexpr` or `inline`
4. `virtual`

### Trailing Keywords Ordering
Sort function trailing keywords by:
1. `const`
2. `noexcept`
3. `override`
4. `final`

## The `main` Function

### Function Signature
- Always use `auto main(int argc, char* argv[]) -> int`
- Do not use other variants

```cpp
// √ Good
auto main(int argc, char* argv[]) -> int
// √ Good
auto main(int, char*[]) -> int
// × Bad
auto main() -> int
```

### Return Value
- Always write return value for `main` function
- Use `EXIT_SUCCESS` or `EXIT_FAILURE` for `main` function return value
```cpp
auto main(int argc, char* argv[]) {
    /* ... */
    return EXIT_SUCCESS;
}

auto CommandLineInterface() {
    /* ... */
    if (failed) {
        std::exit(EXIT_FAILURE);
    }
}
```

## Member Function Declaration

### Return Type Syntax
- For simple getters returning member variables, use `auto` or its reference without explicit type

```cpp
// Getters returning references/values
auto S1Length() const -> auto { return fS1Length; }
auto Name() const -> const auto& { return fName; }

// Setters
auto S1Length(double val) -> void { fS1Length = val; }

// Functions with explicit return types
auto Density() const -> double;
auto DetailTypeString() const -> std::string_view;
```

### Const Correctness
- Mark all non-modifying member functions as `const`
- Getters should always be `const`

```cpp
// const member functions
auto Calculate() const -> double;
auto Name() const -> const auto& { return fName; }
```

### Virtual Functions and Override
- **Always use `override` keyword for overridden virtual functions**
- **Mark destructors `virtual` in base classes if its derived classes might be polymorphically destructed**
- Mark `virtual` if a function can be further overridden, otherwise
- Mark `final` when it should never be further overridden

## Enumeration

### Use scoped enumeration
- **Never use unscoped enumeration (i.e. plain `enum`), use scoped enumeration (i.e. `enum struct`) instead**
- Use `enum struct` instead of `enum class`
- Explicitly specify the underlying type if necessary

```cpp
// √ Good
enum struct Status2 {
    Ok,
    Error,
    Pending
};

// √ Good
enum struct Status3 : unsigned char {
    Ok,
    Error,
    Pending
};

// × Bad
enum Status1 {
    Ok,
    Error,
    Pending
};
```

## Initialization

### Brace Initialization
- **Always use brace initialization `{}` for initializers**
- Only use parentheses `()` when really necessary (e.g., when avoid calling constructors construct from `std::initializer_list`)
- **Never use assignment `=` in initializer lists**

```cpp
// ✓ CORRECT: Brace initialization
template<int N>
Statistic<N>::Statistic() :
    fSumWX{Eigen::Vector<double, N>::Zero()},
    fSumWXX{Eigen::Matrix<double, N, N>::Zero()},
    fSumWX3{Eigen::Vector<double, N>::Zero()},
    fSumWX4{Eigen::Vector<double, N>::Zero()},
    fSumW{},
    fSumW2{} {
    // Same rules apply to local variable initialization
    const auto num{1.4};
    auto localSum{fSumW};
}
```

### Base Class Initialization
- Use brace initialization for base class constructors with arguments
- Order: base classes first, then member variables

### Member Initialization
- Always initialize all member variables in the constructor initializer list
- Initialize in the same order as declaration to avoid warnings
- Use brace initialization `{}` in initializer lists
- Use in-class initializers only for `constexpr` member constants

```cpp
AdaptiveMTMGenerator<M, N, A>::AdaptiveMTMGenerator(
    const InitialStateMomenta& pI, 
    const std::array<int, N>& pdgID, 
    const std::array<double, N>& mass,
    std::optional<double> thinningRatio, 
    std::optional<unsigned> acfSampleSize) :
    Base{pI, pdgID, mass, std::move(thinningRatio), std::move(acfSampleSize)},
    fGaussian{},
    fIteration{},
    fLearningRate{},
    fRunningMean{},
    fProposalCovariance{},
    fProposalSigma{} {}
```

### Delegating Constructors
- Use brace initialization when delegating to other constructors
- Prefer delegating constructors over code duplication

```cpp
template<std::ranges::input_range S>
constexpr Statistic<1>::Statistic(const S& sample, double weight) :
    Statistic{} {  // Delegate to default constructor
    Fill(sample, weight);
}
```

### Move Semantics in Initialization
- Use `std::move()` for movable parameters in base class initialization

```cpp
Example::Example(int val, std::vector<int> vec, std::string str = {}) :
    fValue{val},                           // Value initialization
    fString{std::move(str)},               // Move initialization
    fVector{std::move(vec)},               // Move initialization
    fPointer{std::make_unique<int>(val)},  // Construction
    fOptional{42} {}                       // Optional with value
```

### Local Variable Initialization
- Use brace initialization for local variables as well
- Use `auto` with brace initialization

```cpp
auto RAMBO<M, N>::RAMBO(const std::array<int, N>& pdgID, const std::array<double, N>& mass) :
    VersatileEventGenerator<M, N, 4 * N>{pdgID, mass},
    fWeightFactor{} {
    constexpr auto po2log{0.45158270528945486};  // Brace initialization
    fWeightFactor[1] = po2log;
    for (auto k{2}; k < N; ++k) {  // Brace initialization in for-loop
        fWeightFactor[k] = fWeightFactor[k - 1] + po2log - 2 * std::log(k - 1);
    }
}
```

## Modern C++ Features

### Concepts and Requires Clauses
Use C++20 concepts extensively for template constraints:

```cpp
template<int N>
    requires(N > 0)
class Statistic { /* ... */ };

template<typename T>
concept Description =
    requires {
        requires std::derived_from<T, DescriptionBase<>>;
        requires Env::Memory::Singletonified<T>;
    };

// Function templates with concept constraints
auto UnpackToLeafNodeForImporting(const YAML::Node& node, 
                                  std::convertible_to<std::string> auto&&... names) 
    -> std::optional<const YAML::Node>;
```

### Auto Return Types
- **Always use `auto` with trailing return type**
- Use `[[nodiscard]]`, `[[noreturn]]` attributes where appropriate

```cpp
[[nodiscard]] auto GetPrimaryVertex() const -> const Vertex&;
auto EmitYAMLImpl(const YAML::Node& node, YAML::Emitter& emitter, bool inFlow = false) -> void;
```

### Range-based For Loops and Structured Bindings
- Use `auto&&` in range-based for loops for universal references
- Use `std::as_const()` for read-only range-based for loops
- Use placeholder names `_1`, `_2`, `_3`, etc. for unused bindings
```cpp
for (auto&& [modID, splitHit] : std::as_const(fSplitHit)) {
    /* ... */
}

// use placeholder names for unused bindings
auto [weight, pdgID, p]{generator(rng)};
const auto& [p0, _1, _2, p3, p4]{momenta};
```

### Smart Pointers and Memory Management
- **Use smart pointers (`std::unique_ptr`, `std::shared_ptr`) for ownership**
- Use `gsl::not_null` for non-nullable pointers
- **Prefer references over pointers when ownership is not transferred**

```cpp
std::unique_ptr<Detector> fDetector;
gsl::not_null<const Process*> fProcess;
const muc::unique_ptrvec<...>* fPrimaryVertex;  // non-owning pointer
```

## Constructor/Destructor Patterns

### Default/Delete Special Members
- **Do not provide constructor or destructor if you need a default one; compiler will implicitly define them**
- Only when necessary, explicitly `= default` or `= delete` special members

```cpp
class BadDefaultInitializedClass {
public:
    BadDefaultInitializedClass() {}
    ~BadDefaultInitializedClass() {}
    /* ... */
};

class GoodDefaultInitializedClass {
    // no explicit constructor or destructor
    /* ... */
};

class GoodProtectedDefaultInitializedClass {
protected: // use `= default` if constructor or destructor should be protected
    GoodProtectedDefaultInitializedClass() = default;
    ~GoodProtectedDefaultInitializedClass() = default;

public:
    /* ... */
};
```

### Singleton Pattern
- Use `Mustard::Env::Singleton` and its variants
- **Never write your own singleton**
```cpp
class Solenoid final : public Mustard::Detector::Description::DescriptionBase<Solenoid> {
    friend Mustard::Env::Memory::SingletonInstantiator;

private:
    Solenoid();
    ~Solenoid() = default;

public:
    // public interface
};
```

## Template Programming

### Template Declaration
For complex template declarations, place constraints on separate lines:

```cpp
template<typename... Ts>
    requires(sizeof...(Ts) <= 1)
class DescriptionBase;

template<int N>
    requires(N > 0)
class Statistic;
```

### Template Specialization
Declare the general template first, then specializations:

```cpp
template<int N>
    requires(N > 0)
class Statistic;

template<>
class Statistic<1> {
    // Specialization for N=1
};

template<int N>
    requires(N > 0)
class Statistic {
    // General implementation
};
```

### Explicit Template Instantiation
Declare extern templates in headers when specializations are defined in source file:

```cpp
extern template class UseXoshiro<256>;
extern template class UseXoshiro<512>;

extern template class File<std::ifstream>;
extern template class File<std::wifstream>;
```

## Type Aliases and Nested Classes

### Type Aliases Placement
- Put `using` declarations at the top of the public section
- Group related aliases together

```cpp
class FinderBase {
public:
    using Hit = AHit;
    using Track = ATrack;
    using Result = std::vector<Track>;

public:
    // constructors and methods...
};
```

### Nested Classes
Declare nested classes in the appropriate access section:

```cpp
class Target final : public Mustard::Detector::Description::DescriptionBase<Target> {
    /* ... */
    class MultiLayerTarget final : public ShapeBase<MultiLayerTarget> {
    public:
        enum struct ShapeDetailType {
            Flat,
            Perforated
        };

    public:
        MultiLayerTarget();
        /* ... */
    };
};
```

## Operator Overloading

### Comparison Operators
- Use C++20 three-way comparison (`<=>`) when appropriate
- Implement `operator==` for equality comparison

```cpp
template<ValueAcceptable U, muc::ceta_string N, muc::ceta_string D>
constexpr auto operator==(const Value<U, N, D>& that) const -> bool { 
    return **this == *that; 
}

template<ValueAcceptable U, muc::ceta_string N, muc::ceta_string D>
constexpr auto operator<=>(const Value<U, N, D>& that) const -> auto { 
    return **this <=> *that; 
}
```

### Conversion Operators
```cpp
constexpr operator const T&() const& { return fObject; }
constexpr operator T&() & { return fObject; }
constexpr operator T&&() && { return std::move(fObject); }
constexpr operator const T&&() const&& { return std::move(fObject); }
```

### Dereference and Arrow Operators
```cpp
constexpr auto operator*() const& -> const T& { return fObject; }
constexpr auto operator*() & -> T& { return fObject; }
constexpr auto operator*() && -> T&& { return std::move(fObject); }
constexpr auto operator*() const&& -> const T&& { return std::move(fObject); }

constexpr auto operator->() const -> const T* { return std::addressof(fObject); }
constexpr auto operator->() -> T* { return std::addressof(fObject); }
```

### Subscript Operators
```cpp
constexpr auto operator[](auto&& i) const& -> decltype(auto)
    requires requires(const T fObject) { fObject[std::forward<decltype(i)>(i)]; }
{ return fObject[std::forward<decltype(i)>(i)]; }

constexpr auto operator[](auto&& i) & -> decltype(auto)
    requires requires(T fObject) { fObject[std::forward<decltype(i)>(i)]; }
{ return fObject[std::forward<decltype(i)>(i)]; }
```

## Comments and Documentation

### Documentation
- **Use Doxygen-style comments `///` for public APIs**

```cpp
/// @brief Class for maximization based on "Whale Optimization Algorithm"
/// doi:10.1016/j.advengsoft.2016.01.008
class EMMaximization { 
    /// @brief Calculate the objective function
    /// @param parameters The input parameters
    /// @return The objective function value
    auto Calculate(const std::vector<double>& parameters) -> double;
    /* ... */
};
```

```cpp
/// @brief Computes the squared Euclidean norm of a floating-point vector
///
/// Calculates the sum of squares of all components in a numeric vector.
/// Compile-time optimized using component unpacking.
///
/// @tparam V Numeric vector type (satisfies NumericVectorFloatingPoint concept)
/// @param x Vector input (must support index-based element access)
///
/// @return Sum of squares: x₀² + x₁² + ... + xₙ²
constexpr auto NormSq(const Concept::NumericVectorFloatingPoint auto& x) {
    // implementation
}
```

### Implementation Comments
- Use `//` for implementation comments
- Section dividers with multiple slashes
- Placeholder comments with `/* ... */`

```cpp
///////////////////////////////////////////////////////////
// Geometry Construction
///////////////////////////////////////////////////////////

class Example {
public:
    void PublicMethod();
    /* ... */
private:
    int fMemberVar;
};
```

## Error Handling

### Exceptions
- **Use `Mustard::Throw<exception_type>()` for exceptions, do not use plain `throw` directly**
- Usually use standard library exceptions: `std::runtime_error`, `std::invalid_argument`, etc.

```cpp
if (not opened) {
    Mustard::Throw<std::runtime_error>(fmt::format("Cannot open file '{}' with mode '{}'", outputPath, mode));
}

if (std::ranges::size(sample) > std::ranges::size(weight)) {
    Throw<std::invalid_argument>("Size of sample > size of weight");
}
```

### Assertions
- Use GSL `Expects(...)` for precondition and `Ensures(...)` for postcondition
- Reduce the use of `assert`
```cpp
auto Calculate(double x) {
    Expects(x > 0);
    /* ... */
    Ensures(result < 0);
    return result;
}
```

## Constexpr

### Constexpr Functions
- Use `constexpr` for compile-time evaluable functions
- Mark constructors and member functions `constexpr` when possible and reasonable

```cpp
constexpr auto NormSq(const Concept::NumericVectorFloatingPoint auto& x) {
    return [&x]<gsl::index... Is>(gslx::index_sequence<Is...>) {
        return muc::hypot_sq(x[Is]...);
    }(gslx::make_index_sequence<VectorDimension<std::decay_t<decltype(x)>>>());
}

constexpr auto Fill(double sample, double weight) -> void {
    const auto wx{weight * sample};
    fSumWX += wx;
    /* ... */
}
```

### Constexpr Variables
- Use `constexpr` instead of `const` or `#define` for compile-time constants
```cpp
// √ Good
constexpr auto po2log{0.45158270528945486};
// × bad
const auto alpha{1 / 137.035999084};
// × bad
#define PI 3.14159265358979324
```

## Miscellaneous Conventions

### Lambda Expressions
- Write "one-time" functions as Lambda expressions instead of independent functions
- Use `auto` parameters for generic lambdas

```cpp
// √ Good
auto SortTrack1(std::vector<Track>& event) -> void {
    constexpr auto CompareTrackID{[](const auto& hit1, const auto& hit2) {
        return Get<"TrkID">(*hit1) < Get<"TrkID">(*hit2);
    }};
    std::ranges::sort(event, CompareTrackID);
}

// × Bad
namespace {

auto CompareTrackID(const auto& hit1, const auto& hit2) -> auto {
    return Get<"TrkID">(*hit1) < Get<"TrkID">(*hit2);
}

} // namespace

auto SortTrack1(std::vector<Track>& event) -> void {
    std::ranges::sort(event, CompareTrackID);
}
```

### Literals
- Use physical unit literals for literal values

```cpp
using namespace Mustard::LiteralUnit::Length;
using namespace Mustard::LiteralUnit::Time;
using namespace Mustard::LiteralUnit::Energy;
// or simply `using namespace Mustard::LiteralUnit`
using namespace std::string_literals;

// Usage
104_mm
3_ns
52.8_MeV
3 * MeV // bad, literal values should use literal suffix
"filename"s  // std::string literal
```

### String Formatting
- Use `fmt::format` for string formatting (not `std::format` or iostreams for formatting)

```cpp
fmt::format("Cannot open file '{}' with mode '{}'", outputPath, mode)
fmt::format("Entries of provided event split ({}) is inconsistent with the dataset ({})",
            nEntry, nEntryRDF)
```

## Macros
Minimize macro usage, prefer constexpr functions and variables.

When necessary:
- Use `ALL_CAPS` naming
- Define in implementation files when possible
- Undefine macros after use when appropriate

```cpp
#define EMLogger(level, msg, type)     \
    if (level > EMLog::ReportingLevel) \
        ;                              \
    else                               \
        EMLog().Get(level, msg, type)

// Macro with cleanup
#define MUSTARD_UTILITY_VECTOR_ARITHMETIC_OPERATOR_VECTOR2_SCALAR_OPERATORS(T) \
    /* ... macro body ... */

// Usage
MUSTARD_UTILITY_VECTOR_ARITHMETIC_OPERATOR_VECTOR2_SCALAR_OPERATORS(float)
MUSTARD_UTILITY_VECTOR_ARITHMETIC_OPERATOR_VECTOR2_SCALAR_OPERATORS(double)

#undef MUSTARD_UTILITY_VECTOR_ARITHMETIC_OPERATOR_VECTOR2_SCALAR_OPERATORS
```

## Forward Declarations
Use explicit namespace and class forward declaration when needed:

```cpp
namespace Mustard::Detector::Definition {
class DefinitionBase;
} // Mustard::Detector::Definition

class G4Material;
```

## Friend Declarations
- Declare friend at the top of class definition, before any access control labels, or
- Declare nested class as friend just after its definition

```cpp
class Solenoid final : public Mustard::Detector::Description::DescriptionBase<Solenoid> {
    friend Mustard::Env::Memory::SingletonInstantiator;

private:
    Solenoid();
    /* ... */
};

class Container {
    /* ... */
public:
    class iterator {
        /* ... */
    };
    friend class iterator;
};
```

## Summary Checklist

### Tooling
- [ ] Run `clang-format`

### File Organization
- [ ] Use `.h++` for headers and `.c++` for implementation files
- [ ] Include standard copyright header in all files
- [ ] Use `#pragma once` instead of traditional include guards
- [ ] Organize includes in correct order: project → framework → external → standard library
- [ ] Use full paths relative to source root for project includes
- [ ] Use proper namespace structure with closing comments

### Formatting
- [ ] Use 4 spaces for indentation (no tabs)
- [ ] Follow Allman brace style (braces on new lines)
- [ ] Use spaces around binary operators
- [ ] No space between function name and opening parenthesis
- [ ] Space after control statement keywords (`if`, `for`, `while`)
- [ ] Use brace initialization `{}` consistently
- [ ] Keep lambda names and initializers on the same line
- [ ] No wrapping after `=` in concept definitions
- [ ] Add empty lines between functions, classes, and namespaces
- [ ] Add empty lines after namespace opening braces
- [ ] Add empty lines before namespace closing braces
- [ ] No empty lines after functions, classes, branch opening braces
- [ ] No empty lines before functions, classes, branch closing braces

### Naming Conventions
- [ ] Use `camelCase` for local variables and constants
- [ ] Use `fPascalCase` for non-static member variables
- [ ] Use `fgPascalCase` for static member variables
- [ ] Use `PascalCase` for classes, structs, enums, type aliases, and concepts
- [ ] Use `PascalCase` for all functions
- [ ] Use `PascalCase` for namespaces (except `internal`)
- [ ] Use descriptive verb-based names for functions
- [ ] Boolean flags/queries use adjectives directly (no `is`/`has`/`can` prefixes)
- [ ] Template parameters use single uppercase letters or `APascalCase`
- [ ] Template parameter packs use plural forms (`Ts` or `ADescriptions`)
- [ ] No plural forms with `s`/`es` suffixes (except template packs)
- [ ] Never use underscores except for structural binding placeholders

### Class Design
- [ ] Declare member functions before member variables
- [ ] Group related functions and variables together
- [ ] Order members by logical relationship, not type
- [ ] Use `final` when classes shouldn't be further derived
- [ ] Follow typical declaration order: types → constructors → destructor → functions → variables
- [ ] Declare type aliases at top of public section
- [ ] Place single base class on same line, multiple base classes on successive lines
- [ ] Use `virtual` destructors in base classes when needed
- [ ] Always use `override` for overridden virtual functions
- [ ] Mark non-modifying member functions as `const`
- [ ] Use scoped enumerations (`enum struct`) only

### Initialization
- [ ] Always use brace initialization `{}`
- [ ] Initialize all member variables in constructor initializer list
- [ ] Initialize members in declaration order
- [ ] Use `std::move()` for movable parameters in initialization
- [ ] Use brace initialization for local variables
- [ ] Use in-class initializers only for `constexpr` members
- [ ] Prefer delegating constructors over code duplication

### Functions and Methods
- [ ] Always use trailing return type syntax (`auto ... -> Type`)
- [ ] Use `const` for all non-modifying member functions
- [ ] Mark getters as `const`
- [ ] Use `auto` without explicit type for simple getters
- [ ] Use `T` for light parameters, `const T&` for heavy read-only parameters
- [ ] Use `T&` for in-place modification, `T&&` for ownership transfer
- [ ] Use `auto` extensively for type deduction
- [ ] Properly order function specifiers: attributes → static → constexpr → virtual
- [ ] Properly order trailing keywords: const → noexcept → override → final
- [ ] Use `main(int argc, char* argv[]) -> int` signature
- [ ] Always return explicit value from `main` function

### Modern C++ Features
- [ ] Use C++20 concepts extensively for template constraints
- [ ] Use `[[nodiscard]]`, `[[noreturn]]` attributes where appropriate
- [ ] Use `auto&&` in range-based for loops for universal references
- [ ] Use `std::as_const()` for read-only range-based for loops
- [ ] Use structured bindings with placeholder names for unused variables
- [ ] Use smart pointers for ownership management
- [ ] Use `gsl::not_null` for non-nullable pointers
- [ ] Prefer references over pointers when ownership isn't transferred
- [ ] Use `constexpr` for compile-time evaluable functions and variables
- [ ] Use physical unit literals for literal values
- [ ] Use `fmt::format` for string formatting

### Templates
- [ ] Use `template<typename T>` instead of `template<class T>`
- [ ] Use concepts and requires clauses for template constraints
- [ ] Use abbreviated function templates when appropriate
- [ ] Use `concept auto` to constrain abbreviated templates
- [ ] Place complex constraints on separate lines
- [ ] Use `auto&&` with `std::forward` for perfect forwarding
- [ ] Declare general template before specializations
- [ ] Use explicit template instantiation declarations when needed
- [ ] Use extern templates for explicit instantiation

### Namespaces
- [ ] Use multi-level namespace definitions with `::` syntax
- [ ] Use `inline` namespaces for organization and versioning
- [ ] Always add closing namespace comments
- [ ] Never use `using namespace` in header files
- [ ] Limit `using` declarations to implementation files
- [ ] Use anonymous namespaces only for internal linkage
- [ ] Place `namespace internal` outside anonymous namespaces

### Documentation and Comments
- [ ] Use Doxygen-style `///` comments for public APIs
- [ ] Use `//` for implementation comments
- [ ] Add `@brief`, `@param`, `@return` tags for documentation
- [ ] Document parameters, return values, and exceptions
- [ ] Use section dividers with multiple slashes for organization
- [ ] Use placeholder comments `/* ... */` where needed

### Error Handling
- [ ] Use `Mustard::Throw<exception_type>()` instead of plain `throw`
- [ ] Use standard library exceptions appropriately (`runtime_error`, `invalid_argument`)
- [ ] Use GSL `Expects()` for preconditions and `Ensures()` for postconditions
- [ ] Minimize use of `assert`

### Best Practices
- [ ] Follow principle of least surprise
- [ ] Maintain consistency with existing code
- [ ] Prioritize readability and maintainability
- [ ] Use `const` correctness extensively
- [ ] Use `const` for all unmodified local variables
- [ ] Use `const T&` for read-only heavy parameters
- [ ] Write "one-time" functions as lambdas instead of independent functions
- [ ] Minimize macro usage, prefer constexpr alternatives
- [ ] Use proper forward declarations
- [ ] Place friend declarations at top of class definition
- [ ] Use Mustard singleton patterns instead of writing custom ones
- [ ] Use `Mustard::Env::Singleton` instead of writing custom singletons
- [ ] Don't provide constructors/destructors if default ones are sufficient

### Code Review Focus Areas
- [ ] Naming convention compliance
- [ ] Const correctness
- [ ] Proper initialization
- [ ] Memory management and ownership
- [ ] Exception safety
- [ ] Template constraints and concepts
- [ ] Documentation quality
- [ ] Consistency with existing codebase
- [ ] Modern C++ feature usage
- [ ] Error handling approach
- [ ] Performance considerations
- [ ] Testability and documentation
- [ ] Readability and maintainability

## Maintainer
[**@zhao-shihan**](https://github.com/zhao-shihan)

Thank you for contributing to Mustard!
