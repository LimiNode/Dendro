# Dendro

Dendro is a C++17 command-line tool for compact project context. The 2.x
architecture starts with a deterministic filesystem tree and is designed to
grow into a codebase graph and context engine for coding agents.

## Build

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

The current build has no mandatory third-party dependencies. The old Consolix
integration was removed from the core so the project can be built and tested
from a clean checkout.

## Usage

```sh
build/dendro --root-path . --show-root --output structure.txt
```

Available options:

```text
-i, --include <paths...>       Include directories
-e, --exclude <paths...>       Exclude files or directories
-o, --output <file>            Write the tree to a file
    --allow-ext <ext...>       Include only these file extensions
    --exclude-ext <ext...>     Exclude these file extensions
    --root-path <path>         Set the scan root
    --show-root                Include the root directory name
-c, --clipboard                Copy output to the clipboard
-h, --help                     Show help
```

The filesystem tree is now represented as a typed graph:

```text
filesystem provider → dendro::graph::Graph → tree formatter
```

The graph currently contains only `Directory`, `File`, and `Contains`. Each
node also has a provider-defined `identity`, separate from its display name;
the filesystem provider uses `filesystem:` plus a path relative to the indexed
snapshot. An explicitly included root outside the snapshot uses the
`filesystem-absolute:` fallback. Future indexers and query domains will extend
this model without changing the tree projection.

The initial query API is available through `dendro::query`:

```cpp
dendro::query::nodes(graph, dendro::graph::NodeKind::File);
dendro::query::children(graph, directory_id);
dendro::query::parents(graph, file_id);
graph.find_by_identity("filesystem:src/main.cpp");
```
