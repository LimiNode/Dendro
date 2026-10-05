#include <dendro/cpp/compilation_provider.hpp>

#include <dendro/cpp/compilation_database.hpp>
#include <dendro/filesystem/graph_builder.hpp>

#include <stdexcept>
#include <utility>

namespace dendro::cpp {
namespace {

namespace fs = std::filesystem;

fs::path canonical_path(const fs::path& path) {
    std::error_code error;
    const fs::path absolute = fs::absolute(path, error);
    const fs::path canonical = fs::weakly_canonical(absolute, error);
    return error ? absolute : canonical;
}

} // namespace

CompilationProvider::CompilationProvider(CompilationProviderConfig config)
    : config_(std::move(config)) {}

void CompilationProvider::populate(const provider::Project& project, graph::Graph& graph) const {
    if (project.root.empty()) {
        throw std::invalid_argument("provider project root must not be empty");
    }
    if (config_.compilation_database.empty()) {
        throw std::invalid_argument("compilation database path must not be empty");
    }

    const fs::path database_path = canonical_path(config_.compilation_database);
    const CompilationDatabase database = CompilationDatabase::load(database_path);
    const fs::path database_root = database_path.parent_path();
    const fs::path project_root = canonical_path(project.root);
    for (const CompilationCommand& command : database.commands()) {
        const fs::path raw_directory = command.directory.is_absolute()
            ? command.directory
            : database_root / command.directory;
        const fs::path directory = canonical_path(raw_directory);
        const fs::path source = canonical_path(command.file.is_absolute() ? command.file
                                                                        : directory / command.file);
        const std::string file_identity = filesystem::identity_for_path(source, project_root);
        const auto file_id = graph.find_by_identity(file_identity);
        if (!file_id.has_value()) {
            throw std::invalid_argument("compilation unit file is missing from graph: " +
                                        file_identity);
        }
        if (graph.node(*file_id).kind != graph::NodeKind::File) {
            throw std::invalid_argument("compilation unit identity is not a file: " +
                                        file_identity);
        }

        const std::string translation_unit_identity =
            compilation::translation_unit_identity(command, directory, file_identity);
        auto translation_unit_id = graph.find_by_identity(translation_unit_identity);
        if (!translation_unit_id.has_value()) {
            graph::Node node;
            node.kind = graph::NodeKind::TranslationUnit;
            node.name = source.filename().generic_string() + " [translation unit]";
            node.identity = translation_unit_identity;
            node.source = graph::SourceLocation{file_identity};
            translation_unit_id = graph.add_node(std::move(node));
        } else if (graph.node(*translation_unit_id).kind != graph::NodeKind::TranslationUnit) {
            throw std::invalid_argument("translation unit identity kind collision: " +
                                        translation_unit_identity);
        }
        graph.add_edge({*translation_unit_id, *file_id, graph::EdgeKind::Compiles});
    }
}

} // namespace dendro::cpp
