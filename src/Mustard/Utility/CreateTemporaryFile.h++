#pragma once

#include <filesystem>
#include <string_view>

namespace Mustard::inline Utility {

/// @brief Create an empty temporary file and close it. The file is guaranteed to be created if this function return, so its file name can be safely reused.
///        In other words, the intention of this function is to occupy a available temporary file name (p.s. std::tmpnam is dangerous).
/// @param signature An optional file name signature.
/// @param extension The extension of the file name. No extension if not provided.
/// @return A std::filesystem::path refering to the created file.
/// @exception Throw a std::runtime_error if failed.
auto CreateTemporaryFile(std::string_view signature = {}, std::filesystem::path extension = {}) -> std::filesystem::path;

} // namespace Mustard::inline Utility
