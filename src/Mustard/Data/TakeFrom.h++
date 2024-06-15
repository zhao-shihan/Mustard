#pragma once

#include "Mustard/Data/Tuple.h++"
#include "Mustard/Data/internal/TypeTraits.h++"
#include "Mustard/Extension/ROOTX/RDataFrame.h++"
#include "Mustard/Extension/gslx/index_sequence.h++"
#include "Mustard/Utility/NonConstructibleBase.h++"

#include "ROOT/RVec.hxx"

#include "muc/concepts"

#include "gsl/gsl"

#include <concepts>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

namespace Mustard::Data {

template<TupleModelizable... Ts>
class Take : public NonConstructibleBase {
public:
    static auto From(ROOTX::RDataFrame auto&& dataframe) -> std::vector<std::shared_ptr<Tuple<Ts...>>>;

private:
    template<gsl::index... Is>
    class TakeOne;

    template<gsl::index... Is>
    TakeOne(std::vector<std::shared_ptr<Tuple<Ts...>>>, gslx::index_sequence<Is...>) -> TakeOne<Is...>;
};

} // namespace Mustard::Data

#include "Mustard/Data/TakeFrom.inl"
