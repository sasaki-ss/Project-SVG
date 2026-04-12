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

    std::vector<std::string> tokens = tokenize_path(d);
    if(tokens.empty()) {
        return std::nullopt;
    }

    std::optional<PathCommand> current_command;
    for(const auto& token : tokens) {
        std::optional<PathCommandType> command_type = parse_command_type(token);
        if(command_type == std::nullopt) {
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

auto SvgPathParser::tokenize_path(const std::string& d) -> std::vector<std::string> {
    std::vector<std::string> token;
    int i = 0;
    int length = d.length();
    while(i < length) {
        char c = d.at(i);
        if(std::isspace(static_cast<unsigned char>(c)) != 0 || c == ',') {
            ++i;
            continue;
        }

        if(is_command(c)) {
            token.emplace_back(1, c);
            ++i;
            continue;
        }

        if(is_parameter(c)) {
            token.emplace_back(read_parameter(d, i));
            continue;
        }

        return std::vector<std::string>();
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

std::string SvgPathParser::read_parameter(const std::string& d, int& index) {
    return "";
}

}
}
