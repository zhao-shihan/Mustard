namespace Mustard::inline Extension::MPIX::inline Execution {

template<std::integral T>
auto StaticScheduler<T>::PreLoopAction() -> void {
    const auto& mpiEnv{Env::MPIEnv::Instance()};
    this->fExecutingTask = this->fTask.first + (mpiEnv.CommWorldSize() - 1 - mpiEnv.CommWorldRank());
    if (this->fExecutingTask > this->fTask.last) [[unlikely]] {
        this->fExecutingTask = this->fTask.last;
    }
}

template<std::integral T>
auto StaticScheduler<T>::PostTaskAction() -> void {
    this->fExecutingTask += Env::MPIEnv::Instance().CommWorldSize();
    if (this->fExecutingTask > this->fTask.last) [[unlikely]] {
        this->fExecutingTask = this->fTask.last;
    }
}

template<std::integral T>
auto StaticScheduler<T>::NExecutedTask() const -> std::pair<bool, T> {
    return {this->fNLocalExecutedTask > 10,
            this->fExecutingTask - this->fTask.first};
}

} // namespace Mustard::inline Extension::MPIX::inline Execution
