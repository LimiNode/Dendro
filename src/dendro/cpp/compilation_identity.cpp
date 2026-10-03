#include <dendro/cpp/compilation_identity.hpp>

#include <cstdint>
#include <iomanip>
#include <sstream>

namespace dendro::cpp::detail {

std::string fnv1a_64(std::string_view value) {
    std::uint64_t digest = 14695981039346656037ULL;
    for (const unsigned char byte : value) {
        digest ^= byte;
        digest *= 1099511628211ULL;
    }
    std::ostringstream result;
    result << std::hex << std::setw(16) << std::setfill('0') << digest;
    return result.str();
}

std::string translation_unit_identity(const CompilationCommand& command,
                                      const std::filesystem::path& normalized_directory,
                                      std::string_view file_identity) {
    std::string serialized = normalized_directory.generic_string();
    serialized.push_back('\0');
    serialized += file_identity;
    serialized.push_back('\0');
    if (command.arguments.has_value()) {
        serialized += "arguments";
        serialized.push_back('\0');
        for (const std::string& argument : *command.arguments) {
            serialized += argument;
            serialized.push_back('\0');
        }
    }
    if (command.command.has_value()) {
        serialized += "command";
        serialized.push_back('\0');
        serialized += *command.command;
        serialized.push_back('\0');
    }
    if (command.output.has_value()) {
        serialized += "output";
        serialized.push_back('\0');
        serialized += *command.output;
    }
    return "cpp:translation-unit:" + std::string(file_identity) + ":" + fnv1a_64(serialized);
}

} // namespace dendro::cpp::detail
