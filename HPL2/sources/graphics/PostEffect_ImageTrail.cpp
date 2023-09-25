/*
 * Copyright © 2009-2020 Frictional Games
 *
 * This file is part of Amnesia: The Dark Descent.
 *
 * Amnesia: The Dark Descent is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Amnesia: The Dark Descent is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Amnesia: The Dark Descent.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "graphics/PostEffect_ImageTrail.h"

#include "Common_3/Graphics/Interfaces/IGraphics.h"
#include "graphics/Enum.h"
#include "graphics/ForgeHandles.h"
#include "graphics/Graphics.h"

#include "graphics/FrameBuffer.h"
#include "graphics/Image.h"
#include "graphics/LowLevelGraphics.h"
#include "graphics/PostEffectComposite.h"
#include "graphics/RenderTarget.h"
#include "graphics/Texture.h"

#include "math/MathTypes.h"
#include "system/PreprocessParser.h"
#include <memory>

namespace hpl {
    cPostEffectType_ImageTrail::cPostEffectType_ImageTrail(cGraphics* apGraphics, cResources* apResources)
        : iPostEffectType("ImageTrail", apGraphics, apResources) {
        auto* forgeRenderer = Interface<ForgeRenderer>::Get();
        m_shader.Load(forgeRenderer->Rend(),[&](Shader** shader) {
            ShaderLoadDesc loadDesc{};
            loadDesc.mStages[0].pFileName = "fullscreen.vert";
            loadDesc.mStages[1].pFileName = "image_trail_posteffect.frag";
            addShader(forgeRenderer->Rend(),&loadDesc, shader);
            return true;
        });
        m_inputSampler.Load(forgeRenderer->Rend(), [&](Sampler** sampler) {
            SamplerDesc samplerDesc = {};
            addSampler(forgeRenderer->Rend(), &samplerDesc,  sampler);
            return true;
        });

        m_rootSignature.Load(forgeRenderer->Rend(), [&](RootSignature** rootSignature) {
            std::array shaders = {
                m_shader.m_handle
            };
            RootSignatureDesc rootDesc{};
            const char* pStaticSamplers[] = { "inputSampler" };
            rootDesc.ppShaders = shaders.data();
            rootDesc.mShaderCount = shaders.size();
            rootDesc.mStaticSamplerCount = 1;
            rootDesc.ppStaticSamplers = &m_inputSampler.m_handle;
            rootDesc.ppStaticSamplerNames = pStaticSamplers;
            addRootSignature(forgeRenderer->Rend(), &rootDesc, rootSignature);
            return true;
        });

        for(auto& descSet: m_perFrameDescriptorSet) {
            descSet.Load(forgeRenderer->Rend(), [&](DescriptorSet** set) {
                DescriptorSetDesc setDesc = { m_rootSignature.m_handle, DESCRIPTOR_UPDATE_FREQ_PER_FRAME, cPostEffectType_ImageTrail::DescSetSize };
                addDescriptorSet(forgeRenderer->Rend(), &setDesc, set);
                return true;
            });
        }
        m_pipeline.Load(forgeRenderer->Rend(), [&](Pipeline** pipeline) {
            BlendStateDesc blendStateDesc{};
            blendStateDesc.mSrcFactors[0] = BC_SRC_ALPHA;
            blendStateDesc.mDstFactors[0] = BC_ONE_MINUS_SRC_ALPHA;
            blendStateDesc.mBlendModes[0] = BM_ADD;
            blendStateDesc.mSrcAlphaFactors[0] = BC_SRC_ALPHA;
            blendStateDesc.mDstAlphaFactors[0] = BC_ONE_MINUS_SRC_ALPHA;
            blendStateDesc.mBlendAlphaModes[0] = BM_ADD;
            #ifdef USE_THE_FORGE_LEGACY
                blendStateDesc.mMasks[0] = ALL;
            #else
                blendStateDesc.mColorWriteMasks[0] = ColorMask::COLOR_MASK_ALL;
            #endif

            blendStateDesc.mRenderTargetMask = BLEND_STATE_TARGET_0;
            blendStateDesc.mIndependentBlend = false;

            TinyImageFormat inputFormat = TinyImageFormat_R8G8B8A8_UNORM;
            DepthStateDesc depthStateDisabledDesc = {};
            depthStateDisabledDesc.mDepthWrite = false;
            depthStateDisabledDesc.mDepthTest = false;

            RasterizerStateDesc rasterStateNoneDesc = {};
            rasterStateNoneDesc.mCullMode = CULL_MODE_NONE;

            PipelineDesc pipelineDesc = {};
            pipelineDesc.mType = PIPELINE_TYPE_GRAPHICS;
            GraphicsPipelineDesc& graphicsPipelineDesc = pipelineDesc.mGraphicsDesc;
            graphicsPipelineDesc.mPrimitiveTopo = PRIMITIVE_TOPO_TRI_LIST;
            graphicsPipelineDesc.pShaderProgram = m_shader.m_handle;
            graphicsPipelineDesc.pRootSignature = m_rootSignature.m_handle;
            graphicsPipelineDesc.mRenderTargetCount = 1;
            graphicsPipelineDesc.mDepthStencilFormat = TinyImageFormat_UNDEFINED;
            graphicsPipelineDesc.pVertexLayout = NULL;
            graphicsPipelineDesc.pRasterizerState = &rasterStateNoneDesc;
            graphicsPipelineDesc.pDepthState = &depthStateDisabledDesc;
            graphicsPipelineDesc.pBlendState = NULL;
            graphicsPipelineDesc.mSampleCount = SAMPLE_COUNT_1;
            graphicsPipelineDesc.mSampleQuality = 0;
            graphicsPipelineDesc.pBlendState = &blendStateDesc;
            graphicsPipelineDesc.pColorFormats = &inputFormat;
            addPipeline(forgeRenderer->Rend(), &pipelineDesc, pipeline);
            return true;
        });
    }

    cPostEffectType_ImageTrail::~cPostEffectType_ImageTrail() {
    }

    iPostEffect* cPostEffectType_ImageTrail::CreatePostEffect(iPostEffectParams* apParams) {
        cPostEffect_ImageTrail* pEffect = hplNew(cPostEffect_ImageTrail, (mpGraphics, mpResources, this));
        cPostEffectParams_ImageTrail* pImageTrailParams = static_cast<cPostEffectParams_ImageTrail*>(apParams);
        return pEffect;
    }

    cPostEffect_ImageTrail::cPostEffect_ImageTrail(cGraphics* apGraphics, cResources* apResources, iPostEffectType* apType)
        : iPostEffect(apGraphics, apResources, apType) {
        mpImageTrailType = static_cast<cPostEffectType_ImageTrail*>(mpType);
    }

    cPostEffect_ImageTrail::~cPostEffect_ImageTrail() {

    }

    void cPostEffect_ImageTrail::Reset() {
        mbClearFrameBuffer = true;
    }

    void cPostEffect_ImageTrail::OnSetParams() {
    }

    void cPostEffect_ImageTrail::OnSetActive(bool abX) {
        if (abX == false) {
            Reset();
        }
    }


    void cPostEffect_ImageTrail::RenderEffect(cPostEffectComposite& compositor, cViewport& viewport, const ForgeRenderer::Frame& frame, Texture* inputTexture, RenderTarget* renderTarget) {

        cmdBeginDebugMarker(frame.m_cmd, 0, 1, 0, "Image Trail PostEffect");
        auto* renderer = Interface<ForgeRenderer>::Get();
        auto imageTrailData = m_boundImageTrailData.resolve(viewport);
        if (!imageTrailData || imageTrailData->m_size != viewport.GetSize()) {
            auto* renderer = Interface<ForgeRenderer>::Get();
            auto trailData = std::make_unique<ImageTrailData>();
            trailData->m_size = viewport.GetSize();
            trailData->m_accumulationTarget.Load(renderer->Rend(),[&](RenderTarget** texture) {
                RenderTargetDesc renderTarget = {};
                renderTarget.mArraySize = 1;
                renderTarget.mDepth = 1;
                renderTarget.mDescriptors = DESCRIPTOR_TYPE_TEXTURE;
                renderTarget.mWidth = viewport.GetSize().x;
                renderTarget.mHeight = viewport.GetSize().y;
                renderTarget.mSampleCount = SAMPLE_COUNT_1;
                renderTarget.mSampleQuality = 0;
                renderTarget.mStartState = RESOURCE_STATE_SHADER_RESOURCE;
                renderTarget.mFormat = TinyImageFormat_R8G8B8A8_UNORM;
                addRenderTarget(renderer->Rend(), &renderTarget, texture);
                return true;
            });
            mbClearFrameBuffer = true;
            imageTrailData = m_boundImageTrailData.update(viewport, std::move(trailData));
        }

        {
            cmdBindRenderTargets(frame.m_cmd, 0, NULL, NULL, NULL, NULL, NULL, -1, -1);
            std::array rtBarriers = {
                RenderTargetBarrier{
                    imageTrailData->m_accumulationTarget.m_handle, RESOURCE_STATE_SHADER_RESOURCE, RESOURCE_STATE_RENDER_TARGET },
            };
            cmdResourceBarrier(frame.m_cmd, 0, NULL, 0, NULL, rtBarriers.size(), rtBarriers.data());
        }
        float alpha = 0.0f;
        if (mbClearFrameBuffer) {
            alpha = 1.0f;
            mbClearFrameBuffer = false;
        } else {
            // Get the amount of blur depending frame time.
            //*30 is just so that good amount values are still between 0 - 1
            float fFrameTime = compositor.GetCurrentFrameTime();
            float fPow = (1.0f / fFrameTime) * mParams.mfAmount; // The higher this is, the more blur!
            alpha = exp(-fPow * 0.015f);
        }

        {
            Texture* input[] = { inputTexture };
            std::array<DescriptorData, 1> params = {};
            params[0].pName = "sourceInput";
            params[0].ppTextures = input;

            updateDescriptorSet(renderer->Rend(), mpImageTrailType->m_descIndex, mpImageTrailType->m_perFrameDescriptorSet[frame.m_frameIndex].m_handle, params.size(), params.data());
        }
        LoadActionsDesc loadActions = {};
        loadActions.mLoadActionsColor[0] = LOAD_ACTION_LOAD;
        loadActions.mLoadActionDepth = LOAD_ACTION_DONTCARE;

        std::array inputTargets = { imageTrailData->m_accumulationTarget.m_handle };
        cmdBindRenderTargets(frame.m_cmd, inputTargets.size(), inputTargets.data(), NULL, &loadActions, NULL, NULL, -1, -1);

        uint32_t rootConstantIndex = getDescriptorIndexFromName(mpImageTrailType->m_rootSignature.m_handle, "rootConstant");
        cmdBindPushConstants(frame.m_cmd, mpImageTrailType->m_rootSignature.m_handle, rootConstantIndex, &alpha);
        cmdSetViewport(frame.m_cmd, 0.0f, 0.0f, static_cast<float>(renderTarget->mWidth), static_cast<float>(renderTarget->mHeight), 0.0f, 1.0f);
        cmdSetScissor(frame.m_cmd, 0, 0, static_cast<float>(renderTarget->mWidth), static_cast<float>(renderTarget->mHeight));
        cmdBindDescriptorSet(frame.m_cmd, mpImageTrailType->m_descIndex, mpImageTrailType->m_perFrameDescriptorSet[frame.m_frameIndex].m_handle);
        cmdBindPipeline(frame.m_cmd, mpImageTrailType->m_pipeline.m_handle);
        cmdDraw(frame.m_cmd, 3, 0);
        mpImageTrailType->m_descIndex = (mpImageTrailType->m_descIndex + 1) % cPostEffectType_ImageTrail::DescSetSize;
        {
            cmdBindRenderTargets(frame.m_cmd, 0, NULL, NULL, NULL, NULL, NULL, -1, -1);
            std::array rtBarriers = {
                RenderTargetBarrier{
                    imageTrailData->m_accumulationTarget.m_handle, RESOURCE_STATE_RENDER_TARGET, RESOURCE_STATE_SHADER_RESOURCE },
            };
            cmdResourceBarrier(frame.m_cmd, 0, NULL, 0, NULL, rtBarriers.size(), rtBarriers.data());
        }
        renderer->cmdCopyTexture(frame.m_cmd, imageTrailData->m_accumulationTarget.m_handle->pTexture, renderTarget);
        cmdEndDebugMarker(frame.m_cmd);
    }

} // namespace hpl
