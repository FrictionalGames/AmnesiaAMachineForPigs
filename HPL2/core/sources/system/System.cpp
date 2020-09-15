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

#include "system/System.h"
#include "system/LowLevelSystem.h"
#include "system/LogicTimer.h"
#include "system/String.h"

namespace hpl {
	
	//////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------
	
	cSystem::cSystem(iLowLevelSystem *apLowLevelSystem)
	{
		mpLowLevelSystem = apLowLevelSystem;
	}
	
	//-----------------------------------------------------------------------
	
	cSystem::~cSystem()
	{
		Log("Exiting System Module\n");
		Log("--------------------------------------------------------\n");
		
		Log("--------------------------------------------------------\n\n");
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PUBLIC METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------
	cLogicTimer * cSystem::CreateLogicTimer(unsigned int alUpdatesPerSec)
	{
		return hplNew( cLogicTimer, (alUpdatesPerSec, mpLowLevelSystem) );
	}
	
	//-----------------------------------------------------------------------

	iLowLevelSystem* cSystem::GetLowLevel()
	{
		return mpLowLevelSystem;
	}
	
	//-----------------------------------------------------------------------

}
