/*
 * Copyright © 2011-2020 Frictional Games
 * 
 * This file is part of Amnesia: A Machine For Pigs.
 * 
 * Amnesia: A Machine For Pigs is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version. 

 * Amnesia: A Machine For Pigs is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Amnesia: A Machine For Pigs.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef HPL_POSTEFFECT_COLOR_CONV_TEX_H
#define HPL_POSTEFFECT_COLOR_CONV_TEX_H

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
		cPostEffectType_ColorConvTex(cGraphics *apGraphics, cResources *apResources);
		virtual ~cPostEffectType_ColorConvTex();

		iPostEffect *CreatePostEffect(iPostEffectParams *apParams);
	
	private:
		iGpuProgram *mpProgram[2]; //0=no fade, 1=fade
	};
	
	//------------------------------------------

	class cPostEffect_ColorConvTex : public iPostEffect
	{
	public:
		cPostEffect_ColorConvTex(cGraphics *apGraphics, cResources *apResources, iPostEffectType *apType);
		~cPostEffect_ColorConvTex();

	private:
		void OnSetParams();
		iPostEffectParams *GetTypeSpecificParams() { return &mParams; }
		
		iTexture* RenderEffect(iTexture *apInputTexture, iFrameBuffer *apFinalTempBuffer);

		iTexture *mpColorConvTex;

		cPostEffectType_ColorConvTex* mpSpecificType;

		cPostEffectParams_ColorConvTex mParams;
	};

	//------------------------------------------

};
#endif // HPL_POSTEFFECT_COLOR_CONV_TEX_H
