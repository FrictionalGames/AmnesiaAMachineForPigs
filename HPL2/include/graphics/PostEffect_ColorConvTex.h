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

#include "graphics/Image.h"
#include "graphics/PostEffect.h"

namespace hpl {

	//------------------------------------------

	class cPostEffectParams_ColorConvTex : public iPostEffectParams
	{
	public:
		cPostEffectParams_ColorConvTex() : iPostEffectParams("ColorConvTex"),
			msTextureFile(""),
			mfFadeAlpha(1)
		{}

		kPostEffectParamsClassInit(cPostEffectParams_ColorConvTex)

		tString msTextureFile;
		float mfFadeAlpha;
	};

	//------------------------------------------

	class cPostEffectType_ColorConvTex : public iPostEffectType
	{
	friend class cPostEffect_ColorConvTex;
	public:
	    static constexpr uint32_t DescriptorSetSize = 64;

        cPostEffectType_ColorConvTex(cGraphics *apGraphics, cResources *apResources);
		virtual ~cPostEffectType_ColorConvTex();

		iPostEffect *CreatePostEffect(iPostEffectParams *apParams);

	private:
        uint32_t m_descIndex = 0;
        SharedPipeline m_pipeline;
        SharedShader m_shader;
        SharedRootSignature m_rootSignature;
        SharedSampler m_inputSampler;
        std::array<SharedDescriptorSet, ForgeRenderer::SwapChainLength> m_perFrameDescriptorSet;
	};

	//------------------------------------------

	class cPostEffect_ColorConvTex : public iPostEffect
	{
	public:
        virtual void RenderEffect(cPostEffectComposite& compositor, cViewport& viewport, const ForgeRenderer::Frame& frame, Texture* inputTexture, RenderTarget* renderTarget) override;

        cPostEffect_ColorConvTex(cGraphics *apGraphics, cResources *apResources, iPostEffectType *apType);
		virtual ~cPostEffect_ColorConvTex();

	private:
		virtual void OnSetParams() override;
		virtual iPostEffectParams *GetTypeSpecificParams() override { return &mParams; }

		Image* mpColorConvTex = nullptr;
		cPostEffectType_ColorConvTex* mpSpecificType = nullptr;
		cPostEffectParams_ColorConvTex mParams;
	};

	//------------------------------------------

};
