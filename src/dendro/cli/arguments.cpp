#include <dendro/cli/arguments.hpp>

#include <stdexcept>

namespace dendro::cli {
namespace {

bool is_option(const std::string& value) {
    return value.size() >= 2 && value[0] == '-';
}

std::string require_value(int& index, int argc, char* argv[], const std::string& option) {
    if (index + 1 >= argc || is_option(argv[index + 1])) {
        throw std::runtime_error("Missing value for " + option);
    }
    return argv[++index];
}

void append_values(int& index, int argc, char* argv[], std::vector<std::filesystem::path>& values,
                   const std::string& option) {
    if (index + 1 >= argc || is_option(argv[index + 1])) {
        throw std::runtime_error("Missing value for " + option);
    }
    while (index + 1 < argc && !is_option(argv[index + 1])) {
        values.emplace_back(argv[++index]);
    }
}

void append_values(int& index, int argc, char* argv[], std::vector<std::string>& values,
                   const std::string& option) {
    if (index + 1 >= argc || is_option(argv[index + 1])) {
        throw std::runtime_error("Missing value for " + option);
    }
    while (index + 1 < argc && !is_option(argv[index + 1])) {
        values.emplace_back(argv[++index]);
    }
}

} // namespace

Arguments parse_arguments(int argc, char* argv[]) {
    Arguments result;
    for (int index = 1; index < argc; ++index) {
        const std::string option = argv[index];
        if (option == "-h" || option == "--help") {
            result.help = true;
        } else if (option == "-i" || option == "--include") {
            append_values(index, argc, argv, result.config.include_dirs, option);
        } else if (option == "-e" || option == "--exclude") {
            append_values(index, argc, argv, result.config.exclude_paths, option);
        } else if (option == "-o" || option == "--output") {
            result.config.output_file = require_value(index, argc, argv, option);
        } else if (option == "--allow-ext") {
            append_values(index, argc, argv, result.config.allowed_extensions, option);
        } else if (option == "--exclude-ext") {
            append_values(index, argc, argv, result.config.excluded_extensions, option);
        } else if (option == "--root-path") {
            result.config.root_path = require_value(index, argc, argv, option);
        } else if (option == "--show-root") {
            result.config.show_root = true;
        } else if (option == "-c" || option == "--clipboard") {
            result.config.copy_to_clipboard = true;
        } else {
            throw std::runtime_error("Unknown option: " + option);
        }
    }
    return result;
}

std::string usage() {
    return "Usage: dendro [options]\n"
           "  -i, --include <paths...>       Include directories\n"
           "  -e, --exclude <paths...>       Exclude files or directories\n"
           "  -o, --output <file>            Write tree to a file\n"
           "      --allow-ext <ext...>       Include only these file extensions\n"
           "      --exclude-ext <ext...>     Exclude these file extensions\n"
           "      --root-path <path>         Set the scan root\n"
           "      --show-root                Include the root directory name\n"
           "  -c, --clipboard                Copy output to the clipboard\n"
           "  -h, --help                     Show this help\n";
}

} // namespace dendro::cli
