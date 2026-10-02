#include <dendro/cpp/compilation_database.hpp>

#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace dendro::cpp {
namespace {

std::string decode_json_string(const std::string& encoded) {
    std::string decoded;
    decoded.reserve(encoded.size());
    bool escaped = false;
    for (const char character : encoded) {
        if (!escaped) {
            if (character == '\\') {
                escaped = true;
            } else {
                decoded.push_back(character);
            }
            continue;
        }

        switch (character) {
        case '"': decoded.push_back('"'); break;
        case '\\': decoded.push_back('\\'); break;
        case '/': decoded.push_back('/'); break;
        case 'b': decoded.push_back('\b'); break;
        case 'f': decoded.push_back('\f'); break;
        case 'n': decoded.push_back('\n'); break;
        case 'r': decoded.push_back('\r'); break;
        case 't': decoded.push_back('\t'); break;
        default:
            throw std::invalid_argument("unsupported JSON escape in compilation database");
        }
        escaped = false;
    }
    if (escaped) {
        throw std::invalid_argument("unterminated JSON escape in compilation database");
    }
    return decoded;
}

std::vector<std::string> object_texts(const std::string& json) {
    std::vector<std::string> objects;
    bool in_string = false;
    bool escaped = false;
    int depth = 0;
    std::size_t start = std::string::npos;

    for (std::size_t index = 0; index < json.size(); ++index) {
        const char character = json[index];
        if (in_string) {
            if (escaped) {
                escaped = false;
            } else if (character == '\\') {
                escaped = true;
            } else if (character == '"') {
                in_string = false;
            }
            continue;
        }
        if (character == '"') {
            in_string = true;
        } else if (character == '{') {
            if (depth == 0) {
                start = index;
            }
            ++depth;
        } else if (character == '}') {
            if (depth == 0) {
                throw std::invalid_argument("unexpected JSON object terminator");
            }
            --depth;
            if (depth == 0 && start != std::string::npos) {
                objects.push_back(json.substr(start, index - start + 1));
                start = std::string::npos;
            }
        }
    }
    if (in_string || depth != 0) {
        throw std::invalid_argument("unterminated JSON in compilation database");
    }
    return objects;
}

std::string field(const std::string& object, const std::string& key, bool required) {
    const std::string quoted_key = "\"" + key + "\"";
    const std::size_t key_position = object.find(quoted_key);
    if (key_position == std::string::npos) {
        if (required) {
            throw std::invalid_argument("compilation database entry is missing '" + key + "'");
        }
        return {};
    }

    std::size_t position = key_position + quoted_key.size();
    while (position < object.size() && std::isspace(static_cast<unsigned char>(object[position]))) {
        ++position;
    }
    if (position >= object.size() || object[position] != ':') {
        throw std::invalid_argument("invalid compilation database field: " + key);
    }
    ++position;
    while (position < object.size() && std::isspace(static_cast<unsigned char>(object[position]))) {
        ++position;
    }
    if (position >= object.size() || object[position] != '"') {
        throw std::invalid_argument("compilation database field is not a JSON string: " + key);
    }

    ++position;
    std::string encoded;
    bool escaped = false;
    for (; position < object.size(); ++position) {
        const char character = object[position];
        if (!escaped && character == '"') {
            return decode_json_string(encoded);
        }
        encoded.push_back(character);
        if (escaped) {
            escaped = false;
        } else if (character == '\\') {
            escaped = true;
        }
    }
    throw std::invalid_argument("unterminated JSON string for field: " + key);
}

} // namespace

CompilationDatabase::CompilationDatabase(std::vector<CompilationCommand> commands)
    : commands_(std::move(commands)) {}

CompilationDatabase CompilationDatabase::load(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        throw std::invalid_argument("cannot open compilation database: " + path.string());
    }
    std::ostringstream contents;
    contents << input.rdbuf();

    std::vector<CompilationCommand> commands;
    for (const std::string& object : object_texts(contents.str())) {
        commands.push_back({field(object, "directory", true),
                            field(object, "file", true),
                            field(object, "command", false)});
    }
    return CompilationDatabase(std::move(commands));
}

const std::vector<CompilationCommand>& CompilationDatabase::commands() const noexcept {
    return commands_;
}

} // namespace dendro::cpp
