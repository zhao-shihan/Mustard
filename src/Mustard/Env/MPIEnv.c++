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

#include "Mustard/Env/MPIEnv.h++"
#include "Mustard/Env/Print.h++"

#include "TROOT.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <iomanip>
#include <iterator>
#include <map>
#include <memory>
#include <stdexcept>
#include <thread>
#include <utility>

namespace Mustard::Env {

MPIEnv::MPIEnv(NoBanner, int argc, char* argv[],
               std::optional<std::reference_wrapper<CLI::CLI<>>> cli,
               enum VerboseLevel verboseLevel,
               bool showBannerHint) :
    BasicEnv{{}, argc, argv, cli, verboseLevel, showBannerHint},
    PassiveSingleton<MPIEnv>{},
    fMPIThreadSupport{
        [&argc, &argv] {
            int mpiThreadSupport;
            MPI_Init_thread(&argc,               // argc
                            &argv,               // argv
                            MPI_THREAD_MULTIPLE, // required
                            &mpiThreadSupport);  // provided
            if (mpiThreadSupport < MPI_THREAD_FUNNELED) {
                throw std::runtime_error{"the MPI library thread support is less than MPI_THREAD_FUNNELED"};
            }
            return mpiThreadSupport;
        }()},
    fCommWorldRank{
        [] {
            int rank;
            MPI_Comm_rank(MPI_COMM_WORLD, // comm
                          &rank);         // rank
            return rank;
        }()},
    fCommWorldSize{
        [] {
            int size;
            MPI_Comm_size(MPI_COMM_WORLD, // comm
                          &size);         // size
            return size;
        }()},
    fCluster{
        [this] {
            // Member fCluster to be initialized
            std::remove_cv_t<decltype(fCluster)> cluster;
            // Each process get its processor name
            std::array<char, MPI_MAX_PROCESSOR_NAME> nodeNameSend;
            int nameLength;
            MPI_Get_processor_name(nodeNameSend.data(), // name
                                   &nameLength);        // resultlen
            // Master collects processor names
            std::vector<std::array<char, MPI_MAX_PROCESSOR_NAME>> nodeNamesRecv;
            if (OnCommWorldMaster()) { nodeNamesRecv.resize(fCommWorldSize); }
            MPI_Request gatherNodeNamesRequest;
            MPI_Igather(nodeNameSend.data(),      // sendbuf
                        MPI_MAX_PROCESSOR_NAME,   // sendcount
                        MPI_CHAR,                 // sendtype
                        nodeNamesRecv.data(),     // recvbuf
                        MPI_MAX_PROCESSOR_NAME,   // recvcount
                        MPI_CHAR,                 // recvtype
                        0,                        // root
                        MPI_COMM_WORLD,           // comm
                        &gatherNodeNamesRequest); // request
            // Processor name list
            std::vector<int> nodeIDSend;
            int nodeIDRecv;
            int nodeCount;
            struct NodeInfoForMPI {
                int size;
                std::array<char, MPI_MAX_PROCESSOR_NAME> name;
            };
            MPI_Datatype structNodeInfoForMPI;
            MPI_Type_create_struct(2,                                                      // count
                                   std::array<int, 2>{1,                                   // array_of_block_lengths
                                                      MPI_MAX_PROCESSOR_NAME}              // array_of_block_lengths
                                       .data(),                                            // array_of_block_lengths
                                   std::array<MPI_Aint, 2>{offsetof(NodeInfoForMPI, size), // array_of_displacements
                                                           offsetof(NodeInfoForMPI, name)} // array_of_displacements
                                       .data(),                                            // array_of_displacements
                                   std::array<MPI_Datatype, 2>{MPI_INT,                    // array_of_types
                                                               MPI_CHAR}                   // array_of_types
                                       .data(),                                            // array_of_types
                                   &structNodeInfoForMPI);                                 // newtype
            MPI_Type_commit(&structNodeInfoForMPI);
            MPI_Wait(&gatherNodeNamesRequest, // request
                     MPI_STATUS_IGNORE);      // status
            // Master find all unique processor names and assign node ID and count
            std::vector<NodeInfoForMPI> nodeList;
            if (OnCommWorldMaster()) {
                // key: node name, mapped: rank
                std::multimap<const std::array<char, MPI_MAX_PROCESSOR_NAME>&, int> nodeMap;
                for (int rank{};
                     auto&& name : std::as_const(nodeNamesRecv)) {
                    nodeMap.emplace(name, rank++);
                }

                std::vector<std::pair<std::vector<int>, const std::array<char, MPI_MAX_PROCESSOR_NAME>*>> rankNode;
                auto currentNodeName{&nodeMap.begin()->first};
                rankNode.push_back({{}, currentNodeName});
                for (auto&& [nodeName, rank] : std::as_const(nodeMap)) {
                    if (std::strncmp(nodeName.data(), currentNodeName->data(), MPI_MAX_PROCESSOR_NAME)) {
                        currentNodeName = &nodeName;
                        rankNode.push_back({{}, currentNodeName});
                    }
                    rankNode.back().first.emplace_back(rank);
                }
                std::ranges::sort(rankNode);

                nodeIDSend.resize(fCommWorldSize);
                for (int nodeID{};
                     auto&& [rankList, nodeName] : std::as_const(rankNode)) {
                    nodeList.push_back({static_cast<int>(rankList.size()), *nodeName});
                    for (auto&& rank : rankList) {
                        nodeIDSend.at(rank) = nodeID;
                    }
                    ++nodeID;
                }
                nodeCount = ssize(rankNode);
            }
            // Scatter node ID
            MPI_Request scatterNodeIDRequest;
            MPI_Iscatter(nodeIDSend.data(),      // sendbuf
                         1,                      // sendcount
                         MPI_INT,                // sendtype
                         &nodeIDRecv,            // recvbuf
                         1,                      // recvcount
                         MPI_INT,                // recvtype
                         0,                      // root
                         MPI_COMM_WORLD,         // comm
                         &scatterNodeIDRequest); // request
            // Broadcast node count
            MPI_Bcast(&nodeCount,      // buffer
                      1,               // count
                      MPI_INT,         // datatype
                      0,               // root
                      MPI_COMM_WORLD); // comm
            // Master send unique node name list
            if (OnCommWorldWorker()) { nodeList.resize(nodeCount); }
            MPI_Bcast(nodeList.data(),      // buffer
                      nodeList.size(),      // count
                      structNodeInfoForMPI, // datatype
                      0,                    // root
                      MPI_COMM_WORLD);      // comm
            MPI_Type_free(&structNodeInfoForMPI);
            // Assign to the list, convert node names to std::string
            cluster.node.reserve(nodeCount);
            for (auto&& [size, name] : std::as_const(nodeList)) {
                cluster.node.push_back({size, name.data()});
                cluster.node.back().name.shrink_to_fit();
            }
            cluster.node.shrink_to_fit();
            // Assign local processor ID
            MPI_Wait(&scatterNodeIDRequest, // request
                     MPI_STATUS_IGNORE);    // status
            cluster.local = nodeIDRecv;
            // Return the cluster info
            return cluster;
        }()},
    fCommNode{
        [this] {
            MPI_Comm comm;
            // Constructs shared communicator
            MPI_Comm_split(MPI_COMM_WORLD, // comm
                           fCluster.local, // color
                           0,              // key
                           &comm);         // newcomm
            return comm;
        }()},
    fCommNodeRank{
        [this] {
            int rank;
            // Initialize rank ID in the local communicator
            MPI_Comm_rank(fCommNode, // comm
                          &rank);    // rank
            return rank;
        }()},
    fCommNodeSize{
        [this] {
            int size;
            // Initialize size of the local communicator
            MPI_Comm_size(fCommNode, // comm
                          &size);    // size
            return size;
        }()} {
    // Disable ROOT implicit multi-threading
    if (ROOT::IsImplicitMTEnabled()) {
        ROOT::DisableImplicitMT();
    }
}

MPIEnv::MPIEnv(int argc, char* argv[],
               std::optional<std::reference_wrapper<CLI::CLI<>>> cli,
               enum VerboseLevel verboseLevel,
               bool showBannerHint) :
    MPIEnv{{}, argc, argv, cli, verboseLevel, showBannerHint} {
    if (fShowBanner and OnCommWorldMaster()) {
        PrintStartBannerSplitLine();
        PrintStartBannerBody(argc, argv);
        PrintStartBannerSplitLine();
    }
}

MPIEnv::~MPIEnv() {
    // Destructs the local communicator
    auto commNode{fCommNode};
    MPI_Comm_free(&commNode);
    // Wait all processes before finalizing
    MPI_Request barrier;
    MPI_Ibarrier(MPI_COMM_WORLD, &barrier);
    while (true) {
        int reached;
        MPI_Test(&barrier, &reached, MPI_STATUS_IGNORE);
        if (reached) { break; }
        using std::chrono_literals::operator""ms;
        std::this_thread::sleep_for(100ms);
    }
    // Finalize MPI
    MPI_Finalize();
    // Show exit banner
    if (fShowBanner and OnCommWorldMaster()) {
        PrintExitBanner();
    }
    fShowBanner = false;
}

auto MPIEnv::PrintStartBannerBody(int argc, char* argv[]) const -> void {
    BasicEnv::PrintStartBannerBody(argc, argv);
    // MPI library version
    char mpiLibVersion[MPI_MAX_LIBRARY_VERSION_STRING];
    int mpiLibVersionStringLength;
    MPI_Get_library_version(mpiLibVersion,               // version
                            &mpiLibVersionStringLength); // resultlen
    // MPI version at runtime
    std::pair<int, int> mpiRuntimeVersion;
    MPI_Get_version(&mpiRuntimeVersion.first,   // version
                    &mpiRuntimeVersion.second); // subversion
    // Messages
    Print(fmt::emphasis::bold,
          "\n"
          " Parallelized with MPI, running {}\n",
          Parallel() ? "in parallel" : "sequentially");
    PrintLn<'I'>(" Compiled with MPI {}.{}, running with MPI {}.{}", MPI_VERSION, MPI_SUBVERSION, mpiRuntimeVersion.first, mpiRuntimeVersion.second);
    Print<'V'>("--------------------> MPI library information (begin) <--------------------\n"
               "{}\n"
               "-------------------->  MPI library information (end)  <--------------------\n"
               "\n",
               mpiLibVersion);
    Print(fmt::emphasis::bold, " Size of the MPI world communicator: {}\n", fCommWorldSize);
    if (OnSingleNode()) {
        Print(fmt::emphasis::bold, " Running on '{}'\n", LocalNode().name);
    } else {
        Print(fmt::emphasis::bold, " Running on {} nodes:\n", ClusterSize());
        const auto maxNameWidth{std::ranges::max_element(NodeList(),
                                                         [](auto&& node1, auto&& node2) { return node1.name.size() < node2.name.size(); })
                                    ->name.size()};
        const auto format{fmt::format("  name: {{:{}}}  size: {{}}\n", maxNameWidth)};
        for (int nodeID{}; nodeID < ClusterSize(); ++nodeID) {
            const auto& node{Node(nodeID)};
            VPrint(stdout, fmt::emphasis::bold, format, fmt::make_format_args(node.name, node.size));
        }
    }
}

} // namespace Mustard::Env
