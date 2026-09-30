#include <dendro.hpp>
#include <dendro/filesystem/graph_builder.hpp>
#include <dendro/filesystem/tree.hpp>
#include <dendro/graph/graph.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs = std::filesystem;

#define CHECK(expression)                                                       \
    do {                                                                        \
        if (!(expression)) {                                                    \
            std::cerr << __FILE__ << ':' << __LINE__                              \
                      << ": CHECK failed: " #expression << '\n';              \
            ++failures;                                                         \
        }                                                                       \
    } while (false)

int main() {
    int failures = 0;
    const fs::path root = fs::temp_directory_path() / "dendro-filesystem-graph-tests";
    fs::remove_all(root);
    fs::create_directories(root / "src" / "nested");
    fs::create_directories(root / "src" / "foo");
    fs::create_directories(root / "tests" / "foo");
    std::ofstream(root / "src" / "nested" / "main.cpp") << "int main() {}\n";
    std::ofstream(root / "src" / "nested" / "main.hpp") << "#pragma once\n";
    std::ofstream(root / "src" / "foo" / "config.hpp") << "#pragma once\n";
    std::ofstream(root / "tests" / "foo" / "config.hpp") << "#pragma once\n";
    std::ofstream(root / "src" / "a.cpp") << "void a() {}\n";
    std::ofstream(root / "src" / "z.cpp") << "void z() {}\n";
    std::ofstream(root / "README.md") << "readme\n";

    dendro::DendroConfig config;
    config.allowed_extensions = {"cpp", "hpp"};
    config.excluded_extensions = {"hpp"};
    config.include_dirs = {"src", "src"};

    const dendro::graph::Graph graph = dendro::filesystem::build_filesystem_graph(root, config);
    CHECK(graph.roots().size() == 1);
    CHECK(graph.nodes().size() == 6);
    CHECK(graph.edges().size() == 5);

    const auto root_id = graph.roots().front();
    const auto root_children = graph.outgoing(root_id, dendro::graph::EdgeKind::Contains);
    CHECK(root_children.size() == 4);
    CHECK(graph.node(root_children[0]).name == "foo");
    CHECK(graph.node(root_children[1]).name == "nested");
    CHECK(graph.node(root_children[2]).name == "a.cpp");
    CHECK(graph.node(root_children[3]).name == "z.cpp");
    CHECK(graph.node(root_id).identity == "filesystem:src");

    const std::string tree = dendro::filesystem::format_tree(graph, true);
    CHECK(tree.find("src/") != std::string::npos);
    CHECK(tree.find("main.cpp") != std::string::npos);
    CHECK(tree.find("main.hpp") == std::string::npos);
    CHECK(tree.find("README.md") == std::string::npos);
    CHECK(tree.find("a.cpp") < tree.find("z.cpp"));

    config.exclude_paths = {root / "src" / "nested"};
    const dendro::graph::Graph excluded_graph =
        dendro::filesystem::build_filesystem_graph(root, config);
    CHECK(dendro::filesystem::format_tree(excluded_graph, true).find("nested/") ==
          std::string::npos);

    config.root_path = root;
    const std::string facade_tree = dendro::generate_structure(config);
    CHECK(facade_tree.find("a.cpp") != std::string::npos);

    dendro::DendroConfig identity_config;
    identity_config.allowed_extensions = {"hpp"};
    const dendro::graph::Graph identity_graph =
        dendro::filesystem::build_filesystem_graph(root, identity_config);
    std::vector<std::string> config_identities;
    for (const auto& node : identity_graph.nodes()) {
        if (node.name == "config.hpp") {
            config_identities.push_back(node.identity);
        }
    }
    CHECK(config_identities.size() == 2);
    CHECK(config_identities[0] == "filesystem:src/foo/config.hpp");
    CHECK(config_identities[1] == "filesystem:tests/foo/config.hpp");
    CHECK(config_identities[0] != config_identities[1]);

    const fs::path external_root = fs::temp_directory_path() / "dendro-external-identity-tests";
    fs::remove_all(external_root);
    fs::create_directories(external_root);
    std::ofstream(external_root / "external.cpp") << "void external() {}\n";
    dendro::DendroConfig external_config;
    external_config.include_dirs = {external_root};
    external_config.allowed_extensions = {"cpp"};
    const dendro::graph::Graph external_graph =
        dendro::filesystem::build_filesystem_graph(root, external_config);
    CHECK(external_graph.roots().size() == 1);
    CHECK(external_graph.node(external_graph.roots().front()).identity.rfind(
              "filesystem-absolute:", 0) == 0);

    dendro::graph::Graph graph_api;
    const auto node_id =
        graph_api.add_node({0, dendro::graph::NodeKind::File, "one", "manual:one"});
    graph_api.add_root(node_id);
    graph_api.add_root(node_id);
    graph_api.add_edge({node_id, node_id, dendro::graph::EdgeKind::Contains});
    graph_api.add_edge({node_id, node_id, dendro::graph::EdgeKind::Contains});
    CHECK(graph_api.roots().size() == 1);
    CHECK(graph_api.edges().size() == 1);
    CHECK(graph_api.node(node_id).identity == "manual:one");

    bool empty_identity_threw = false;
    try {
        graph_api.add_node({0, dendro::graph::NodeKind::File, "empty", ""});
    } catch (const std::invalid_argument&) {
        empty_identity_threw = true;
    }
    CHECK(empty_identity_threw);

    bool duplicate_identity_threw = false;
    try {
        graph_api.add_node({0, dendro::graph::NodeKind::File, "duplicate", "manual:one"});
    } catch (const std::invalid_argument&) {
        duplicate_identity_threw = true;
    }
    CHECK(duplicate_identity_threw);

    bool invalid_id_threw = false;
    try {
        (void)graph_api.node(999);
    } catch (const std::out_of_range&) {
        invalid_id_threw = true;
    }
    CHECK(invalid_id_threw);

    fs::remove_all(root);
    fs::remove_all(external_root);
    return failures == 0 ? 0 : 1;
}
