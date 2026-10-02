#pragma once

#include <filesystem>

#include <dendro/provider/graph_provider.hpp>

namespace dendro::cpp {

struct CompilationProviderConfig {
    std::filesystem::path compilation_database;
};

class CompilationProvider final : public provider::IGraphProvider {
public:
    explicit CompilationProvider(CompilationProviderConfig config);

    void populate(const provider::Project& project, graph::Graph& graph) const override;

private:
    CompilationProviderConfig config_;
};

} // namespace dendro::cpp
