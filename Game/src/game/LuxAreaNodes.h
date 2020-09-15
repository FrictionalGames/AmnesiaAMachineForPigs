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

#ifndef LUX_AREA_NODES_H
#define LUX_AREA_NODES_H

//----------------------------------------------

#include "LuxTypes.h"

//----------------------------------------------

class cLuxNode_PlayerStart
{
friend class cLuxAreaNodeLoader_PlayerStart;
public:
	cLuxNode_PlayerStart(const tString& asName);
	
	const tString& GetName(){ return msName;}

	const cVector3f& GetPosition(){ return mvPos;}
	float GetAngle(){ return mfAngle;}

private:
	tString msName;
	cVector3f mvPos;
	float mfAngle;
};

//----------------------------------------------

class cLuxAreaNodeLoader_PlayerStart : public iAreaLoader
{
public:
	cLuxAreaNodeLoader_PlayerStart(const tString& asName);
	
	void Load(const tString &asName, int alID, bool abActive, const cVector3f &avSize, const cMatrixf &a_mtxTransform,cWorld *apWorld);
};

//----------------------------------------------

class cLuxAreaNodeLoader_PathNode : public iAreaLoader
{
public:
	cLuxAreaNodeLoader_PathNode(const tString& asName); 

	void Load(const tString &asName, int alID, bool abActive, const cVector3f &avSize, const cMatrixf &a_mtxTransform,cWorld *apWorld);
};

//----------------------------------------------

class cLuxAreaNodeLoader_PosNode : public iAreaLoader
{
public:
	cLuxAreaNodeLoader_PosNode(const tString& asName); 

	void Load(const tString &asName, int alID, bool abActive, const cVector3f &avSize, const cMatrixf &a_mtxTransform,cWorld *apWorld);
};

//----------------------------------------------


#endif// LUX_AREA_NODES_H
