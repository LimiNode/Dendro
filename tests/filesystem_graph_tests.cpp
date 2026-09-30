#include <dendro/filesystem/graph_builder.hpp>
#include <dendro/filesystem/tree.hpp>

#include <cassert>
#include <filesystem>
#include <fstream>
#include <string>

namespace fs = std::filesystem;

int main() {
    const fs::path root = fs::temp_directory_path() / "dendro-filesystem-graph-tests";
    fs::remove_all(root);
    fs::create_directories(root / "src" / "nested");
    std::ofstream(root / "src" / "nested" / "main.cpp") << "int main() {}\n";
    std::ofstream(root / "src" / "nested" / "main.hpp") << "#pragma once\n";
    std::ofstream(root / "README.md") << "readme\n";

    dendro::DendroConfig config;
    config.allowed_extensions = {"cpp"};
    const dendro::graph::Graph graph = dendro::filesystem::build_filesystem_graph(root, config);

    assert(graph.roots().size() == 1);
    assert(graph.nodes().size() == 4);
    assert(graph.edges().size() == 3);

    const auto root_id = graph.roots().front();
    const auto root_children = graph.outgoing(root_id, dendro::graph::EdgeKind::Contains);
    assert(root_children.size() == 1);
    assert(graph.node(root_children.front()).name == "src");

    const std::string tree = dendro::filesystem::format_tree(graph, true);
    assert(tree.find("src/") != std::string::npos);
    assert(tree.find("main.cpp") != std::string::npos);
    assert(tree.find("main.hpp") == std::string::npos);
    assert(tree.find("README.md") == std::string::npos);

    config.exclude_paths = {root / "src" / "nested"};
    const dendro::graph::Graph excluded_graph =
        dendro::filesystem::build_filesystem_graph(root, config);
    assert(dendro::filesystem::format_tree(excluded_graph, true).find("main.cpp") ==
           std::string::npos);

    fs::remove_all(root);
    return 0;
}
