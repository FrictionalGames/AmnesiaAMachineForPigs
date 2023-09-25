
#pragma once

#include <algorithm>
#include <cstddef>
namespace hpl {
    template<size_t N>
    struct StringLiteral {
        constexpr StringLiteral(const char (&str)[N]) {
            std::copy_n(str, N, m_str);
        }

        constexpr size_t size() const {
            return N - 1;
        }

        char m_str[N];
    };

} // namespace hpl
