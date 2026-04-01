#ifndef SRC_LOADER_SVGLOADER_H_
#define SRC_LOADER_SVGLOADER_H_

#include <filesystem>
#include <optional>
#include <string>

class SvgLoader
{
public:
    static std::optional<std::string> LoadFromFile(const std::filesystem::path& file_path);
};

#endif  // SRC_LOADER_SVGLOADER_H_
