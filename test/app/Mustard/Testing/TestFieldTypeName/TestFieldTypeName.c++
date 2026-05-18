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

#include "Mustard/CLI/BasicCLI.h++"
#include "Mustard/Data/Object/Field.h++"
#include "Mustard/Data/Object/FieldTypeName.h++"
#include "Mustard/Env/BasicEnv.h++"
#include "Mustard/Testing/TestFieldTypeName/TestFieldTypeName.h++"

#include <array>
#include <bitset>
#include <cstdint>
#include <cstdlib>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <tuple>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace Mustard::Testing {

TestFieldTypeName::TestFieldTypeName() :
    Subprogram{"TestFieldTypeName", "Test Mustard::Data::FieldTypeName."} {}

namespace {

template<typename T>
auto ExpectTypeName(std::string_view expected) -> bool {
    const auto name{Mustard::Data::FieldTypeName<T>()};
    fmt::println("{}", muc::try_demangle(typeid(T).name()));
    fmt::println("==> {}", name);
    return name == expected;
}

template<typename T>
auto ExpectObjectName(const T& object, std::string_view expected) -> bool {
    const auto name{Mustard::Data::FieldTypeName(object)};
    fmt::println("{}", muc::try_demangle(typeid(T).name()));
    fmt::println("==> {}", name);
    return name == expected;
}

} // namespace

auto TestFieldTypeName::Main(int argc, char* argv[]) const -> int {
    Mustard::CLI::BasicCLI<> cli;
    Mustard::Env::BasicEnv env{argc, argv, cli};

    using namespace Mustard::Data;

    if (not ExpectTypeName<bool>("b")) {
        return EXIT_FAILURE;
    }
    if (not ExpectTypeName<char>("ch")) {
        return EXIT_FAILURE;
    }
    if (not ExpectTypeName<std::int8_t>("i8")) {
        return EXIT_FAILURE;
    }
    if (not ExpectTypeName<std::uint8_t>("u8")) {
        return EXIT_FAILURE;
    }
    if (not ExpectTypeName<std::int16_t>("i16")) {
        return EXIT_FAILURE;
    }
    if (not ExpectTypeName<std::uint16_t>("u16")) {
        return EXIT_FAILURE;
    }
    if (not ExpectTypeName<std::int32_t>("i32")) {
        return EXIT_FAILURE;
    }
    if (not ExpectTypeName<std::uint32_t>("u32")) {
        return EXIT_FAILURE;
    }
    if (not ExpectTypeName<std::int64_t>("i64")) {
        return EXIT_FAILURE;
    }
    if (not ExpectTypeName<std::uint64_t>("u64")) {
        return EXIT_FAILURE;
    }
    if (not ExpectTypeName<float>("f32")) {
        return EXIT_FAILURE;
    }
    if (not ExpectTypeName<double>("f64")) {
        return EXIT_FAILURE;
    }
    if (not ExpectTypeName<std::string>("str")) {
        return EXIT_FAILURE;
    }
    if (not ExpectTypeName<std::bitset<8>>("bset<8>")) {
        return EXIT_FAILURE;
    }
    if (not ExpectTypeName<std::pair<bool, std::int32_t>>("pair<b,i32>")) {
        return EXIT_FAILURE;
    }
    if (not ExpectTypeName<std::tuple<bool, std::vector<std::int32_t>, std::pair<std::string, double>>>("tup<b,vec<i32>,pair<str,f64>>")) {
        return EXIT_FAILURE;
    }
    if (not ExpectTypeName<std::vector<std::string>>("vec<str>")) {
        return EXIT_FAILURE;
    }
    if (not ExpectTypeName<std::array<std::uint16_t, 3>>("arr<u16,3>")) {
        return EXIT_FAILURE;
    }
    if (not ExpectTypeName<std::optional<std::pair<bool, std::int32_t>>>("opt<pair<b,i32>>")) {
        return EXIT_FAILURE;
    }
    if (not ExpectTypeName<std::set<std::int32_t>>("set<i32>")) {
        return EXIT_FAILURE;
    }
    if (not ExpectTypeName<std::unordered_set<std::string>>("uset<str>")) {
        return EXIT_FAILURE;
    }
    if (not ExpectTypeName<std::multiset<std::int32_t>>("mset<i32>")) {
        return EXIT_FAILURE;
    }
    if (not ExpectTypeName<std::unordered_multiset<std::string>>("umset<str>")) {
        return EXIT_FAILURE;
    }
    if (not ExpectTypeName<std::map<std::int32_t, std::pair<bool, std::string>>>("map<i32,pair<b,str>>")) {
        return EXIT_FAILURE;
    }
    if (not ExpectTypeName<std::unordered_map<std::int32_t, std::vector<double>>>("umap<i32,vec<f64>>")) {
        return EXIT_FAILURE;
    }
    if (not ExpectTypeName<std::multimap<std::int32_t, std::optional<char>>>("mmap<i32,opt<ch>>")) {
        return EXIT_FAILURE;
    }
    if (not ExpectTypeName<std::unordered_multimap<std::int32_t, std::tuple<bool, std::string>>>("ummap<i32,tup<b,str>>")) {
        return EXIT_FAILURE;
    }

    if (not ExpectObjectName(std::vector<std::int32_t>{}, "vec<i32>")) {
        return EXIT_FAILURE;
    }
    if (not ExpectObjectName(std::pair<bool, std::int32_t>{}, "pair<b,i32>")) {
        return EXIT_FAILURE;
    }
    if (not ExpectObjectName(std::array<std::uint16_t, 3>{}, "arr<u16,3>")) {
        return EXIT_FAILURE;
    }

    using WrappedVector = Field<std::vector<int>, "vi32", std::vector<std::int32_t>>;
    const WrappedVector wrappedVector{};
    if (not ExpectTypeName<WrappedVector>("vec<i32>")) {
        return EXIT_FAILURE;
    }
    if (not ExpectObjectName(wrappedVector, "vec<i32>")) {
        return EXIT_FAILURE;
    }

    using WrappedTuple = Field<std::tuple<bool, std::string>, "tbstr", std::tuple<bool, std::string>>;
    const WrappedTuple wrappedTuple{};
    if (not ExpectTypeName<WrappedTuple>("tup<b,str>")) {
        return EXIT_FAILURE;
    }
    if (not ExpectObjectName(wrappedTuple, "tup<b,str>")) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

} // namespace Mustard::Testing
