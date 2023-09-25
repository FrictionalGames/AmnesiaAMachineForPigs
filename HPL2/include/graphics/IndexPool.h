#pragma once

#include <cstdint>

#include <folly/small_vector.h>
#include <set>
#include <vector>

namespace hpl {
    class IndexPool {
    public:
        IndexPool(uint32_t reserve);

        uint32_t requestId();
        void returnId(uint32_t);
    private:
        struct IdRange {
            uint32_t m_start;
            uint32_t m_end;
        };
        folly::small_vector<IdRange, 256> m_avaliable;
    };

    class IndexPoolHandle {
    public:
        explicit inline IndexPoolHandle(): m_pool(nullptr){}
        explicit inline IndexPoolHandle(IndexPool* pool): m_pool(pool) {
            m_index = m_pool->requestId();
        }
        inline ~IndexPoolHandle() {
            if(m_index != UINT32_MAX) {
                m_pool->returnId(m_index);
            }
            m_index = UINT32_MAX;
        }

        explicit operator uint32_t() const {
            return m_index;
        }
        inline bool isValid() {
            return m_index != UINT32_MAX;
        }
        inline uint32_t get() {
            return m_index;
        }
        inline IndexPoolHandle(IndexPoolHandle&& handle) {
            if(m_index != UINT32_MAX) {
                m_pool->returnId(m_index);
            }
            m_index = handle.m_index;
            m_pool = handle.m_pool;
            handle.m_index = UINT32_MAX;
        }
        inline IndexPoolHandle(IndexPoolHandle& handle) = delete;

        inline IndexPoolHandle& operator=(IndexPoolHandle& handle) = delete;
        inline IndexPoolHandle& operator=(IndexPoolHandle&& handle) {
            if (m_index != UINT32_MAX) {
                m_pool->returnId(m_index);
            }
            m_index = handle.m_index;
            m_pool = handle.m_pool;
            handle.m_index = UINT32_MAX;
            return *this;
        }
    private:
        uint32_t m_index = UINT32_MAX;
        IndexPool* m_pool = nullptr;
    };

}

