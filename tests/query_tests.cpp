#include <dendro/query/query.hpp>

#include <iostream>
#include <string>

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
    dendro::graph::Graph graph;
    const auto root = graph.add_node(
        {0, dendro::graph::NodeKind::Directory, "repo", "filesystem:."});
    const auto src = graph.add_node(
        {0, dendro::graph::NodeKind::Directory, "src", "filesystem:src"});
    const auto main_file = graph.add_node(
        {0, dendro::graph::NodeKind::File, "main.cpp", "filesystem:src/main.cpp"});
    const auto readme = graph.add_node(
        {0, dendro::graph::NodeKind::File, "README.md", "filesystem:README.md"});
    graph.add_root(root);
    graph.add_edge({root, src, dendro::graph::EdgeKind::Contains});
    graph.add_edge({root, readme, dendro::graph::EdgeKind::Contains});
    graph.add_edge({src, main_file, dendro::graph::EdgeKind::Contains});

    const auto found = graph.find_by_identity("filesystem:src/main.cpp");
    CHECK(found.has_value());
    CHECK(found.value_or(0) == main_file);
    CHECK(!graph.find_by_identity("filesystem:missing.cpp").has_value());

    const auto directories = dendro::query::nodes(graph, dendro::graph::NodeKind::Directory);
    const auto files = dendro::query::nodes(graph, dendro::graph::NodeKind::File);
    CHECK(directories.size() == 2);
    CHECK(files.size() == 2);

    const auto root_children = dendro::query::children(graph, root);
    CHECK(root_children.size() == 2);
    CHECK(root_children[0] == src);
    CHECK(root_children[1] == readme);

    const auto main_parents = dendro::query::parents(graph, main_file);
    CHECK(main_parents.size() == 1);
    CHECK(main_parents.front() == src);

    return failures == 0 ? 0 : 1;
}
