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

#ifndef HPL_HAPTIC_FORCE_HAPTX_SAWWAVE_H
#define HPL_HAPTIC_FORCE_HAPTX_SAWWAVE_H

#include "impl/HapticForceHaptX.h"

#include <HaptX.h>

namespace hpl {

	class cHapticForceHaptX_SawWave : public iHapticForceHaptX
	{
	public:
		cHapticForceHaptX_SawWave(	HaptX::HaptXInterface *apInterface,
									HaptX::iHapticDeviceID	aDeviceID,
									const cVector3f& avDirection, 
									float afAmp, float afFreq);
		~cHapticForceHaptX_SawWave();

	protected:
		HaptX::ForceModelSawWaveInfo *mpSawWaveForce;		
	};

};
#endif // HPL_HAPTIC_FORCE_SAWWAVE_H
