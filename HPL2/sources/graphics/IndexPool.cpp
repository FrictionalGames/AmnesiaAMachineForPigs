#include "graphics/IndexPool.h"
#include <algorithm>
#include <cstdint>

#include "Common_3/Utilities/Interfaces/ILog.h"
#include <FixPreprocessor.h>

namespace hpl {

    IndexPool::IndexPool(uint32_t reserve) {
        m_avaliable.push_back({0, reserve - 1});
    }

    uint32_t IndexPool::requestId() {
        if(m_avaliable.size() > 0) {
            auto& entry = m_avaliable[m_avaliable.size() - 1];
            if(entry.m_end == entry.m_start) {
                const uint32_t res = entry.m_start;
                m_avaliable.pop_back();
                return res;
            }
            const uint32_t res = entry.m_end;
            entry.m_end--;
            return res;
        }
        return UINT32_MAX;
    }

    void IndexPool::returnId(uint32_t index) {
        auto lower = m_avaliable.begin();
        auto upper = m_avaliable.end();
        auto mid = lower;
        while(lower != upper) {
            mid = lower + ((upper - lower) / 2);
            if(index >= mid->m_start && index <= mid->m_end) {
                assert(false && "found within range");
                return;
            } else if(index < mid->m_start) {
                upper = mid;
            } else {
                lower = mid + 1;
            }
        }

        if(index < mid->m_start) {
            if(mid->m_start - index == 1) {
                mid->m_start--;
                if(mid - 1 >= m_avaliable.begin() &&
                        (mid - 1)->m_end + 1 == mid->m_start) {
                    (mid - 1)->m_end = mid->m_end;
                    m_avaliable.erase(mid);
                }
            } else {
                m_avaliable.insert(mid, {index, index});
            }
        } else if(index > mid->m_end ) {
            if(index - mid->m_end == 1) {
                mid->m_end++;
                if(mid + 1 < m_avaliable.end()
                    && (mid + 1)->m_start + 1 == mid->m_end) {
                    (mid + 1)->m_start = mid->m_start;
                    m_avaliable.erase(mid);
                }
            } else {
                m_avaliable.insert(mid + 1, {index, index});
            }
        }
    }
}
