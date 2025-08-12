namespace Mustard::inline IO {

template<std::constructible_from<std::string> AException>
[[noreturn]] auto Throw(std::string_view message, const std::source_location& location) -> void {
    throw AException(internal::PrettyException(message, location));
}

} // namespace Mustard::inline IO
