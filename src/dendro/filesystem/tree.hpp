#pragma once

#include <string>

#include <dendro/config.hpp>
#include <dendro/graph/graph.hpp>

namespace dendro::filesystem {

/// Format a graph's filesystem projection as a deterministic UTF-8 tree.
std::string format_tree(const graph::Graph& graph, bool show_roots = false);

/// Build the filesystem graph and format it as a tree.
std::string generate_structure(const DendroConfig& config);

} // namespace dendro::filesystem
