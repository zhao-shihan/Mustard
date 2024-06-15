namespace Mustard::Data {

template<TupleModelizable... Ts>
Sheet<Ts...>::Sheet(const std::string& name, const std::string& file) :
    Sheet{name, std::array{file}} {}

template<TupleModelizable... Ts>
template<std::ranges::input_range R>
    requires std::same_as<std::decay_t<std::ranges::range_value_t<R>>, std::string>
Sheet<Ts...>::Sheet(const std::string& name, const R& files) :
    fEntry{},
    fData{std::make_shared<Dataset>(name, files)} {
    fEntry.reserve(fData->Size());
    for (gsl::index i{0}; i < muc::to_signed(fData->Size()); ++i) {
        fEntry.emplace_back(i, *fData);
    }
    fEntry.shrink_to_fit();
}

template<TupleModelizable... Ts>
template<muc::ceta_string... ANames>
auto Sheet<Ts...>::DoWith(std::invocable auto&& F) const -> decltype(auto) {
    const auto originalStatus = fData->Status();
    fData->Status(false);
    (..., fData->template Status<ANames>(true));
    fData->UpdateStatus();
    decltype(auto) ret{F()};
    fData->Status(originalStatus);
    fData->UpdateStatus();
    return ret;
}

template<TupleModelizable... Ts>
template<muc::ceta_string... ANames>
auto Sheet<Ts...>::DoWithout(std::invocable auto&& F) const -> decltype(auto) {
    const auto originalStatus = fData->Status();
    fData->Status(true);
    (..., fData->template Status<ANames>(false));
    fData->UpdateStatus();
    decltype(auto) ret{F()};
    fData->Status(originalStatus);
    fData->UpdateStatus();
    return ret;
}

template<TupleModelizable... Ts>
template<std::ranges::input_range R>
    requires std::same_as<std::decay_t<std::ranges::range_value_t<R>>, std::string>
Sheet<Ts...>::Dataset::Dataset(const std::string& name, const R& files) :
    fChain{name.c_str(), "", TChain::kWithoutGlobalRegistration},
    fSize{},
    fStatus{},
    fFetchedIndex{-1},
    fFetchedEntry{},
    fBranchHelper{fFetchedEntry} {
    fChain.ResetBit(TObject::kMustCleanup);
    for (auto&& file : files) {
        fChain.Add(file.c_str());
    }
    fSize = fChain.GetEntries();
    // 1: check tree type
    [this]<gsl::index... Is>(gslx::index_sequence<Is...>) {
        (...,
         [this]<gsl::index I>(std::integral_constant<gsl::index, I>) {
             using TheValue = std::tuple_element_t<I, Tuple<Ts...>>;
             constexpr auto name{TheValue::Name()};
             const auto [ec, branch]{fBranchHelper.template ConnectBranch<name>(fChain)};
             switch (ec) {
             case TChain::kMissingBranch:
                 throw std::runtime_error{fmt::format("Mustard::Data::Sheet<Ts...>::Dataset: "
                                                      "error occured when setting address for branch \"{}\" of tree \"{}\" "
                                                      "(TChain::SetBranchAddress returned kMissingBranch: Missing branch)",
                                                      name.StringView(), fChain.GetName())};
             case TChain::kInternalError:
                 throw std::runtime_error{fmt::format("Mustard::Data::Sheet<Ts...>::Dataset: "
                                                      "error occured when setting address for branch \"{}\" of tree \"{}\" "
                                                      "(TChain::SetBranchAddress returned kInternalError: Internal error (could not find the type corresponding to a data type number))",
                                                      name.StringView(), fChain.GetName())};
             case TChain::kMissingCompiledCollectionProxy:
                 throw std::runtime_error{fmt::format("Mustard::Data::Sheet<Ts...>::Dataset: "
                                                      "error occured when setting address for branch \"{}\" of tree \"{}\" "
                                                      "(TChain::SetBranchAddress returned kMissingCompiledCollectionProxy: Missing compiled collection proxy for a compiled collection)",
                                                      name.StringView(), fChain.GetName())};
             case TChain::kMismatch:
                 throw std::runtime_error{fmt::format("Mustard::Data::Sheet<Ts...>::Dataset: "
                                                      "error occured when setting address for branch \"{}\" of tree \"{}\" "
                                                      "(TChain::SetBranchAddress returned kMismatch: Non-Class Pointer type given does not match the type expected by the branch",
                                                      name.StringView(), fChain.GetName())};
             case TChain::kClassMismatch:
                 throw std::runtime_error{fmt::format("Mustard::Data::Sheet<Ts...>::Dataset: "
                                                      "error occured when setting address for branch \"{}\" of tree \"{}\" "
                                                      "(TChain::SetBranchAddress returned kClassMismatch: Class Pointer type given does not match the type expected by the branch",
                                                      name.StringView(), fChain.GetName())};
             }
             branch->SetAutoDelete(false);
         }(std::integral_constant<gsl::index, Is>{}));
    }(gslx::make_index_sequence<EntrySize()>());
    // 2: set chain branch address by updating status
    Status(true);
    UpdateStatus();
    // 3. enable cache
    fChain.EnableCache();
    fChain.AddBranchToCache("*", true);
}

template<TupleModelizable... Ts>
template<muc::ceta_string AName>
auto Sheet<Ts...>::Dataset::Status(bool s) -> void {
    fChain.SetBranchStatus(AName, s);
    std::get<TupleModel<Ts...>::template Index<AName>()>(fStatus) = s;
}

template<TupleModelizable... Ts>
auto Sheet<Ts...>::Dataset::Status(const std::array<bool, EntrySize()>& s) -> void {
    [&]<gsl::index... Is>(gslx::index_sequence<Is...>) {
        (..., Status<std::tuple_element_t<Is, Tuple<Ts...>>::Name()>(std::get<Is>(s)));
    }(gslx::make_index_sequence<EntrySize()>{});
}

template<TupleModelizable... Ts>
auto Sheet<Ts...>::Dataset::Status(bool s) -> void {
    fChain.SetBranchStatus("*", s);
    fStatus.fill(s);
}

template<TupleModelizable... Ts>
auto Sheet<Ts...>::Dataset::Fetch(gsl::index i) -> const auto& {
    if (i == fFetchedIndex) {
        return fFetchedEntry;
    }
    if (fChain.GetEntry(i) == 0) {
        throw std::out_of_range{fmt::format("Mustard::Data::Sheet::Dataset::Fetch: i={} is out of range", i)};
    }
    fFetchedIndex = i;
    return fFetchedEntry;
}

template<TupleModelizable... Ts>
auto Sheet<Ts...>::Dataset::UpdateStatus() -> void {
    [this]<gsl::index... Is>(gslx::index_sequence<Is...>) {
        (...,
         [this]<gsl::index I>(std::integral_constant<gsl::index, I>) {
             constexpr auto name{std::tuple_element_t<Is, Tuple<Ts...>>::Name()};
             fBranchHelper.template ConnectBranchNoCheck<name>(fChain)->SetAutoDelete(false);
         }(std::integral_constant<gsl::index, Is>{}));
    }(gslx::make_index_sequence<EntrySize()>());
}

template<TupleModelizable... Ts>
Sheet<Ts...>::Entry::Entry(gsl::index index, Dataset& data) :
    EnableGet<Entry>{},
    fIndex{index},
    fData{&data} {}

template<TupleModelizable... Ts>
template<muc::ceta_string... ANames>
auto Sheet<Ts...>::Entry::Get() const -> auto {
    if ((... or (fData->template Status<ANames>() == false))) {
        throw std::logic_error{"Mustard::Data::Sheet<Ts...>::Dataset: "
                               "trying to access disabled field"};
    }
    return fData->Fetch(fIndex).template Get<ANames...>();
}

template<TupleModelizable... Ts>
auto Sheet<Ts...>::Entry::Get() const -> auto {
    if (std::ranges::any_of(fData->Status(), [](auto s) { return s == false; })) {
        throw std::logic_error{"Mustard::Data::Sheet<Ts...>::Dataset: "
                               "trying to access disabled field"};
    }
    return fData->Fetch(fIndex);
}

template<TupleModelizable... Ts>
template<typename I>
Sheet<Ts...>::IteratorAdaptor<I>::IteratorAdaptor(I i, S* s) :
    fIterator{std::move(i)},
    fSheet{s} {}

} // namespace Mustard::Data
