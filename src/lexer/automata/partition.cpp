#include "lexer/automata/partition.h"

namespace alien::lexer::automata::partition {

    std::size_t partition::size(std::size_t index) {
        return last[index] - first[index];
    }

    std::size_t partition::set(std::size_t element) {
        return sidx[element];
    }

    std::ptrdiff_t partition::get_first(std::size_t index) {
        return (std::ptrdiff_t) elements[first[index]];
    }

    std::ptrdiff_t partition::get_next(std::size_t element) {
        if (loc[element] + 1 >= last[sidx[element]]) {
            return -1;
        }

        return (std::ptrdiff_t) elements[loc[element] + 1];
    }

    bool partition::no_marks(std::size_t index) {
        return mid[index] == first[index];
    }

    void partition::mark(std::size_t element) {
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

    std::size_t partition::split(std::size_t index) {
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

}