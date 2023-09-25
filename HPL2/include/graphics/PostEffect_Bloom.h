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

#include "graphics/PostEffect.h"
#include "graphics/RenderTarget.h"
#include "math/MathTypes.h"

namespace hpl {

	//------------------------------------------

	class cPostEffectParams_Bloom : public iPostEffectParams
	{
	public:
		cPostEffectParams_Bloom() : iPostEffectParams("Bloom"),
			mvRgbToIntensity(0.3f, 0.58f, 0.12f),
			mlBlurIterations(2),
			mfBlurSize(1.0f)
		{}

		kPostEffectParamsClassInit(cPostEffectParams_Bloom)

        cVector3f mvRgbToIntensity;

		float mfBlurSize;
		int mlBlurIterations;
	};

	//------------------------------------------

	class cPostEffectType_Bloom : public iPostEffectType
	{
	friend class cPostEffect_Bloom;
	public:
		cPostEffectType_Bloom(cGraphics *apGraphics, cResources *apResources);
		virtual ~cPostEffectType_Bloom();

		iPostEffect *CreatePostEffect(iPostEffectParams *apParams);

	private:
        static constexpr uint32_t DescriptorSetSize = 16;

        SharedSampler m_inputSampler;
        SharedShader m_blurShader;
        SharedRootSignature m_blurSignature;
        std::array<SharedDescriptorSet, ForgeRenderer::SwapChainLength> m_perFrameDescriptorSets;
        uint32_t m_setIndex = 0;
        SharedPipeline m_blurPipeline;

        SharedRootSignature m_bloomRootSignature;
        SharedShader m_bloomShader;
        std::array<SharedDescriptorSet, ForgeRenderer::SwapChainLength> m_perFrameDescriptorBloomSets;
        uint32_t m_setBloomIndex = 0;
        SharedPipeline m_bloomPipeline;
	};

	//------------------------------------------

	class cPostEffect_Bloom : public iPostEffect
	{
	public:
		cPostEffect_Bloom(cGraphics *apGraphics,cResources *apResources, iPostEffectType *apType);
		~cPostEffect_Bloom();

		struct BloomData {
		public:
			BloomData() = default;
			BloomData(const BloomData&) = delete;
			BloomData(BloomData&& buffer):
				m_size(buffer.m_size),
				m_blurTargets(std::move(buffer.m_blurTargets))
			{}

			BloomData& operator=(const BloomData&) = delete;
			BloomData& operator=(BloomData&& buffer) {
				m_blurTargets = std::move(buffer.m_blurTargets);
				m_size = buffer.m_size;
				return *this;
			}
			cVector2l m_size;
            std::array<SharedRenderTarget, 2> m_blurTargets;
		};
	private:
		virtual void OnSetParams() override;
		virtual iPostEffectParams *GetTypeSpecificParams() override { return &mParams; }
        virtual void RenderEffect(cPostEffectComposite& compositor, cViewport& viewport, const ForgeRenderer::Frame& frame, Texture* inputTexture, RenderTarget* renderTarget) override;

        UniqueViewportData<BloomData> m_boundBloomData;

		cPostEffectType_Bloom *mpBloomType;
		cPostEffectParams_Bloom mParams;
	};

	//------------------------------------------

};
