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

#ifndef HPL_RENDERER_SIMPLE_H
#define HPL_RENDERER_SIMPLE_H

#include "graphics/Renderer.h"

namespace hpl {

    //---------------------------------------------
	
	class iFrameBuffer;
	class iDepthStencilBuffer;
	class iTexture;
	class iLight;
	
	//---------------------------------------------
	
	class cRendererSimple : public  iRenderer
	{
	public:
		cRendererSimple(cGraphics *apGraphics,cResources* apResources);
		~cRendererSimple();
		
		bool LoadData();
		void DestroyData();

		static void SetUseShaders(bool abX){mbUseShaders = abX; }
		static bool GetUseShaders(){return mbUseShaders; }

	private:
		void CopyToFrameBuffer();
		void SetupRenderList();
		void RenderObjects();

		static bool mbUseShaders;

		iGpuProgram *mpFlatProgram;
		iGpuProgram *mpDiffuseProgram;
	};

	//---------------------------------------------

};
#endif // HPL_RENDERER_WIRE_FRAME_H
