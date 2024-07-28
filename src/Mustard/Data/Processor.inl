namespace Mustard::Data {

template<muc::instantiated_from<MPIX::Executor> AExecutor>
Processor<AExecutor>::Processor(AExecutor executor, typename AExecutor::Index batchSize) :
    fExecutor{std::move(executor)},
    fBatchSize{batchSize} {
    fExecutor.ExecutionName("Event loop");
    fExecutor.TaskName("Batch");
}

template<muc::instantiated_from<MPIX::Executor> AExecutor>
template<muc::ceta_string AEventIDBranchName, TupleModelizable... Ts>
auto Processor<AExecutor>::Process(ROOTX::RDataFrame auto&& rdf,
                                   std::invocable<std::vector<std::shared_ptr<Tuple<Ts...>>>&> auto&& F) -> typename AExecutor::Index {
    const auto esp{RDFEventSplitPoint(rdf, AEventIDBranchName.sv())};
    const auto nEvent{static_cast<typename AExecutor::Index>(esp.size() - 1)};
    const auto nBatch{nEvent / fBatchSize + 1};

    typename AExecutor::Index nEventProcessed{};
    fExecutor.Execute(
        nBatch,
        [&](auto k) {
            const auto iFirst{k * fBatchSize};
            if (iFirst >= nEvent) { return; }
            const auto iLast{std::min(iFirst + fBatchSize, nEvent)};
            const auto data{Take<Ts...>::From(rdf.Range(esp[iFirst], esp[iLast]))};

            std::vector<std::shared_ptr<Tuple<Ts...>>> event;
            for (auto i{iFirst}; i < iLast; ++i) {
                const std::ranges::subrange eventData{data.cbegin() + (esp[i] - esp[iFirst]),
                                                      data.cbegin() + (esp[i + 1] - esp[iFirst])};
                event.clear();
                event.resize(eventData.size());
                std::ranges::copy(eventData, event.begin());
                F(event);
            }
            nEventProcessed += iLast - iFirst;
        });
    return nEventProcessed;
}

} // namespace Mustard::Data
