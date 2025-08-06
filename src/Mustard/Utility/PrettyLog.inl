namespace Mustard::inline Utility {

template<std::constructible_from<std::string> AException>
[[noreturn]] auto Throw(std::string_view message, const std::source_location& location) -> void {
    throw AException(PrettyException(message, location));
}

} // namespace Mustard::inline Utility
