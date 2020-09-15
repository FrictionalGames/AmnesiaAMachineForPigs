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

#ifndef HPL_LIGHT_POINT_H
#define HPL_LIGHT_POINT_H

#include "scene/Light.h"

namespace hpl {

	//------------------------------------------

	class cLightPoint : public iLight
	{
	#ifdef __GNUC__
		typedef iLight __super;
	#endif
	public:
		cLightPoint(tString asName, cResources *apResources);

	private:
		void UpdateBoundingVolume();
	};

};
#endif // HPL_LIGHT_POINT_H
