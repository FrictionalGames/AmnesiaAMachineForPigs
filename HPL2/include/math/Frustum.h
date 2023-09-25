#pragma once

#include "Common_3/Utilities/Math/MathTypes.h"
#include <FixPreprocessor.h>

namespace hpl::math {

    class Frustum {
    public:
        static Frustum CreateProjection();
        static Frustum CreateOrthographic();
        Frustum();

        Vector3 GetForward();

    private:
		Matrix4 m_projection;
		Matrix4 m_viewProjection;
		Matrix4 m_view;
    };

}
