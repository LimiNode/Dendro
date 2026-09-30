#pragma once

#include <filesystem>

#include <dendro/config.hpp>
#include <dendro/graph/graph.hpp>

namespace dendro::filesystem {

graph::Graph build_filesystem_graph(const std::filesystem::path& root,
                                    const DendroConfig& config = {});

} // namespace dendro::filesystem
