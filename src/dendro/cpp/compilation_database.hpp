#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace dendro::cpp {

struct CompilationCommand {
    std::filesystem::path directory;
    std::filesystem::path file;
    std::string command;
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
