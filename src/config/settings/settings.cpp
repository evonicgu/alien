#include "config/config_value_access.h"
#include "config/settings/settings.h"

#include "util/typeutils.h"

namespace alien::config {

    template<typename T>
    T* get_typed_value(const std::unique_ptr<config::settings::value>& value) {
        return util::check<T>(
                value.get()
        );
    }

    bool& get_bool_value(const std::unique_ptr<config::settings::value>& value) {
        return get_typed_value<config::settings::bool_value>(value)->val;
    }

    util::u8string& get_string_value(const std::unique_ptr<config::settings::value>& value) {
        return get_typed_value<config::settings::string_value>(value)->str;
    }

    long long& get_number_value(const std::unique_ptr<config::settings::value>& value) {
        return get_typed_value<config::settings::number_value>(value)->number;
    }


}