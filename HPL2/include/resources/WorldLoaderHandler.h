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

#ifndef HPL_WORLD_LOADER_HANDLER_H
#define HPL_WORLD_LOADER_HANDLER_H

#include "resources/ResourceLoaderHandler.h"

#include "math/MathTypes.h"
#include "system/SystemTypes.h"
#include "resources/ResourcesTypes.h"

namespace hpl {

	class cResources;
	class cWorld;
	class cScene;
	class cGraphics;
	class cPhysics;

	//--------------------------------

	class cWorldLoaderHandler : public iResourceLoaderHandler
	{
	public:
		cWorldLoaderHandler(cResources* apResources,cGraphics *apGraphics, cScene *apScene, cPhysics *apPhysics);
		~cWorldLoaderHandler();

		cWorld* LoadWorld(const tWString& asFile,tWorldLoadFlag aFlags);

	private:
		void SetupLoader(iResourceLoader *apLoader);

		cGraphics *mpGraphics;
		cResources* mpResources;
		cScene* mpScene;
		cPhysics *mpPhysics;
	};

};
#endif // HPL_WORLD_LOADER_HANDLER_H
