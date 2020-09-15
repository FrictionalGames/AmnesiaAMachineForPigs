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

#include "scene/LightBox.h"

#include "graphics/LowLevelGraphics.h"
#include "scene/Camera.h"
#include "math/Math.h"

#include "scene/World.h"
#include "scene/Scene.h"
#include "engine/Engine.h"


namespace hpl {

	//////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	cLightBox::cLightBox(tString asName, cResources *apResources) : iLight(asName,apResources)
	{
		mLightType = eLightType_Box;

		mvSize = 1;
		mBlendFunc = eLightBoxBlendFunc_Replace;
		mlBoxLightPrio =0;

		UpdateBoundingVolume();
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PUBLIC METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	void cLightBox::SetSize(const cVector3f& avSize)
	{ 
		mvSize = avSize;

		mbUpdateBoundingVolume = true;

		//This is so that the render container is updated.
		SetTransformUpdated();
	}

	//-----------------------------------------------------------------------

	bool cLightBox::IsVisible()
	{
		if(mDiffuseColor.r <=0 && mDiffuseColor.g <=0 && mDiffuseColor.b <=0 && mDiffuseColor.a <=0) 
			return false;
		
		return mbIsVisible; 
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PRIVATE METHODS
	//////////////////////////////////////////////////////////////////////////
	
	//-----------------------------------------------------------------------
	
	void cLightBox::UpdateBoundingVolume()
	{
		mBoundingVolume.SetSize(mvSize);
		mBoundingVolume.SetPosition(GetWorldPosition());
	}
	//-----------------------------------------------------------------------

}
