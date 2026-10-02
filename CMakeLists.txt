cmake_minimum_required(VERSION 3.20)

project(Dendro VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(DENDRO_BUILD_TESTS "Build Dendro tests" ON)

add_library(dendro_graph STATIC
    src/dendro/graph/graph.cpp
)
add_library(dendro::graph ALIAS dendro_graph)

target_include_directories(dendro_graph
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(dendro_graph PUBLIC cxx_std_17)

add_library(dendro_query STATIC
    src/dendro/query/query.cpp
)
add_library(dendro::query ALIAS dendro_query)

target_include_directories(dendro_query
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(dendro_query PUBLIC dendro::graph)
target_compile_features(dendro_query PUBLIC cxx_std_17)

add_library(dendro_filesystem STATIC
    src/dendro/filesystem/graph_builder.cpp
    src/dendro/filesystem/filesystem_provider.cpp
    src/dendro/filesystem/tree.cpp
)
add_library(dendro::filesystem ALIAS dendro_filesystem)

target_include_directories(dendro_filesystem
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(dendro_filesystem PUBLIC dendro::graph)
target_compile_features(dendro_filesystem PUBLIC cxx_std_17)

add_executable(dendro
    src/main.cpp
    src/dendro/cli/arguments.cpp
    src/dendro/cli/clipboard.cpp
)
target_link_libraries(dendro PRIVATE dendro::filesystem)
target_compile_features(dendro PRIVATE cxx_std_17)
if(WIN32)
    target_link_libraries(dendro PRIVATE User32)
endif()

if(DENDRO_BUILD_TESTS)
    enable_testing()
    add_executable(dendro_filesystem_graph_tests tests/filesystem_graph_tests.cpp)
    target_link_libraries(dendro_filesystem_graph_tests PRIVATE dendro::filesystem)
    target_compile_features(dendro_filesystem_graph_tests PRIVATE cxx_std_17)
    add_test(NAME dendro.filesystem_graph COMMAND dendro_filesystem_graph_tests)

    add_executable(dendro_query_tests tests/query_tests.cpp)
    target_link_libraries(dendro_query_tests PRIVATE dendro::query)
    target_compile_features(dendro_query_tests PRIVATE cxx_std_17)
    add_test(NAME dendro.query COMMAND dendro_query_tests)
endif()
