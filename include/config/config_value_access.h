#ifndef ALIEN_CONFIG_VALUE_ACCESS_H
#define ALIEN_CONFIG_VALUE_ACCESS_H

#include <memory>

#include "config/settings/settings.h"
#include "util/typeutils.h"

namespace alien::config {

    bool& get_bool_value(const std::unique_ptr<settings::value>& value);

    util::u8string& get_string_value(const std::unique_ptr<settings::value>& value);

    long long& get_number_value(const std::unique_ptr<settings::value>& value);
}

#endif //ALIEN_CONFIG_VALUE_ACCESS_H