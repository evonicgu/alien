#ifndef ALIEN_COMPARATORS_H
#define ALIEN_COMPARATORS_H

#include <vector>

namespace alien::util::comparators {

    struct index_storage {
        std::size_t index;

        explicit index_storage(std::size_t index)
            : index(index) {}
    };

    template<typename T>
    struct access_less {
        const std::vector<T>* arr;

        using is_transparent = std::true_type;

        explicit access_less(const std::vector<T>* arr) : arr(arr) {}

        template<typename V>
        bool operator()(const V& lhs, const index_storage rhs) const {
            return lhs < arr->at(rhs.index);
        }

        template<typename V>
        bool operator()(const index_storage lhs, const V& rhs) const {
            return arr->at(lhs.index) < rhs;
        }

        bool operator()(const index_storage lhs, index_storage rhs) const {
            return arr->at(lhs.index) < arr->at(rhs.index);
        }
    };

    template<typename T>
    struct ptr_less {
        bool operator()(const T* lhs, const T* rhs) const {
            if (lhs == nullptr) {
                return false;
            }

            if (rhs == nullptr) {
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