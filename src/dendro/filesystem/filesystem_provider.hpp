#pragma once

#include <dendro/filesystem/graph_builder.hpp>
#include <dendro/provider/graph_provider.hpp>

namespace dendro::filesystem {

class FilesystemProvider final : public provider::IGraphProvider {
public:
    explicit FilesystemProvider(DendroConfig config = {});

    void populate(const provider::Project& project, graph::Graph& graph) const override;

private:
    DendroConfig config_;
};

} // namespace dendro::filesystem
