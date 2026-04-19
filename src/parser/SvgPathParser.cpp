#include "SvgPathParser.h"

#include <cctype>
#include <unordered_map>

namespace svg{
namespace parser{

static const std::unordered_map<char, PathCommandType> PATH_COMMAND_TYPE = {
    {'M', PathCommandType::MoveTo},
    {'L', PathCommandType::LineTo},
    {'H', PathCommandType::HorizontalTo},
    {'V', PathCommandType::VerticalTo},
    {'C', PathCommandType::CubicBezierTo},
    {'Z', PathCommandType::ClosePath},
    {'S', PathCommandType::SmoothCubicBezierTo},
    {'Q', PathCommandType::QuadraticBezierTo},
    {'T', PathCommandType::SmoothQuadraticBezierTo},
    {'A', PathCommandType::ArcTo},
};

auto SvgPathParser::parse(const std::string& d) -> std::optional<std::vector<PathCommand>> {
    std::vector<PathCommand> path_commands;

    auto tokens = tokenize_path(d);
    if(!tokens.has_value() || tokens->empty()) {
        return std::nullopt;
    }

    std::optional<PathCommand> current_command;
    for(const auto& token : *tokens) {
        std::optional<PathCommandType> command_type = parse_command_type(token);
        if(!command_type.has_value()) {
            if(!current_command.has_value()) {
                return std::nullopt;
            }

            current_command->parameters.emplace_back(token);
            continue;
        }

        if(current_command.has_value()) {
            path_commands.emplace_back(std::move(*current_command));
        }

        PathCommand command;
        command.type = *command_type;
        command.is_absolute = std::isupper(static_cast<unsigned char>(token.at(0)));
        current_command = std::move(command);
    }

    if(current_command.has_value()) {
        path_commands.emplace_back(std::move(*current_command));
    }

    return path_commands;
}

auto SvgPathParser::tokenize_path(const std::string& d) -> std::optional<std::vector<std::string>> {
    std::vector<std::string> token;
    int i = 0;
    int length = static_cast<int>(d.length());
    while(i < length) {
        char c = d.at(i);
        if(is_separator(c)) {
            ++i;
            continue;
        }

        if(is_command(c)) {
            token.emplace_back(1, c);
            ++i;
            continue;
        }

        if(is_parameter(c)) {
            auto parameter = read_parameter(d, i);
            if(!parameter.has_value()) {
                return std::nullopt;
            }

            token.emplace_back(*parameter);
            continue;
        }

        return std::nullopt;
    }

    return token;
}

auto SvgPathParser::parse_command_type(const std::string& token) -> std::optional<PathCommandType> {
    if(token.length() != 1) {
        return std::nullopt;
    }

    char command = std::toupper(static_cast<unsigned char>(token.at(0)));
    auto it = PATH_COMMAND_TYPE.find(command);
    if(it != PATH_COMMAND_TYPE.end()) {
        return it->second;
    }
    
    return std::nullopt;
}

bool SvgPathParser::is_command(char c) {
    char command = std::toupper(static_cast<unsigned char>(c));
    return PATH_COMMAND_TYPE.find(command) != PATH_COMMAND_TYPE.end();
}

bool SvgPathParser::is_parameter(char c) {
    return std::isdigit(static_cast<unsigned char>(c)) || c == '-' || c == '.';
}

bool SvgPathParser::is_separator(char c) {
    return std::isspace(static_cast<unsigned char>(c)) != 0 || c == ',';
}

auto SvgPathParser::read_parameter(const std::string& d, int& index) -> std::optional<std::string> {
    std::string parameter;
    int start_pos = index;

    int size = static_cast<int>(d.size());
    if(start_pos >= size) {
        return std::nullopt;
    }

    char start_char = d.at(start_pos);
    bool has_dot = start_char == '.' ? true : false;
    parameter.push_back(start_char);
    
    for(int i = start_pos + 1; i < size; ++i) {
        char c = d.at(i);

        if(is_separator(c) || is_command(c) || c == '-') {
            index = i;
            return parameter;
        }

        if(c == '.') {
            if(!has_dot) {
                parameter.push_back(c);
                has_dot = true;
                continue;
            }

            index = i;
            return parameter;
        }

        parameter.push_back(c);
    }

    index = size;
    return parameter;
}

}
}
