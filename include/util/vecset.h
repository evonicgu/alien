#ifndef ALIEN_UTIL_VECSET_H
#define ALIEN_UTIL_VECSET_H

#include <vector>
#include <set>
#include <type_traits>

namespace alien::util {

    template<typename T>
    struct access_less {
        const std::vector<T>* arr;

        using is_transparent = std::true_type;

        explicit access_less(const std::vector<T>* arr) : arr(arr) {}

        bool operator()(const T& lhs, const unsigned int rhs) const {
            return lhs < arr->at(rhs);
        }

        bool operator()(const unsigned int lhs, const T& rhs) const {
            return arr->at(lhs) < rhs;
        }

        bool operator()(const unsigned int lhs, unsigned int rhs) const {
            return arr->at(lhs) < arr->at(rhs);
        }
    };

    template<typename T>
    class vecset {
        using set_type = std::set<unsigned int, util::access_less<T>>;

        std::vector<T> vec;
        set_type set{util::access_less<T>(&vec)};

    public:
        vecset() = default;

        explicit vecset(const std::vector<T>& values) {
            vec.reserve(values.size());

            for (const T& value : values) {
                vec.push_back(value);
                set.insert(vec.size() - 1);
            }
        }

        explicit vecset(std::vector<T>&& values) {
            vec.reserve(values.size());

            for (T& value : values) {
                vec.push_back(std::move(value));
                set.insert(vec.size() - 1);
            }
        }

        explicit vecset(const std::set<T>& values) {
            vec.reserve(values.size());

            for (const T& value : values) {
                vec.push_back(value);
                set.insert(set.end(), vec.size() - 1);
            }
        }

        explicit vecset(std::set<T>&& values) {
            vec.reserve(values.size());

            for (auto& value : values) {
                vec.push_back(std::move(const_cast<T&&>(value)));
                set.insert(set.end(), vec.size() - 1);
            }
        }

        unsigned int push_back(const T& value) {
            auto it = set.find(value);

            if (it != set.end()) {
                return *it;
            }

            vec.push_back(value);
            set.insert(vec.size() - 1);

            return vec.size() - 1;
        }

        unsigned int push_back(T&& value) {
            auto it = set.find(value);

            if (it != set.end()) {
                return *it;
            }

            vec.push_back(std::move(value));
            set.insert(vec.size() - 1);

            return vec.size() - 1;
        }

        unsigned int push_back(typename set_type::iterator pos, const T& value) {
            auto it = set.find(value);

            if (it != set.end()) {
                return *it;
            }

            vec.push_back(value);
            set.insert(pos, vec.size() - 1);

            return vec.size() - 1;
        }

        unsigned int push_back(typename set_type::iterator pos, T&& value) {
            auto it = set.find(value);

            if (it != set.end()) {
                return *it;
            }

            vec.push_back(std::move(value));
            set.insert(pos, vec.size() - 1);

            return vec.size() - 1;
        }

        typename std::vector<T>::iterator find(const T& value) {
            auto it = set.find(value);

            if (it == set.end()) {
                return vend();
            }

            return vbegin() + *it;
        }

        typename std::vector<T>::const_iterator find(const T& value) const {
            auto it = set.find(value);

            if (it == set.end()) {
                return cvend();
            }

            return cvbegin() + *it;
        }

        T& operator[](unsigned int index) {
            if (index < 0 || index >= vec.size()) {
                throw std::out_of_range("Index out of range");
            }

            return vec[index];
        }

        const T& operator[](unsigned int index) const {
            if (index < 0 || index >= vec.size()) {
                throw std::out_of_range("Index out of range");
            }

            return vec[index];
        }

        typename std::vector<T>::iterator vbegin() {
            return vec.begin();
        }

        typename std::vector<T>::iterator vend() {
            return vec.end();
        }

        typename std::vector<T>::const_iterator cvbegin() const {
            return vec.cbegin();
        }

        typename std::vector<T>::const_iterator cvend() const {
            return vec.cend();
        }

        typename set_type::iterator sbegin() {
            return set.begin();
        }

        typename set_type::iterator send() {
            return set.end();
        }

        unsigned int size() const {
            return vec.size();
        }

        vecset(const vecset<T>& other) {
            vec = other.vec;

            for (unsigned int i : other.set) {
                set.insert(set.end(), i);
            }
        }

        vecset(vecset<T>&& other) {
            vec = std::move(other.vec);

            for (unsigned int i : other.set) {
                set.insert(set.end(), i);
            }

            other.set.clear();
        }

        vecset<T>& operator=(const vecset<T>& other) {
            if (this == &other) {
                return *this;
            }

            vec.clear();
            set.clear();

            vec = other.vec;
            for (unsigned int i : other.set) {
                set.insert(set.end(), i);
            }

            return *this;
        }

        vecset<T>& operator=(vecset<T>&& other) noexcept {
            if (this == &other) {
                return *this;
            }

            vec.clear();
            set.clear();

            vec = std::move(other.vec);
            for (unsigned int i : other.set) {
                set.insert(set.end(), i);
            }

            other.set.clear();

            return *this;
        }

        explicit operator std::set<T>() {
            std::set<T> out;

            for (unsigned int index : set) {
                out.insert(out.end(), std::move(vec[index]));
            }

            return out;
        }

        explicit operator std::vector<T>() {
            return std::move(this->vec);
        }
    };

}

#endif //ALIEN_UTIL_VECSET_H