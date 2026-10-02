#pragma once

#include <filesystem>

namespace dendro::provider {

struct Project {
    std::filesystem::path root;
    std::filesystem::path compile_commands;
};

} // namespace dendro::provider
