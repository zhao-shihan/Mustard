namespace Mustard::Data {

template<TupleModelizable... Ts>
Output<Ts...>::Output(const std::string& name, const std::string& title) :
    NonMoveableBase{},
    fEntry{},
    fROOTPath{},
    fTree{},
    fBranchHelper{fEntry} {
    const auto treeNameIndex{name.find_last_of('/') + 1};
    fROOTPath = treeNameIndex != std::string::npos ?
                    gDirectory->mkdir(name.substr(0, treeNameIndex).c_str(), "", true)->GetPath() :
                    gDirectory->GetPath();
    fTree = std::make_unique<TTree>(treeNameIndex != std::string::npos ?
                                        name.substr(treeNameIndex, -1).c_str() :
                                        name.c_str(),
                                    title.c_str(), 99, nullptr);

    [this]<gsl::index... Is>(gslx::index_sequence<Is...>) {
        (...,
         [this]<gsl::index I>(std::integral_constant<gsl::index, I>) {
             using TheValue = std::tuple_element_t<I, Tuple<Ts...>>;
             const auto branch{fBranchHelper.template CreateBranch<TheValue::Name()>(*fTree)};
             branch->SetAutoDelete(false);
             if constexpr (TheValue::Description().has_value()) {
                 const auto title{fmt::format("({}) {}", branch->GetLeaf(TheValue::Name())->GetTypeName(), TheValue::Description().sv())};
                 branch->SetTitle(title.c_str());
             }
         }(std::integral_constant<gsl::index, Is>{}));
    }(gslx::make_index_sequence<Tuple<Ts...>::Size()>());
}

template<TupleModelizable... Ts>
template<typename T>
    requires std::assignable_from<Tuple<Ts...>&, T&&>
auto Output<Ts...>::Fill(T&& tuple) -> std::size_t {
    fEntry = std::forward<T>(tuple);
    return fTree->Fill();
}

template<TupleModelizable... Ts>
template<typename T>
    requires ProperSubTuple<Tuple<Ts...>, std::decay_t<T>>
auto Output<Ts...>::Fill(T&& tuple) -> std::size_t {
    fEntry = std::move(std::forward<T>(tuple).template As<Tuple<Ts...>>());
    return fTree->Fill();
}

template<TupleModelizable... Ts>
template<std::ranges::input_range R>
    requires std::assignable_from<Tuple<Ts...>&, std::ranges::range_reference_t<R>> or
             ProperSubTuple<Tuple<Ts...>, std::ranges::range_value_t<R>>
auto Output<Ts...>::Fill(R&& data) -> std::size_t {
    std::size_t nByte{};
    for (auto&& tuple : std::forward<R>(data)) {
        nByte += Fill(muc::forward_like<R>(tuple));
    }
    return nByte;
}

template<TupleModelizable... Ts>
template<std::ranges::input_range R>
    requires std::indirectly_readable<std::ranges::range_reference_t<R>> and
             (std::assignable_from<Tuple<Ts...>&, std::iter_reference_t<std::ranges::range_value_t<R>>> or
              ProperSubTuple<Tuple<Ts...>, std::iter_value_t<std::ranges::range_value_t<R>>>)
auto Output<Ts...>::Fill(R&& data) -> std::size_t {
    std::size_t nByte{};
    for (auto&& i : std::forward<R>(data)) {
        nByte += Fill(std::forward<decltype(*i)>(*i));
    }
    return nByte;
}

template<TupleModelizable... Ts>
auto Output<Ts...>::Write(int option, int bufferSize) const -> auto {
    const std::string cwd{gDirectory->GetPath()};
    gDirectory->cd(fROOTPath.c_str());
    const auto nByte{fTree->Write(nullptr, option, bufferSize)};
    gDirectory->cd(cwd.c_str());
    return nByte;
}

template<TupleModelizable... Ts>
Output<Ts...>::OutputIterator::OutputIterator(Output* output) :
    fOutput{output} {}

} // namespace Mustard::Data
