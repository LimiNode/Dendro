#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace dendro::cpp {

struct CompilationCommand {
    std::filesystem::path directory;
    std::filesystem::path file;
    std::optional<std::string> command;
    std::optional<std::vector<std::string>> arguments;
    std::optional<std::string> output;
};

class CompilationDatabase {
public:
    static CompilationDatabase load(const std::filesystem::path& path);

    const std::vector<CompilationCommand>& commands() const noexcept;

private:
    explicit CompilationDatabase(std::vector<CompilationCommand> commands);
    std::vector<CompilationCommand> commands_;
};

} // namespace dendro::cpp
