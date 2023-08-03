#include "lexer/config/rules/rules.h"

namespace alien::lexer::rules {

    void to_json(nlohmann::json& json, const action& act) {
        json["code"] = util::u8string_to_bytes(act.code);
        json["symbol"] = util::u8string_to_bytes(act.symbol);
    }

}