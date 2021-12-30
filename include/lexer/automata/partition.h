#ifndef ALIEN_AUTOMATA_PARTITION_H
#define ALIEN_AUTOMATA_PARTITION_H

#include <vector>

namespace alien::lexer::automata::partition {

    struct partition {
        std::vector<std::size_t> elements, loc, sidx, first, last, mid;
        std::size_t sets = 1;

        explicit partition(std::size_t max) {
            elements.resize(max);
            loc.resize(max);
            sidx.resize(max);
            first.resize(max);
            last.resize(max);
            mid.resize(max);

            first[0] = mid[0] = 0;
            last[0] = max;

            for (std::size_t i = 0; i < max; ++i) {
                elements[i] = loc[i] = i;
                sidx[i] = 0;
            }
        }

        std::size_t size(std::size_t index) {
            return last[index] - first[index];
        }

        std::size_t set(std::size_t element) {
            return sidx[element];
        }

        std::ptrdiff_t get_first(std::size_t index) {
            return (std::ptrdiff_t) elements[first[index]];
        }

        std::ptrdiff_t get_next(std::size_t element) {
            if (loc[element] + 1 >= last[sidx[element]]) {
                return -1;
            }

            return (std::ptrdiff_t) elements[loc[element] + 1];
        }

        bool no_marks(std::size_t index) {
            return mid[index] == first[index];
        }

        void mark(std::size_t element) {
            std::size_t s = sidx[element];
            std::size_t l = loc[element], m = mid[s];

            if (l >= m) {
                elements[l] = elements[m];
                loc[elements[l]] = l;

                elements[m] = element;
                loc[element] = m;
                mid[s] = m + 1;
            }
        }

        std::size_t split(std::size_t index) {
            if (mid[index] == last[index]) {
                mid[index] = first[index];
            }

            if (mid[index] == first[index]) {
                return 0;
            }
            ++sets;

            first[sets - 1] = first[index];
            mid[sets - 1] = first[index];
            last[sets - 1] = mid[index];
            first[index] = mid[index];

            for (std::size_t l = first[sets - 1]; l < last[sets - 1]; ++l) {
                sidx[elements[l]] = sets - 1;
            }

            return sets - 1;
        }
    };

}

#endif //ALIEN_AUTOMATA_PARTITION_H