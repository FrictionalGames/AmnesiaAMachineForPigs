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

#include "graphics/PostEffectComposite.h"

#include "engine/Interface.h"

#include "math/MathTypes.h"
#include "scene/Viewport.h"
#include "system/LowLevelSystem.h"

#include "graphics/Graphics.h"
#include "graphics/LowLevelGraphics.h"
#include "graphics/PostEffect.h"
#include "graphics/Texture.h"
#include "graphics/VertexBuffer.h"
#include "graphics/ForgeHandles.h"
#include "graphics/GraphicsTypes.h"
#include "graphics/Image.h"
#include "graphics/RenderTarget.h"

#include <algorithm>
#include <functional>
#include <memory>

namespace hpl {

    cPostEffectComposite::cPostEffectComposite(cGraphics* apGraphics) {
        mpGraphics = apGraphics;
        SetupRenderFunctions(mpGraphics->GetLowLevel());
    }

    cPostEffectComposite::~cPostEffectComposite() {
    }

    bool cPostEffectComposite::Draw(const ForgeRenderer::Frame& frame, cViewport& viewport, float frameTime, Texture* inputTexture, RenderTarget* renderTarget) {
        mfCurrentFrameTime = frameTime;
        auto renderer = Interface<ForgeRenderer>::Get();
        auto boundData = m_boundCompositorData.resolve(viewport);
        if(!boundData || boundData->m_size != viewport.GetSize()) {
            auto viewPortSize = viewport.GetSize();
            auto result = std::make_unique<PostEffectCompositorData>();
            result->m_size = viewPortSize;
            auto* renderer = Interface<ForgeRenderer>::Get();
            result->m_renderTarget[0].Load(renderer->Rend(),[&](RenderTarget** texture) {
                RenderTargetDesc renderTarget = {};
                renderTarget.mArraySize = 1;
                renderTarget.mDepth = 1;
                renderTarget.mDescriptors = DESCRIPTOR_TYPE_TEXTURE;
                renderTarget.mWidth = result->m_size.x;
                renderTarget.mHeight = result->m_size.y;
                renderTarget.mSampleCount = SAMPLE_COUNT_1;
                renderTarget.mSampleQuality = 0;
                renderTarget.mFormat = TinyImageFormat_R8G8B8A8_UNORM;
                renderTarget.mStartState = RESOURCE_STATE_SHADER_RESOURCE;
                addRenderTarget(renderer->Rend(), &renderTarget, texture);
                return true;
            });

            result->m_renderTarget[1].Load(renderer->Rend(),[&](RenderTarget** texture) {
                RenderTargetDesc renderTarget = {};
                renderTarget.mArraySize = 1;
                renderTarget.mDepth = 1;
                renderTarget.mDescriptors = DESCRIPTOR_TYPE_TEXTURE;
                renderTarget.mWidth = result->m_size.x;
                renderTarget.mHeight = result->m_size.y;
                renderTarget.mSampleCount = SAMPLE_COUNT_1;
                renderTarget.mSampleQuality = 0;
                renderTarget.mFormat = TinyImageFormat_R8G8B8A8_UNORM;
                renderTarget.mStartState = RESOURCE_STATE_RENDER_TARGET;
                addRenderTarget(renderer->Rend(), &renderTarget, texture);
                return true;
            });
            boundData = m_boundCompositorData.update(viewport, std::move(result));
        }

        frame.m_resourcePool->Push(boundData->m_renderTarget[0]);
        frame.m_resourcePool->Push(boundData->m_renderTarget[1]);

        auto cmdBindRenderTarget = [&](size_t pogoTargetIndex) {
            if(pogoTargetIndex == boundData->m_pogoTargetIndex) {
                return;
            }
            if(pogoTargetIndex == 0) {
                cmdBindRenderTargets(frame.m_cmd, 0, NULL, NULL, NULL, NULL, NULL, -1, -1);
                std::array rtBarriers = {
                    RenderTargetBarrier{ boundData->m_renderTarget[0].m_handle, RESOURCE_STATE_SHADER_RESOURCE, RESOURCE_STATE_RENDER_TARGET },
                    RenderTargetBarrier{ boundData->m_renderTarget[1].m_handle, RESOURCE_STATE_RENDER_TARGET, RESOURCE_STATE_SHADER_RESOURCE},
                };
                cmdResourceBarrier(frame.m_cmd, 0, NULL, 0, NULL, rtBarriers.size(), rtBarriers.data());
            } else {
                cmdBindRenderTargets(frame.m_cmd, 0, NULL, NULL, NULL, NULL, NULL, -1, -1);
                std::array rtBarriers = {
                    RenderTargetBarrier{ boundData->m_renderTarget[0].m_handle, RESOURCE_STATE_RENDER_TARGET, RESOURCE_STATE_SHADER_RESOURCE},
                    RenderTargetBarrier{ boundData->m_renderTarget[1].m_handle, RESOURCE_STATE_SHADER_RESOURCE, RESOURCE_STATE_RENDER_TARGET },
                };
                cmdResourceBarrier(frame.m_cmd, 0, NULL, 0, NULL, rtBarriers.size(), rtBarriers.data());
            }
            boundData->m_pogoTargetIndex = pogoTargetIndex;
        };

        auto it = m_postEffects.begin();
        size_t currentIndex = 0;
        bool isSavedToPrimaryRenderTarget = false;
        bool hasEffect = false;
        for (; it != m_postEffects.end(); ++it) {
            if (!it->_effect->IsActive()) {
                continue;
            }
            hasEffect = true;
            cmdBindRenderTarget(currentIndex);
            it->_effect->RenderEffect(*this, viewport, frame, inputTexture, boundData->m_renderTarget[currentIndex].m_handle);
        }

        // this happens when there are no post effects
        if (!hasEffect) {
            renderer->cmdCopyTexture(frame.m_cmd, inputTexture, renderTarget);
            return false;
        }

        while (it != m_postEffects.end()) {
            if (!it->_effect->IsActive()) {
                it++;
                continue;
            }
            auto nextIt = ([&]() {
                for (; it != m_postEffects.end(); ++it) {
                    if (!it->_effect->IsActive()) {
                        continue;
                    }
                    return it;
                }
                return m_postEffects.end();
            })();
            size_t nextIndex = (currentIndex + 1) % 2;
            if (nextIt == m_postEffects.end()) {
                isSavedToPrimaryRenderTarget = true;
                cmdBindRenderTarget(currentIndex);
                it->_effect->RenderEffect(*this, viewport, frame, boundData->m_renderTarget[currentIndex].m_handle->pTexture, renderTarget);
            } else {
                cmdBindRenderTarget(nextIndex);
                it->_effect->RenderEffect(*this, viewport, frame, boundData->m_renderTarget[currentIndex].m_handle->pTexture, boundData->m_renderTarget[nextIndex].m_handle);
            }
            currentIndex = nextIndex;
            it = nextIt;
        }
        if (!isSavedToPrimaryRenderTarget) {
            cVector2l vRenderTargetSize = viewport.GetSize();
            cRect2l rect = cRect2l(0, 0, vRenderTargetSize.x, vRenderTargetSize.y);
            cmdBindRenderTarget((currentIndex + 1) % 2);
            renderer->cmdCopyTexture(frame.m_cmd, boundData->m_renderTarget[currentIndex].m_handle->pTexture, renderTarget);
        }
        return true;
    }

    void cPostEffectComposite::AddPostEffect(iPostEffect* apPostEffect, int alPrio) {
        ASSERT(apPostEffect && "Post Effect is not defined");
        if (!apPostEffect) {
            return;
        }
        const auto id = m_postEffects.size();
        m_postEffects.push_back({ id, alPrio, apPostEffect });
        std::sort(m_postEffects.begin(), m_postEffects.end(), [](const auto& a, const auto& b) {
            return a._index > b._index;
        });
    }

    //-----------------------------------------------------------------------

    bool cPostEffectComposite::HasActiveEffects() {
        for (const auto& eff : m_postEffects) {
            if (eff._effect->IsActive()) {
                return true;
            }
        }
        return false;
    }

} // namespace hpl
