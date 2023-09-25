#pragma once

#include <Common_3/Utilities/Math/MathTypes.h>
#include <FixPreprocessor.h>

namespace hpl::math {
        class PlaneF {
        public:
            static PlaneF CreateFromNormalPoint(const Vector3& normal, const Vector3& point) {
                return PlaneF(Vector4(
                    normal.getX(),
                    normal.getY(),
                    normal.getZ(),
                    -dot(normal, point)));
            }

            static PlaneF CreateFromPoints(const Vector3& p0, const Vector3& p1, const Vector3& p2 ) {
                Vector3 p1p0 = p1 - p0;
                Vector3 p2p0 = p2 - p0;
                Vector3 normal = normalize(cross(p1p0 , p2p0));
                return PlaneF(Vector4(normal, -dot(normal, p0)));
            }

           explicit inline PlaneF(const PlaneF& plane): m_value(plane.m_value) {}
           explicit inline PlaneF(): m_value(Vector4(0,0,0,0)) {}
           explicit inline PlaneF(const Vector4& value): m_value(value) {}
           explicit inline PlaneF(float na, float nb, float nc, float d): m_value(Vector4(na,nb,nc, d)) {}

            inline PlaneF operator*(const float& value) const {
                return PlaneF(m_value * value);
            }

            inline PlaneF& operator*=(const float& value) {
                m_value *= value;
                return *this;
            }

            inline Vector3 Normal() const {
                return m_value.getXYZ();
            }

            inline Vector4 Coefficients() const {
                return m_value;
            }
        private:
            Vector4 m_value;
        };
}
