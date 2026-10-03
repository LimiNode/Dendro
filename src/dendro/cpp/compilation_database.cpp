#include <dendro/cpp/compilation_database.hpp>

#include <cctype>
#include <fstream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace dendro::cpp {
namespace {

struct JsonValue {
    enum class Kind { String, Array, Object };
    Kind kind;
    std::string string;
    std::vector<JsonValue> array;
    std::map<std::string, JsonValue> object;
};

class JsonParser {
public:
    explicit JsonParser(std::string text) : text_(std::move(text)) {}

    JsonValue parse() {
        skip_space();
        JsonValue value = parse_value();
        skip_space();
        if (position_ != text_.size()) {
            throw std::invalid_argument("unexpected data after compilation database JSON");
        }
        return value;
    }

private:
    void skip_space() {
        while (position_ < text_.size() &&
               std::isspace(static_cast<unsigned char>(text_[position_]))) {
            ++position_;
        }
    }

    char take() {
        if (position_ >= text_.size()) {
            throw std::invalid_argument("unexpected end of compilation database JSON");
        }
        return text_[position_++];
    }

    void expect(char expected) {
        if (take() != expected) {
            throw std::invalid_argument("malformed compilation database JSON");
        }
    }

    static void append_utf8(std::string& result, unsigned codepoint) {
        if (codepoint <= 0x7f) {
            result.push_back(static_cast<char>(codepoint));
        } else if (codepoint <= 0x7ff) {
            result.push_back(static_cast<char>(0xc0 | (codepoint >> 6)));
            result.push_back(static_cast<char>(0x80 | (codepoint & 0x3f)));
        } else if (codepoint <= 0xffff) {
            result.push_back(static_cast<char>(0xe0 | (codepoint >> 12)));
            result.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3f)));
            result.push_back(static_cast<char>(0x80 | (codepoint & 0x3f)));
        } else {
            result.push_back(static_cast<char>(0xf0 | (codepoint >> 18)));
            result.push_back(static_cast<char>(0x80 | ((codepoint >> 12) & 0x3f)));
            result.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3f)));
            result.push_back(static_cast<char>(0x80 | (codepoint & 0x3f)));
        }
    }

    std::string parse_string() {
        expect('"');
        std::string result;
        while (position_ < text_.size()) {
            const char character = take();
            if (character == '"') {
                return result;
            }
            if (character != '\\') {
                if (static_cast<unsigned char>(character) < 0x20) {
                    throw std::invalid_argument("control character in JSON string");
                }
                result.push_back(character);
                continue;
            }
            const char escape = take();
            switch (escape) {
            case '"': result.push_back('"'); break;
            case '\\': result.push_back('\\'); break;
            case '/': result.push_back('/'); break;
            case 'b': result.push_back('\b'); break;
            case 'f': result.push_back('\f'); break;
            case 'n': result.push_back('\n'); break;
            case 'r': result.push_back('\r'); break;
            case 't': result.push_back('\t'); break;
            case 'u': {
                unsigned codepoint = 0;
                for (int index = 0; index < 4; ++index) {
                    const char digit = take();
                    codepoint <<= 4;
                    if (digit >= '0' && digit <= '9') codepoint += digit - '0';
                    else if (digit >= 'a' && digit <= 'f') codepoint += digit - 'a' + 10;
                    else if (digit >= 'A' && digit <= 'F') codepoint += digit - 'A' + 10;
                    else throw std::invalid_argument("invalid JSON unicode escape");
                }
                append_utf8(result, codepoint);
                break;
            }
            default:
                throw std::invalid_argument("unsupported JSON escape in compilation database");
            }
        }
        throw std::invalid_argument("unterminated JSON string");
    }

    JsonValue parse_value() {
        skip_space();
        if (position_ >= text_.size()) throw std::invalid_argument("missing JSON value");
        if (text_[position_] == '"') return {JsonValue::Kind::String, parse_string(), {}, {}};
        if (text_[position_] == '[') return parse_array();
        if (text_[position_] == '{') return parse_object();
        throw std::invalid_argument("compilation database supports only JSON strings, arrays, and objects");
    }

    JsonValue parse_array() {
        expect('[');
        JsonValue result{JsonValue::Kind::Array, {}, {}, {}};
        skip_space();
        if (position_ < text_.size() && text_[position_] == ']') { ++position_; return result; }
        while (true) {
            result.array.push_back(parse_value());
            skip_space();
            const char separator = take();
            if (separator == ']') return result;
            if (separator != ',') throw std::invalid_argument("malformed JSON array");
            skip_space();
        }
    }

    JsonValue parse_object() {
        expect('{');
        JsonValue result{JsonValue::Kind::Object, {}, {}, {}};
        skip_space();
        if (position_ < text_.size() && text_[position_] == '}') { ++position_; return result; }
        while (true) {
            skip_space();
            if (position_ >= text_.size() || text_[position_] != '"')
                throw std::invalid_argument("JSON object key must be a string");
            const std::string key = parse_string();
            skip_space();
            expect(':');
            result.object.emplace(key, parse_value());
            skip_space();
            const char separator = take();
            if (separator == '}') return result;
            if (separator != ',') throw std::invalid_argument("malformed JSON object");
        }
    }

    std::string text_;
    std::size_t position_ = 0;
};

const JsonValue& required_field(const JsonValue& object, const char* name) {
    const auto found = object.object.find(name);
    if (found == object.object.end()) {
        throw std::invalid_argument(std::string("compilation database entry is missing '") + name + "'");
    }
    return found->second;
}

const JsonValue* optional_field(const JsonValue& object, const char* name) {
    const auto found = object.object.find(name);
    return found == object.object.end() ? nullptr : &found->second;
}

const std::string& string_field(const JsonValue& value, const char* name) {
    if (value.kind != JsonValue::Kind::String) {
        throw std::invalid_argument(std::string("compilation database field is not a JSON string: ") + name);
    }
    return value.string;
}

} // namespace

CompilationDatabase::CompilationDatabase(std::vector<CompilationCommand> commands)
    : commands_(std::move(commands)) {}

CompilationDatabase CompilationDatabase::load(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) throw std::invalid_argument("cannot open compilation database: " + path.string());
    std::ostringstream contents;
    contents << input.rdbuf();
    const JsonValue root = JsonParser(contents.str()).parse();
    if (root.kind != JsonValue::Kind::Array) {
        throw std::invalid_argument("compilation database root must be a JSON array");
    }

    std::vector<CompilationCommand> commands;
    for (const JsonValue& entry : root.array) {
        if (entry.kind != JsonValue::Kind::Object) {
            throw std::invalid_argument("compilation database entries must be JSON objects");
        }
        CompilationCommand command;
        command.directory = string_field(required_field(entry, "directory"), "directory");
        command.file = string_field(required_field(entry, "file"), "file");
        if (const JsonValue* value = optional_field(entry, "command")) {
            command.command = string_field(*value, "command");
        }
        if (const JsonValue* value = optional_field(entry, "arguments")) {
            if (value->kind != JsonValue::Kind::Array) {
                throw std::invalid_argument("compilation database field 'arguments' is not an array");
            }
            std::vector<std::string> arguments;
            for (const JsonValue& argument : value->array) {
                arguments.push_back(string_field(argument, "arguments"));
            }
            command.arguments = std::move(arguments);
        }
        if (!command.command.has_value() && !command.arguments.has_value()) {
            throw std::invalid_argument("compilation database entry requires 'command' or 'arguments'");
        }
        if (const JsonValue* value = optional_field(entry, "output")) {
            command.output = string_field(*value, "output");
        }
        commands.push_back(std::move(command));
    }
    return CompilationDatabase(std::move(commands));
}

const std::vector<CompilationCommand>& CompilationDatabase::commands() const noexcept {
    return commands_;
}

} // namespace dendro::cpp
