# C++ semantic provider

The next provider will use Clang/Clang Tooling as the semantic source for C++.
The first version will require a `compile_commands.json` compilation database;
without it, include paths, defines, language mode, and generated headers cannot
be interpreted reliably.

The provider boundary is deliberately small:

```cpp
namespace dendro::provider {

struct Project {
    std::filesystem::path root;
};

class IGraphProvider {
public:
    virtual ~IGraphProvider() = default;
    virtual void populate(const Project& project, graph::Graph& graph) const = 0;
};

}
```

`FilesystemProvider` is the first implementation and serves as the reference
adapter. A future `ClangProvider` will add namespaces, types, functions,
definitions, references, and calls using the same graph and query layers.

Tree-sitter may be added later as a syntax-only fallback, but it is not the
primary C++ semantic source: it does not provide Clang-equivalent
preprocessing, name lookup, overload/template resolution, or
cross-translation-unit semantic resolution. Regex-based parsing is not a
provider option.

Providers are composable and populate one shared graph. Their contract is:

1. A provider owns identities in its own namespace.
2. A provider may reference nodes created by another provider.
3. Existing node identities are reused, not duplicated.
4. A provider never silently replaces an existing node.
5. Provider failures are fail-fast for the current operation.
6. Provider execution order is explicit.

The filesystem provider is therefore able to create `filesystem:src/foo.cpp`
first, while a future Clang provider can reuse that file node and add
`cpp:function:...` nodes and semantic edges to the same graph.

`Project.root` is required for provider operations. Providers reject an empty
root with `std::invalid_argument`; interpreting an empty path as the current
working directory remains a convenience of the legacy filesystem helper, not
of the provider boundary.

The initial Clang milestone is intentionally limited to:

- symbols with source locations;
- declarations and definitions;
- include edges;
- basic references and calls where Clang resolves the target.

CFG/DFG, persistence, slicing, and MCP remain outside this provider milestone.
