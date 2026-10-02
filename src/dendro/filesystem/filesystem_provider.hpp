#pragma once

#include <dendro/filesystem/graph_builder.hpp>
#include <dendro/provider/graph_provider.hpp>

namespace dendro::filesystem {

class FilesystemProvider final : public provider::IGraphProvider {
public:
    explicit FilesystemProvider(DendroConfig config = {});

    graph::Graph build(const provider::Project& project) const override;

private:
    DendroConfig config_;
};

} // namespace dendro::filesystem
