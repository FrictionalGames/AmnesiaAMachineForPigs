// This code contains NVIDIA Confidential Information and is disclosed to you
// under a form of NVIDIA software license agreement provided separately to you.
//
// Notice
// NVIDIA Corporation and its licensors retain all intellectual property and
// proprietary rights in and to this software and related documentation and
// any modifications thereto. Any use, reproduction, disclosure, or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA Corporation is strictly prohibited.
//
// ALL NVIDIA DESIGN SPECIFICATIONS, CODE ARE PROVIDED "AS IS.". NVIDIA MAKES
// NO WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES OF NONINFRINGEMENT,
// MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Information and code furnished is believed to be accurate and reliable.
// However, NVIDIA Corporation assumes no responsibility for the consequences of use of such
// information or for any infringement of patents or other rights of third parties that may
// result from its use. No license is granted by implication or otherwise under any patent
// or patent rights of NVIDIA Corporation. Details are subject to change without notice.
// This code supersedes and replaces all information previously supplied.
// NVIDIA Corporation products are not authorized for use as critical
// components in life support devices or systems without express written approval of
// NVIDIA Corporation.
//
// Copyright (c) 2008-2021 NVIDIA Corporation. All rights reserved.

// Copyright 2023 Michael Pollind
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "graphics/ForgeHandles.h"
#include "scene/Viewport.h"

#include "Common_3/Graphics/Interfaces/IGraphics.h"
#include "Common_3/Utilities/Math/MathTypes.h"
#include "FixPreprocessor.h"

namespace hpl::renderer {

class PassHBAOPlus final {
public:
    static constexpr uint32_t PreparedDepthCount = 16; // 16 1/4th size depth buffers

    struct ViewportData {
        ViewportData() = default;
        ViewportData(const ViewportData&) = delete;
        ViewportData(ViewportData&& buffer)
            : m_preparedDepth(std::move(buffer.m_preparedDepth))
            , m_perFrameDescriptorSet(std::move(buffer.m_perFrameDescriptorSet))
            , m_constDescriptorSet(std::move(buffer.m_constDescriptorSet))
            , m_size(buffer.m_size)
            , m_frustum(buffer.m_frustum)
            , m_constBuffer(buffer.m_constBuffer) {
        }

        ViewportData& operator=(const ViewportData&) = delete;

        void operator=(ViewportData&& buffer) {
            m_size = buffer.m_size;
            m_preparedDepth = std::move(buffer.m_preparedDepth);
            m_perFrameDescriptorSet = std::move(buffer.m_perFrameDescriptorSet);
            m_constDescriptorSet = std::move(buffer.m_constDescriptorSet);
            m_preparedDepth = std::move(buffer.m_preparedDepth);
            m_constBuffer = std::move(buffer.m_constBuffer);
            m_frustum =  buffer.m_frustum;
        }

        // 16 depth intermediary buffers
        SharedTexture m_preparedDepth = {};
        SharedTexture m_aoQurter = {};
        SharedDescriptorSet m_constDescriptorSet = {};
        uint2 m_size = uint2(0, 0);
        cFrustum* m_frustum = nullptr;
        std::array<SharedDescriptorSet, ForgeRenderer::SwapChainLength> m_perFrameDescriptorSet = {};
        SharedBuffer m_constBuffer;
    };

    struct HBAORootConstant {
        float2 NDCtoViewMul;
        float2 NDCtoViewAdd;

        float tanHalfFOV;
        float zFar;
        float zNear;
        float outputExp;

        int2 viewportDim;
        float2 viewportTexel;

        int2 viewportQuarterDim;
        float2 viewportQuarterTexel;

        float backgroundViewDepth;
        float foregroundViewDepth;
        float pad[2];
    };

    PassHBAOPlus();
    void cmdDraw(
        const ForgeRenderer::Frame& frame,
        cFrustum* apFrustum,
        cViewport* viewport,
        Texture* depthBuffer,
        Texture* normalBuffer,
        Texture* outputBuffer);

private:
    UniqueViewportData<ViewportData> m_boundViewportData;
    SharedRootSignature m_rootSignature;

    SharedShader m_shaderDeinterleave;
    SharedPipeline m_pipelineDeinterleave;

    SharedShader m_shaderCourseAO;
    SharedPipeline m_pipelineCourseAO;

    SharedShader m_shaderReinterleave;
    SharedPipeline m_pipelineReinterleave;

    SharedSampler m_pointSampler;
};

}

