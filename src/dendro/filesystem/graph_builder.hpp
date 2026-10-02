#pragma once

#include <filesystem>
#include <string>

#include <dendro/config.hpp>
#include <dendro/graph/graph.hpp>

namespace dendro::filesystem {

std::string identity_for_path(const std::filesystem::path& path,
                              const std::filesystem::path& snapshot_root);

graph::Graph build_filesystem_graph(const std::filesystem::path& root,
                                    const DendroConfig& config = {});

void populate_filesystem_graph(const std::filesystem::path& root,
                               const DendroConfig& config,
                               graph::Graph& graph);

} // namespace dendro::filesystem
