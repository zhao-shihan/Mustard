// -*- C++ -*-
//
// Copyright (C) 2020-2025  The Mustard development team
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

namespace Mustard::Detector::Description {

namespace internal {
namespace {

template<gsl::index i, typename T>
struct FillDescriptionArray {
    constexpr void operator()(std::array<DescriptionBase<>*, std::tuple_size_v<T>>& descriptions) const {
        std::get<i>(descriptions) = &std::tuple_element_t<i, T>::Instance();
    }
};

template<gsl::index Begin, gsl::index End,
         template<gsl::index, typename...> typename, typename...>
    requires(Begin >= End)
constexpr void StaticForEach(auto&&...) {}

template<gsl::index Begin, gsl::index End,
         template<gsl::index, typename...> typename AFunctor, typename... AFunctorArgs>
    requires(Begin < End and std::default_initializable<AFunctor<Begin, AFunctorArgs...>>)
constexpr void StaticForEach(auto&&... args) {
    AFunctor<Begin, AFunctorArgs...>()(std::forward<decltype(args)>(args)...);
    StaticForEach<Begin + 1, End,
                  AFunctor, AFunctorArgs...>(std::forward<decltype(args)>(args)...);
}

} // namespace
} // namespace internal

template<Description... Ds>
auto DescriptionIO::Import(const std::filesystem::path& yamlPath) -> void {
    Import<std::tuple<Ds...>>(yamlPath);
}

template<Description... Ds>
auto DescriptionIO::Export(const std::filesystem::path& yamlPath, const std::string& fileComment) -> std::filesystem::path {
    return Export<std::tuple<Ds...>>(yamlPath, fileComment);
}

template<Description... Ds>
auto DescriptionIO::Emport(const std::filesystem::path& yamlPath, const std::string& fileComment) -> std::pair<std::filesystem::path, std::filesystem::path> {
    return Emport<std::tuple<Ds...>>(yamlPath, fileComment);
}

template<Description... Ds>
auto DescriptionIO::ToString() -> std::string {
    return ToString<std::tuple<Ds...>>();
}

template<muc::tuple_like T>
auto DescriptionIO::Import(const std::filesystem::path& yamlPath) -> void {
    std::array<DescriptionBase<>*, std::tuple_size_v<T>> descriptions;
    internal::StaticForEach<0, descriptions.size(),
                            internal::FillDescriptionArray, T>(descriptions);
    ImportImpl(yamlPath, descriptions);
}

template<muc::tuple_like T>
auto DescriptionIO::Export(const std::filesystem::path& yamlPath, const std::string& fileComment) -> std::filesystem::path {
    std::array<DescriptionBase<>*, std::tuple_size_v<T>> descriptions;
    internal::StaticForEach<0, descriptions.size(),
                            internal::FillDescriptionArray, T>(descriptions);
    return ExportImpl(yamlPath, fileComment, descriptions);
}

template<muc::tuple_like T>
auto DescriptionIO::Emport(const std::filesystem::path& yamlPath, const std::string& fileComment) -> std::pair<std::filesystem::path, std::filesystem::path> {
    std::array<DescriptionBase<>*, std::tuple_size_v<T>> descriptions;
    internal::StaticForEach<0, descriptions.size(),
                            internal::FillDescriptionArray, T>(descriptions);
    return EmportImpl(yamlPath, fileComment, descriptions);
}

template<muc::tuple_like T>
auto DescriptionIO::ToString() -> std::string {
    std::array<DescriptionBase<>*, std::tuple_size_v<T>> descriptions;
    internal::StaticForEach<0, descriptions.size(),
                            internal::FillDescriptionArray, T>(descriptions);
    return ToStringImpl(descriptions);
}

template<typename... ArgsOfImport>
auto DescriptionIO::Import(const std::ranges::range auto& yamlText) -> void
    requires std::convertible_to<typename std::decay_t<decltype(yamlText)>::value_type, std::string> {
    const auto tempYAMLPath{CreateTemporaryFile("geom", ".yaml")};
    const auto _{gsl::finally([&] {
        std::error_code muteRemoveError;
        std::filesystem::remove(tempYAMLPath, muteRemoveError);
    })};
    {
        File<std::FILE> tempYAMLFile{tempYAMLPath, "w"};
        for (auto&& line : yamlText) {
            fmt::println(tempYAMLFile, "{}", line);
        }
    }
    Import<ArgsOfImport...>(tempYAMLPath);
}

auto DescriptionIO::ImportImpl(const std::filesystem::path& yamlPath, std::ranges::input_range auto& descriptions) -> void {
    const auto geomYaml{YAML::LoadFile(yamlPath.generic_string())};
    for (auto&& description : std::as_const(descriptions)) {
        description->Import(geomYaml);
    }
}

auto DescriptionIO::ExportImpl(const std::filesystem::path& yamlPath, const std::string& fileComment, const std::ranges::input_range auto& descriptions) -> std::filesystem::path {
    std::vector<std::pair<std::string_view, DescriptionBase<>*>> sortedDescriptions;
    sortedDescriptions.reserve(descriptions.size());
    for (auto&& description : descriptions) {
        sortedDescriptions.emplace_back(description->Name(), description);
    }
    std::ranges::sort(sortedDescriptions);

    YAML::Node geomYaml;
    for (auto&& [_, description] : std::as_const(sortedDescriptions)) {
        description->Export(geomYaml);
    }

    ProcessSpecificFile<std::ofstream> yamlOut{yamlPath};
    if (not yamlOut.Opened()) [[unlikely]] {
        PrintError("Cannot open yaml file, export failed");
        return {};
    }
    EmitYAML(geomYaml, fileComment, yamlOut);
    return yamlOut.Path();
}

auto DescriptionIO::EmportImpl(const std::filesystem::path& yamlPath, const std::string& fileComment, const std::ranges::input_range auto& descriptions) -> std::pair<std::filesystem::path, std::filesystem::path> {
    auto path1{ExportImpl(std::filesystem::path{yamlPath}.replace_extension(".prev.yaml"), fileComment, descriptions)};
    ImportImpl(yamlPath, descriptions);
    auto path2{ExportImpl(std::filesystem::path{yamlPath}.replace_extension(".curr.yaml"), fileComment, descriptions)};
    return {std::move(path1), std::move(path2)};
}

auto DescriptionIO::ToStringImpl(const std::ranges::input_range auto& descriptions) -> std::string {
    std::vector<std::pair<std::string_view, DescriptionBase<>*>> sortedDescriptions;
    sortedDescriptions.reserve(descriptions.size());
    for (auto&& description : descriptions) {
        sortedDescriptions.emplace_back(description->Name(), description);
    }
    std::ranges::sort(sortedDescriptions);

    YAML::Node geomYaml;
    for (auto&& [_, description] : std::as_const(sortedDescriptions)) {
        description->Export(geomYaml);
    }

    std::ostringstream oss;
    EmitYAML(geomYaml, {}, oss);
    return oss.str();
}

} // namespace Mustard::Detector::Description
