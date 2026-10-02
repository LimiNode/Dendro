#pragma once

#include <filesystem>

namespace dendro::provider {

struct Project {
    std::filesystem::path root;
};

} // namespace dendro::provider
