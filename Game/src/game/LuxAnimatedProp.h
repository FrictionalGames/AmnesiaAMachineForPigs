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

#ifndef LUX_ANIMATED_PROP_H
#define LUX_ANIMATED_PROP_H


//----------------------------------------------

#include "LuxBase.h"

//----------------------------------------------

class cLuxMap;

//----------------------------------------------
// ATTENTION!
// The entity classes is only be used if for some reason 
// static props needs to be turned of or what not.
//----------------------------------------------

class iLuxAnimatedProp_SaveData : public iSerializable
{
	kSerializableClassInit(iLuxEntity_SaveData)
public:
	virtual ~iLuxAnimatedProp_SaveData() {}

	tString msName;
	int mlID;

	tString msFileName;
	cMatrixf m_mtxOnLoadTransform;
};

//----------------------------------------------

class cLuxAnimatedProp
{
friend class cLuxAnimatedPropLoader;
public:
	cLuxAnimatedProp(const tString &asName, int alID);
	~cLuxAnimatedProp();
	
private:
	tString msName;
	int mlID;

	cLuxMap *mpMap;
	tString msFileName;
	cMatrixf m_mtxOnLoadTransform;

	cMeshEntity *mpMeshEntity;
	
	std::vector<iPhysicsBody*> mvBodies;
	std::vector<iPhysicsJoint*> mvJoints;

	std::vector<iLight*> mvLights;
	std::vector<cParticleSystem*> mvParticleSystems;
	std::vector<cBillboard*> mvBillboards;
	std::vector<cBeam*> mvBeams;
	std::vector<cSoundEntity*> mvSoundEntities;
};


//----------------------------------------------

class cLuxAnimatedPropLoader : public cEntityLoader_Object
{
public:
	cLuxAnimatedPropLoader(const tString& asName);
	virtual ~cLuxAnimatedPropLoader(){}

	void BeforeLoad(cXmlElement *apRootElem, const cMatrixf &a_mtxTransform,cWorld *apWorld, cResourceVarsObject *apInstanceVars);
	void AfterLoad(cXmlElement *apRootElem, const cMatrixf &a_mtxTransform,cWorld *apWorld, cResourceVarsObject *apInstanceVars);
};

//----------------------------------------------

#endif // LUX_ANIMATED_PROP_H
