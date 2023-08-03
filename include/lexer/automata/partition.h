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

        std::size_t size(std::size_t index);

        std::size_t set(std::size_t element);

        std::ptrdiff_t get_first(std::size_t index);

        std::ptrdiff_t get_next(std::size_t element);

        bool no_marks(std::size_t index);

        void mark(std::size_t element);

        std::size_t split(std::size_t index);
    };

}

#endif //ALIEN_AUTOMATA_PARTITION_H