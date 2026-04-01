#include "SvgLoader.h"

#include <fstream>

namespace svg{
namespace loader{

std::optional<std::string> SvgLoader::load_from_file(const std::filesystem::path& file_path) {
    try {
        const std::filesystem::path absolute_path = std::filesystem::absolute(file_path);

        if (!std::filesystem::exists(absolute_path)) {
            return std::nullopt;
        }

        if (!std::filesystem::is_regular_file(absolute_path)) {
            return std::nullopt;
        }

        std::ifstream input_stream(absolute_path, std::ios::binary);
        if (!input_stream) {
            return std::nullopt;
        }

        return std::string(
            std::istreambuf_iterator<char>(input_stream),
            std::istreambuf_iterator<char>());
    } catch (...) {
        return std::nullopt;
    }
}

}
}
