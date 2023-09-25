#pragma once

#include "Common_3/Utilities/Math/MathTypes.h"
#include "FixPreprocessor.h"

namespace hpl::math {
    class SphereF {
    public:
        SphereF()
            : m_position()
            , m_radius(0) {
        }

        static SphereF CreateUnitShere() {
           return SphereF(Vector3(0,0,0), 1.0f);
        }

        explicit SphereF(const Vector3& position, float radius)
            : m_position(position)
            , m_radius(radius) {
        }

        inline void SetPosition(const Vector3& position) {
            m_position = position;
        }
        inline void SetRadius(float radius) {
            m_radius = radius;
        }

        inline const Vector3& Position() const { return m_position; }
        inline const float Radius() const { return m_radius; }
    private:
        Vector3 m_position;
        float m_radius;
    };
}
