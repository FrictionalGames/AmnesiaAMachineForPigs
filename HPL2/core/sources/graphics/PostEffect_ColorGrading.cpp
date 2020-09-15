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

#include "graphics/PostEffect_ColorGrading.h"

#include "graphics/Graphics.h"
#include "resources/Resources.h"

#include "graphics/LowLevelGraphics.h"
#include "graphics/PostEffectComposite.h"
#include "graphics/FrameBuffer.h"
#include "graphics/Texture.h"
#include "graphics/GPUProgram.h"
#include "graphics/GPUShader.h"

#include "resources/TextureManager.h"

#include "system/PreprocessParser.h"
#include "system/String.h"

#include "math/Math.h"

#include "system/Platform.h"

namespace hpl {
	
	//////////////////////////////////////////////////////////////////////////
	// PROGRAM VARS
	//////////////////////////////////////////////////////////////////////////

	#define kVar_afCrossFadeAlpha	0

	//////////////////////////////////////////////////////////////////////////
	// POST EFFECT BASE
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------
	
	cPostEffectType_ColorGrading::cPostEffectType_ColorGrading(cGraphics *apGraphics, cResources *apResources) : iPostEffectType("ColorGrading",apGraphics,apResources)
	{
		///////////////////////////
		// Load programs
		for(int i=0; i<2; ++i)
		{
			cParserVarContainer vars;
			vars.Add("UseUv");
			if(i==1) vars.Add("IsCrossFading");

			mpProgram[i] = mpGraphics->CreateGpuProgramFromShaders("ColorGrading"+cString::ToString(i),"deferred_base_vtx.glsl",
																	"posteffect_color_grading_frag.glsl", &vars);
			if(i==1)
			{
				if(mpProgram[i])
				{
					mpProgram[i]->GetVariableAsId("afCrossFadeAlpha",kVar_afCrossFadeAlpha);
				}
			}
		}
	}
	
	//-----------------------------------------------------------------------

	cPostEffectType_ColorGrading::~cPostEffectType_ColorGrading()
	{

	}

	//-----------------------------------------------------------------------

	iPostEffect * cPostEffectType_ColorGrading::CreatePostEffect(iPostEffectParams *apParams)
	{
		cPostEffect_ColorGrading *pEffect = hplNew(cPostEffect_ColorGrading, (mpGraphics,mpResources,this));
		cPostEffectParams_ColorGrading *pColorCorrectionParams = static_cast<cPostEffectParams_ColorGrading*>(apParams);

		return pEffect;
	}
	
	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// POST EFFECT
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	cPostEffect_ColorGrading::cPostEffect_ColorGrading(cGraphics *apGraphics,cResources *apResources, iPostEffectType *apType) : iPostEffect(apGraphics,apResources,apType)
	{
		mpSpecificType = static_cast<cPostEffectType_ColorGrading*>(mpType);
		
		mpLUT1 = NULL;
		mpLUT2 = NULL;
	}

	//-----------------------------------------------------------------------

	cPostEffect_ColorGrading::~cPostEffect_ColorGrading()
	{
        for(tTextureMapIt it = mLUTMap.begin(); it != mLUTMap.end(); ++it)
        {
            if ( it->second )
            {
                mpResources->GetTextureManager()->Destroy(it->second);
            }
        }
	}

	//-----------------------------------------------------------------------

	void cPostEffect_ColorGrading::OnSetParams()
	{
        if ( mParams.mbIsReinitialisation )
        {
            for(tTextureMapIt it = mLUTMap.begin(); it != mLUTMap.end(); ++it)
            {
                mpResources->GetTextureManager()->Destroy(it->second);
            }
            
            mLUTMap.clear();
        }

        mpLUT1 = LoadLUT( mParams.msTextureFile1 );
        mpLUT2 = LoadLUT( mParams.msTextureFile2 );
	}

	//-----------------------------------------------------------------------

    void cPostEffect_ColorGrading::SetCrossFadeAlpha( float afCrossFadeAlpha )
    {
        mParams.mfCrossFadeAlpha = afCrossFadeAlpha;
    }

	//-----------------------------------------------------------------------

    iTexture * cPostEffect_ColorGrading::LoadLUT( tString asLUTName )
    {
		if ( asLUTName == "" || IsDisabled()) return NULL;

		asLUTName = cString::GetFileName(asLUTName);

        tTextureMapIt it = mLUTMap.find(asLUTName);

		if(it != mLUTMap.end())
		{
			return it->second;
		}
        else
        {
            iTexture
                * texture;

            if ( cPlatform::FileExists( _W("textures/gradingmaps/") + cString::To16Char(asLUTName) )
                || cPlatform::FileExists( cString::To16Char(asLUTName) ) )
            {
		        texture = mpResources->GetTextureManager()->CreateFlattened3D(asLUTName,false);
                texture->SetWrapSTR(eTextureWrap_ClampToEdge);
                texture->SetFilter(eTextureFilter_Trilinear);
            }
            else
            {
		        return LoadLUT("colorgrading_base.png");
            }

            mLUTMap[ asLUTName ] = texture;
            return texture;
        }
    }

	//-----------------------------------------------------------------------

	iTexture* cPostEffect_ColorGrading::RenderEffect(iTexture *apInputTexture, iFrameBuffer *apFinalTempBuffer)
	{
		if(mpLUT1 == NULL && mpLUT2 == NULL) return apInputTexture;

		/////////////////////////
		// Init render states
		mpCurrentComposite->SetFlatProjection();
		mpCurrentComposite->SetBlendMode(eMaterialBlendMode_None);
		mpCurrentComposite->SetChannelMode(eMaterialChannelMode_RGBA);

		/////////////////////////
		// Render the to final buffer
		// This function sets to frame buffer is post effect is last!
        SetFinalFrameBuffer(apFinalTempBuffer);

		mpCurrentComposite->SetTexture(0, apInputTexture);

        float fAlpha = cMath::Clamp(mParams.mfCrossFadeAlpha,0.0f, 1.0f );

        if ( fAlpha == 0.0f || std::abs(fAlpha - 1.0f ) < kEpsilonf )
        {
            mpCurrentComposite->SetTexture(1, fAlpha == 0.0f ? mpLUT1 : mpLUT2);
		    mpCurrentComposite->SetProgram(mpSpecificType->mpProgram[0]);
        }
        else
        {
            mpCurrentComposite->SetTexture(1, mpLUT1);
		    mpCurrentComposite->SetTexture(2, mpLUT2);

		    mpCurrentComposite->SetProgram(mpSpecificType->mpProgram[1]);
		    mpSpecificType->mpProgram[1]->SetFloat(kVar_afCrossFadeAlpha,fAlpha);
        }
		
		DrawQuad(0,1,apInputTexture, true);
		
		return apFinalTempBuffer->GetColorBuffer(0)->ToTexture();
	}

	//-----------------------------------------------------------------------

}
