#pragma once

#include "Mustard/Data/Algorithm.h++"
#include "Mustard/Data/Tuple.h++"
#include "Mustard/Data/TupleModel.h++"
#include "Mustard/Data/internal/BranchHelper.h++"
#include "Mustard/Extension/gslx/index_sequence.h++"
#include "Mustard/Math/Statistic.h++"
#include "Mustard/Utility/NonMoveableBase.h++"

#include "TBranch.h"
#include "TChain.h"

#include "muc/ceta_string"
#include "muc/tuple"
#include "muc/utility"

#include "gsl/gsl"

#include "fmt/format.h"

#include <algorithm>
#include <array>
#include <compare>
#include <concepts>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace Mustard::Data {

template<TupleModelizable... Ts>
class Sheet {
public:
    using Model = TupleModel<Ts...>;

public:
    /// @brief Construct a sheet from a tree (or trees) of specific name in a file (or different files).
    /// @param name Tree name.
    /// @param file File name, wildcards are accepted (same as TChain::Add).
    explicit Sheet(const std::string& name, const std::string& file);
    /// @brief Construct a sheet from a tree (or trees) of specific name in a file (or different files).
    /// @param name Tree name.
    /// @param files List of file names, wildcards are accepted (same as TChain::Add).
    template<std::ranges::input_range R = std::initializer_list<std::string>>
        requires std::same_as<std::decay_t<std::ranges::range_value_t<R>>, std::string>
    explicit Sheet(const std::string& name, const R& files);

    /// @brief Get number of rows. This size is decoupled from the dataset size since rows (as proxy objects) can be dropped freely.
    /// @note This is not the size of the original dataset.
    /// @return Number of rows
    auto Size() const -> auto { return fEntry.size(); }
    /// @brief Get size of the full dataset. This size will change if the dataset extension is modified.
    /// @note This is not the number of rows.
    /// @return Size of the full dataset.
    auto DataSize() const -> auto { return fData->Size(); }

    auto Empty() const { return Size() == 0; }
    auto DataEmpty() const -> auto { return DataSize() == 0; }

    template<muc::ceta_string... ANames>
    auto DoWith(std::invocable auto&& F) const -> decltype(auto);
    template<muc::ceta_string... ANames>
    auto DoWithout(std::invocable auto&& F) const -> decltype(auto);

public:
    template<muc::ceta_string ANames>
    auto Statistic() const -> decltype(auto) {}

    auto begin() noexcept -> auto { return iterator{fEntry.begin(), this}; }
    auto begin() const noexcept -> auto { return const_iterator{fEntry.begin(), this}; }
    auto cbegin() const noexcept -> auto { return const_iterator{fEntry.cbegin(), this}; }
    auto end() noexcept -> auto { return iterator{fEntry.end(), this}; }
    auto end() const noexcept -> auto { return const_iterator{fEntry.end(), this}; }
    auto cend() const noexcept -> auto { return const_iterator{fEntry.cend(), this}; }
    auto size() const noexcept -> auto { return fEntry.size(); }
    auto empty() const noexcept -> auto { return fEntry.empty(); }

    static constexpr auto EntrySize() noexcept -> auto { return Entry::Size(); }

private:
    class Dataset final {
    public:
        template<std::ranges::input_range R>
            requires std::same_as<std::decay_t<std::ranges::range_value_t<R>>, std::string>
        Dataset(const std::string& name, const R& files);

        auto Size() const noexcept -> auto { return fSize; }

        template<muc::ceta_string AName>
        auto Status() const noexcept -> const auto& { return std::get<TupleModel<Ts...>::template Index<AName>()>(fStatus); }
        auto Status() const noexcept -> const auto& { return fStatus; }

        template<muc::ceta_string AName>
        auto Status(bool s) -> void;
        auto Status(const std::array<bool, EntrySize()>& s) -> void;
        auto Status(bool s) -> void;
        auto UpdateStatus() -> void;

        auto Fetch(gsl::index i) -> const auto&;

    private:
        TChain fChain;
        std::size_t fSize;
        std::array<bool, EntrySize()> fStatus;

        gsl::index fFetchedIndex;
        Tuple<Ts...> fFetchedEntry;

        internal::BranchHelper<Tuple<Ts...>> fBranchHelper;
    };

    class [[nodiscard]] Entry final : public EnableGet<Entry> {
    public:
        using Model = typename Sheet::Model;

    public:
        Entry(gsl::index index, Dataset& data);

        auto Index() const -> auto { return fIndex; }

        /// @brief Get specific value(s) of this entry.
        /// @exception If (any of) specified value(s) is/are disabled, this call will raise a std::logic_error.
        /// @tparam ...ANames Value name(s).
        /// @return The value or a data tuple of specified values.
        template<muc::ceta_string... ANames>
        auto Get() const -> auto;
        /// @brief Get the data tuple of this entry.
        /// @note If only one (or some of) value(s) are required, it is recommended to get specific value(s) instead of get the whole data tuple,
        /// for better performance and avoid unexpected exception (see the exception below).
        /// @exception If any value is disabled, this call will raise a std::logic_error.
        /// @return The data tuple.
        auto Get() const -> auto;

        auto operator*() const -> const Entry& { return *this; }
        auto operator*() -> Entry& { return *this; }
        auto operator->() const -> const Entry* { return this; }
        auto operator->() -> Entry* { return this; }

        auto operator==(const Entry& that) const -> auto { return fIndex == that.fIndex; }
        auto operator<=>(const Entry& that) const -> auto { return fIndex <=> that.fIndex; }

        static constexpr auto Size() -> auto { return Model::Size(); }

    private:
        gsl::index fIndex;
        Dataset* fData;
    };

    using EntryArray = std::vector<Entry>;

    template<typename I>
    class IteratorAdaptor final {
    public:
        using difference_type = typename std::iterator_traits<I>::difference_type;
        using value_type = typename std::iterator_traits<I>::value_type;
        using pointer = typename std::iterator_traits<I>::pointer;
        using reference = typename std::iterator_traits<I>::reference;
        using iterator_category = std::forward_iterator_tag;

    private:
        using S = std::conditional_t<std::same_as<I, typename EntryArray::iterator>, Sheet, const Sheet>;

    public:
        IteratorAdaptor() = default;
        IteratorAdaptor(I i, S* s);

        operator IteratorAdaptor<typename EntryArray::const_iterator>() const
            requires std::same_as<I, typename EntryArray::iterator>
        { return {fIterator, fSheet}; }

        auto operator*() const -> decltype(auto) { return *fIterator; }
        auto operator->() const -> decltype(auto) { return fIterator.operator->(); }

        auto operator++() -> auto& { return (++fIterator, *this); }
        auto operator++(int) -> IteratorAdaptor { return {fIterator++, fSheet}; }

        template<typename J>
        auto operator==(const IteratorAdaptor<J>& that) const -> auto { return fIterator == that.fIterator; }

        auto TheSheet() const -> S& { return *fSheet; }

    private:
        I fIterator;
        S* fSheet;
    };

    // template<muc::ceta_string... ANames>
    // class [[nodiscard]] Statistic : public Math::Statistic<(... +
    //                                                         ([] {
    //                                                             using ValueType = typename ValueOf<ANames>::Type;
    //                                                             if constexpr (std::is_arithmetic_v<ValueType>) {
    //                                                                 return 1;
    //                                                             } else if constexpr (internal::IsStdArray<ValueType>{}) {
    //                                                                 return std::tuple_size_v<ValueType>;
    //                                                             } else {
    //                                                                 static_assert(std::is_arithmetic_v<ValueType>, "Value of statistic is not a numeric type");
    //                                                             }
    //                                                         }()))> {};

public:
    using value_type = typename EntryArray::value_type;
    using size_type = typename EntryArray::size_type;
    using difference_type = typename EntryArray::difference_type;
    using reference = typename EntryArray::reference;
    using const_reference = typename EntryArray::const_reference;
    using pointer = typename EntryArray::pointer;
    using const_pointer = typename EntryArray::const_pointer;
    using iterator = IteratorAdaptor<typename EntryArray::iterator>;
    using const_iterator = IteratorAdaptor<typename EntryArray::const_iterator>;

private:
    EntryArray fEntry;
    std::shared_ptr<Dataset> fData;
};

} // namespace Mustard::Data

#include "Mustard/Data/Sheet.inl"
