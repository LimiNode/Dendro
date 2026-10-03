#include <dendro.hpp>
#include <dendro/cpp/compilation_database.hpp>
#include <dendro/cpp/compilation_provider.hpp>
#include <dendro/filesystem/graph_builder.hpp>
#include <dendro/filesystem/filesystem_provider.hpp>
#include <dendro/filesystem/tree.hpp>
#include <dendro/graph/graph.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
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

    std::unique_ptr<dendro::provider::IGraphProvider> filesystem_provider =
        std::make_unique<dendro::filesystem::FilesystemProvider>(config);
    const dendro::provider::Project project{root};
    dendro::graph::Graph provider_graph;
    filesystem_provider->populate(project, provider_graph);
    CHECK(provider_graph.roots().size() == 1);
    CHECK(provider_graph.node(provider_graph.roots().front()).identity == "filesystem:src");
    const auto provider_node_count = provider_graph.nodes().size();
    const auto provider_edge_count = provider_graph.edges().size();
    filesystem_provider->populate(project, provider_graph);
    CHECK(provider_graph.nodes().size() == provider_node_count);
    CHECK(provider_graph.edges().size() == provider_edge_count);

    dendro::graph::Graph collision_graph;
    collision_graph.add_node(
        {0, dendro::graph::NodeKind::File, "src", "filesystem:src"});
    bool kind_collision_threw = false;
    try {
        filesystem_provider->populate(project, collision_graph);
    } catch (const std::invalid_argument&) {
        kind_collision_threw = true;
    }
    CHECK(kind_collision_threw);

    bool empty_project_root_threw = false;
    try {
        filesystem_provider->populate({}, provider_graph);
    } catch (const std::invalid_argument&) {
        empty_project_root_threw = true;
    }
    CHECK(empty_project_root_threw);

    const fs::path compilation_database_path = root / "compile_commands.json";
    {
        std::ofstream compilation_database(compilation_database_path);
        compilation_database << "[{\"directory\":\"" << root.generic_string()
                             << "\",\"file\":\"src/a.cpp\",\"command\":\"g++ -DMODE_A -c src/a.cpp\"},"
                             << "{\"directory\":\"" << root.generic_string()
                             << "\",\"file\":\"src/a.cpp\",\"arguments\":[\"g++\",\"-DMODE_B\",\"-c\",\"src/a.cpp\"]},"
                             << "{\"directory\":\"" << root.generic_string()
                             << "\",\"file\":\"src/a.cpp\",\"arguments\":[\"g++\",\"-DNAME=\\u0410\\uD83D\\uDE00\",\"-c\",\"src/a.cpp\"],\"output\":\"obj/a.o\"}]";
    }
    const auto parsed_database = dendro::cpp::CompilationDatabase::load(compilation_database_path);
    CHECK(parsed_database.commands().size() == 3);
    CHECK(parsed_database.commands()[1].arguments.has_value());
    CHECK(parsed_database.commands()[1].arguments->size() == 4);
    CHECK(parsed_database.commands()[2].output == std::optional<std::string>("obj/a.o"));
    CHECK(parsed_database.commands()[2].arguments->at(1) == "-DNAME=А😀");

    const fs::path invalid_database_path = root / "invalid-compile_commands.json";
    std::ofstream(invalid_database_path)
        << "[{\"directory\":\"" << root.generic_string()
        << "\",\"file\":\"src/a.cpp\"}]";
    bool missing_command_threw = false;
    try {
        (void)dendro::cpp::CompilationDatabase::load(invalid_database_path);
    } catch (const std::invalid_argument&) {
        missing_command_threw = true;
    }
    CHECK(missing_command_threw);

    dendro::cpp::CompilationProvider compilation_provider({compilation_database_path});
    const auto before_compilation_nodes = provider_graph.nodes().size();
    compilation_provider.populate(project, provider_graph);
    CHECK(provider_graph.nodes().size() == before_compilation_nodes + 3);
    std::size_t translation_unit_count = 0;
    std::vector<std::string> translation_unit_identities;
    for (const auto& node : provider_graph.nodes()) {
        if (node.kind != dendro::graph::NodeKind::TranslationUnit) continue;
        ++translation_unit_count;
        translation_unit_identities.push_back(node.identity);
        CHECK(node.source.has_value());
        CHECK(node.source->file_identity == "filesystem:src/a.cpp");
        const auto compiled_files =
            provider_graph.outgoing(node.id, dendro::graph::EdgeKind::Compiles);
        CHECK(compiled_files.size() == 1);
        CHECK(provider_graph.node(compiled_files.front()).identity == "filesystem:src/a.cpp");
    }
    CHECK(translation_unit_count == 3);
    CHECK(translation_unit_identities.size() == 3);
    CHECK(translation_unit_identities[0] != translation_unit_identities[1]);
    CHECK(translation_unit_identities[1] != translation_unit_identities[2]);
    CHECK(translation_unit_identities[0].size() >= 16);
    CHECK(dendro::cpp::detail::fnv1a_64("hello") == "a430d84680aabd0b");
    compilation_provider.populate(project, provider_graph);
    CHECK(provider_graph.nodes().size() == before_compilation_nodes + 3);

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

    dendro::graph::Node symbol_node;
    symbol_node.kind = dendro::graph::NodeKind::Function;
    symbol_node.name = "foo";
    symbol_node.identity = "cpp:usr:c:@F@foo";
    symbol_node.occurrences.push_back(
        {{"filesystem:src/a.cpp", 1, 1, 1, 12},
         dendro::graph::SourceOccurrenceKind::Declaration});
    symbol_node.occurrences.push_back(
        {{"filesystem:src/a.cpp", 3, 1, 3, 12},
         dendro::graph::SourceOccurrenceKind::Definition});
    CHECK(symbol_node.occurrences.size() == 2);
    CHECK(symbol_node.occurrences[0].kind ==
          dendro::graph::SourceOccurrenceKind::Declaration);
    CHECK(symbol_node.occurrences[1].kind ==
          dendro::graph::SourceOccurrenceKind::Definition);

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
