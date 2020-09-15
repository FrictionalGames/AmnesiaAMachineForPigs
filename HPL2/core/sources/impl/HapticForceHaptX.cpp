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

#include "impl/HapticForceHaptX.h"

#include "system/LowLevelSystem.h"

namespace hpl {

	using namespace HaptX;
	using namespace Vectors;

	//////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	iHapticForceHaptX::iHapticForceHaptX(	eHapticForceType aType,
											HaptX::HaptXInterface *apInterface,
											HaptX::iHapticDeviceID	aDeviceID)
											: iHapticForce(aType)
	{
		mpInterface = apInterface;
		mDeviceID = aDeviceID;
		mpForce = NULL;
	}
	
	iHapticForceHaptX::~iHapticForceHaptX()
	{
		if(mpForce) mpInterface->ForceModelDestroy(mpForce->GetForceModelID());
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PUBLIC METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	void iHapticForceHaptX::SetActive(bool abX)
	{
		if(abX)	mpInterface->ForceModelStart(mpForce->GetForceModelID());
		else	mpInterface->ForceModelStop(mpForce->GetForceModelID());
	}
	bool iHapticForceHaptX::IsActive()
	{
		bool bRet;
		mpInterface->ForceModelIsStarted(bRet,mpForce->GetForceModelID());
		return bRet;
	}
	
	void iHapticForceHaptX::SetTimeControl(bool abLoop,float afTime,float afIdleTime, float afFadeInTime, float afFadeOutTime)
	{
		mpForce->SetTimeControl(abLoop,afTime,afIdleTime,
								afFadeInTime, afFadeOutTime);
	}

	//-----------------------------------------------------------------------

}
