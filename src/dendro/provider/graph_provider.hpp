#pragma once

#include <dendro/graph/graph.hpp>
#include <dendro/provider/project.hpp>

namespace dendro::provider {

/// A source-specific graph contributor. Providers own parsing and source
/// metadata; all providers contribute to one provider-independent graph.
class IGraphProvider {
public:
    virtual ~IGraphProvider() = default;
    virtual void populate(const Project& project, graph::Graph& graph) const = 0;
};

} // namespace dendro::provider
