#pragma once

#include <vector>

#include <dendro/graph/graph.hpp>

namespace dendro::query {

std::vector<graph::NodeId> nodes(const graph::Graph& graph, graph::NodeKind kind);
std::vector<graph::NodeId> children(const graph::Graph& graph, graph::NodeId node);
std::vector<graph::NodeId> parents(const graph::Graph& graph, graph::NodeId node);

} // namespace dendro::query
