#include <dendro/query/query.hpp>

namespace dendro::query {

std::vector<graph::NodeId> nodes(const graph::Graph& graph, graph::NodeKind kind) {
    std::vector<graph::NodeId> result;
    for (const graph::Node& node : graph.nodes()) {
        if (node.kind == kind) {
            result.push_back(node.id);
        }
    }
    return result;
}

std::vector<graph::NodeId> children(const graph::Graph& graph, graph::NodeId node) {
    return graph.outgoing(node, graph::EdgeKind::Contains);
}

std::vector<graph::NodeId> parents(const graph::Graph& graph, graph::NodeId node) {
    return graph.incoming(node, graph::EdgeKind::Contains);
}

} // namespace dendro::query
