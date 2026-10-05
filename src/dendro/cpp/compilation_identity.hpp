#pragma once

#include <filesystem>
#include <string>
#include <string_view>

#include <dendro/cpp/compilation_database.hpp>

namespace dendro::cpp {

namespace compilation {

/// Deterministic FNV-1a 64-bit digest used for provider identities.
std::string fnv1a_64(std::string_view value);

/// Builds the identity shared by all providers for one compile action.
std::string translation_unit_identity(const CompilationCommand& command,
                                      const std::filesystem::path& normalized_directory,
                                      std::string_view file_identity);

} // namespace compilation

} // namespace dendro::cpp
