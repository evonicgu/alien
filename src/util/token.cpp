#include "util/token.h"

namespace alien::util {

    pos pos::forwards(std::size_t lines, std::size_t columns) const {
        return pos{line + lines, column + columns};
    }

    pos pos::backwards(std::size_t lines, std::size_t columns) const {
        return pos{lines >= line ? 0 : line - lines, columns >= column ? 0 : column - columns};
    }

    bool operator==(const pos& lhs, const pos& rhs) {
        return lhs.line == rhs.line && lhs.column == rhs.column;
    }

}