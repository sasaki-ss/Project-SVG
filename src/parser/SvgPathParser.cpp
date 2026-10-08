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

static const int PARAMETER_SIZE_M = 2;
static const int PARAMETER_SIZE_L = 2;
static const int PARAMETER_SIZE_H = 1;
static const int PARAMETER_SIZE_V = 1;
static const int PARAMETER_SIZE_C = 6;
static const int PARAMETER_SIZE_Z = 0;
static const int PARAMETER_SIZE_S = 4;
static const int PARAMETER_SIZE_Q = 4;
static const int PARAMETER_SIZE_T = 2;
static const int PARAMETER_SIZE_A = 7;

auto SvgPathParser::parse(const std::string& d) -> std::optional<std::vector<PathCommand>> {
    std::vector<PathCommand> path_commands;

    auto tokens = tokenize_path(d);
    if(!tokens.has_value() || tokens->empty()) {
        return std::nullopt;
    }

    std::optional<PathCommand> current_command;
    for(const auto& token : *tokens) {
        std::optional<PathCommandType> command_type = parse_command_type(token);
        if(!command_type.has_value() && !current_command.has_value()) {
            return std::nullopt;
        }

        if(!command_type.has_value()) {
            current_command->parameters.emplace_back(token);
            continue;
        }

        if(!push_finalized_command(current_command, path_commands)) {
            return std::nullopt;
        }

        PathCommand command;
        command.type = *command_type;
        command.is_absolute = std::isupper(static_cast<unsigned char>(token.at(0)));
        current_command = std::move(command);
    }

    if(!push_finalized_command(current_command, path_commands)) {
        return std::nullopt;
    }

    return path_commands;
}

bool SvgPathParser::push_finalized_command(
    std::optional<PathCommand>& command,
    std::vector<PathCommand>& path_commands) {
    if(!command.has_value()) {
        return true;
    }

    if(!finalize_command(*command)) {
        return false;
    }

    path_commands.emplace_back(std::move(*command));
    command.reset();
    return true;
}

bool SvgPathParser::finalize_command(PathCommand& command) {
    if(command.type == PathCommandType::ArcTo) {
        const auto normalized_parameters = normalize_arc_parameters(command.parameters);
        if(!normalized_parameters.has_value()) {
            return false;
        }
        command.parameters = *normalized_parameters;
    }

    return validate_command(command);
}

bool SvgPathParser::is_command(char c) {
    char command = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return PATH_COMMAND_TYPE.find(command) != PATH_COMMAND_TYPE.end();
}

bool SvgPathParser::is_parameter(char c) {
    return std::isdigit(static_cast<unsigned char>(c)) || c == '-' || c == '.';
}

bool SvgPathParser::is_separator(char c) {
    return std::isspace(static_cast<unsigned char>(c)) != 0 || c == ',';
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

        if(!is_parameter(c)) {
            return std::nullopt;
        }

        auto parameter = read_parameter(d, i);
        if(!parameter.has_value()) {
            return std::nullopt;
        }

        token.emplace_back(*parameter);
    }

    return token;
}

auto SvgPathParser::parse_command_type(const std::string& token) -> std::optional<PathCommandType> {
    if(token.length() != 1) {
        return std::nullopt;
    }

    char command = static_cast<char>(std::toupper(static_cast<unsigned char>(token.at(0))));
    auto it = PATH_COMMAND_TYPE.find(command);
    if(it != PATH_COMMAND_TYPE.end()) {
        return it->second;
    }
    
    return std::nullopt;
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

        if(c == '.' && !has_dot) {
            parameter.push_back(c);
            has_dot = true;
            continue;
        }

        if(c == '.') {
            index = i;
            return parameter;
        }

        parameter.push_back(c);
    }

    index = size;
    return parameter;
}

auto SvgPathParser::normalize_arc_parameters(
    const std::vector<std::string>& parameters)
    -> std::optional<std::vector<std::string>> {
    constexpr std::size_t ARC_PARAMETER_SIZE = 7;
    constexpr std::size_t LARGE_ARC_FLAG_INDEX = 3;
    constexpr std::size_t SWEEP_FLAG_INDEX = 4;

    std::vector<std::string> normalized_parameters;
    std::size_t parameter_index = 0;
    std::size_t character_index = 0;

    while (parameter_index < parameters.size()) {
        while (parameter_index < parameters.size()
            && character_index >= parameters[parameter_index].size()) {
            ++parameter_index;
            character_index = 0;
        }

        if (parameter_index >= parameters.size()) {
            break;
        }

        const std::size_t parameter_position =
            normalized_parameters.size() % ARC_PARAMETER_SIZE;
        const bool is_arc_flag = parameter_position == LARGE_ARC_FLAG_INDEX
            || parameter_position == SWEEP_FLAG_INDEX;

        const char current_character = parameters[parameter_index][character_index];
        if (is_arc_flag && current_character != '0' && current_character != '1') {
            return std::nullopt;
        }

        if (is_arc_flag) {
            normalized_parameters.emplace_back(1, current_character);
            ++character_index;
            continue;
        }

        if (character_index != 0) {
            normalized_parameters.push_back(
                parameters[parameter_index].substr(character_index));
            ++parameter_index;
            character_index = 0;
            continue;
        }

        normalized_parameters.push_back(parameters[parameter_index]);
        ++parameter_index;
    }

    return normalized_parameters;
}

bool SvgPathParser::validate_command(const PathCommand& command){
    int parameter_size = static_cast<int>(command.parameters.size());
    switch(command.type){
    case PathCommandType::MoveTo:
        return parameter_size >= PARAMETER_SIZE_M && parameter_size % PARAMETER_SIZE_M == 0;
    case PathCommandType::LineTo:
        return parameter_size >= PARAMETER_SIZE_L && parameter_size % PARAMETER_SIZE_L == 0;
    case PathCommandType::HorizontalTo:
        return parameter_size >= PARAMETER_SIZE_H;
    case PathCommandType::VerticalTo:
        return parameter_size >= PARAMETER_SIZE_V;
    case PathCommandType::CubicBezierTo:
        return parameter_size >= PARAMETER_SIZE_C && parameter_size % PARAMETER_SIZE_C == 0;
    case PathCommandType::ClosePath:
        return parameter_size == PARAMETER_SIZE_Z;
    case PathCommandType::SmoothCubicBezierTo:
        return parameter_size >= PARAMETER_SIZE_S && parameter_size % PARAMETER_SIZE_S == 0;
    case PathCommandType::QuadraticBezierTo:
        return parameter_size >= PARAMETER_SIZE_Q && parameter_size % PARAMETER_SIZE_Q == 0;
    case PathCommandType::SmoothQuadraticBezierTo:
        return parameter_size >= PARAMETER_SIZE_T && parameter_size % PARAMETER_SIZE_T == 0;
    case PathCommandType::ArcTo:
        return parameter_size >= PARAMETER_SIZE_A && parameter_size % PARAMETER_SIZE_A == 0;
    }
    return false;
}

}
}
