#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace dendro::graph {

using NodeId = std::uint64_t;

enum class NodeKind {
    Directory,
    File,
    TranslationUnit,
    Namespace,
    Type,
    Function,
    Variable,
};

enum class EdgeKind {
    Contains,
    Compiles,
    Includes,
    Declares,
    Defines,
    References,
    Calls,
};

struct SourceLocation {
    std::string file_identity;
    std::uint32_t begin_line = 0;
    std::uint32_t begin_column = 0;
    std::uint32_t end_line = 0;
    std::uint32_t end_column = 0;
};

struct Node {
    NodeId id = 0;
    NodeKind kind = NodeKind::File;
    /// Human-readable label for presentation.
    std::string name;
    /// Provider-defined stable identity within the indexed snapshot.
    std::string identity;
    std::optional<SourceLocation> source;
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
    std::optional<NodeId> find_by_identity(std::string_view identity) const;
    std::vector<NodeId> outgoing(NodeId id, EdgeKind kind) const;
    std::vector<NodeId> incoming(NodeId id, EdgeKind kind) const;

private:
    std::vector<Node> nodes_;
    std::vector<Edge> edges_;
    std::vector<NodeId> roots_;
};

} // namespace dendro::graph
