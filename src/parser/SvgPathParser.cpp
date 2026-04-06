#include "SvgPathParser.h"

namespace svg{
namespace parser{

auto SvgPathParser::parse(const std::string& d) -> std::optional<std::vector<PathCommand>> {
    std::vector<PathCommand> path_commands;

    std::vector<std::string> tokens = tokenize_path(d);
    if(tokens.empty()){
        return std::nullopt;
    }

    std::optional<PathCommand> current_command;
    for(const auto& token : tokens){
        std::optional<PathCommandType> command_type = parse_command_type(token);
        if(command_type == std::nullopt){
            if(!current_command.has_value()){
                return std::nullopt;
            }

            current_command->parameters.emplace_back(token);
            continue;
        }

        if(current_command.has_value()){
            path_commands.emplace_back(std::move(*current_command));
        }

        PathCommand command;
        command.type = *command_type;
        command.is_absolute = is_absolute(token);
        current_command = std::move(command);
    }

    if(current_command.has_value()){
        path_commands.emplace_back(std::move(*current_command));
    }

    return path_commands;
}

auto SvgPathParser::tokenize_path(const std::string& d) -> std::vector<std::string> {
    std::vector<std::string> token;

    return token;
}

auto SvgPathParser::parse_command_type(const std::string& token) -> std::optional<PathCommandType>{
    PathCommandType type;

    return type;
}

bool SvgPathParser::is_absolute(const std::string& token){
    return true;
}

}
}
