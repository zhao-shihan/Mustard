#pragma once

#include "Mustard/Env/MPIEnv.h++"
#include "Mustard/Env/Print.h++"
#include "Mustard/Extension/MPIX/DataType.h++"
#include "Mustard/Extension/ROOTX/RDataFrame.h++"

#include "mpi.h"

#include "fmt/format.h"

#include <cassert>
#include <concepts>
#include <cstdio>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

namespace Mustard::inline Utility {

template<std::integral T = int>
auto RDFEventSplitPoint(ROOTX::RDataFrame auto&& rdf, std::string_view eventIDBranchName = "EvtID") -> std::vector<unsigned> {
    std::vector<unsigned> eventSplitPoint;

    if (Env::MPIEnv::Instance().OnCommWorldMaster()) {
        unsigned index{};
        T lastEventID{-1};
        std::unordered_set<T> eventIDSet;
        rdf.Foreach(
            [&](T eventID) {
                assert(eventID >= 0);
                if (eventID != lastEventID) {
                    if (not eventIDSet.emplace(eventID).second) {
                        Env::PrintLnWarning("Warning: Disordered dataset (event {} has appeared before)", eventID);
                    }
                    lastEventID = eventID;
                    eventSplitPoint.emplace_back(index);
                }
                ++index;
            },
            {std::string{eventIDBranchName}});
        eventSplitPoint.emplace_back(index);
    }

    auto eventSplitPointSize{eventSplitPoint.size()};
    MPI_Bcast(&eventSplitPointSize,                // buffer
              1,                                   // count
              MPIX::DataType(eventSplitPointSize), // datatype
              0,                                   // root
              MPI_COMM_WORLD);                     // comm
    eventSplitPoint.resize(eventSplitPointSize);

    MPI_Bcast(eventSplitPoint.data(), // buffer
              eventSplitPoint.size(), // count
              MPI_UNSIGNED,           // datatype
              0,                      // root
              MPI_COMM_WORLD);        // comm

    return eventSplitPoint;
}

} // namespace Mustard::inline Utility
