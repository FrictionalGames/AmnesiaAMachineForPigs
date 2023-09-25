
#pragma once


#include "Common_3/Utilities/Math/MathTypes.h"
#include "graphics/ForgeRenderer.h"
#include "impl/LegacyVertexBuffer.h"
#include <FixPreprocessor.h>

#include "math/MathTypes.h"
#include <graphics/ForgeHandles.h>




class ForgeImmediateDraw {
public:
    struct DebugDrawOptions {
    public:
        DebugDrawOptions() {}
        CompareMode m_depthTest = CompareMode::CMP_LEQUAL;
        mat4 m_transform = mat4().identity();
    };
    void DrawQuad(const vec3& v1, const vec3& v2, const vec3& v3, const vec3& v4, const vec2& uv0, const vec2& uv1, hpl::ForgeTextureHandle& image, const vec4& aTint, const DebugDrawOptions& options = DebugDrawOptions());
    void DrawQuad(const vec3& v1, const vec3& v2, const vec3& v3, const vec3& v4, const vec4& color, const DebugDrawOptions& options = DebugDrawOptions());

    void DrawTri(const vec3& v1, const vec3& v2, const vec3& v3, const vec4& color, const DebugDrawOptions& options = DebugDrawOptions());
    void DrawPyramid(const vec3& baseCenter, const vec3& top, float halfWidth, const vec4& color, const DebugDrawOptions& options = DebugDrawOptions());

    void DrawBillboard(const vec3& pos, const vec2& size, const vec2& uv0, const vec2& uv1, hpl::ForgeRenderer& image, const vec4& aTint, const DebugDrawOptions& options = DebugDrawOptions());

    void DebugDraw2DLine(const vec2& start, const vec2& end, const vec4& color);
    void DebugDraw2DLineQuad(hpl::cRect2f rect, const vec4& color);

    // draws line
    void DebugSolidFromVertexBuffer( hpl::LegacyVertexBuffer* vertexBuffer, const vec4& color, const DebugDrawOptions& options = DebugDrawOptions());
    void DebugWireFrameFromVertexBuffer(const hpl::LegacyVertexBuffer& vertexBuffer, const vec4& color, const DebugDrawOptions& options = DebugDrawOptions());
    void DebugDrawLine(const vec3& start, const vec3& end, const vec4& color, const DebugDrawOptions& options = DebugDrawOptions());
    void DebugDrawBoxMinMax(const vec3& start, const vec3& end, const vec4& color, const DebugDrawOptions& options = DebugDrawOptions());
    void DebugDrawSphere(const vec3& pos, float radius, const vec4& color, const DebugDrawOptions& options = DebugDrawOptions());
    void DebugDrawSphere(const vec3& pos, float radius, const vec4& c1, const vec4& c2, const vec4& c3, const DebugDrawOptions& options = DebugDrawOptions());
    void flush();
};
