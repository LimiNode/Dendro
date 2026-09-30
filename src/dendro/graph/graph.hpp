#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace dendro::graph {

using NodeId = std::uint64_t;

enum class NodeKind {
    Directory,
    File,
};

enum class EdgeKind {
    Contains,
};

struct Node {
    NodeId id = 0;
    NodeKind kind = NodeKind::File;
    std::string name;
    std::filesystem::path path;
};

struct Edge {
    NodeId from = 0;
    NodeId to = 0;
    EdgeKind kind = EdgeKind::Contains;
};

class Graph {
public:
    NodeId add_node(Node node);
    void add_edge(Edge edge);
    void add_root(NodeId id);

    const Node& node(NodeId id) const;
    const std::vector<Node>& nodes() const noexcept;
    const std::vector<Edge>& edges() const noexcept;
    const std::vector<NodeId>& roots() const noexcept;
    std::vector<NodeId> outgoing(NodeId id, EdgeKind kind) const;
    std::vector<NodeId> incoming(NodeId id, EdgeKind kind) const;

private:
    std::vector<Node> nodes_;
    std::vector<Edge> edges_;
    std::vector<NodeId> roots_;
};

} // namespace dendro::graph
