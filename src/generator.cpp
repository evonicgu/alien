#include "generator.h"

#include "lexer/lexer_generator.h"
#include "parser/parser_generator.h"
#include "languages/cpp_language.h"

namespace alien {

    void generator::generate() {
        using namespace util::literals;

        auto& language = find_language(config.lang);

        input::stream_input input_stream(config.input);

        alphabet::alphabet alphabet;

        lexer::lexer_generator lexer_gen(input_stream, language, alphabet, err);
        parser::parser_generator parser_gen(input_stream, language, alphabet, err);

        auto lexer_config = lexer_gen.parse_lexer_config();

        auto parser_config = parser_gen.parse_parser_config();

        auto renderer_opt = language->create_renderer(alphabet, std::move(lexer_config), std::move(parser_config));

//        util::u8string& lexer_namespace = util::check<config::settings::string_value>(
//                lexer_gen.get_param("generation.namespace"_u8).get()
//        )->str;
//
//        util::u8string& parser_namespace = util::check<config::settings::string_value>(
//                parser_gen.get_param("generation.namespace"_u8).get()
//        )->str;
//
//        util::u8string guard_prefix = std::move(util::check<config::settings::string_value>(
//                lexer_gen.get_param("general.cpp.guard_prefix"_u8).get()
//        )->str);
//
//        bool track_lines = util::check<config::settings::bool_value>(
//                lexer_gen.get_param("generation.track_lines"_u8).get()
//        )->val;
//
//        bool monomorphic = util::check<config::settings::bool_value>(
//                lexer_gen.get_param("generation.monomorphic"_u8).get()
//        )->val;
//
//        util::u8string lexer_relative_namespace = get_lexer_relative_namespace(lexer_namespace, parser_namespace);
//
//        for (std::size_t i = 0; i < alphabet.terminals.size(); ++i) {
//            if (alphabet.terminals[i].type == lexer::settings::default_token_typename) {
//                alphabet.terminals[i].type = "token_t"_u8;
//            }
//        }
//
//        if (!err.empty()) {
//            // there are errors
//            return;
//        }

        if (!renderer_opt) {
            return;
        }

        auto& renderer = renderer_opt.value();

        auto lexer_data = lexer_gen.generate_lexer();

        auto parser_data = parser_gen.generate_parser();

        if (!lexer_data || !parser_data) {
            return;
        }

        renderer->render(env, std::move(lexer_data.value()), std::move(parser_data.value()));
    }

    const std::list<util::u8string>& generator::get_errors() const {
        return err;
    }

    void generator::setup_default_languages() {
        add_language("c++", std::make_unique<languages::cpp_language>(config, err));
    }

    std::unique_ptr<languages::base_language>& generator::find_language(const std::string& name) {
        auto it = languages.find(name);

        if (it == languages.end()) {
            throw std::runtime_error("Language '" + name + "' not found");
        }

        return it->second;
    }

    void generator::add_language(const std::string& name, std::unique_ptr<languages::base_language>&& language) {
        languages[name] = std::move(language);
    }

    void generator::setup_inja_env() {
        env.add_callback("bytes", 1, [](inja::Arguments& args) {
            return util::u8string_to_bytes(args.at(0)->get<util::u8string>());
        });

        env.add_callback("lexer_collect", 2, [](inja::Arguments& args) {
            auto& state = *args.at(0);
            auto& automata = *args.at(1);

            std::vector<std::vector<util::u8char>> symbols(automata["states"].size());
            for (const auto& transition_index : state["transitions"]) {
                auto& transition = automata["transitions"][transition_index.get<std::size_t>()];

                symbols[transition["head"].get<std::size_t>()].push_back(transition["label"].get<util::u8char>());
            }

            return symbols;
        });

        env.add_callback("create_lexer_range", 2, [](inja::Arguments& args) {
            util::u8char start = args.at(0)->get<util::u8char>(), end = args.at(1)->get<util::u8char>();
            std::string varname = "c_class";
            std::string_view v = varname;

            if (start >= 0) {
                v = v.substr(0, 1);
            }

            return create_range(v, start, end);
        });

        env.add_callback("create_parser_range", 3, [](inja::Arguments& args) {
            std::ptrdiff_t start = args.at(0)->get<std::ptrdiff_t>(), end = args.at(1)->get<std::ptrdiff_t>();
            bool is_rule = args.at(2)->get<bool>();

            std::string varname = "rule_lookahead.index";
            std::string_view v = varname;

            if (!is_rule) {
                v = v.substr(5);
            }

            return create_range(v, start, end);
        });

//        env.add_callback("is_default_token_type", 1, [this](inja::Arguments& args) {
//            std::size_t token = args.at(0)->get<std::size_t>();
//
//            return alphabet.terminals[token].default_type;
//        });

        env.set_comment("#{", "}#");
    }

//    util::u8string generator::get_lexer_relative_namespace(const util::u8string& lexer_namespace,
//                                                           const util::u8string& parser_namespace) {
//        using namespace util::literals;
//
//        std::size_t common_prefix_length = 0;
//
//        for (std::size_t i = 0; i < std::min(lexer_namespace.size(), parser_namespace.size()); ++i) {
//            if (lexer_namespace[i] != parser_namespace[i]) {
//                break;
//            }
//
//            ++common_prefix_length;
//        }
//
//        util::u8string lexer_relative_namespace = lexer_namespace.substr(common_prefix_length);
//
//        if (lexer_relative_namespace.find("::"_u8) == 0) {
//            lexer_relative_namespace = lexer_relative_namespace.substr(2);
//        }
//
//        if (!lexer_relative_namespace.empty()) {
//            lexer_relative_namespace += "::"_u8;
//        }
//
//        return lexer_relative_namespace;
//    }

    std::string generator::create_range(std::string_view var, std::ptrdiff_t start, std::ptrdiff_t end) {
        if (start == end) {
            return std::string(var) + " == " + std::to_string(start);
        }

        return std::string(var) + " >= " + std::to_string(start) + " && " + std::string(var) + " <= " + std::to_string(end);
    }
}