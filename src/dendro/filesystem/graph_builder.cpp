#include <dendro/filesystem/graph_builder.hpp>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <map>
#include <stdexcept>
#include <system_error>
#include <utility>
#include <vector>

namespace dendro::filesystem {
namespace {

namespace fs = std::filesystem;

fs::path canonical_path(const fs::path& path) {
    std::error_code ec;
    const fs::path absolute = fs::absolute(path, ec);
    const fs::path result = fs::weakly_canonical(absolute, ec);
    return ec ? absolute : result;
}

std::string lower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}

std::string extension_key(std::string extension) {
    if (!extension.empty() && extension.front() != '.') {
        extension.insert(extension.begin(), '.');
    }
    return lower(std::move(extension));
}

bool is_excluded(const fs::path& path, const DendroConfig& config) {
    const fs::path normalized = canonical_path(path);
    return std::any_of(config.exclude_paths.begin(), config.exclude_paths.end(),
                       [&normalized](const fs::path& candidate) {
                           return canonical_path(candidate) == normalized;
                       });
}

bool extension_allowed(const fs::path& path, const DendroConfig& config) {
    if (!fs::is_regular_file(path)) {
        return true;
    }

    const std::string extension = extension_key(path.extension().string());
    const auto contains = [&extension](const std::vector<std::string>& values) {
        return std::any_of(values.begin(), values.end(), [&extension](const std::string& value) {
            return extension_key(value) == extension;
        });
    };

    if (!config.excluded_extensions.empty() && contains(config.excluded_extensions)) {
        return false;
    }
    return config.allowed_extensions.empty() || contains(config.allowed_extensions);
}

struct Entry {
    fs::path path;
    bool directory = false;
};

std::string identity_for_path_impl(const std::filesystem::path& path,
                                   const std::filesystem::path& snapshot_root) {
    const fs::path normalized = canonical_path(path);
    const fs::path normalized_root = canonical_path(snapshot_root);
    const fs::path relative = normalized.lexically_relative(normalized_root);
    if (relative == ".") {
        return "filesystem:.";
    }

    bool outside_snapshot = relative.empty();
    for (const fs::path& component : relative) {
        if (component == "..") {
            outside_snapshot = true;
            break;
        }
    }
    if (outside_snapshot) {
        return "filesystem-absolute:" + normalized.generic_string();
    }
    return "filesystem:" + relative.generic_string();
}

std::vector<Entry> children(const fs::path& directory, const DendroConfig& config) {
    std::vector<Entry> result;
    std::error_code ec;
    for (const fs::directory_entry& entry :
         fs::directory_iterator(directory, fs::directory_options::skip_permission_denied, ec)) {
        const fs::path path = entry.path();
        if (is_excluded(path, config)) {
            continue;
        }

        std::error_code type_ec;
        if (entry.is_symlink(type_ec) || type_ec) {
            continue;
        }
        const bool directory_entry = entry.is_directory(type_ec);
        if (type_ec || (!directory_entry && !extension_allowed(path, config))) {
            continue;
        }
        result.push_back({path, directory_entry});
    }

    std::sort(result.begin(), result.end(), [](const Entry& left, const Entry& right) {
        if (left.directory != right.directory) {
            return left.directory > right.directory;
        }
        return left.path.filename().generic_string() < right.path.filename().generic_string();
    });
    return result;
}

class Builder {
public:
    Builder(const DendroConfig& config, graph::Graph& graph, fs::path snapshot_root)
        : config_(config), graph_(graph), snapshot_root_(std::move(snapshot_root)) {}

    void add_root(const fs::path& path) {
        const fs::path normalized = canonical_path(path);
        std::error_code ec;
        if (!fs::is_directory(normalized, ec) || is_excluded(normalized, config_)) {
            return;
        }
        const graph::NodeId id = add_directory(normalized);
        graph_.add_root(id);
    }

private:
    graph::NodeId ensure_node(const fs::path& normalized,
                              graph::NodeKind kind,
                              std::string name) {
        const std::string identity = identity_for_path_impl(normalized, snapshot_root_);
        const auto existing = graph_.find_by_identity(identity);
        if (existing.has_value()) {
            if (graph_.node(*existing).kind != kind) {
                throw std::invalid_argument("filesystem identity kind collision: " + identity);
            }
            return *existing;
        }

        graph::Node node;
        node.kind = kind;
        node.name = std::move(name);
        node.identity = identity;
        return graph_.add_node(std::move(node));
    }

    graph::NodeId add_directory(const fs::path& path) {
        const fs::path normalized = canonical_path(path);
        const auto existing = ids_.find(normalized);
        if (existing != ids_.end()) {
            return existing->second;
        }

        std::string name = normalized.filename().generic_string();
        if (name.empty()) {
            name = normalized.generic_string();
        }
        const graph::NodeId id = ensure_node(normalized, graph::NodeKind::Directory, std::move(name));
        ids_.emplace(normalized, id);

        for (const Entry& entry : children(normalized, config_)) {
            const graph::NodeId child = entry.directory ? add_directory(entry.path) : add_file(entry.path);
            graph_.add_edge({id, child, graph::EdgeKind::Contains});
        }
        return id;
    }

    graph::NodeId add_file(const fs::path& path) {
        const fs::path normalized = canonical_path(path);
        const auto existing = ids_.find(normalized);
        if (existing != ids_.end()) {
            return existing->second;
        }

        const graph::NodeId id = ensure_node(
            normalized, graph::NodeKind::File, normalized.filename().generic_string());
        ids_.emplace(normalized, id);
        return id;
    }

    const DendroConfig& config_;
    graph::Graph& graph_;
    fs::path snapshot_root_;
    std::map<fs::path, graph::NodeId> ids_;
};

} // namespace

std::string identity_for_path(const std::filesystem::path& path,
                              const std::filesystem::path& snapshot_root) {
    return identity_for_path_impl(path, snapshot_root);
}

graph::Graph build_filesystem_graph(const std::filesystem::path& root,
                                    const DendroConfig& config) {
    graph::Graph graph;
    populate_filesystem_graph(root, config, graph);
    return graph;
}

void populate_filesystem_graph(const std::filesystem::path& root,
                               const DendroConfig& config,
                               graph::Graph& graph) {
    const fs::path base = canonical_path(root);
    Builder builder(config, graph, base);

    if (config.include_dirs.empty()) {
        builder.add_root(base);
    } else {
        for (const fs::path& include : config.include_dirs) {
            builder.add_root(include.is_absolute() ? include : base / include);
        }
    }
}

} // namespace dendro::filesystem
