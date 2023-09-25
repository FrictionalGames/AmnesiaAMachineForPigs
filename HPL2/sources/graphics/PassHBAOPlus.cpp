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

#include <cstdlib>
#include <graphics/PassHBAOPlus.h>

#include "tinyimageformat_base.h"
#include <memory>

#include "Common_3/Resources/ResourceLoader/Interfaces/IResourceLoader.h"
#include "Common_3/Graphics/Interfaces/IGraphics.h"
#include <folly/small_vector.h>
#include <FixPreprocessor.h>

namespace hpl::renderer {

    PassHBAOPlus::PassHBAOPlus() {
        auto* forgeRenderer = Interface<ForgeRenderer>::Get();
        m_shaderDeinterleave.Load(forgeRenderer->Rend(),[&](Shader** handle) {
            ShaderLoadDesc loadDesc{};
            loadDesc.mStages[0].pFileName = "hbao_prepareNativeDepths.comp";
            addShader(forgeRenderer->Rend(),&loadDesc, handle);
            return true;
        });
        m_shaderCourseAO.Load(forgeRenderer->Rend(),[&](Shader** handle) {
            ShaderLoadDesc loadDesc{};
            loadDesc.mStages[0].pFileName = "hbao_courseAO.comp";
            addShader(forgeRenderer->Rend(),&loadDesc, handle);
            return true;
        });
        m_shaderReinterleave.Load(forgeRenderer->Rend(),[&](Shader** handle) {
            ShaderLoadDesc loadDesc{};
            loadDesc.mStages[0].pFileName = "hbao_reinterleave.comp";
            addShader(forgeRenderer->Rend(),&loadDesc, handle);
            return true;
        });
        m_pointSampler.Load(forgeRenderer->Rend(), [&](Sampler** sampler) {
            SamplerDesc samplerDesc = {};
            addSampler(forgeRenderer->Rend(), &samplerDesc,  sampler);
            return true;
        });
        m_rootSignature.Load(forgeRenderer->Rend(), [&](RootSignature** sig) {
            std::array samplerNames = {
                "pointSampler"
            };
            std::array samplers = {
                m_pointSampler.m_handle
            };

            std::array shaders = { m_shaderReinterleave.m_handle, m_shaderDeinterleave.m_handle, m_shaderCourseAO.m_handle };
            RootSignatureDesc rootSignatureDesc = {};
            rootSignatureDesc.ppStaticSamplers = samplers.data();
            rootSignatureDesc.mStaticSamplerCount = samplers.size();
            rootSignatureDesc.ppStaticSamplerNames = samplerNames.data();
            rootSignatureDesc.ppShaders = shaders.data();
            rootSignatureDesc.mShaderCount = shaders.size();
            addRootSignature(forgeRenderer->Rend(), &rootSignatureDesc, sig);
            return true;
        });

        m_pipelineDeinterleave.Load(forgeRenderer->Rend(), [&](Pipeline** handle) {
            PipelineDesc pipelineDesc = {};
            pipelineDesc.mType = PIPELINE_TYPE_COMPUTE;
            ComputePipelineDesc& computePipelineDesc = pipelineDesc.mComputeDesc;
            computePipelineDesc.pShaderProgram = m_shaderDeinterleave.m_handle;
            computePipelineDesc.pRootSignature = m_rootSignature.m_handle;
            addPipeline(forgeRenderer->Rend(), &pipelineDesc, handle);
            return true;
        });
        m_pipelineCourseAO.Load(forgeRenderer->Rend(), [&](Pipeline** handle) {
            PipelineDesc pipelineDesc = {};
            pipelineDesc.mType = PIPELINE_TYPE_COMPUTE;
            ComputePipelineDesc& computePipelineDesc = pipelineDesc.mComputeDesc;
            computePipelineDesc.pShaderProgram = m_shaderCourseAO.m_handle;
            computePipelineDesc.pRootSignature = m_rootSignature.m_handle;
            addPipeline(forgeRenderer->Rend(), &pipelineDesc, handle);
            return true;
        });
        m_pipelineReinterleave.Load(forgeRenderer->Rend(), [&](Pipeline** handle) {
            PipelineDesc pipelineDesc = {};
            pipelineDesc.mType = PIPELINE_TYPE_COMPUTE;
            ComputePipelineDesc& computePipelineDesc = pipelineDesc.mComputeDesc;
            computePipelineDesc.pShaderProgram = m_shaderReinterleave.m_handle;
            computePipelineDesc.pRootSignature = m_rootSignature.m_handle;
            addPipeline(forgeRenderer->Rend(), &pipelineDesc, handle);
            return true;
        });
    }
    void PassHBAOPlus::cmdDraw(
        const ForgeRenderer::Frame& frame,
        cFrustum* apFrustum,
        cViewport* viewport,
        Texture* depthBuffer,
        Texture* normalBuffer,
        Texture* outputBuffer) {
        auto* forgeRenderer = Interface<ForgeRenderer>::Get();
        auto common = m_boundViewportData.resolve(viewport);
        if(!common || (
            common->m_size.x != viewport->GetSizeU().x ||
            common->m_size.y != viewport->GetSizeU().y ||
            common->m_frustum != apFrustum) ) {

            auto viewportData = std::make_unique<ViewportData>();
            viewportData->m_frustum = apFrustum;
            viewportData->m_size = viewport->GetSizeU();

            for(auto& desc: viewportData->m_perFrameDescriptorSet) {
                desc.Load(forgeRenderer->Rend(),[&](DescriptorSet** set) {
                    DescriptorSetDesc setDesc = { m_rootSignature.m_handle, DESCRIPTOR_UPDATE_FREQ_PER_FRAME, 1};
                    addDescriptorSet(forgeRenderer->Rend(), &setDesc, set);
                    return true;
                });
            }
            viewportData->m_constBuffer.Load([&](Buffer** buffer) {
                BufferLoadDesc desc = {};
                desc.mDesc.mDescriptors = DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                desc.mDesc.mMemoryUsage = RESOURCE_MEMORY_USAGE_CPU_TO_GPU;
                desc.mDesc.mSize = sizeof(HBAORootConstant);
                desc.mDesc.mFlags = BUFFER_CREATION_FLAG_PERSISTENT_MAP_BIT;
                desc.pData = nullptr;
                desc.ppBuffer = buffer;
                addResource(&desc, nullptr);
                return true;
            });

            viewportData->m_constDescriptorSet.Load(forgeRenderer->Rend(),[&](DescriptorSet** set) {
                DescriptorSetDesc setDesc = { m_rootSignature.m_handle, DESCRIPTOR_UPDATE_FREQ_NONE, 1};
                addDescriptorSet(forgeRenderer->Rend(), &setDesc, set);
                return true;
            });

            HBAORootConstant constData = {};
            constData.viewportDim = int2(viewportData->m_size.x, viewportData->m_size.y );
            constData.viewportTexel = float2(1.0f / constData.viewportDim.x,1.0f / constData.viewportDim.y);

            constData.viewportQuarterDim = int2(viewportData->m_size.x / 4, viewportData->m_size.y / 4);
            constData.viewportQuarterTexel = float2(1.0f / constData.viewportQuarterDim.x, 1.0f / constData.viewportQuarterDim.y);

            auto projMatrix = apFrustum->GetProjectionMatrix();
            float tanHalfFOVY = 1.0f / projMatrix.m[1][1]; // = tanf( drawContext.Camera.GetYFOV( ) * 0.5f );
            float tanHalfFOVX = 1.0f / projMatrix.m[0][0]; // = tanHalfFOVY * drawContext.Camera.GetAspect( );
            constData.tanHalfFOV = tanHalfFOVY;
            constData.NDCtoViewMul = { tanHalfFOVX * 2.0f, tanHalfFOVY * -2.0f };
            constData.NDCtoViewAdd = { tanHalfFOVX * -1.0f, tanHalfFOVY  * 1.0f };
            constData.zNear = apFrustum->GetNearPlane();
            constData.zFar = apFrustum->GetFarPlane();
            constData.foregroundViewDepth = -1.0f;
            constData.backgroundViewDepth = -1.0f;
            constData.outputExp = 1.0f;
            SyncToken token = {};
            viewportData->m_preparedDepth.Load([&](Texture** texture) {
                TextureLoadDesc loadDesc = {};
                loadDesc.ppTexture = texture;
                TextureDesc textureDesc = {};
                textureDesc.mArraySize = PreparedDepthCount;
                textureDesc.mDepth = 1;
                textureDesc.mMipLevels = 1;
                textureDesc.mFormat = TinyImageFormat_R16_SFLOAT;
                textureDesc.mDescriptors = DESCRIPTOR_TYPE_TEXTURE | DESCRIPTOR_TYPE_RW_TEXTURE;
                textureDesc.mWidth = constData.viewportQuarterDim.x;
                textureDesc.mHeight = constData.viewportQuarterDim.y;
                textureDesc.mSampleCount = SAMPLE_COUNT_1;
                textureDesc.mSampleQuality = 0;
                textureDesc.mStartState = RESOURCE_STATE_UNORDERED_ACCESS;
                textureDesc.pName = "preparedDepth";
                loadDesc.pDesc = &textureDesc;
                addResource(&loadDesc, &token);
                return true;
            });

            viewportData->m_aoQurter.Load([&](Texture** texture) {
                TextureLoadDesc loadDesc = {};
                loadDesc.ppTexture = texture;
                TextureDesc textureDesc = {};
                textureDesc.mArraySize = PreparedDepthCount;
                textureDesc.mDepth = 1;
                textureDesc.mMipLevels = 1;
                textureDesc.mFormat = TinyImageFormat_R32_SFLOAT;
                textureDesc.mDescriptors = DESCRIPTOR_TYPE_TEXTURE | DESCRIPTOR_TYPE_RW_TEXTURE;
                textureDesc.mWidth = constData.viewportQuarterDim.x;
                textureDesc.mHeight = constData.viewportQuarterDim.y;
                textureDesc.mSampleCount = SAMPLE_COUNT_1;
                textureDesc.mSampleQuality = 0;
                textureDesc.mStartState = RESOURCE_STATE_UNORDERED_ACCESS;
                textureDesc.pName = "aoQuarter";
                loadDesc.pDesc = &textureDesc;
                addResource(&loadDesc, &token);
                return true;
            });

            size_t viewportIndex = viewport->GetHandle();
            BufferUpdateDesc updateDesc = { viewportData->m_constBuffer.m_handle, 0, sizeof(HBAORootConstant) };
            beginUpdateResource(&updateDesc);
            (*reinterpret_cast<HBAORootConstant *>(updateDesc.pMappedData)) = constData;
            endUpdateResource(&updateDesc, &token);
            waitForToken(&token);
            {
                std::array<DescriptorData, 3> descriptorData = {};
                descriptorData[0].pName = "constUniformBuffer";
                descriptorData[0].ppBuffers = &viewportData->m_constBuffer.m_handle;
                descriptorData[1].pName = "prepareDepths";
                descriptorData[1].ppTextures = &viewportData->m_preparedDepth.m_handle;
                descriptorData[2].pName = "aoQurter";
                descriptorData[2].ppTextures = &viewportData->m_aoQurter.m_handle;
                updateDescriptorSet(forgeRenderer->Rend(), 0, viewportData->m_constDescriptorSet.m_handle, descriptorData.size(), descriptorData.data());
            }

            common = m_boundViewportData.update(viewport, std::move(viewportData));
        }
        {
            std::array<DescriptorData, 3> params = {};
            params[0].pName = "gOutput";
            params[0].ppTextures = &outputBuffer;
            params[1].pName = "depthInput";
            params[1].ppTextures = &depthBuffer;
            params[2].pName = "normalInput";
            params[2].ppTextures = &normalBuffer;

            updateDescriptorSet(
                frame.m_renderer->Rend(),
                0,
                common->m_perFrameDescriptorSet[frame.m_frameIndex].m_handle,
                params.size(),
                params.data());
        }


        auto viewportSize = viewport->GetSizeU();
        uint2 quarterViewportSize = uint2(viewportSize.x / 4, viewportSize.y / 4);

        cmdBindDescriptorSet(frame.m_cmd, 0, common->m_perFrameDescriptorSet[frame.m_frameIndex].m_handle);
        cmdBindDescriptorSet(frame.m_cmd, 0, common->m_constDescriptorSet.m_handle);

        cmdBindPipeline(frame.m_cmd, m_pipelineDeinterleave.m_handle);
        cmdDispatch(frame.m_cmd, static_cast<uint32_t>(quarterViewportSize.x / 16) + 1 , static_cast<uint32_t>(quarterViewportSize.y / 16) + 1, 1);
        {
            std::array textureBarrier = {
                TextureBarrier { common->m_preparedDepth.m_handle, RESOURCE_STATE_UNORDERED_ACCESS, RESOURCE_STATE_UNORDERED_ACCESS },
            };
            cmdResourceBarrier(frame.m_cmd, 0, NULL,  textureBarrier.size(), textureBarrier.data(), 0, NULL);
        }

        cmdBindPipeline(frame.m_cmd, m_pipelineCourseAO.m_handle);
        cmdDispatch(frame.m_cmd, static_cast<uint32_t>(quarterViewportSize.x / 16) + 1 , static_cast<uint32_t>(quarterViewportSize.y / 16) + 1, PreparedDepthCount);

        std::array textureBarrier = {
            TextureBarrier { common->m_aoQurter.m_handle, RESOURCE_STATE_UNORDERED_ACCESS, RESOURCE_STATE_UNORDERED_ACCESS },
        };
        cmdBindPipeline(frame.m_cmd, m_pipelineReinterleave.m_handle);
        cmdDispatch(frame.m_cmd, static_cast<uint32_t>(quarterViewportSize.x / 16) + 1 , static_cast<uint32_t>(quarterViewportSize.y / 16) + 1, PreparedDepthCount);
    }


}; // namespace hpl::renderer
