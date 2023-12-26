#ifndef ALIEN_VECSET_H
#define ALIEN_VECSET_H

#include <set>
#include <initializer_list>
#include <stdexcept>
#include <string>
#include <vector>

#include "comparators.h"

namespace alien::util {

    template<typename T>
    class vecset {
        using set_type = std::set<comparators::index_storage, comparators::access_less<T>>;

        std::vector<T> vec;
        set_type set{comparators::access_less<T>(&vec)};

    public:
        vecset() = default;

        vecset(std::initializer_list<T>&& list) {
            vec.reserve(list.size());

            for (auto it : list) {
                vec.push_back(std::move(it));
                set.insert(comparators::index_storage{vec.size() - 1});
            }
        }

        explicit vecset(const std::vector<T>& values) {
            vec.reserve(values.size());

            for (const T& value : values) {
                vec.push_back(value);
                set.insert(comparators::index_storage{vec.size() - 1});
            }
        }

        explicit vecset(std::vector<T>&& values) {
            vec.reserve(values.size());

            for (T& value : values) {
                vec.push_back(std::move(value));
                set.insert(comparators::index_storage{vec.size() - 1});
            }
        }

        explicit vecset(const std::set<T>& values) {
            vec.reserve(values.size());

            for (const T& value : values) {
                vec.push_back(value);
                set.insert(set.end(), comparators::index_storage{vec.size() - 1});
            }
        }

        explicit vecset(std::set<T>&& values) {
            vec.reserve(values.size());

            for (auto& value : values) {
                vec.push_back(std::move(const_cast<T&&>(value)));
                set.insert(set.end(), comparators::index_storage{vec.size() - 1});
            }
        }

        vecset(const vecset<T>& other) {
            vec = other.vec;

            for (auto i : other.set) {
                set.insert(set.end(), i);
            }
        }

        vecset(vecset<T>&& other) noexcept {
            vec = std::move(other.vec);

            for (auto i : other.set) {
                set.insert(set.end(), i);
            }

            other.clear();
        }

        vecset<T>& operator=(const vecset<T>& other) {
            if (this == &other) {
                return *this;
            }

            vec.clear();
            set.clear();

            vec = other.vec;
            for (auto i : other.set) {
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
            for (auto i : other.set) {
                set.insert(set.end(), i);
            }

            other.set.clear();

            return *this;
        }

        explicit operator std::set<T>() {
            std::set<T> out;

            for (auto index : set) {
                out.insert(out.end(), vec[index.index]);
            }

            return out;
        }

        explicit operator std::vector<T>() {
            return this->vec;
        }

        std::size_t push_back(const T& value) {
            auto it = set.find(value);

            if (it != set.end()) {
                return it->index;
            }

            vec.push_back(value);
            set.insert(comparators::index_storage{vec.size() - 1});

            return vec.size() - 1;
        }

        std::size_t push_back(T&& value) {
            auto it = set.find(value);

            if (it != set.end()) {
                return it->index;
            }

            vec.push_back(std::move(value));
            set.insert(comparators::index_storage{vec.size() - 1});

            return vec.size() - 1;
        }

        std::size_t push_back(typename set_type::iterator pos, const T& value) {
            auto it = set.find(value);

            if (it != set.end()) {
                return it->index;
            }

            vec.push_back(value);
            set.insert(pos, comparators::index_storage{vec.size() - 1});

            return vec.size() - 1;
        }

        std::size_t push_back(typename set_type::iterator pos, T&& value) {
            auto it = set.find(value);

            if (it != set.end()) {
                return it->index;
            }

            vec.push_back(std::move(value));
            set.insert(pos, comparators::index_storage{vec.size() - 1});

            return vec.size() - 1;
        }

        typename std::vector<T>::iterator find(const T& value) {
            auto it = set.find(value);

            if (it == set.end()) {
                return vend();
            }

            return vbegin() + it->index;
        }

        typename std::vector<T>::const_iterator find(const T& value) const {
            auto it = set.find(value);

            if (it == set.end()) {
                return cvend();
            }

            return cvbegin() + it->index;
        }

        template<typename V>
        typename std::vector<T>::iterator find(const V& value) {
            auto it = set.find(value);

            if (it == set.end()) {
                return vend();
            }

            return vbegin() + it->index;
        }

        template<typename V>
        typename std::vector<T>::const_iterator find(const V& value) const {
            auto it = set.find(value);

            if (it == set.end()) {
                return cvend();
            }

            return cvbegin() + it->index;
        }

        T& operator[](std::size_t index) {
            if (index >= vec.size()) {
                throw std::out_of_range(std::string("Index out of range: ") + std::to_string(index));
            }

            return vec[index];
        }

        const T& operator[](std::size_t index) const {
            if (index >= vec.size()) {
                throw std::out_of_range(std::string("Index out of range: ") + std::to_string(index));
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

        typename set_type::const_iterator sbegin() const {
            return set.cbegin();
        }

        typename set_type::const_iterator send() const {
            return set.cend();
        }

        std::size_t size() const {
            return vec.size();
        }

        void clear() {
            vec.clear();
            set.clear();
        }
    };

}

#endif //ALIEN_VECSET_H