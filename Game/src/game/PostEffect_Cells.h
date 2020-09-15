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

#ifndef PIGS_POSTEFFECT_CELLS_H
#define PIGS_POSTEFFECT_CELLS_H

#include "graphics/PostEffect.h"

namespace hpl
{
	class cPostEffectParams_Cells : public iPostEffectParams
	{
	public:

		cPostEffectParams_Cells() : iPostEffectParams( "Cells" )
		{
		}

		kPostEffectParamsClassInit(cPostEffectParams_Cells)
	};

	//-----------------------------------------------------------------------

	class cPostEffectType_Cells : public iPostEffectType
	{
		friend class cPostEffect_Cells;
	
	public:

		cPostEffectType_Cells(cGraphics * apGraphics, cResources * apResources);
		virtual ~cPostEffectType_Cells();

		iPostEffect * CreatePostEffect(iPostEffectParams * apParams);
	};

	//-----------------------------------------------------------------------

	class cPostEffect_Cells : public iPostEffect
	{
	public:
		cPostEffect_Cells(cGraphics *apGraphics,cResources *apResources, iPostEffectType *apType);
		~cPostEffect_Cells();

	private:
		void OnSetParams();
		iPostEffectParams *GetTypeSpecificParams() { return &mParams; }

		iTexture* RenderEffect(iTexture *apInputTexture, iFrameBuffer *apFinalTempBuffer) { return NULL; }

		cPostEffectParams_Cells mParams;
	};
};

#endif