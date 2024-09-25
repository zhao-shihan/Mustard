// -*- C++ -*-
//
// Copyright 2020-2024  The Mustard development team
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

template<std::intmax_t i, typename T>
struct FillDescriptionArray {
    constexpr void operator()(std::array<DescriptionBase<>*, std::tuple_size_v<T>>& descriptions) const {
        std::get<i>(descriptions) = &std::tuple_element_t<i, T>::Instance();
    }
};

template<std::intmax_t Begin, std::intmax_t End,
         template<std::intmax_t, typename...> typename, typename...>
    requires(Begin >= End)
constexpr void StaticForEach(auto&&...) {}

template<std::intmax_t Begin, std::intmax_t End,
         template<std::intmax_t, typename...> typename AFunctor, typename... AFunctorArgs>
    requires(Begin < End and std::default_initializable<AFunctor<Begin, AFunctorArgs...>>)
constexpr void StaticForEach(auto&&... args) {
    AFunctor<Begin, AFunctorArgs...>()(std::forward<decltype(args)>(args)...);
    StaticForEach<Begin + 1, End,
                  AFunctor, AFunctorArgs...>(std::forward<decltype(args)>(args)...);
}

} // namespace
} // namespace internal

template<Description... Ds>
auto Import(const std::filesystem::path& yamlFile) -> void {
    Import<std::tuple<Ds...>>(yamlFile);
}

template<Description... Ds>
auto DescriptionIO::Export(const std::filesystem::path& yamlFile, const std::string& fileComment) -> void {
    Export<std::tuple<Ds...>>(yamlFile, fileComment);
}

template<Description... Ds>
auto DescriptionIO::Ixport(const std::filesystem::path& yamlFile, const std::string& fileComment) -> std::pair<std::filesystem::path, std::filesystem::path> {
    return Ixport<std::tuple<Ds...>>(yamlFile, fileComment);
}

template<Description... Ds>
auto DescriptionIO::ParallelExport(const std::filesystem::path& yamlFile, const std::string& fileComment) -> std::filesystem::path {
    return ParallelExport<std::tuple<Ds...>>(yamlFile, fileComment);
}

template<Description... Ds>
auto DescriptionIO::ParallelIxport(const std::filesystem::path& yamlFile, const std::string& fileComment) -> std::pair<std::filesystem::path, std::filesystem::path> {
    return ParallelIxport<std::tuple<Ds...>>(yamlFile, fileComment);
}

template<muc::tuple_like T>
auto DescriptionIO::Import(const std::filesystem::path& yamlFile) -> void {
    std::array<DescriptionBase<>*, std::tuple_size_v<T>> descriptions;
    internal::StaticForEach<0, descriptions.size(),
                            internal::FillDescriptionArray, T>(descriptions);
    ImportImpl(yamlFile, descriptions);
}

template<muc::tuple_like T>
auto DescriptionIO::Export(const std::filesystem::path& yamlFile, const std::string& fileComment) -> void {
    std::array<DescriptionBase<>*, std::tuple_size_v<T>> descriptions;
    internal::StaticForEach<0, descriptions.size(),
                            internal::FillDescriptionArray, T>(descriptions);
    ExportImpl(yamlFile, fileComment, descriptions);
}

template<muc::tuple_like T>
auto DescriptionIO::Ixport(const std::filesystem::path& yamlFile, const std::string& fileComment) -> std::pair<std::filesystem::path, std::filesystem::path> {
    std::array<DescriptionBase<>*, std::tuple_size_v<T>> descriptions;
    internal::StaticForEach<0, descriptions.size(),
                            internal::FillDescriptionArray, T>(descriptions);
    return IxportImpl(yamlFile, fileComment, descriptions);
}

template<muc::tuple_like T>
auto DescriptionIO::ParallelExport(const std::filesystem::path& yamlFile, const std::string& fileComment) -> std::filesystem::path {
    std::array<DescriptionBase<>*, std::tuple_size_v<T>> descriptions;
    internal::StaticForEach<0, descriptions.size(),
                            internal::FillDescriptionArray, T>(descriptions);
    return ParallelExportImpl(yamlFile, fileComment, descriptions);
}

template<muc::tuple_like T>
auto DescriptionIO::ParallelIxport(const std::filesystem::path& yamlFile, const std::string& fileComment) -> std::pair<std::filesystem::path, std::filesystem::path> {
    std::array<DescriptionBase<>*, std::tuple_size_v<T>> descriptions;
    internal::StaticForEach<0, descriptions.size(),
                            internal::FillDescriptionArray, T>(descriptions);
    return ParallelIxportImpl(yamlFile, fileComment, descriptions);
}

template<typename... ArgsOfImport>
auto DescriptionIO::Import(const std::ranges::range auto& yamlText) -> void
    requires std::convertible_to<typename std::decay_t<decltype(yamlText)>::value_type, std::string>
{
    const auto yamlPath{CreateTemporaryFile("geom", ".yaml")};

    const auto yamlFile{std::fopen(yamlPath.generic_string().c_str(), "w")};
    if (yamlFile == nullptr) {
        throw std::runtime_error(PrettyException("Cannot open temp yaml file"));
    }
    for (auto&& line : yamlText) {
        fmt::println(yamlFile, "{}", line);
    }
    std::fclose(yamlFile);

    Import<ArgsOfImport...>(yamlPath);

    std::error_code muteRemoveError;
    std::filesystem::remove(yamlPath, muteRemoveError);
}

auto DescriptionIO::ImportImpl(const std::filesystem::path& yamlFile, std::ranges::input_range auto& descriptions) -> void {
    const auto geomYaml{YAML::LoadFile(yamlFile.generic_string())};
    for (auto&& description : std::as_const(descriptions)) {
        description->Import(geomYaml);
    }
}

auto DescriptionIO::ExportImpl(const std::filesystem::path& yamlFile, const std::string& fileComment, const std::ranges::input_range auto& descriptions) -> void {
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

    struct InvalidFile {};
    try {
        if (yamlFile.empty()) { throw InvalidFile{}; }

        std::ofstream yamlOut;
        try {
            const auto parent{yamlFile.parent_path()};
            if (not parent.empty()) { std::filesystem::create_directories(parent); }
            yamlOut.open(yamlFile, std::ios::out);
        } catch (const std::filesystem::filesystem_error&) { throw InvalidFile{}; }
        if (not yamlOut.is_open()) { throw InvalidFile{}; }

        YAML::Emitter yamlEmitter{yamlOut};
        yamlEmitter << YAML::Block;
        if (not fileComment.empty()) {
            yamlEmitter << YAML::Comment(fileComment)
                        << YAML::Newline;
        }
        yamlEmitter << geomYaml
                    << YAML::Newline;
    } catch (const InvalidFile&) {
        Env::PrintPrettyError("Cannot open yaml file, export failed");
    }
}

auto DescriptionIO::IxportImpl(const std::filesystem::path& yamlFile, const std::string& fileComment, const std::ranges::input_range auto& descriptions) -> std::pair<std::filesystem::path, std::filesystem::path> {
    auto path1{std::filesystem::path{yamlFile}.replace_extension(".prev.yaml")};
    auto path2{std::filesystem::path{yamlFile}.replace_extension(".curr.yaml")};
    ExportImpl(path1, fileComment, descriptions);
    ImportImpl(yamlFile, descriptions);
    ExportImpl(path2, fileComment, descriptions);
    return {std::move(path1), std::move(path2)};
}

auto DescriptionIO::ParallelExportImpl(const std::filesystem::path& yamlFile, const std::string& fileComment, const std::ranges::input_range auto& descriptions) -> std::filesystem::path {
    auto truePath{MPIX::ParallelizePath(yamlFile)};
    ExportImpl(truePath, fileComment, descriptions);
    return truePath;
}

auto DescriptionIO::ParallelIxportImpl(const std::filesystem::path& yamlFile, const std::string& fileComment, const std::ranges::input_range auto& descriptions) -> std::pair<std::filesystem::path, std::filesystem::path> {
    auto path1{ParallelExportImpl(std::filesystem::path{yamlFile}.replace_extension(".prev.yaml"), fileComment, descriptions)};
    ImportImpl(yamlFile, descriptions);
    auto path2{ParallelExportImpl(std::filesystem::path{yamlFile}.replace_extension(".curr.yaml"), fileComment, descriptions)};
    return {std::move(path1), std::move(path2)};
}

} // namespace Mustard::Detector::Description
