#pragma once

#include <dendro/config.hpp>
#include <dendro/graph/graph.hpp>
#include <dendro/filesystem/graph_builder.hpp>
#include <dendro/filesystem/tree.hpp>

namespace dendro {

// Compatibility facade for the original 1.x API.
std::string generate_structure(const DendroConfig& config);

} // namespace dendro
