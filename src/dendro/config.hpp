#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace dendro {

struct DendroConfig {
    std::vector<std::filesystem::path> include_dirs;
    std::vector<std::filesystem::path> exclude_paths;
    std::vector<std::string> allowed_extensions;
    std::vector<std::string> excluded_extensions;
    std::filesystem::path output_file = "structure.txt";
    std::filesystem::path root_path;
    bool copy_to_clipboard = false;
    bool show_root = false;
};

} // namespace dendro
