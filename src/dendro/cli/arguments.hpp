#pragma once

#include <string>

#include <dendro/config.hpp>

namespace dendro::cli {

struct Arguments {
    DendroConfig config;
    bool help = false;
};

Arguments parse_arguments(int argc, char* argv[]);
std::string usage();

} // namespace dendro::cli
