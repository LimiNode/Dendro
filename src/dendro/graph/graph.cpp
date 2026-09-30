#include <dendro/graph/graph.hpp>

#include <stdexcept>
#include <utility>
#include <algorithm>

namespace dendro::graph {

NodeId Graph::add_node(Node node) {
    if (node.identity.empty()) {
        throw std::invalid_argument("graph node identity must not be empty");
    }
    const auto duplicate = std::find_if(nodes_.begin(), nodes_.end(), [&node](const Node& existing) {
        return existing.identity == node.identity;
    });
    if (duplicate != nodes_.end()) {
        throw std::invalid_argument("graph node identity must be unique: " + node.identity);
    }

    const NodeId id = static_cast<NodeId>(nodes_.size() + 1);
    node.id = id;
    nodes_.push_back(std::move(node));
    return id;
}

void Graph::add_edge(Edge edge) {
    (void)node(edge.from);
    (void)node(edge.to);
    const auto duplicate = std::find_if(edges_.begin(), edges_.end(), [&edge](const Edge& existing) {
        return existing.from == edge.from && existing.to == edge.to && existing.kind == edge.kind;
    });
    if (duplicate != edges_.end()) {
        return;
    }
    edges_.push_back(edge);
}

void Graph::add_root(NodeId id) {
    (void)node(id);
    if (std::find(roots_.begin(), roots_.end(), id) != roots_.end()) {
        return;
    }
    roots_.push_back(id);
}

const Node& Graph::node(NodeId id) const {
    if (id == 0 || id > nodes_.size()) {
        throw std::out_of_range("graph node id is out of range");
    }
    return nodes_[static_cast<std::size_t>(id - 1)];
}

const std::vector<Node>& Graph::nodes() const noexcept {
    return nodes_;
}

const std::vector<Edge>& Graph::edges() const noexcept {
    return edges_;
}

const std::vector<NodeId>& Graph::roots() const noexcept {
    return roots_;
}

std::vector<NodeId> Graph::outgoing(NodeId id, EdgeKind kind) const {
    (void)node(id);
    std::vector<NodeId> result;
    for (const Edge& edge : edges_) {
        if (edge.from == id && edge.kind == kind) {
            result.push_back(edge.to);
        }
    }
    return result;
}

std::vector<NodeId> Graph::incoming(NodeId id, EdgeKind kind) const {
    (void)node(id);
    std::vector<NodeId> result;
    for (const Edge& edge : edges_) {
        if (edge.to == id && edge.kind == kind) {
            result.push_back(edge.from);
        }
    }
    return result;
}

} // namespace dendro::graph
