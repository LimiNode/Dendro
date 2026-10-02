#include <dendro/filesystem/filesystem_provider.hpp>

#include <stdexcept>
#include <utility>

namespace dendro::filesystem {

FilesystemProvider::FilesystemProvider(DendroConfig config) : config_(std::move(config)) {}

void FilesystemProvider::populate(const provider::Project& project, graph::Graph& graph) const {
    if (project.root.empty()) {
        throw std::invalid_argument("provider project root must not be empty");
    }
    populate_filesystem_graph(project.root, config_, graph);
}

} // namespace dendro::filesystem
