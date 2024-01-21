
#pragma once

inline static bool fcomp(const float lhs, const float rhs, const int precision = 2) {
    auto digits = precision * 10;
    auto lhs_int = static_cast<int32_t>(roundf(lhs * digits));
    auto rhs_int = static_cast<int32_t>(roundf(rhs * digits));
    return lhs_int == rhs_int;
}
