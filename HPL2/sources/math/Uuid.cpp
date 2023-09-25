#include <array>
#include <math/Uuid.h>

namespace hpl {

    bool operator==(const Uuid& lhs, const Uuid& rhs) {
        for(size_t i = 0; i < 16; i++) {
            if(lhs.m_data[i] != rhs.m_data[i]) {
                return false;
            }
        }
        return true;
    }
}
