namespace Mustard::inline Extension::MPIX::inline Execution {

template<std::integral T>
    requires(Concept::MPIPredefined<T> and sizeof(T) >= sizeof(short))
auto Scheduler<T>::Reset() -> void {
    fExecutingTask = fTask.first;
    fNLocalExecutedTask = 0;
}

} // namespace Mustard::inline Extension::MPIX::inline Execution
