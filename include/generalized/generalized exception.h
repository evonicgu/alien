#ifndef ALIEN_GENERALIZED_EXCEPTION_H
#define ALIEN_GENERALIZED_EXCEPTION_H

#include <string>

namespace alien::generalized {

    template<const char* Str>
    class generalized_exception : public std::exception {
        std::string description;

        void initialize() {
            description = Str + description;
        }

    public:
        explicit generalized_exception() = default;

        explicit generalized_exception(const std::string& description) : description(description) {
            initialize();
        }

        explicit generalized_exception(std::string&& description) : description(std::move(description)) {
            initialize();
        }

        const char* what() const noexcept override {
            return description.c_str();
        }
    };

}

#endif //ALIEN_GENERALIZED_EXCEPTION_H