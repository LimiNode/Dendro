#pragma once

#include <filesystem>
#include <string>
#include <string_view>

#include <dendro/provider/graph_provider.hpp>

namespace dendro::cpp {

namespace detail {

/// Deterministic FNV-1a 64-bit digest used for provider identities.
std::string fnv1a_64(std::string_view value);

} // namespace detail

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
