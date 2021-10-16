#ifndef ALIEN_UTIL_H
#define ALIEN_UTIL_H

namespace alien::util {

    template<typename T>
    struct access_less {
        const std::vector<T>& arr;

        using is_transparent = std::true_type;

        access_less(const std::vector<T>& arr) : arr(arr) {}

        bool operator()(const T& lhs, const unsigned int rhs) const {
            return lhs < arr[rhs];
        }

        bool operator()(const unsigned int lhs, const T& rhs) const {
            return arr[lhs] < rhs;
        }

        bool operator()(const unsigned int lhs, unsigned int rhs) const {
            return arr[lhs] < arr[rhs];
        }
    };

}

#endif //ALIEN_UTIL_H