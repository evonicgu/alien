#include "languages/base_language.h"

namespace alien::languages {

    bool base_language::check_base_configuration(const lexer::settings::settings_t& lexer_settings,
                                                 const parser::settings::settings_t& parser_settings) {
        using namespace util::literals;

        bool correct = true;

        long long buffer_size = util::check<config::settings::number_value>(
                lexer_settings.config.at("generation.buffer_size"_u8).get()
        )->number;

        long long lexeme_size = util::check<config::settings::number_value>(
                lexer_settings.config.at("generation.lexeme_size"_u8).get()
        )->number;

        if (buffer_size <= 0 || lexeme_size <= 0) {
            correct = false;

            err.push_back("Buffer or lexeme size must be greater than zero"_u8);
        }

        auto& stream_type = util::check<config::settings::string_value>(
                lexer_settings.config.at("generation.stream_type"_u8).get()
                )->str;

        auto monomorphic = util::check<config::settings::bool_value>(
                lexer_settings.config.at("generation.monomorphic"_u8).get()
                )->val;

        if (monomorphic && stream_type.empty()) {
            correct = false;

            err.push_back("Cannot create monomorphic lexer: stream_type is not set"_u8);
        }

        return correct;
    }

}