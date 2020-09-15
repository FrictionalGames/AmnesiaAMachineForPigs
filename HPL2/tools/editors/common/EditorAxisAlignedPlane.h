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

#ifndef HPLEDITOR_EDITOR_AXIS_ALIGNED_PLANE_H
#define HPLEDITOR_EDITOR_AXIS_ALIGNED_PLANE_H

#include "../common/StdAfx.h"

using namespace hpl;

//------------------------------------------------

enum ePlaneNormal
{
	ePlaneNormal_X,
	ePlaneNormal_Y,
	ePlaneNormal_Z,

	ePlaneNormal_LastEnum,
};

//------------------------------------------------

class iEditorAxisAlignedPlane
{
public:
	iEditorAxisAlignedPlane();

	void SetPlaneNormal(ePlaneNormal aNormal);
	ePlaneNormal GetPlaneNormal() { return mNormal; }

	const tWString& GetPlaneString();
	static ePlaneNormal GetPlaneNormalFromString(const tString& asX);

	const cPlanef& GetPlane();
    
	void SetHeight(const float afHeight);
	float GetHeight();

	void SetHeights(const cVector3f& avX);
	const cVector3f& GetHeights();

	void SetVisible(bool abX)	{ mbVisible = abX; }
	bool IsVisible()			{ return mbVisible; }

	cVector3f GetProjectedPosOnPlane(const cVector3f& avWorldPos);

	virtual void Draw(cRendererCallbackFunctions* apFunctions, const cVector3f& avPos)=0;

protected:
	virtual void OnPlaneModified() {}
    /////////////////////////////
	// Data
	ePlaneNormal mNormal;

	bool mbUpdated;
	cPlanef mEnginePlane;
	cVector3f mvHeights;
	bool mbVisible;

	//////////////////////////////
	// Helper data
	static cVector3f mvPlaneNormals[3];
	static tWString mvPlaneStrings[3];
};

//------------------------------------------------

#endif // HPLEDITOR_EDITOR_GRID_H

