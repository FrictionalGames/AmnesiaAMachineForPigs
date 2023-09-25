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
#pragma once

#include "math/MathTypes.h"
#include "scene/Viewport.h"
#include <windowing/NativeWindow.h>

#include <array>
#include <memory>
#include <vector>

#include <graphics/RenderTarget.h>
#include <graphics/ForgeRenderer.h>
#include <graphics/ForgeHandles.h>
#include <graphics/Image.h>
#include <graphics/RenderFunctions.h>

namespace hpl {

    class cGraphics;
    class iLowLevelGraphics;
    class iPostEffect;

    class cPostEffectComposite : public iRenderFunctions {
    public:
        cPostEffectComposite(cGraphics* apGraphics);
        ~cPostEffectComposite();

        struct PostEffectCompositorData {
        public:
        	PostEffectCompositorData() = default;
			PostEffectCompositorData(const PostEffectCompositorData&) = delete;
			PostEffectCompositorData(PostEffectCompositorData&& other):
				m_size(other.m_size),
                m_renderTarget(std::move(other.m_renderTarget)),
		        m_pogoTargetIndex(other.m_pogoTargetIndex)
            {}

			PostEffectCompositorData& operator=(const PostEffectCompositorData&) = delete;
			void operator=(PostEffectCompositorData&& other) {
                m_renderTarget = std::move(other.m_renderTarget);
				m_size = other.m_size;
			    m_pogoTargetIndex = other.m_pogoTargetIndex;
            }

			cVector2l m_size = cVector2l(0, 0);
            uint32_t m_pogoTargetIndex = 1;
            // 2 render targets two ping pong between
            std::array<SharedRenderTarget, 2> m_renderTarget;
        };


        bool Draw(const ForgeRenderer::Frame&, cViewport& viewport, float frameTime, Texture* inputTexture, RenderTarget* renderTarget);

        /**
         * Highest prio is first!
         */
        void AddPostEffect(iPostEffect* apPostEffect, int alPrio);

        inline int GetPostEffectNum() const {
            return m_postEffects.size();
        }

        inline iPostEffect* GetPostEffect(int alIdx) const {
            for (auto& it : m_postEffects) {
                if (it._id == alIdx) {
                    return it._effect;
                }
            }
            return nullptr;
        }

        bool HasActiveEffects();

        float GetCurrentFrameTime() {
            return mfCurrentFrameTime;
        }

    private:
        struct PostEffectEntry {
            size_t _id;
            int _index;
            iPostEffect* _effect;
        };

		UniqueViewportData<PostEffectCompositorData> m_boundCompositorData;
        window::WindowEvent::Handler m_windowEvent;
        std::vector<PostEffectEntry> m_postEffects;
        float mfCurrentFrameTime;
    };

}; // namespace hpl
