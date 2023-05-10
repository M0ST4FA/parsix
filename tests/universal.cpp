#include "universal.h"

std::string toString(const LASharedInfo::Terminal term)
{
    using enum LASharedInfo::Terminal;

    switch (term) {
    case T_A:
        return "A";
    case T_B:
        return "B";
    case T_ALPHA:
        return "ALPHA";

    case T_EQUAL:
        return "=";

    case T_ID:
        return "ID";

    case T_EPSILON:
        return "EPSILON";

    case T_NUM:
        return "NUM";

    case T_EOF:
        return "EOF";

    default:
        std::cerr << "\nUNRECOGNIZED TERMINAL IN FUNCTION `toString(const LASharedInfo::Terminal)`.\n";
        std::abort();
    }
}
