#include <dendro.hpp>
#include <dendro/cli/arguments.hpp>
#include <dendro/cli/clipboard.hpp>

#include <fstream>
#include <iostream>
#include <stdexcept>

int main(int argc, char* argv[]) {
    try {
        const dendro::cli::Arguments arguments = dendro::cli::parse_arguments(argc, argv);
        if (arguments.help) {
            std::cout << dendro::cli::usage();
            return 0;
        }

        const std::string structure = dendro::generate_structure(arguments.config);
        if (arguments.config.copy_to_clipboard) {
            if (!dendro::cli::copy_to_clipboard(structure)) {
                std::cerr << "dendro: clipboard is unavailable\n";
                return 2;
            }
        } else {
            std::ofstream output(arguments.config.output_file, std::ios::binary);
            if (!output) {
                std::cerr << "dendro: cannot open output file: "
                          << arguments.config.output_file.string() << "\n";
                return 2;
            }
            output << structure;
        }

        std::cout << structure;
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "dendro: " << error.what() << "\n";
        std::cerr << dendro::cli::usage();
        return 1;
    }
}
