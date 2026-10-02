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
    std::filesystem::path compile_commands;
};

class IGraphProvider {
public:
    virtual ~IGraphProvider() = default;
    virtual graph::Graph build(const Project& project) const = 0;
};

}
```

`FilesystemProvider` is the first implementation and serves as the reference
adapter. A future `ClangProvider` will add namespaces, types, functions,
definitions, references, and calls using the same graph and query layers.

Tree-sitter may be added later as a syntax-only fallback, but it is not the
primary C++ semantic source: it cannot reliably resolve overloads, templates,
macros, or cross-translation-unit references. Regex-based parsing is not a
provider option.

The initial Clang milestone is intentionally limited to:

- symbols with source locations;
- declarations and definitions;
- include edges;
- basic references and calls where Clang resolves the target.

CFG/DFG, persistence, slicing, and MCP remain outside this provider milestone.
