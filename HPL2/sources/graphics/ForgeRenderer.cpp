#include <graphics/ForgeRenderer.h>

#include "engine/IUpdateEventLoop.h"
#include "engine/Interface.h"
#include "graphics/Material.h"
#include "windowing/NativeWindow.h"

#include "Common_3/Graphics/Interfaces/IGraphics.h"

#ifdef HPL2_RENDERDOC_ENABLED
#ifdef WIN32
#else
    #include <dlfcn.h>
#endif
#include "renderdoc_app.h"
#endif
extern RendererApi gSelectedRendererApi;

namespace hpl {
    RendererApi ForgeRenderer::GetApi() {
        return gSelectedRendererApi;
    }

    void ForgeRenderer::IncrementFrame() {
        // Stall if CPU is running "Swap Chain Buffer Count" frames ahead of GPU
        // m_resourcePoolIndex = (m_resourcePoolIndex + 1) % ResourcePoolSize;
        m_currentFrameCount++;
        auto frame = GetFrame();

        FenceStatus fenceStatus;
        auto& completeFence = frame.m_renderCompleteFence;
        getFenceStatus(m_renderer, completeFence, &fenceStatus);
        if (fenceStatus == FENCE_STATUS_INCOMPLETE) {
            waitForFences(m_renderer, 1, &completeFence);
        }
        acquireNextImage(m_renderer, m_swapChain.m_handle, m_imageAcquiredSemaphore, nullptr, &m_swapChainIndex);

        resetCmdPool(m_renderer, frame.m_cmdPool);
        frame.m_resourcePool->ResetPool();
        beginCmd(frame.m_cmd);

        auto& swapChainImage = frame.m_swapChain->ppRenderTargets[m_swapChainIndex];
        std::array rtBarriers = {
            RenderTargetBarrier { swapChainImage, RESOURCE_STATE_PRESENT, RESOURCE_STATE_RENDER_TARGET },
        };
        frame.m_resourcePool->Push(m_finalRenderTarget[frame.m_frameIndex]);
        cmdResourceBarrier(frame.m_cmd, 0, NULL, 0, NULL, rtBarriers.size(), rtBarriers.data());


    }

    void ForgeRenderer::SubmitFrame() {
        auto frame = GetFrame();
        auto& swapChainTarget = frame.m_swapChain->ppRenderTargets[frame.m_swapChainIndex];
        {
            cmdBindRenderTargets(frame.m_cmd, 0, NULL, NULL, NULL, NULL, NULL, -1, -1);
            std::array rtBarriers = {
                RenderTargetBarrier{ m_finalRenderTarget[frame.m_frameIndex].m_handle, RESOURCE_STATE_RENDER_TARGET, RESOURCE_STATE_SHADER_RESOURCE},
            };
            cmdResourceBarrier(frame.m_cmd, 0, NULL, 0, NULL, rtBarriers.size(), rtBarriers.data());
        }
        {
            cmdBindRenderTargets(frame.m_cmd, 1, &swapChainTarget, NULL, NULL, NULL, NULL, -1, -1);
            uint32_t rootConstantIndex = getDescriptorIndexFromName(m_finalRootSignature , "uRootConstants");

            cmdSetViewport(frame.m_cmd, 0.0f, 0.0f, static_cast<float>(swapChainTarget->mWidth), static_cast<float>(swapChainTarget->mHeight), 0.0f, 1.0f);
            cmdSetScissor(frame.m_cmd, 0, 0, static_cast<float>(swapChainTarget->mWidth), static_cast<float>(swapChainTarget->mHeight));
            cmdBindPipeline(frame.m_cmd, m_finalPipeline.m_handle);
            cmdBindPushConstants(frame.m_cmd, m_finalRootSignature, rootConstantIndex, &m_gamma);

            std::array<DescriptorData, 1> params = {};
            params[0].pName = "sourceInput";
            params[0].ppTextures = &m_finalRenderTarget[frame.m_frameIndex].m_handle->pTexture;
            updateDescriptorSet(
                    frame.m_renderer->Rend(), 0, m_finalPerFrameDescriptorSet[frame.m_frameIndex].m_handle, params.size(), params.data());
            cmdBindDescriptorSet(frame.m_cmd, 0, m_finalPerFrameDescriptorSet[frame.m_frameIndex].m_handle);
            cmdDraw(frame.m_cmd, 3, 0);
        }
        {
            cmdBindRenderTargets(frame.m_cmd, 0, NULL, NULL, NULL, NULL, NULL, -1, -1);
            std::array rtBarriers = {
                RenderTargetBarrier{ m_finalRenderTarget[frame.m_frameIndex].m_handle, RESOURCE_STATE_SHADER_RESOURCE, RESOURCE_STATE_RENDER_TARGET},
                RenderTargetBarrier{ swapChainTarget, RESOURCE_STATE_RENDER_TARGET, RESOURCE_STATE_PRESENT },
            };
            cmdResourceBarrier(frame.m_cmd, 0, NULL, 0, NULL, rtBarriers.size(), rtBarriers.data());
        }
        endCmd(m_cmds[CurrentFrameIndex()]);

        QueueSubmitDesc submitDesc = {};
        submitDesc.mCmdCount = 1;
        submitDesc.mSignalSemaphoreCount = 1;
        submitDesc.mWaitSemaphoreCount = 1;
        submitDesc.ppCmds = &frame.m_cmd;
        submitDesc.ppSignalSemaphores = &frame.m_renderCompleteSemaphore;
        submitDesc.ppWaitSemaphores = &m_imageAcquiredSemaphore;
        submitDesc.pSignalFence = frame.m_renderCompleteFence;
        queueSubmit(m_graphicsQueue, &submitDesc);

        QueuePresentDesc presentDesc = {};
        presentDesc.mIndex = m_swapChainIndex;
        presentDesc.mWaitSemaphoreCount = 1;
        presentDesc.pSwapChain = m_swapChain.m_handle;
        presentDesc.ppWaitSemaphores = &frame.m_renderCompleteSemaphore;
        presentDesc.mSubmitDone = true;
        queuePresent(m_graphicsQueue, &presentDesc);
    }

    void ForgeRenderer::InitializeRenderer(window::NativeWindowWrapper* window) {
        m_window = window;
        SyncToken token = {};
        RendererDesc desc{};
        #ifdef HPL2_RENDERDOC_ENABLED

            static RENDERDOC_API_1_1_2* rdoc_api = NULL;
            #ifdef WIN32
                if (HMODULE mod = LoadLibrary("renderdoc.dll")) {
                    pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");
                    int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_6_0, (void**)&rdoc_api);
                    assert(ret == 1);
                }
            #else
                if (void* mod = dlopen("./librenderdoc.so", RTLD_NOW)) {
                    pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)dlsym(mod, "RENDERDOC_GetAPI");
                    int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_6_0, (void**)&rdoc_api);
                    assert(ret == 1);
                }
            #endif

        #endif

        initRenderer("test", &desc, &m_renderer);

        QueueDesc queueDesc = {};
        queueDesc.mType = QUEUE_TYPE_GRAPHICS;
        queueDesc.mFlag = QUEUE_FLAG_INIT_MICROPROFILE;
        addQueue(m_renderer, &queueDesc, &m_graphicsQueue);

        for (size_t i = 0; i < m_cmds.size(); i++) {
            CmdPoolDesc cmdPoolDesc = {};
            cmdPoolDesc.pQueue = m_graphicsQueue;
            addCmdPool(m_renderer, &cmdPoolDesc, &m_cmdPools[i]);
            CmdDesc cmdDesc = {};
            cmdDesc.pPool = m_cmdPools[i];
            addCmd(m_renderer, &cmdDesc, &m_cmds[i]);
        }

        const auto windowSize = window->GetWindowSize();
        m_swapChain.Load(m_renderer, [&](SwapChain** handle) {
            SwapChainDesc swapChainDesc = {};
            swapChainDesc.mWindowHandle = m_window->ForgeWindowHandle();
            swapChainDesc.mPresentQueueCount = 1;
            swapChainDesc.ppPresentQueues = &m_graphicsQueue;
            swapChainDesc.mWidth = windowSize.x;
            swapChainDesc.mHeight = windowSize.y;
            swapChainDesc.mImageCount = SwapChainLength;
            swapChainDesc.mColorFormat = getRecommendedSwapchainFormat(false, false);
            swapChainDesc.mColorClearValue = { { 1, 1, 1, 1 } };
            swapChainDesc.mEnableVsync = false;
            addSwapChain(m_renderer, &swapChainDesc, handle);
            return true;
        });
        RootSignatureDesc graphRootDesc = {};
        addRootSignature(m_renderer, &graphRootDesc, &m_pipelineSignature);

        addSemaphore(m_renderer, &m_imageAcquiredSemaphore);
        for (auto& completeSem : m_renderCompleteSemaphores) {
            addSemaphore(m_renderer, &completeSem);
        }
        for (auto& completeFence : m_renderCompleteFences) {
            addFence(m_renderer, &completeFence);
        }
        for(auto& rt: m_finalRenderTarget) {
            rt.Load(m_renderer,[&](RenderTarget** target) {
                RenderTargetDesc renderTarget = {};
                renderTarget.mArraySize = 1;
		        renderTarget.mClearValue.depth = 1.0f;
                renderTarget.mDepth = 1;
                renderTarget.mFormat = TinyImageFormat_R8G8B8A8_UNORM;
                renderTarget.mWidth = windowSize.x;
                renderTarget.mHeight = windowSize.y;
                renderTarget.mDescriptors = DESCRIPTOR_TYPE_TEXTURE;
                renderTarget.mSampleCount = SAMPLE_COUNT_1;
                renderTarget.mSampleQuality = 0;
                renderTarget.mStartState = RESOURCE_STATE_RENDER_TARGET;
                renderTarget.pName = "final RT";
                addRenderTarget(m_renderer, &renderTarget, target);
                return true;
            });
        }
        m_pointSampler.Load(m_renderer, [&](Sampler **sampler) {
            SamplerDesc samplerDesc = {};
            addSampler(m_renderer, &samplerDesc,  sampler);
            return true;
        });

        {
            m_finalShader.Load(m_renderer, [&](Shader** shader){
                ShaderLoadDesc shaderLoadDesc = {};
                shaderLoadDesc.mStages[0].pFileName = "fullscreen.vert";
                shaderLoadDesc.mStages[1].pFileName = "final_posteffect.frag";
                addShader(m_renderer, &shaderLoadDesc, shader);
                return true;
            });

            std::array samplers = {
                m_pointSampler.m_handle
            };
            std::array samplerName = {
                (const char*)"inputSampler"
            };
            RootSignatureDesc rootDesc = { &m_finalShader.m_handle, 1 };
            rootDesc.ppStaticSamplers = samplers.data();
            rootDesc.ppStaticSamplerNames = samplerName.data();
            rootDesc.mStaticSamplerCount = 1;
            addRootSignature(m_renderer, &rootDesc, &m_finalRootSignature);

            DescriptorSetDesc setDesc = { m_finalRootSignature, DESCRIPTOR_UPDATE_FREQ_PER_FRAME, 1};
            for(auto& desc: m_finalPerFrameDescriptorSet) {
                desc.Load(m_renderer, [&](DescriptorSet** handle) {
                    addDescriptorSet(m_renderer, &setDesc, handle);
                    return true;
                });
            }
            m_finalPipeline.Load(m_renderer, [&](Pipeline** pipeline) {
                DepthStateDesc depthStateDisabledDesc = {};
                depthStateDisabledDesc.mDepthWrite = false;
                depthStateDisabledDesc.mDepthTest = false;

                RasterizerStateDesc rasterStateNoneDesc = {};
                rasterStateNoneDesc.mCullMode = CULL_MODE_NONE;

                PipelineDesc pipelineDesc = {};
                pipelineDesc.mType = PIPELINE_TYPE_GRAPHICS;
                GraphicsPipelineDesc& graphicsPipelineDesc = pipelineDesc.mGraphicsDesc;
                graphicsPipelineDesc.mPrimitiveTopo = PRIMITIVE_TOPO_TRI_LIST;
                graphicsPipelineDesc.pShaderProgram = m_finalShader.m_handle;
                graphicsPipelineDesc.pRootSignature = m_finalRootSignature;
                graphicsPipelineDesc.mRenderTargetCount = 1;
                graphicsPipelineDesc.mDepthStencilFormat = TinyImageFormat_UNDEFINED;
                graphicsPipelineDesc.pVertexLayout = NULL;
                graphicsPipelineDesc.pRasterizerState = &rasterStateNoneDesc;
                graphicsPipelineDesc.pDepthState = &depthStateDisabledDesc;
                graphicsPipelineDesc.pBlendState = NULL;

                graphicsPipelineDesc.pColorFormats = &m_swapChain.m_handle->ppRenderTargets[0]->mFormat;
                graphicsPipelineDesc.mSampleCount = m_swapChain.m_handle->ppRenderTargets[0]->mSampleCount;
                graphicsPipelineDesc.mSampleQuality = m_swapChain.m_handle->ppRenderTargets[0]->mSampleQuality;
                addPipeline(m_renderer, &pipelineDesc, pipeline);
                return true;
            });
        }
        {
            ShaderLoadDesc postProcessCopyShaderDec = {};
            postProcessCopyShaderDec.mStages[0].pFileName = "fullscreen.vert";
            postProcessCopyShaderDec.mStages[1].pFileName = "post_processing_copy.frag";
            addShader(m_renderer, &postProcessCopyShaderDec, &m_copyShader);

            RootSignatureDesc rootDesc = { &m_copyShader, 1 };
            addRootSignature(m_renderer, &rootDesc, &m_copyPostProcessingRootSignature);
            DescriptorSetDesc setDesc = { m_copyPostProcessingRootSignature, DESCRIPTOR_UPDATE_FREQ_PER_DRAW, MaxCopyFrames };
            addDescriptorSet(m_renderer, &setDesc, &m_copyPostProcessingDescriptorSet);

            DepthStateDesc depthStateDisabledDesc = {};
            depthStateDisabledDesc.mDepthWrite = false;
            depthStateDisabledDesc.mDepthTest = false;

            RasterizerStateDesc rasterStateNoneDesc = {};
            rasterStateNoneDesc.mCullMode = CULL_MODE_NONE;

            PipelineDesc pipelineDesc = {};
            pipelineDesc.mType = PIPELINE_TYPE_GRAPHICS;
            GraphicsPipelineDesc& copyPipelineDesc = pipelineDesc.mGraphicsDesc;
            copyPipelineDesc.mPrimitiveTopo = PRIMITIVE_TOPO_TRI_LIST;
            copyPipelineDesc.pShaderProgram = m_copyShader;
            copyPipelineDesc.pRootSignature = m_copyPostProcessingRootSignature;
            copyPipelineDesc.mRenderTargetCount = 1;
            copyPipelineDesc.mDepthStencilFormat = TinyImageFormat_UNDEFINED;
            copyPipelineDesc.pVertexLayout = NULL;
            copyPipelineDesc.pRasterizerState = &rasterStateNoneDesc;
            copyPipelineDesc.pDepthState = &depthStateDisabledDesc;
            copyPipelineDesc.pBlendState = NULL;

            {
                TinyImageFormat format = TinyImageFormat_R8G8B8A8_UNORM;
                copyPipelineDesc.pColorFormats = &format;
                copyPipelineDesc.mSampleCount = SAMPLE_COUNT_1;
                copyPipelineDesc.mSampleQuality = m_swapChain.m_handle->ppRenderTargets[0]->mSampleQuality;
                addPipeline(m_renderer, &pipelineDesc, &m_copyPostProcessingPipelineToUnormR8G8B8A8);
            }

            {
                copyPipelineDesc.pColorFormats = &m_swapChain.m_handle->ppRenderTargets[0]->mFormat;
                copyPipelineDesc.mSampleCount = m_swapChain.m_handle->ppRenderTargets[0]->mSampleCount;
                copyPipelineDesc.mSampleQuality = m_swapChain.m_handle->ppRenderTargets[0]->mSampleQuality;
                addPipeline(m_renderer, &pipelineDesc, &m_copyPostProcessingPipelineToSwapChain );
            }
        }

        m_windowEventHandler.Connect(window->NativeWindowEvent());
    }

    Sampler* ForgeRenderer::resolve(SamplerPoolKey key) {
        ASSERT(key.m_id < SamplerPoolKey::NumOfVariants);
        auto& sampler = m_samplerPool[key.m_id];
        if (!sampler) {
            auto renderer = Interface<ForgeRenderer>::Get()->Rend();
            SamplerDesc samplerDesc = {};
            samplerDesc.mAddressU = key.m_field.m_addressMode;
            samplerDesc.mAddressV = key.m_field.m_addressMode;
            samplerDesc.mAddressW = key.m_field.m_addressMode;
            addSampler(renderer, &samplerDesc, &sampler);
        }
        return sampler;
    }

    ForgeRenderer::ForgeRenderer():
        m_windowEventHandler(BroadcastEvent::OnPostBufferSwap, [&](window::WindowEventPayload& event) {
            switch (event.m_type) {
            case window::WindowEventType::ResizeWindowEvent: {
                    waitQueueIdle(m_graphicsQueue);
                    const auto windowSize = m_window->GetWindowSize();
                    m_swapChain.Load(m_renderer, [&](SwapChain** handle) {
                        SwapChainDesc swapChainDesc = {};
                        swapChainDesc.mWindowHandle = m_window->ForgeWindowHandle();
                        swapChainDesc.mPresentQueueCount = 1;
                        swapChainDesc.ppPresentQueues = &m_graphicsQueue;
                        swapChainDesc.mWidth = windowSize.x;
                        swapChainDesc.mHeight = windowSize.y;
                        swapChainDesc.mImageCount = SwapChainLength;
                        swapChainDesc.mColorFormat = getRecommendedSwapchainFormat(false, false);
                        swapChainDesc.mColorClearValue = { { 1, 1, 1, 1 } };
                        swapChainDesc.mEnableVsync = false;
                        addSwapChain(m_renderer, &swapChainDesc, handle);
                        return true;
                    });
                    removeSemaphore(m_renderer, m_imageAcquiredSemaphore);
                    addSemaphore(m_renderer, &m_imageAcquiredSemaphore);
                    for(auto& rt: m_finalRenderTarget) {
                        rt.Load(m_renderer,[&](RenderTarget** target) {
                            RenderTargetDesc renderTarget = {};
                            renderTarget.mArraySize = 1;
                            renderTarget.mClearValue.depth = 1.0f;
                            renderTarget.mDepth = 1;
                            renderTarget.mFormat = TinyImageFormat_R8G8B8A8_UNORM;
                            renderTarget.mWidth = windowSize.x;
                            renderTarget.mHeight = windowSize.y;
                            renderTarget.mDescriptors = DESCRIPTOR_TYPE_TEXTURE;
                            renderTarget.mSampleCount = SAMPLE_COUNT_1;
                            renderTarget.mSampleQuality = 0;
                            renderTarget.mStartState = RESOURCE_STATE_RENDER_TARGET;
                            renderTarget.pName = "final RT";
                            addRenderTarget(m_renderer, &renderTarget, target);
                            return true;
                        });
                    }
                break;
            }
            default:
                break;
            }
        }){
    }

    void ForgeRenderer::InitializeResource() {
    }

    void ForgeRenderer::cmdCopyTexture(Cmd* cmd, Texture* srcTexture, RenderTarget* dstTexture) {
        ASSERT(srcTexture !=  nullptr);
        ASSERT(dstTexture !=  nullptr);

        std::array<DescriptorData, 1> params = {};
        params[0].pName = "inputMap";
        params[0].ppTextures = &srcTexture;

        LoadActionsDesc loadActions = {};
        loadActions.mLoadActionsColor[0] = LOAD_ACTION_LOAD;
        loadActions.mLoadActionDepth = LOAD_ACTION_DONTCARE;

        cmdBindRenderTargets(cmd, 1, &dstTexture, NULL, &loadActions, NULL, NULL, -1, -1);
        cmdSetViewport(cmd, 0.0f, 0.0f, static_cast<float>(dstTexture->mWidth), static_cast<float>(dstTexture->mHeight), 0.0f, 1.0f);
        cmdSetScissor(cmd, 0, 0, dstTexture->mWidth, dstTexture->mHeight);

        updateDescriptorSet(m_renderer, m_copyRegionDescriptorIndex, m_copyPostProcessingDescriptorSet, params.size(), params.data());
        auto swapChainFormat = getRecommendedSwapchainFormat(false, false);
        switch(dstTexture->mFormat) {
            case TinyImageFormat_R8G8B8A8_UNORM:
                cmdBindPipeline(cmd, m_copyPostProcessingPipelineToUnormR8G8B8A8);
                break;
            default:
                ASSERT(false && "Unsupported format");
                break;
        }

        cmdBindDescriptorSet(cmd, m_copyRegionDescriptorIndex, m_copyPostProcessingDescriptorSet);
        cmdDraw(cmd, 3, 0);
        m_copyRegionDescriptorIndex = (m_copyRegionDescriptorIndex + 1) % MaxCopyFrames;
    }

}; // namespace hpl
