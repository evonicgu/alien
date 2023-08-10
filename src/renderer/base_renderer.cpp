#include "renderer/base_renderer.h"

namespace alien::renderer {

    void base_renderer::check_configuration() {
        auto info = languages[config.lang];

        auto has_headers = info.has_headers;

        if (has_headers) {
            if (!config.header_output_directory.has_value()) {
                throw std::runtime_error(error_message("Header output directory must be provided", info));
            }
        } else {
            if (!config.output_directory.has_value()) {
                throw std::runtime_error(error_message("Source output directory must be provided", info));
            }

            if (config.header_output_directory.has_value()) {
                throw std::runtime_error(error_message("Header output directory cannot be set", info));
            }

            if (config.lexer_header_template.has_value() || config.parser_header_template.has_value()) {
                throw std::runtime_error(error_message("Header template files cannot be set", info));
            }
        }
    }

    std::string base_renderer::error_message(const std::string& str, const language_info& info) {
        return str + " for " + info.display_name;
    }

}