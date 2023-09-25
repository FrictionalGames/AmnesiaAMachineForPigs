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

#include "graphics/PostEffect_RadialBlur.h"

#include "graphics/Graphics.h"

#include "graphics/FrameBuffer.h"
#include "graphics/LowLevelGraphics.h"
#include "graphics/PostEffectComposite.h"
#include "graphics/Texture.h"

#include "math/MathTypes.h"
#include "system/PreprocessParser.h"

namespace hpl
{

    cPostEffectType_RadialBlur::cPostEffectType_RadialBlur(cGraphics* apGraphics, cResources* apResources)
        : iPostEffectType("RadialBlur", apGraphics, apResources) {
        auto* forgeRenderer = Interface<ForgeRenderer>::Get();
        m_shader.Load(forgeRenderer->Rend(), [&](Shader** shader) {
            ShaderLoadDesc loadDesc{};
            loadDesc.mStages[0].pFileName = "fullscreen.vert";
            loadDesc.mStages[1].pFileName = "radial_blur_posteffect.frag";
            addShader(forgeRenderer->Rend(), &loadDesc, shader);
            return true;
        });
        m_inputSampler.Load(forgeRenderer->Rend(), [&](Sampler** sampler) {
            SamplerDesc samplerDesc = {};
            addSampler(forgeRenderer->Rend(), &samplerDesc, sampler);
            return true;
        });

        m_rootSignature.Load(forgeRenderer->Rend(), [&](RootSignature** rootSignature) {
            std::array shaders = { m_shader.m_handle };
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

        for (auto& descSet : m_perFrameDescriptorSet) {
            descSet.Load(forgeRenderer->Rend(), [&](DescriptorSet** descSet) {
                DescriptorSetDesc setDesc = { m_rootSignature.m_handle,
                                              DESCRIPTOR_UPDATE_FREQ_PER_FRAME,
                                              cPostEffectType_RadialBlur::DescriptorSetSize };
                addDescriptorSet(forgeRenderer->Rend(), &setDesc, descSet);
                return true;
            });
        }
        TinyImageFormat inputFormat = TinyImageFormat_R8G8B8A8_UNORM;
        m_pipeline.Load(forgeRenderer->Rend(), [&](Pipeline** pipeline) {
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
            graphicsPipelineDesc.pColorFormats = &inputFormat;
            addPipeline(forgeRenderer->Rend(), &pipelineDesc, pipeline);
            return true;
        });
    }

    cPostEffectType_RadialBlur::~cPostEffectType_RadialBlur()
    {
    }

    iPostEffect* cPostEffectType_RadialBlur::CreatePostEffect(iPostEffectParams* apParams)
    {
        cPostEffect_RadialBlur* pEffect = hplNew(cPostEffect_RadialBlur, (mpGraphics, mpResources, this));
        cPostEffectParams_RadialBlur* pRadialBlurParams = static_cast<cPostEffectParams_RadialBlur*>(apParams);
        return pEffect;
    }

    cPostEffect_RadialBlur::cPostEffect_RadialBlur(cGraphics* apGraphics, cResources* apResources, iPostEffectType* apType)
        : iPostEffect(apGraphics, apResources, apType)
    {
        mpRadialBlurType = static_cast<cPostEffectType_RadialBlur*>(mpType);
    }

    cPostEffect_RadialBlur::~cPostEffect_RadialBlur()
    {
    }

    void cPostEffect_RadialBlur::Reset()
    {
    }

    void cPostEffect_RadialBlur::OnSetParams()
    {
    }

    void cPostEffect_RadialBlur::RenderEffect(cPostEffectComposite& compositor, cViewport& viewport, const ForgeRenderer::Frame& frame, Texture* inputTexture, RenderTarget* renderTarget) {
        ASSERT(mpRadialBlurType);
        struct {
            float size;
            float blurStartDist;
            float2 screenDim;
        } params = {
            mParams.mfSize,
            mParams.mfBlurStartDist,
            float2(static_cast<float>(renderTarget->mWidth), static_cast<float>(renderTarget->mHeight))
        };

        uint32_t rootConstantIndex = getDescriptorIndexFromName(mpRadialBlurType->m_rootSignature.m_handle, "rootConstant");
        cmdBindPushConstants(frame.m_cmd, mpRadialBlurType->m_rootSignature.m_handle, rootConstantIndex, &params);

        LoadActionsDesc loadActions = {};
        loadActions.mLoadActionsColor[0] = LOAD_ACTION_LOAD;
        loadActions.mLoadActionDepth = LOAD_ACTION_DONTCARE;

        cmdBindRenderTargets(frame.m_cmd, 1, &renderTarget, NULL, &loadActions, NULL, NULL, -1, -1);

        std::array<DescriptorData, 1> descData = {};
        descData[0].pName = "sourceInput";
        descData[0].ppTextures = &inputTexture;
        updateDescriptorSet(
            frame.m_renderer->Rend(), mpRadialBlurType->m_descIndex, mpRadialBlurType->m_perFrameDescriptorSet[frame.m_frameIndex].m_handle, descData.size(), descData.data());

        cmdSetViewport(frame.m_cmd, 0.0f, 0.0f, static_cast<float>(renderTarget->mWidth), static_cast<float>(renderTarget->mHeight), 0.0f, 1.0f);
        cmdSetScissor(frame.m_cmd, 0, 0, static_cast<float>(renderTarget->mWidth), static_cast<float>(renderTarget->mHeight));
        cmdBindPipeline(frame.m_cmd, mpRadialBlurType->m_pipeline.m_handle);

        cmdBindDescriptorSet(frame.m_cmd, mpRadialBlurType->m_descIndex, mpRadialBlurType->m_perFrameDescriptorSet[frame.m_frameIndex].m_handle);
        cmdDraw(frame.m_cmd, 3, 0);
        mpRadialBlurType->m_descIndex = (mpRadialBlurType->m_descIndex + 1) % cPostEffectType_RadialBlur::DescriptorSetSize;
    }

} // namespace hpl
