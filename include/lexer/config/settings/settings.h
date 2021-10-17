#ifndef ALIEN_LEXER_SETTINGS_H
#define ALIEN_LEXER_SETTINGS_H

#include <map>
#include <memory>
#include <string>
#include <set>
#include "config/settings/settings.h"

namespace alien::lexer::config::settings {

    struct settings {
        std::map<std::string, std::shared_ptr<alien::config::settings::value>> config;

        std::map<std::string, std::string> tokens;
    };

}

#endif //ALIEN_LEXER_SETTINGS_H