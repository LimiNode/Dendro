# Clang symbol provider: first semantic slice

The first semantic provider is intentionally narrower than a complete C++
indexer. It will consume the existing `CompilationProvider` translation-unit
nodes and materialize declarations and definitions discovered by Clang.

## Boundary

```text
compile action
      |
      v
TranslationUnit --Compiles--> filesystem File
      |
      v
Clang AST adapter --> symbol occurrences --> graph nodes and edges
```

The adapter must use Clang's compilation-database loader and the exact
arguments of each compile action. It must not reconstruct a shell command from
`file` alone. The current self-contained JSON reader remains a preparation
fallback; production Clang integration belongs behind an optional Clang/LLVM
build dependency.

The provider reloads the compilation database and resolves each existing
translation unit with the shared
`compilation::translation_unit_identity()` helper.
That helper receives the canonical working directory, the filesystem identity,
and the complete command/arguments/output tuple. This is the explicit binding
between a graph node and the exact compile action; no provider is allowed to
reimplement the digest algorithm.

## Identity and provenance

For declarations where Clang provides a USR, the provider identity is:

```text
cpp:usr:<clang-usr>
```

USR is the semantic identity. It must not be replaced by a display name or a
source path: overloads, namespaces, templates, and declarations in different
translation units can share names while representing different entities.

Source location is provenance, not identity. A source occurrence records the
filesystem file identity and concrete begin/end coordinates. A symbol may have
multiple occurrences (for example a declaration and a definition), so the
implementation must not silently overwrite an earlier occurrence. Graph nodes
now retain a primary `SourceLocation` for compatibility plus an
`occurrences` collection of `{location, kind}` records, where `kind` is
`Declaration` or `Definition`.
The first inserted occurrence initializes the primary `SourceLocation` when it
has not been set already; the full `occurrences` collection is authoritative
for semantic nodes.

Occurrence identity is the tuple `(kind, file_identity, begin_line,
begin_column, end_line, end_column)`. `Graph::add_occurrence()` uses this tuple
to suppress exact duplicates, making repeated provider population idempotent.

## Initial graph vocabulary

The first implementation uses the existing kinds without further splitting:

```text
Namespace  Type  Function  Variable
```

For this slice the edge semantics are explicit:

```text
TranslationUnit --Declares--> Symbol   (declaration occurrence)
TranslationUnit --Defines--> Symbol    (definition occurrence)
```

The symbol node is identified by USR and stores all known occurrences. A
`File` is the provenance referenced by each occurrence's `file_identity`; it
is not the source/target of `Declares` or `Defines`. `References`, `Calls`,
control-flow, data-flow, persistence, and MCP remain separate milestones.

The provider must validate that every referenced `TranslationUnit` belongs to
the same project snapshot and must reuse existing filesystem nodes by identity.
Provider population remains fail-fast and non-transactional, as documented for
the provider layer generally.

## Toolchain availability

This repository currently builds without LLVM/Clang installed. Therefore the
Clang adapter is not enabled by default and no regex or ad-hoc parser is used
as a substitute. Once an LLVM/Clang dependency is selected, CMake detection,
Debug/Release coverage, and a small fixture with declarations, definitions,
overloads, and a macro-controlled compile action will be added together.
