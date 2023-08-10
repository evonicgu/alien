#include "config/generator_config.h"

namespace alien::config {

    void generator_streams::check_out_stream(std::ofstream& out_stream) {
        if (!out_stream) {
            throw std::runtime_error("Unable to open output stream");
        }
    }

    void generator_streams::ensure_directory_exists(const std::filesystem::path& directory) {
        if (!std::filesystem::exists(directory)) {
            std::filesystem::create_directory(directory);
        } else if (!std::filesystem::is_directory(directory)) {
            throw std::runtime_error("Output dir is not a directory");
        }
    }

    language language_from_string(const std::string& str) {
        if (str == "c++") {
            return language::CPP;
        }

        throw std::runtime_error("Unexpected language value");
    }
}
