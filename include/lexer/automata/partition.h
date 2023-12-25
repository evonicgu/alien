#ifndef ALIEN_AUTOMATA_PARTITION_H
#define ALIEN_AUTOMATA_PARTITION_H

#include <vector>

namespace alien::lexer::automata::partition {

    /**
     * Refinable partition data structure described in paper https://arxiv.org/pdf/0802.2826.pdf
     * By Antti Valmari, Petri Lehtinen
     */
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

        /**
         * Returns the number of elements in the set with index 'index'
         */
        std::size_t size(std::size_t index);

        /**
         * Returns the index of the set that element 'element' belongs to
         */
        std::size_t set(std::size_t element);

        /**
         * Returns first element of the set with index 'index'
         * To be continued by repetitive calls to 'get_next'
         */
        std::ptrdiff_t get_first(std::size_t index);

        /**
         * Returns next element of the set with index 'index'
         * Each element will be returned exactly once, however, the order is unspecified
         * When -1 is returned, no more elements are left
         * While scanning a set, mark and split must not be called
         */
        std::ptrdiff_t get_next(std::size_t element);

        /**
         * Returns True if and only if none of the elements of set with index 'index' is marked
         */
        bool no_marks(std::size_t index);

        /**
         * Marks the element 'element' for splitting of a set
         */
        void mark(std::size_t element);

        /**
         * If either none or all elements of set with index 'index' have been marked, returns 0. Otherwise
         * removes the marked elements from the set, makes a new set of the marked elements,
         * and returns its index. In both cases, unmarks all the elements in the set or sets
         */
        std::size_t split(std::size_t index);
    };

}

#endif //ALIEN_AUTOMATA_PARTITION_H