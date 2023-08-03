#ifndef ALIEN_UNORDERED_VECSET_H
#define ALIEN_UNORDERED_VECSET_H

namespace alien::util {

    template<typename T>
    struct access_eq_pred {
        const std::vector<T>* arr;

        using is_transparent = std::true_type;

        explicit access_eq_pred(const std::vector<T>* arr) : arr(arr) {}

        template<typename V>
        bool operator()(const V& lhs, const std::size_t rhs) const {
            return lhs == arr->at(rhs);
        }

        template<typename V>
        bool operator()(const std::size_t lhs, const V& rhs) const {
            return arr->at(lhs) == rhs;
        }

        bool operator()(const std::size_t lhs, std::size_t rhs) const {
            return arr->at(lhs) == arr->at(rhs);
        }
    };

    template<typename T, typename Hash>
    struct access_hasher {
        const std::vector<T>* arr;
        Hash hasher;

        using is_transparent = std::true_type;

        explicit access_hasher(const std::vector<T>* arr) : arr(arr) {}

        std::size_t operator()(const std::size_t value) const {
            return hasher(arr->at(value));
        }

        std::size_t operator()(const T& value) const {
            return hasher(value);
        }
    };

    template<typename T, typename Hash = std::hash<T>>
    class hash_vecset {
        using set_type = std::unordered_set<std::size_t, access_hasher<T, Hash>, access_eq_pred<T>>;

        std::vector<T> vec;
        set_type set{0, access_hasher<T, Hash>(&vec), access_eq_pred<T>(&vec),};

    public:
        hash_vecset() = default;

        hash_vecset(std::initializer_list<T>&& list) {
            vec.reserve(list.size());

            for (auto it : list) {
                vec.push_back(std::move(it));
                set.insert(vec.size() - 1);
            }
        }

        explicit hash_vecset(const std::vector<T>& values) {
            vec.reserve(values.size());

            for (const T& value : values) {
                vec.push_back(value);
                set.insert(vec.size() - 1);
            }
        }

        explicit hash_vecset(std::vector<T>&& values) {
            vec.reserve(values.size());

            for (T& value : values) {
                vec.push_back(std::move(value));
                set.insert(vec.size() - 1);
            }
        }

        explicit hash_vecset(const std::unordered_set<T, Hash>& values) {
            vec.reserve(values.size());

            for (const T& value : values) {
                vec.push_back(value);
                set.insert(vec.size() - 1);
            }
        }

        explicit hash_vecset(std::unordered_set<T, Hash>&& values) {
            vec.reserve(values.size());

            for (auto& value : values) {
                vec.push_back(std::move(const_cast<T&&>(value)));
                set.insert(vec.size() - 1);
            }
        }

        hash_vecset(const hash_vecset<T>& other) {
            vec = other.vec;

            for (std::size_t i : other.set) {
                set.insert(i);
            }
        }

        hash_vecset(hash_vecset<T>&& other) noexcept {
            vec = std::move(other.vec);

            for (std::size_t i : other.set) {
                set.insert(i);
            }

            other.set.clear();
        }

        hash_vecset<T>& operator=(const hash_vecset<T>& other) {
            if (this == &other) {
                return *this;
            }

            vec.clear();
            set.clear();

            vec = other.vec;
            for (std::size_t i : other.set) {
                set.insert(i);
            }

            return *this;
        }

        hash_vecset<T>& operator=(hash_vecset<T>&& other) noexcept {
            if (this == &other) {
                return *this;
            }

            vec.clear();
            set.clear();

            vec = std::move(other.vec);
            for (std::size_t i : other.set) {
                set.insert(i);
            }

            other.set.clear();

            return *this;
        }

        explicit operator std::unordered_set<T, Hash>() {
            std::unordered_set<T, Hash> out;

            for (std::size_t index : set) {
                out.insert(std::move(vec[index]));
            }

            return out;
        }

        explicit operator std::vector<T>() {
            return std::move(this->vec);
        }

        std::size_t push_back(const T& value) {
            auto it = set.find(value);

            if (it != set.end()) {
                return *it;
            }

            vec.push_back(value);
            set.insert(vec.size() - 1);

            return vec.size() - 1;
        }

        std::size_t push_back(T&& value) {
            auto it = set.find(value);

            if (it != set.end()) {
                return *it;
            }

            vec.push_back(std::move(value));
            set.insert(vec.size() - 1);

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

        template<typename V>
        typename std::vector<T>::iterator find(const V& value) {
            auto it = set.find(value);

            if (it == set.end()) {
                return vend();
            }

            return vbegin() + *it;
        }

        template<typename V>
        typename std::vector<T>::const_iterator find(const V& value) const {
            auto it = set.find(value);

            if (it == set.end()) {
                return cvend();
            }

            return cvbegin() + *it;
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

        typename set_type::iterator sbegin() const {
            return set.begin();
        }

        typename set_type::iterator send() const {
            return set.end();
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

#endif //ALIEN_UNORDERED_VECSET_H