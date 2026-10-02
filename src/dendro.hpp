#pragma once

#include <dendro/config.hpp>
#include <dendro/graph/graph.hpp>
#include <dendro/filesystem/graph_builder.hpp>
#include <dendro/filesystem/filesystem_provider.hpp>
#include <dendro/filesystem/tree.hpp>
#include <dendro/provider/graph_provider.hpp>
#include <dendro/provider/project.hpp>
#include <dendro/query/query.hpp>

namespace dendro {

// Compatibility facade for the original 1.x API.
std::string generate_structure(const DendroConfig& config);

} // namespace dendro
