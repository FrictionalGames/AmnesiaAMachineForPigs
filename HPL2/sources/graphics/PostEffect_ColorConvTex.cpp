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

#include "graphics/PostEffect_ColorConvTex.h"

#include "graphics/Enum.h"
#include "graphics/Graphics.h"
#include "math/MathTypes.h"
#include "resources/Resources.h"

#include "graphics/FrameBuffer.h"
#include "graphics/LowLevelGraphics.h"
#include "graphics/PostEffectComposite.h"
#include "graphics/Texture.h"

#include "resources/TextureManager.h"

#include "Common_3/Graphics/Interfaces/IGraphics.h"
#include "system/PreprocessParser.h"
#include "system/String.h"

#include "math/Math.h"

namespace hpl
{

    cPostEffectType_ColorConvTex::cPostEffectType_ColorConvTex(cGraphics* apGraphics, cResources* apResources)
        : iPostEffectType("ColorConvTex", apGraphics, apResources)
    {
        auto* forgeRenderer = Interface<ForgeRenderer>::Get();
        m_shader.Load(forgeRenderer->Rend(), [&](Shader** shader) {
            ShaderLoadDesc loadDesc{};
            loadDesc.mStages[0].pFileName = "fullscreen.vert";
            loadDesc.mStages[1].pFileName = "color_conv_posteffect.frag";
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
            descSet.Load(forgeRenderer->Rend(), [&](DescriptorSet** descSet) {
                DescriptorSetDesc setDesc = { m_rootSignature.m_handle, DESCRIPTOR_UPDATE_FREQ_PER_FRAME, cPostEffectType_ColorConvTex::DescriptorSetSize  };
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

    cPostEffectType_ColorConvTex::~cPostEffectType_ColorConvTex()
    {
    }

    iPostEffect* cPostEffectType_ColorConvTex::CreatePostEffect(iPostEffectParams* apParams)
    {
        cPostEffect_ColorConvTex* pEffect = hplNew(cPostEffect_ColorConvTex, (mpGraphics, mpResources, this));
        cPostEffectParams_ColorConvTex* pBloomParams = static_cast<cPostEffectParams_ColorConvTex*>(apParams);

        return pEffect;
    }

    cPostEffect_ColorConvTex::cPostEffect_ColorConvTex(cGraphics* apGraphics, cResources* apResources, iPostEffectType* apType)
        : iPostEffect(apGraphics, apResources, apType)
    {
        mpSpecificType = static_cast<cPostEffectType_ColorConvTex*>(mpType);
        mpColorConvTex = NULL;
    }

    cPostEffect_ColorConvTex::~cPostEffect_ColorConvTex()
    {
    }


    void cPostEffect_ColorConvTex::OnSetParams()
    {
        if (mParams.msTextureFile == "") {
            return;
        }

        if (mpColorConvTex) {
            mpResources->GetTextureManager()->Destroy(mpColorConvTex);
        }
        cTextureManager::ImageOptions options;
		options.m_UWrap = WrapMode::Clamp;
		options.m_VWrap = WrapMode::Clamp;
        mpColorConvTex = mpResources->GetTextureManager()->Create1DImage(mParams.msTextureFile, false, eTextureUsage_Normal, 0);
    }

    void cPostEffect_ColorConvTex::RenderEffect(cPostEffectComposite& compositor, cViewport& viewport, const ForgeRenderer::Frame& frame, Texture* inputTexture, RenderTarget* renderTarget) {
        ASSERT(mpColorConvTex);

        float alphaFade = cMath::Clamp(mParams.mfFadeAlpha, 0.0f, 1.0f);
        uint32_t rootConstantIndex = getDescriptorIndexFromName(mpSpecificType->m_rootSignature.m_handle, "rootConstant");
        cmdBindPushConstants(frame.m_cmd, mpSpecificType->m_rootSignature.m_handle, rootConstantIndex, &alphaFade);

        LoadActionsDesc loadActions = {};
        loadActions.mLoadActionsColor[0] = LOAD_ACTION_LOAD;
        loadActions.mLoadActionDepth = LOAD_ACTION_DONTCARE;

        cmdBindRenderTargets(frame.m_cmd, 1, &renderTarget, NULL, &loadActions, NULL, NULL, -1, -1);

        std::array<DescriptorData, 2> params = {};
        params[0].pName = "sourceInput";
        params[0].ppTextures = &inputTexture;
        params[1].pName = "colorConv";
        params[1].ppTextures = &mpColorConvTex->GetTexture().m_handle;
        updateDescriptorSet(
            frame.m_renderer->Rend(), mpSpecificType->m_descIndex, mpSpecificType->m_perFrameDescriptorSet[frame.m_frameIndex].m_handle, params.size(), params.data());

        cmdSetViewport(frame.m_cmd, 0.0f, 0.0f, static_cast<float>(renderTarget->mWidth), static_cast<float>(renderTarget->mHeight), 0.0f, 1.0f);
        cmdSetScissor(frame.m_cmd, 0, 0, static_cast<float>(renderTarget->mWidth), static_cast<float>(renderTarget->mHeight));
        cmdBindPipeline(frame.m_cmd, mpSpecificType->m_pipeline.m_handle);

        cmdBindDescriptorSet(frame.m_cmd, mpSpecificType->m_descIndex, mpSpecificType->m_perFrameDescriptorSet[frame.m_frameIndex].m_handle);
        cmdDraw(frame.m_cmd, 3, 0);
        mpSpecificType->m_descIndex = (mpSpecificType->m_descIndex + 1) % cPostEffectType_ColorConvTex::DescriptorSetSize;
    }

} // namespace hpl
