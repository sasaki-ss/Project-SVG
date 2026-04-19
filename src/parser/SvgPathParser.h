#ifndef PROJECT_SVG_PARSER_SVG_PATH_PARSER_H_
#define PROJECT_SVG_PARSER_SVG_PATH_PARSER_H_

#include <optional>
#include <string>
#include <vector>

#include "PathCommand.h"

namespace svg{
namespace parser{

class SvgPathParser {
public:
    auto parse(const std::string& d) -> std::optional<std::vector<PathCommand>>;
private:
    bool is_command(char c);
    bool is_parameter(char c);
    bool is_separator(char c);

    auto tokenize_path(const std::string& d) -> std::optional<std::vector<std::string>>;
    auto parse_command_type(const std::string& token) -> std::optional<PathCommandType>;
    auto read_parameter(const std::string& d, int& index) -> std::optional<std::string>;
    bool validate_command(const PathCommand& command);
};

}
}

#endif  // PROJECT_SVG_PARSER_SVG_PATH_PARSER_H_
