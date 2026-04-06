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
    explicit SvgPathParser(std::string_view xml_content);

    auto parse(const std::string& d) -> std::optional<std::vector<PathCommand>>;
private:
    auto tokenize_path(const std::string& d) -> std::vector<std::string>;
    auto parse_command_type(const std::string& token) -> std::optional<PathCommandType>;
    bool is_absolute(const std::string& token);
};

}
}

#endif  // PROJECT_SVG_PARSER_SVG_PATH_PARSER_H_
