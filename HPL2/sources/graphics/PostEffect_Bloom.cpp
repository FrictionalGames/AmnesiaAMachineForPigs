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

#include "graphics/PostEffect_Bloom.h"

#include "Common_3/Graphics/Interfaces/IGraphics.h"
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

namespace hpl
{

    cPostEffectType_Bloom::cPostEffectType_Bloom(cGraphics* apGraphics, cResources* apResources)
        : iPostEffectType("Bloom", apGraphics, apResources)
    {
        auto* forgeRenderer = Interface<ForgeRenderer>::Get();
        m_blurShader.Load(forgeRenderer->Rend(),[&](Shader** shader) {
            ShaderLoadDesc loadDesc{};
            loadDesc.mStages[0].pFileName = "fullscreen.vert";
            loadDesc.mStages[1].pFileName = "blur_posteffect.frag";
            addShader(forgeRenderer->Rend(),&loadDesc, shader);
            return true;
        });
        m_bloomShader.Load(forgeRenderer->Rend(),[&](Shader** shader) {
            ShaderLoadDesc loadDesc{};
            loadDesc.mStages[0].pFileName = "fullscreen.vert";
            loadDesc.mStages[1].pFileName = "bloom_add_posteffect.frag";
            addShader(forgeRenderer->Rend(),&loadDesc, shader);
            return true;
        });
        m_inputSampler.Load(forgeRenderer->Rend(), [&](Sampler** sampler) {
            SamplerDesc samplerDesc = {};
            addSampler(forgeRenderer->Rend(), &samplerDesc,  sampler);
            return true;
        });
        m_blurSignature.Load(forgeRenderer->Rend(), [&](RootSignature** sig) {
            std::array shaders = {
                m_blurShader.m_handle
            };
            RootSignatureDesc rootDesc{};
            const char* pStaticSamplers[] = { "inputSampler" };
            rootDesc.ppShaders = shaders.data();
            rootDesc.mShaderCount = shaders.size();
            rootDesc.mStaticSamplerCount = 1;
            rootDesc.ppStaticSamplers = &m_inputSampler.m_handle;
            rootDesc.ppStaticSamplerNames = pStaticSamplers;
            addRootSignature(forgeRenderer->Rend(), &rootDesc, sig);
            return true;
        });

        for(auto& descSet: m_perFrameDescriptorSets) {
            descSet.Load(forgeRenderer->Rend(), [&](DescriptorSet** desc) {
                DescriptorSetDesc setDesc = { m_blurSignature.m_handle, DESCRIPTOR_UPDATE_FREQ_PER_FRAME, cPostEffectType_Bloom::DescriptorSetSize  };
                addDescriptorSet(forgeRenderer->Rend(), &setDesc, desc);
                return true;
            });
        }
        m_bloomRootSignature.Load(forgeRenderer->Rend(), [&](RootSignature** signature) {
            std::array shaders = {
                m_bloomShader.m_handle
            };
            RootSignatureDesc rootDesc{};
            const char* pStaticSamplers[] = { "inputSampler" };
            rootDesc.ppShaders = shaders.data();
            rootDesc.mShaderCount = shaders.size();
            rootDesc.mStaticSamplerCount = 1;
            rootDesc.ppStaticSamplers = &m_inputSampler.m_handle;
            rootDesc.ppStaticSamplerNames = pStaticSamplers;
            addRootSignature(forgeRenderer->Rend(), &rootDesc, signature);
            return true;
        });

        for(auto& descSet: m_perFrameDescriptorBloomSets) {
            descSet.Load(forgeRenderer->Rend(), [&](DescriptorSet** descriptorSet) {
                DescriptorSetDesc setDesc = { m_bloomRootSignature.m_handle, DESCRIPTOR_UPDATE_FREQ_PER_FRAME, cPostEffectType_Bloom::DescriptorSetSize  };
                addDescriptorSet(forgeRenderer->Rend(), &setDesc, descriptorSet);
                return true;
            });
        }
        TinyImageFormat inputFormat = TinyImageFormat_R8G8B8A8_UNORM;
        m_blurPipeline.Load(forgeRenderer->Rend(), [&](Pipeline** pipeline){
            DepthStateDesc depthStateDisabledDesc = {};
            depthStateDisabledDesc.mDepthWrite = false;
            depthStateDisabledDesc.mDepthTest = false;

            RasterizerStateDesc rasterStateNoneDesc = {};
            rasterStateNoneDesc.mCullMode = CULL_MODE_NONE;

            PipelineDesc pipelineDesc = {};
            pipelineDesc.mType = PIPELINE_TYPE_GRAPHICS;
            GraphicsPipelineDesc& graphicsPipelineDesc = pipelineDesc.mGraphicsDesc;
            graphicsPipelineDesc.mPrimitiveTopo = PRIMITIVE_TOPO_TRI_LIST;
            graphicsPipelineDesc.pShaderProgram = m_blurShader.m_handle;
            graphicsPipelineDesc.pRootSignature = m_blurSignature.m_handle;
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
        m_bloomPipeline.Load(forgeRenderer->Rend(), [&](Pipeline** pipeline){
            DepthStateDesc depthStateDisabledDesc = {};
            depthStateDisabledDesc.mDepthWrite = false;
            depthStateDisabledDesc.mDepthTest = false;

            RasterizerStateDesc rasterStateNoneDesc = {};
            rasterStateNoneDesc.mCullMode = CULL_MODE_NONE;

            PipelineDesc pipelineDesc = {};
            pipelineDesc.mType = PIPELINE_TYPE_GRAPHICS;
            GraphicsPipelineDesc& graphicsPipelineDesc = pipelineDesc.mGraphicsDesc;
            graphicsPipelineDesc.mPrimitiveTopo = PRIMITIVE_TOPO_TRI_LIST;
            graphicsPipelineDesc.pShaderProgram = m_bloomShader.m_handle;
            graphicsPipelineDesc.pRootSignature = m_bloomRootSignature.m_handle;
            graphicsPipelineDesc.mDepthStencilFormat = TinyImageFormat_UNDEFINED;
            graphicsPipelineDesc.pVertexLayout = NULL;
            graphicsPipelineDesc.pRasterizerState = &rasterStateNoneDesc;
            graphicsPipelineDesc.pDepthState = &depthStateDisabledDesc;
            graphicsPipelineDesc.pBlendState = NULL;
            graphicsPipelineDesc.mSampleCount = SAMPLE_COUNT_1;
            graphicsPipelineDesc.mSampleQuality = 0;
            graphicsPipelineDesc.mRenderTargetCount = 1;
            graphicsPipelineDesc.pColorFormats = &inputFormat;
            addPipeline(forgeRenderer->Rend(), &pipelineDesc, pipeline);
           return true;
        });
    }

    cPostEffectType_Bloom::~cPostEffectType_Bloom()
    {
    }

    iPostEffect* cPostEffectType_Bloom::CreatePostEffect(iPostEffectParams* apParams)
    {
        cPostEffect_Bloom* pEffect = hplNew(cPostEffect_Bloom, (mpGraphics, mpResources, this));
        cPostEffectParams_Bloom* pBloomParams = static_cast<cPostEffectParams_Bloom*>(apParams);

        return pEffect;
    }

    cPostEffect_Bloom::cPostEffect_Bloom(cGraphics* apGraphics, cResources* apResources, iPostEffectType* apType)
        : iPostEffect(apGraphics, apResources, apType)
    {
        mpBloomType = static_cast<cPostEffectType_Bloom*>(mpType);
    }

    cPostEffect_Bloom::~cPostEffect_Bloom()
    {
    }

    void cPostEffect_Bloom::OnSetParams()
    {
    }

    void cPostEffect_Bloom::RenderEffect(cPostEffectComposite& compositor, cViewport& viewport, const ForgeRenderer::Frame& frame, Texture* inputTexture, RenderTarget* renderTarget)  {
        auto bloomData = m_boundBloomData.resolve(viewport);

        if(!bloomData || bloomData->m_size != viewport.GetSize()) {
            auto* renderer = Interface<ForgeRenderer>::Get();
            auto data = std::make_unique<BloomData>();
            data->m_blurTargets[0].Load(renderer->Rend(),[&](RenderTarget** target) {
                RenderTargetDesc renderTarget = {};
                renderTarget.mArraySize = 1;
                renderTarget.mDepth = 1;
                renderTarget.mDescriptors = DESCRIPTOR_TYPE_TEXTURE;
                renderTarget.mWidth = viewport.GetSize().x / 4.0f;
                renderTarget.mHeight = viewport.GetSize().y / 4.0f;
                renderTarget.mSampleCount = SAMPLE_COUNT_1;
                renderTarget.mSampleQuality = 0;
                renderTarget.mStartState = RESOURCE_STATE_SHADER_RESOURCE;
                renderTarget.mFormat = TinyImageFormat_R8G8B8A8_UNORM;
                addRenderTarget(renderer->Rend(), &renderTarget, target);
                return true;
            });
            data->m_blurTargets[1].Load(renderer->Rend(), [&](RenderTarget** target) {
                RenderTargetDesc renderTarget = {};
                renderTarget.mArraySize = 1;
                renderTarget.mDepth = 1;
                renderTarget.mDescriptors = DESCRIPTOR_TYPE_TEXTURE;
                renderTarget.mWidth = viewport.GetSize().x / 4.0f;
                renderTarget.mHeight = viewport.GetSize().y / 4.0f;
                renderTarget.mSampleCount = SAMPLE_COUNT_1;
                renderTarget.mSampleQuality = 0;
                renderTarget.mStartState = RESOURCE_STATE_RENDER_TARGET;
                renderTarget.mFormat = TinyImageFormat_R8G8B8A8_UNORM;
                addRenderTarget(renderer->Rend(), &renderTarget, target);
                return true;
            });
            data->m_size = viewport.GetSize();
            bloomData = m_boundBloomData.update(viewport, std::move(data));
        }

        auto requestBlur = [&](Texture** input) {
            ASSERT(input && "Invalid input texture");
            uint32_t blurPostEffectConstIndex = getDescriptorIndexFromName(mpBloomType->m_blurSignature.m_handle, "rootConstant");
            {
                cmdBindRenderTargets(frame.m_cmd, 0, NULL, NULL, NULL, NULL, NULL, -1, -1);
                std::array rtBarriers = {
                    RenderTargetBarrier{ bloomData->m_blurTargets[0].m_handle, RESOURCE_STATE_SHADER_RESOURCE, RESOURCE_STATE_RENDER_TARGET },
                    RenderTargetBarrier{ bloomData->m_blurTargets[1].m_handle, RESOURCE_STATE_RENDER_TARGET, RESOURCE_STATE_SHADER_RESOURCE},
                };
                cmdResourceBarrier(frame.m_cmd, 0, NULL, 0, NULL, rtBarriers.size(), rtBarriers.data());
            }
            {
                LoadActionsDesc loadActions = {};
                loadActions.mLoadActionsColor[0] = LOAD_ACTION_LOAD;
                loadActions.mLoadActionDepth = LOAD_ACTION_DONTCARE;
                auto& blurTarget = bloomData->m_blurTargets[0].m_handle;
                cmdBindRenderTargets(frame.m_cmd, 1, &blurTarget , NULL, &loadActions, NULL, NULL, -1, -1);

                std::array<DescriptorData, 1> params = {};
                params[0].pName = "sourceInput";
                params[0].ppTextures = input;
                updateDescriptorSet(
                    frame.m_renderer->Rend(), mpBloomType->m_setIndex, mpBloomType->m_perFrameDescriptorSets[frame.m_frameIndex].m_handle, params.size(), params.data());

                cmdSetViewport(frame.m_cmd, 0.0f, 0.0f, static_cast<float>(blurTarget->mWidth), static_cast<float>(blurTarget->mHeight), 0.0f, 1.0f);
                cmdSetScissor(frame.m_cmd, 0, 0, static_cast<float>(blurTarget->mWidth), static_cast<float>(blurTarget->mHeight));
                cmdBindPipeline(frame.m_cmd, mpBloomType->m_blurPipeline.m_handle);

                cmdBindDescriptorSet(frame.m_cmd, mpBloomType->m_setIndex, mpBloomType->m_perFrameDescriptorSets[frame.m_frameIndex].m_handle);
                float2 blurScale = float2(mParams.mfBlurSize, 0.0f);
                cmdDraw(frame.m_cmd, 3, 0);

                mpBloomType->m_setIndex = (mpBloomType->m_setIndex + 1) % cPostEffectType_Bloom::DescriptorSetSize;
            }
            {
                cmdBindRenderTargets(frame.m_cmd, 0, NULL, NULL, NULL, NULL, NULL, -1, -1);
                std::array rtBarriers = {
                    RenderTargetBarrier{
                        bloomData->m_blurTargets[0].m_handle, RESOURCE_STATE_RENDER_TARGET, RESOURCE_STATE_SHADER_RESOURCE },
                    RenderTargetBarrier{
                        bloomData->m_blurTargets[1].m_handle, RESOURCE_STATE_SHADER_RESOURCE, RESOURCE_STATE_RENDER_TARGET },
                };
                cmdResourceBarrier(frame.m_cmd, 0, NULL, 0, NULL, rtBarriers.size(), rtBarriers.data());
            }
            {
                LoadActionsDesc loadActions = {};
                loadActions.mLoadActionsColor[0] = LOAD_ACTION_LOAD;
                loadActions.mLoadActionDepth = LOAD_ACTION_DONTCARE;

                auto& blurTarget = bloomData->m_blurTargets[1].m_handle;
                cmdBindRenderTargets(frame.m_cmd, 1, &blurTarget, NULL, &loadActions, NULL, NULL, -1, -1);

                std::array<DescriptorData, 1> params = {};
                params[0].pName = "sourceInput";
                params[0].ppTextures = &bloomData->m_blurTargets[0].m_handle->pTexture;
                updateDescriptorSet(
                    frame.m_renderer->Rend(), mpBloomType->m_setIndex, mpBloomType->m_perFrameDescriptorSets[frame.m_frameIndex].m_handle, params.size(), params.data());

                cmdSetViewport(frame.m_cmd, 0.0f, 0.0f, static_cast<float>(blurTarget->mWidth), static_cast<float>(blurTarget->mHeight), 0.0f, 1.0f);
                cmdSetScissor(frame.m_cmd, 0, 0, static_cast<float>(blurTarget->mWidth), static_cast<float>(blurTarget->mHeight));
                cmdBindPipeline(frame.m_cmd, mpBloomType->m_blurPipeline.m_handle);

                float2 blurScale = float2(0.0f, mParams.mfBlurSize);
                cmdBindPushConstants(frame.m_cmd, mpBloomType->m_blurSignature.m_handle, blurPostEffectConstIndex, &blurScale);
                cmdBindDescriptorSet(frame.m_cmd, mpBloomType->m_setIndex, mpBloomType->m_perFrameDescriptorSets[frame.m_frameIndex].m_handle);
                cmdDraw(frame.m_cmd, 3, 0);

                mpBloomType->m_setIndex= (mpBloomType->m_setIndex + 1) % cPostEffectType_Bloom::DescriptorSetSize;
            }
        };
        cmdBeginDebugMarker(frame.m_cmd, 0, 1, 0, "Bloom Blur");
        requestBlur(&inputTexture);
        for (int i = 1; i < mParams.mlBlurIterations; ++i)
        {
            requestBlur(&bloomData->m_blurTargets[1].m_handle->pTexture);
        }
        cmdEndDebugMarker(frame.m_cmd);
        {
            cmdBindRenderTargets(frame.m_cmd, 0, NULL, NULL, NULL, NULL, NULL, -1, -1);
            std::array rtBarriers = {
                RenderTargetBarrier{
                    bloomData->m_blurTargets[1].m_handle, RESOURCE_STATE_RENDER_TARGET, RESOURCE_STATE_SHADER_RESOURCE },
            };
            cmdResourceBarrier(frame.m_cmd, 0, NULL, 0, NULL, rtBarriers.size(), rtBarriers.data());
        }
        {
            float rgbIntensity[] = {mParams.mvRgbToIntensity.x, mParams.mvRgbToIntensity.y, mParams.mvRgbToIntensity.z, 0.0f};
            uint32_t rootConstantIndex = getDescriptorIndexFromName(mpBloomType->m_bloomRootSignature.m_handle, "rootConstant");
            cmdBindPushConstants(frame.m_cmd, mpBloomType->m_bloomRootSignature.m_handle, rootConstantIndex, rgbIntensity);
        }
        {
            cmdBeginDebugMarker(frame.m_cmd, 0, 1, 0, "Bloom Add");
            LoadActionsDesc loadActions = {};
            loadActions.mLoadActionsColor[0] = LOAD_ACTION_LOAD;
            loadActions.mLoadActionDepth = LOAD_ACTION_DONTCARE;

            cmdBindRenderTargets(frame.m_cmd, 1, &renderTarget, NULL, &loadActions, NULL, NULL, -1, -1);

            std::array<DescriptorData, 2> params = {};
            params[0].pName = "sourceInput";
            params[0].ppTextures = &inputTexture;
            params[1].pName = "blurInput";
            params[1].ppTextures = &bloomData->m_blurTargets[1].m_handle->pTexture;

            updateDescriptorSet(
                frame.m_renderer->Rend(), mpBloomType->m_setBloomIndex, mpBloomType->m_perFrameDescriptorBloomSets[frame.m_frameIndex].m_handle, params.size(), params.data());

            cmdSetViewport(frame.m_cmd, 0.0f, 0.0f, static_cast<float>(renderTarget->mWidth), static_cast<float>(renderTarget->mHeight), 0.0f, 1.0f);
            cmdSetScissor(frame.m_cmd, 0, 0, static_cast<float>(renderTarget->mWidth), static_cast<float>(renderTarget->mHeight));
            cmdBindPipeline(frame.m_cmd, mpBloomType->m_bloomPipeline.m_handle);

            cmdBindDescriptorSet(frame.m_cmd, mpBloomType->m_setBloomIndex, mpBloomType->m_perFrameDescriptorBloomSets[frame.m_frameIndex].m_handle);
            cmdDraw(frame.m_cmd, 3, 0);
            mpBloomType->m_setBloomIndex = (mpBloomType->m_setBloomIndex + 1) % cPostEffectType_Bloom::DescriptorSetSize;
            cmdEndDebugMarker(frame.m_cmd);
        }
        {
            cmdBindRenderTargets(frame.m_cmd, 0, NULL, NULL, NULL, NULL, NULL, -1, -1);
            std::array rtBarriers = {
                RenderTargetBarrier{
                    bloomData->m_blurTargets[1].m_handle, RESOURCE_STATE_SHADER_RESOURCE, RESOURCE_STATE_RENDER_TARGET },
            };
            cmdResourceBarrier(frame.m_cmd, 0, NULL, 0, NULL, rtBarriers.size(), rtBarriers.data());
        }
    }
} // namespace hpl
