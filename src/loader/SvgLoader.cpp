#include "src/loader/SvgLoader.h"

#include <fstream>

std::optional<std::string> SvgLoader::LoadFromFile(const std::filesystem::path& file_path)
{
    try {
        const std::filesystem::path absolute_path = std::filesystem::absolute(file_path);

        std::error_code error_code;
        if (!std::filesystem::exists(absolute_path, error_code) || error_code) {
            return std::nullopt;
        }

        error_code.clear();
        if (!std::filesystem::is_regular_file(absolute_path, error_code) || error_code) {
            return std::nullopt;
        }

        std::ifstream input_stream(absolute_path, std::ios::binary);
        if (!input_stream.is_open()) {
            return std::nullopt;
        }

        return std::string(
            std::istreambuf_iterator<char>(input_stream),
            std::istreambuf_iterator<char>());
    } catch (...) {
        return std::nullopt;
    }
}
