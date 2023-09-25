#pragma once

#include <Eigen/Dense>

#include "graphics/Enum.h"
#include "graphics/Image.h"
#include "scene/Camera.h"

#include "math/MathTypes.h"
#include "math/Matrix.h"

namespace hpl
{

class ImmediateDrawBatch {
    public:
        struct DebugDrawOptions {
        public:
            DebugDrawOptions() {}
            DepthTest m_depthTest = DepthTest::LessEqual;
            cMatrixf m_transform = cMatrixf(cMatrixf::Identity);
        };

//        inline bgfx::ViewId GetOrthographicView() const { return m_orthographicView; }
//        inline bgfx::ViewId GetPerspectiveView() const { return m_perspectiveView; }


        ImmediateDrawBatch();
        // takes 3 points and the other 1 is calculated
        [[deprecated("Use DrawQuad with Eigen")]]
        void DrawQuad(const cVector3f& v1, const cVector3f& v2, const cVector3f& v3,  const cVector3f& v4, const cVector2f& uv0,const cVector2f& uv1, hpl::Image* image , const cColor& aTint, const DebugDrawOptions& options = DebugDrawOptions());
        void DrawQuad(const Eigen::Vector3f& v1, const Eigen::Vector3f& v2, const Eigen::Vector3f& v3, const Eigen::Vector3f& v4, const Eigen::Vector2f& uv0, const Eigen::Vector2f& uv1, hpl::Image* image, const Eigen::Vector4f& aTint, const DebugDrawOptions& options = DebugDrawOptions());

        [[deprecated("Use DrawQuad with Eigen")]]
        void DrawQuad(const cVector3f& v1, const cVector3f& v2, const cVector3f& v3, const cVector3f& v4, const cColor& aColor, const DebugDrawOptions& options = DebugDrawOptions());
        void DrawQuad(const Eigen::Vector3f& v1, const Eigen::Vector3f& v2, const Eigen::Vector3f& v3, const Eigen::Vector3f& v4, const Eigen::Vector4f& color, const DebugDrawOptions& options = DebugDrawOptions());

        [[deprecated("Use DrawQuad with Eigen")]]
        void DrawTri(const cVector3f& v1, const cVector3f& v2, const cVector3f& v3, const cColor& color, const DebugDrawOptions& options = DebugDrawOptions());
        void DrawTri(const Eigen::Vector3f& v1, const Eigen::Vector3f& v2, const Eigen::Vector3f& v3, const Eigen::Vector4f& color, const DebugDrawOptions& options = DebugDrawOptions());
        void DrawPyramid(const cVector3f& baseCenter, const cVector3f& top, float halfWidth, const cColor& color, const DebugDrawOptions& options = DebugDrawOptions());

        [[deprecated("Use Drawbillboard with Eigen")]]
        void DrawBillboard(const cVector3f& pos, const cVector2f& size, const cVector2f& uv0, const cVector2f& uv1, hpl::Image* image, const cColor& aTint, const DebugDrawOptions& options = DebugDrawOptions());
        void DrawBillboard(const Eigen::Vector3f& pos, const Eigen::Vector2f& size, const Eigen::Vector2f& uv0, const Eigen::Vector2f& uv1, hpl::Image* image, const Eigen::Vector4f& aTint, const DebugDrawOptions& options = DebugDrawOptions());

        void DebugDraw2DLine(const cVector2f& start, const cVector2f& end, const cColor& color);
        void DebugDraw2DLineQuad(cRect2f rect, const cColor& color);

        // draws line
        void DebugSolidFromVertexBuffer( iVertexBuffer* vertexBuffer, const cColor& color, const DebugDrawOptions& options = DebugDrawOptions());
        void DebugWireFrameFromVertexBuffer( iVertexBuffer* vertexBuffer, const cColor& color, const DebugDrawOptions& options = DebugDrawOptions());
        void DebugDrawLine(const cVector3f& start, const cVector3f& end, const cColor& color, const DebugDrawOptions& options = DebugDrawOptions());
        void DebugDrawBoxMinMax(const cVector3f& start, const cVector3f& end, const cColor& color, const DebugDrawOptions& options = DebugDrawOptions());
        void DebugDrawSphere(const cVector3f& pos, float radius, const cColor& color, const DebugDrawOptions& options = DebugDrawOptions());
        void DebugDrawSphere(const cVector3f& pos, float radius, const cColor& c1, const cColor& c2, const cColor& c3, const DebugDrawOptions& options = DebugDrawOptions());
        void flush();

        // scale based on distance from camera
        static float BillboardScale(cCamera* apCamera, const Eigen::Vector3f& pos);
    private:

        struct ColorQuadRequest {
            DepthTest m_depthTest = DepthTest::LessEqual;
            Eigen::Vector3f m_v1;
            Eigen::Vector3f m_v2;
            Eigen::Vector3f m_v3;
            Eigen::Vector3f m_v4;
            Eigen::Vector4f m_color;
        };

        struct ColorTriRequest {
            DepthTest m_depthTest = DepthTest::LessEqual;
            Eigen::Vector3f m_v1;
            Eigen::Vector3f m_v2;
            Eigen::Vector3f m_v3;
            Eigen::Vector4f m_color;
        };

        struct UVQuadRequest {
            DepthTest m_depthTest = DepthTest::LessEqual;
            bool m_billboard;
            Eigen::Vector3f m_v1;
            Eigen::Vector3f m_v2;
            Eigen::Vector3f m_v3;
            Eigen::Vector3f m_v4;
            Eigen::Vector2f m_uv0;
            Eigen::Vector2f m_uv1;
            hpl::Image* m_uvImage;
            cColor m_color;
        };

        struct DebugMeshRequest {
            //GraphicsContext::LayoutStream m_layout;
            DepthTest m_depthTest;
            cMatrixf m_transform;
            cColor m_color;
        };

        struct LineSegmentRequest {
            DepthTest m_depthTest = DepthTest::LessEqual;
            Eigen::Vector3f m_start;
            Eigen::Vector3f m_end;
            Eigen::Vector4f m_color;
        };

        struct Line2DSegmentRequest {
            Eigen::Vector2f m_start;
            Eigen::Vector2f m_end;
            Eigen::Vector4f m_color;
        };
        // Orthgraphic projection
        std::vector<Line2DSegmentRequest> m_line2DSegments;
        std::vector<UVQuadRequest> m_uvQuads;
        std::vector<ColorQuadRequest> m_colorQuads;
        std::vector<LineSegmentRequest> m_lineSegments;
        std::vector<ColorTriRequest> m_colorTriangles;

        cMatrixf m_view;
        cMatrixf m_projection;

        // bgfx::ProgramHandle m_colorProgram = BGFX_INVALID_HANDLE;
        // bgfx::ProgramHandle m_uvProgram = BGFX_INVALID_HANDLE;
        // bgfx::ProgramHandle m_meshColorProgram = BGFX_INVALID_HANDLE;

        // bgfx::ViewId m_orthographicView = 0;
        // bgfx::ViewId m_perspectiveView = 0;

       // UniformWrapper<StringLiteral("s_diffuseMap"), bgfx::UniformType::Sampler> m_s_diffuseMap;
       // UniformWrapper<StringLiteral("u_normalMtx"),  bgfx::UniformType::Mat4> m_u_normalMtx;
       // UniformWrapper<StringLiteral("u_color"),      bgfx::UniformType::Vec4> m_u_color;

    };
}
