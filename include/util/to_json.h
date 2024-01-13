#ifndef ALIEN_TO_JSON_H
#define ALIEN_TO_JSON_H

#include <map>
#include <string>
#include <vector>

#include "u8string.h"
#include "vecset.h"

namespace alien::util {

    template<typename T>
    std::map<std::string, T> to_json(const std::map<u8string, T>& data) {
        std::map<std::string, T> transformed;

        std::for_each(data.cbegin(), data.cend(), [&](const std::pair<u8string, T>& p) {
            transformed[u8string_to_bytes(p.first)] = p.second;
        });

        return transformed;
    }

    template<typename T, typename Func>
    auto to_json(const vecset<T>& data, Func f) -> std::vector<decltype(f(std::declval<T>()))> {
        std::vector<decltype(f(std::declval<T>()))> transformed;
        transformed.resize(data.size());

        std::transform(data.cvbegin(), data.cvend(), transformed.begin(), f);

        return transformed;
    }

    template<typename T, typename Func>
    auto to_json(const std::vector<T>& data, Func f) -> std::vector<decltype(f(std::declval<T>()))> {
        std::vector<decltype(f(std::declval<T>()))> transformed;
        transformed.resize(data.size());

        std::transform(data.cbegin(), data.cend(), transformed.begin(), f);

        return transformed;
    }

}

#endif //ALIEN_TO_JSON_H