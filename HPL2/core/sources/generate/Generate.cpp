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

#include "generate/Generate.h"

#include "system/LowLevelSystem.h"

#include "generate/VoxelMap.h"


namespace hpl {

	//////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	cGenerate::cGenerate()
	{
	}

	//-----------------------------------------------------------------------

	cGenerate::~cGenerate()
	{
		Log("Exiting Generate Module\n");
		Log("--------------------------------------------------------\n");

		Log("--------------------------------------------------------\n\n");
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PUBLIC METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------
	
	void cGenerate::Init(cResources *apResources, cGraphics *apGraphics)
	{
		mpResources = apResources;
		mpGraphics = apGraphics;
	}

	//-----------------------------------------------------------------------

	cVoxelMap* cGenerate::CreateVoxelMap(const cVector3l& avSize)
	{
		return hplNew(cVoxelMap, (avSize));
	}

	void cGenerate::DestroyVoxelMap(cVoxelMap* apMap)
	{
		hplDelete(apMap);
	}

	//-----------------------------------------------------------------------

}
