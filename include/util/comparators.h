#ifndef ALIEN_COMPARATORS_H
#define ALIEN_COMPARATORS_H

#include <vector>

namespace alien::util::comparators {

    template<typename T>
    struct access_less {
        const std::vector<T>* arr;

        using is_transparent = std::true_type;

        explicit access_less(const std::vector<T>* arr) : arr(arr) {}

        template<typename V>
        bool operator()(const V& lhs, const std::size_t rhs) const {
            return lhs < arr->at(rhs);
        }

        template<typename V>
        bool operator()(const std::size_t lhs, const V& rhs) const {
            return arr->at(lhs) < rhs;
        }

        bool operator()(const std::size_t lhs, std::size_t rhs) const {
            return arr->at(lhs) < arr->at(rhs);
        }
    };

    template<typename T>
    struct ptr_less {
        bool operator()(const T* lhs, const T* rhs) const {
            if (rhs == nullptr) {
                return false;
            }

            if (lhs == nullptr) {
                return true;
            }

            return *lhs < *rhs;
        }
    };

    template<typename It>
    struct iterator_less {
        bool operator()(It lhs, It rhs) const {
            return *lhs < *rhs;
        }
    };

    template<typename It>
    struct iterator_elem_addr_less {
        bool operator()(It lhs, It rhs) const {
            return &(*lhs) < &(*rhs);
        }
    };

}

#endif //ALIEN_COMPARATORS_H