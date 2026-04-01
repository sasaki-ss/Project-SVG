#ifndef PROJECT_SVG_LOADER_SVG_LOADER_H_
#define PROJECT_SVG_LOADER_SVG_LOADER_H_

#include <filesystem>
#include <optional>
#include <string>

namespace svg{
namespace loader{

class SvgLoader {
public:
    static std::optional<std::string> load_from_file(const std::filesystem::path& file_path);
};

}
}

#endif  // PROJECT_SVG_LOADER_SVG_LOADER_H_
