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

#ifndef LUX_PROP_VoiceFlashbackTrigger_H
#define LUX_PROP_VoiceFlashbackTrigger_H

//----------------------------------------------

#include "LuxProp.h"

//----------------------------------------------

class cLuxProp_VoiceFlashbackTrigger_SaveData : public iLuxProp_SaveData
{
	kSerializableClassInit(cLuxProp_VoiceFlashbackTrigger_SaveData)
public:
	tString msVoiceFile;
	tString msTextEntry;
	int mlSpawnContainerID;
};


//----------------------------------------------

class cLuxProp_VoiceFlashbackTrigger : public iLuxProp
{
typedef iLuxProp super_class;
friend class cLuxPropLoader_VoiceFlashbackTrigger;
public:	
	cLuxProp_VoiceFlashbackTrigger(const tString &asName, int alID, cLuxMap *apMap);
	virtual ~cLuxProp_VoiceFlashbackTrigger();

	//////////////////////
	//General
	bool CanInteract(iPhysicsBody *apBody);
	bool OnInteract(iPhysicsBody *apBody, const cVector3f &avPos);

	void OnSetupAfterLoad(cWorld *apWorld);

	void OnResetProperties();

	void UpdatePropSpecific(float afTimeStep);
	
	void BeforePropDestruction();

	eLuxFocusCrosshair GetFocusCrosshair(iPhysicsBody *apBody, const cVector3f &avPos);

	//////////////////////
	//Properties
	int GetSpawnContainerID() { return mlSpawnContainerID;}
	void SetSpawnContainerID(int alX) { mlSpawnContainerID = alX;}
	
	//////////////////////
	//Connection callbacks
	void OnConnectionStateChange(iLuxEntity *apEntity, int alState);

	//////////////////////
	//Save data stuff
	iLuxEntity_SaveData* CreateSaveData();
	void SaveToSaveData(iLuxEntity_SaveData* apSaveData);
	void LoadFromSaveData(iLuxEntity_SaveData* apSaveData);
	void SetupSaveData(iLuxEntity_SaveData *apSaveData);


private:
	////////////////
	//Variables
	tString msVoiceFile;
	tString msTextEntry;

	int mlSpawnContainerID;

	float mfFlashAlpha;

	bool mbPlayerHasInteracted;

    float
        mfMoveSpeedMultiplier,
        mfMouseSensitivityModifier;
};

//----------------------------------------------

class cLuxPropLoader_VoiceFlashbackTrigger : public iLuxPropLoader
{
public:
	cLuxPropLoader_VoiceFlashbackTrigger(const tString& asName);
	virtual ~cLuxPropLoader_VoiceFlashbackTrigger(){}

	iLuxProp *CreateProp(const tString& asName, int alID, cLuxMap *apMap);
	void LoadVariables(iLuxProp *apProp, cXmlElement *apRootElem);
	void LoadInstanceVariables(iLuxProp *apProp, cResourceVarsObject *apInstanceVars);

private:
};

//----------------------------------------------


#endif // LUX_PROP_VoiceFlashbackTrigger_H
