#ifndef ALIEN_TYPEUTILS_H
#define ALIEN_TYPEUTILS_H

#include <stdexcept>
#include <string>
#include <typeinfo>

namespace alien::util {

    template<typename To, typename From>
    To* check(From* from) {
        auto* casted = dynamic_cast<To*>(from);

        if (casted == nullptr) {
            std::string type = typeid(To).name();

            throw std::logic_error(std::string("Invalid object type. Expected: ") + type.substr(1));
        }

        return casted;
    }

}

#endif //ALIEN_TYPEUTILS_H