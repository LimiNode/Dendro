#include <dendro/filesystem/tree.hpp>

#include <sstream>

#include <dendro/filesystem/graph_builder.hpp>

namespace dendro::filesystem {
namespace {

using graph::EdgeKind;
using graph::Graph;
using graph::NodeId;
using graph::NodeKind;

void append_children(const Graph& graph,
                     NodeId parent,
                     std::ostringstream& output,
                     const std::string& prefix) {
    const std::vector<NodeId> children = graph.outgoing(parent, EdgeKind::Contains);
    for (std::size_t index = 0; index < children.size(); ++index) {
        const bool last = index + 1 == children.size();
        const NodeId child = children[index];
        const auto& node = graph.node(child);
        output << prefix << (last ? "└── " : "├── ") << node.name
               << (node.kind == NodeKind::Directory ? "/" : "") << "\n";
        if (node.kind == NodeKind::Directory) {
            append_children(graph, child, output, prefix + (last ? "    " : "│   "));
        }
    }
}

} // namespace

std::string format_tree(const graph::Graph& graph, bool show_roots) {
    std::ostringstream output;
    for (const NodeId root : graph.roots()) {
        const auto& node = graph.node(root);
        if (show_roots) {
            output << node.name << (node.kind == NodeKind::Directory ? "/" : "") << "\n";
        }
        append_children(graph, root, output, "");
    }
    return output.str();
}

std::string generate_structure(const DendroConfig& config) {
    const auto base = config.root_path.empty() ? std::filesystem::current_path() : config.root_path;
    const graph::Graph graph = build_filesystem_graph(base, config);
    return format_tree(graph, config.show_root || !config.include_dirs.empty());
}

} // namespace dendro::filesystem

namespace dendro {

std::string generate_structure(const DendroConfig& config) {
    return filesystem::generate_structure(config);
}

} // namespace dendro
