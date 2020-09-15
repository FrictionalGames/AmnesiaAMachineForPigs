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

#include "ai/AI.h"

#include "ai/AINodeGenerator.h"
#include "system/LowLevelSystem.h"
#include "system/MemoryManager.h"

namespace hpl {

	//////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	cAI::cAI() : iUpdateable("HPL_AI")
	{
		mpAINodeGenerator = hplNew( cAINodeGenerator, () );
	}

	//-----------------------------------------------------------------------

	cAI::~cAI()
	{
		hplDelete(mpAINodeGenerator);
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PUBLIC METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------
	
	void cAI::Reset()
	{

	}

	//-----------------------------------------------------------------------

	void cAI::Update(float afTimeStep)
	{
		
	}
	
	//-----------------------------------------------------------------------

	void cAI::Init()
	{

	}
	
	//-----------------------------------------------------------------------
}
