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

#include "graphics/Enum.h"
#include <graphics/GraphicsTypes.h>
#include <span>
#include <array>

namespace hpl {

	class iRenderable;
	class iLight;
	class cFrustum;
	class cFogArea;

	class cRenderList
	{
	public:

		cRenderList();
		~cRenderList();

        [[deprecated("use BeginAndReset")]]
		void Setup(float afFrameTime, cFrustum *apFrustum);
        [[deprecated("Use BeginAndReset")]]
		void Clear();

		void AddObject(iRenderable *apObject);

        [[deprecated("Use End")]]
		void Compile(tRenderListCompileFlag aFlags);

		bool ArrayHasObjects(eRenderListType aType);

		std::span<iRenderable*> GetRenderableItems(eRenderListType aType);
		std::span<iRenderable*> GetOcclusionQueryItems();
		std::span<cFogArea*> GetFogAreas();
		std::span<iLight*> GetLights();

        void BeginAndReset(float frameTime, cFrustum* frustum);
        void End(tRenderListCompileFlag aFlags);

		iLight* GetLight(int alIdx){ return m_lights[alIdx];}
		int GetLightNum(){ return(int)m_lights.size();}

		cFogArea* GetFogArea(int alIdx){ return m_fogAreas[alIdx];}
		int GetFogAreaNum(){ return(int)m_fogAreas.size();}

		void PrintAllObjects();

		//Temp:
		int GetSolidObjectNum(){ return (int)m_solidObjects.size();}
		iRenderable* GetSolidObject(int alIdx){ return m_solidObjects[alIdx];}

		int GetTransObjectNum(){ return (int)m_transObjects.size();}
		iRenderable* GetTransObject(int alIdx){ return m_transObjects[alIdx];}

	private:
		float m_frameTime = 0.0f;
		cFrustum* m_frustum = nullptr;

		std::vector<iRenderable*> m_occlusionQueryObjects;
		std::vector<iRenderable*> m_solidObjects;
		std::vector<iRenderable*> m_transObjects;
		std::vector<iRenderable*> m_decalObjects;
		std::vector<iRenderable*> m_illumObjects;
		std::vector<iLight*> m_lights;
		std::vector<cFogArea*> m_fogAreas;
		std::array<std::vector<iRenderable*>,eRenderListType_LastEnum> m_sortedArrays;
	};

	//---------------------------------------------

};
