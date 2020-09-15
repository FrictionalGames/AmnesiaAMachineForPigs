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

#ifndef HPL_POSTEFFECT_COLOR_GRADING_H
#define HPL_POSTEFFECT_COLOR_GRADING_H

#include "graphics/PostEffect.h"

namespace hpl {

	//------------------------------------------
	
    typedef std::map<tString, iTexture*> tTextureMap;

    typedef std::map<tString, iTexture*>::iterator tTextureMapIt;

	class cPostEffectParams_ColorGrading : public iPostEffectParams
	{
	public:
		cPostEffectParams_ColorGrading() : iPostEffectParams("ColorGrading"),  
			msTextureFile1("colorgrading_base.png"),
			msTextureFile2(""),
            mfCrossFadeAlpha( 0.0f ),
            mbIsReinitialisation( false )
		{}
		
		kPostEffectParamsClassInit(cPostEffectParams_ColorGrading)
		
		tString msTextureFile1;
		tString msTextureFile2;
		float mfCrossFadeAlpha;
        bool mbIsReinitialisation;
	};

	//------------------------------------------
	
	class cPostEffectType_ColorGrading : public iPostEffectType
	{
	friend class cPostEffect_ColorGrading;
	public:
		cPostEffectType_ColorGrading(cGraphics *apGraphics, cResources *apResources);
		virtual ~cPostEffectType_ColorGrading();

		iPostEffect *CreatePostEffect(iPostEffectParams *apParams);
	
	private:
		iGpuProgram *mpProgram[2]; //0=no fade, 1=fade
	};
	
	//------------------------------------------

	class cPostEffect_ColorGrading : public iPostEffect
	{
	public:
		cPostEffect_ColorGrading(cGraphics *apGraphics, cResources *apResources, iPostEffectType *apType);
		~cPostEffect_ColorGrading();

        void SetCrossFadeAlpha( float afCrossFadeAlpha );

	private:
		void OnSetParams();
		iPostEffectParams *GetTypeSpecificParams() { return &mParams; }
		
		iTexture* RenderEffect(iTexture *apInputTexture, iFrameBuffer *apFinalTempBuffer);

        iTexture * LoadLUT( tString asLUTName );
		
		cPostEffectType_ColorGrading* mpSpecificType;

		cPostEffectParams_ColorGrading mParams;

        iTexture
            * mpLUT1,
            * mpLUT2;
        tTextureMap
            mLUTMap;
	};

	//------------------------------------------

};
#endif // HPL_POSTEFFECT_COLOR_GRADING_H
