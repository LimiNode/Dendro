#include <dendro/filesystem/filesystem_provider.hpp>

#include <utility>

namespace dendro::filesystem {

FilesystemProvider::FilesystemProvider(DendroConfig config) : config_(std::move(config)) {}

graph::Graph FilesystemProvider::build(const provider::Project& project) const {
    return build_filesystem_graph(project.root, config_);
}

} // namespace dendro::filesystem
