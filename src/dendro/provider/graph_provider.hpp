#pragma once

#include <dendro/graph/graph.hpp>
#include <dendro/provider/project.hpp>

namespace dendro::provider {

/// A source-specific graph builder. Providers own parsing and source metadata;
/// the returned graph remains provider-independent.
class IGraphProvider {
public:
    virtual ~IGraphProvider() = default;
    virtual graph::Graph build(const Project& project) const = 0;
};

} // namespace dendro::provider
