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

namespace Mustard::Data {

namespace internal {

template<std::integral T>
auto MakeFlatRDFEventSplitPoint(ROOT::RDF::RNode rdf, std::string eventIDBranchName) -> std::pair<std::vector<T>, std::vector<unsigned>> {
    std::vector<T> eventIDList;
    std::vector<unsigned> eventSplitPoint;

    unsigned index{};
    std::unordered_set<T> eventIDSet;
    rdf.Foreach(
        [&](T eventID) {
            if (eventIDList.empty() or eventID != eventIDList.back()) {
                if (not eventIDSet.emplace(eventID).second) {
                    PrintWarning(fmt::format("Disordered dataset (event {} has appeared before)", eventID));
                }
                eventIDList.emplace_back(eventID);
                eventSplitPoint.emplace_back(index);
            }
            ++index;
        },
        {std::move(eventIDBranchName)});
    eventSplitPoint.emplace_back(index);

    return {eventIDList, eventSplitPoint};
}

} // namespace internal

template<std::integral T>
auto RDFEventSplitPoint(ROOT::RDF::RNode rdf, std::string_view eventIDBranchName) -> std::vector<unsigned> {
    auto eventSplitPoint{Env::MPIEnv::Instance().OnCommWorldMaster() ?
                             internal::MakeFlatRDFEventSplitPoint<T>(std::move(rdf), std::string{eventIDBranchName}).second :
                             std::vector<unsigned>{}};

    auto eventSplitPointSize{static_cast<unsigned>(eventSplitPoint.size())};
    MPI_Bcast(&eventSplitPointSize, 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
    eventSplitPoint.resize(eventSplitPointSize);
    MPI_Bcast(eventSplitPoint.data(), eventSplitPointSize, MPI_UNSIGNED, 0, MPI_COMM_WORLD);

    return eventSplitPoint;
}

template<std::integral T>
auto RDFEventSplitPoint(ROOT::RDF::RNode masterRDF,
                        std::vector<ROOT::RDF::RNode> slaveRDF,
                        std::string_view masterEventIDBranchName,
                        std::vector<std::string> slaveEventIDBranchName) -> MasterSlaveRDFEventSplitPoint<T> {
    if (slaveRDF.empty()) {
        Throw<std::invalid_argument>("Empty slave RDF");
    }

    if (slaveEventIDBranchName.empty()) {
        slaveEventIDBranchName.resize(slaveRDF.size());
        std::ranges::fill(slaveEventIDBranchName, masterEventIDBranchName);
    } else if (slaveRDF.size() != slaveEventIDBranchName.size()) {
        Throw<std::invalid_argument>("Inconsistent size between slave RDF and slave RDF event ID branch name");
    }

    // Parallel build all RDF event split point (1 (master RDF) + n (slave RDF))

    const auto& mpiEnv{Env::MPIEnv::Instance()};
    const auto worldSize{mpiEnv.CommWorldSize()};

    std::vector<std::pair<std::vector<T>, std::vector<unsigned>>> flatEventSplitPoint(1 + slaveRDF.size());
    for (gsl::index i{}; i < ssize(flatEventSplitPoint); ++i) {
        if (mpiEnv.CommWorldRank() == i % worldSize) {
            flatEventSplitPoint[i] =
                i == 0 ?
                    internal::MakeFlatRDFEventSplitPoint<T>(std::move(masterRDF), std::string{masterEventIDBranchName}) :
                    internal::MakeFlatRDFEventSplitPoint<T>(std::move(slaveRDF[i - 1]), std::move(slaveEventIDBranchName[i - 1]));
        }
    }

    // Broadcast to all processes

    for (gsl::index i{}; i < ssize(flatEventSplitPoint); ++i) {
        auto& [eventID, esp]{flatEventSplitPoint[i]};
        unsigned eventSize{eventID.size()};
        MPI_Bcast(&eventSize, 1, MPI_UNSIGNED, i % worldSize, MPI_COMM_WORLD);
        eventID.resize(eventSize);
        esp.resize(eventSize + 1);
        MPI_Bcast(&eventID, eventSize, MPIX::DataType<T>(), i % worldSize, MPI_COMM_WORLD);
        MPI_Bcast(&esp, eventSize + 1, MPI_UNSIGNED, i % worldSize, MPI_COMM_WORLD);
    }

    // Make result

    MasterSlaveRDFEventSplitPoint<T> result;

    const auto& [masterEventID, masterESP]{flatEventSplitPoint.front()};
    result.master.reserve(masterEventID.size());
    for (unsigned i{}; i < masterEventID.size(); ++i) {
        result.master.emplace_back(masterEventID[i], masterESP[i]);
    }
    result.master.emplace_back(std::numeric_limits<T>::max(), masterESP.back());

    for (gsl::index iSlave{}; iSlave < ssize(slaveRDF); ++iSlave) {
        const auto& [slaveEventID, slaveESP]{flatEventSplitPoint[iSlave + 1]};
        result.slave[iSlave].reserve(slaveEventID.size());
        for (unsigned i{}; i < slaveEventID.size(); ++i) {
            result.slave[iSlave].try_emplace(slaveEventID[i], slaveESP[i], slaveESP[i + 1]);
        }
    }

    return result;
}

} // namespace Mustard::Data
