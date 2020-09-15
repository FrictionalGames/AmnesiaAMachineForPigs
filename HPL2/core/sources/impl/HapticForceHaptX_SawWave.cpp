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

#include "impl/HapticForceHaptX_SawWave.h"

#include "system/LowLevelSystem.h"

namespace hpl {

	using namespace HaptX;
	using namespace Vectors;

	//////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	cHapticForceHaptX_SawWave::cHapticForceHaptX_SawWave(	HaptXInterface *apInterface,
															iHapticDeviceID	aDeviceID,
															const cVector3f& avDirection, 
															float afAmp, float afFreq)
						: iHapticForceHaptX(eHapticForceType_SawWave,apInterface, aDeviceID)
	{
		mpInterface->ForceModelCreateSawWave(&mpSawWaveForce,mDeviceID,Vec3f(avDirection.x,avDirection.y,avDirection.z),
												afAmp,afFreq);
		mpForce = mpSawWaveForce;
	}
	
	cHapticForceHaptX_SawWave::~cHapticForceHaptX_SawWave()
	{
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PUBLIC METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------
	
	//-----------------------------------------------------------------------

}
