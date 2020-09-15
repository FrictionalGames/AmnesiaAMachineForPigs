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

#ifndef LUX_PROP_PHONEBOX_H
#define LUX_PROP_PHONEBOX_H

//----------------------------------------------

#include "LuxProp.h"

//----------------------------------------------

class cLuxProp_PhoneBox_SaveData : public iLuxProp_SaveData
{
	kSerializableClassInit(cLuxProp_PhoneBox_SaveData)
public:

	tString
		msVoiceFile;
	tString
		msTextEntry1,
		msTextEntry2,
		msTextEntry3,
		msTextEntry4,
		msTextEntry5,
		msTextEntry6,
		msTextEntry7;
	float
		mfDelay1,
		mfDelay2,
		mfDelay3,
		mfDelay4,
		mfDelay5,
		mfDelay6,
		mfDelay7;
	bool
        mbPlayerHasInteracted,
        mbIsRinging,
		mbIsPlayingVoice;
};


//----------------------------------------------

class cLuxProp_PhoneBox : public iLuxProp
{
typedef iLuxProp super_class;
friend class cLuxPropLoader_PhoneBox;
public:	
	cLuxProp_PhoneBox(const tString &asName, int alID, cLuxMap *apMap);
	virtual ~cLuxProp_PhoneBox();

	//////////////////////
	//General
	bool CanInteract(iPhysicsBody *apBody);
	bool OnInteract(iPhysicsBody *apBody, const cVector3f &avPos);

	void OnSetupAfterLoad(cWorld *apWorld);

	void OnResetProperties();

	void UpdatePropSpecific(float afTimeStep);
	
	void BeforePropDestruction();

	eLuxFocusCrosshair GetFocusCrosshair(iPhysicsBody *apBody, const cVector3f &avPos);

    void StartRinging();
    void StopRinging();
    void HangUp();
    void VoiceStopped();

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
	tString
		msVoiceFile;
	tString
		msTextEntry1,
		msTextEntry2,
		msTextEntry3,
		msTextEntry4,
		msTextEntry5,
		msTextEntry6,
		msTextEntry7;
	float
		mfDelay1,
		mfDelay2,
		mfDelay3,
		mfDelay4,
		mfDelay5,
		mfDelay6,
		mfDelay7;

	int mlSpawnContainerID;

	float mfFlashAlpha;

    float
        mfMoveSpeedMultiplier,
        mfMouseSensitivityModifier;

	bool
        mbPlayerHasInteracted,
        mbIsRinging;
    bool
        mbIsPlayingVoice;

    cSoundEntity
        * mpRingingSound;
};

//----------------------------------------------

class cLuxPropLoader_PhoneBox : public iLuxPropLoader
{
public:
	cLuxPropLoader_PhoneBox(const tString& asName);
	virtual ~cLuxPropLoader_PhoneBox(){}

	iLuxProp *CreateProp(const tString& asName, int alID, cLuxMap *apMap);
	void LoadVariables(iLuxProp *apProp, cXmlElement *apRootElem);
	void LoadInstanceVariables(iLuxProp *apProp, cResourceVarsObject *apInstanceVars);

private:
};

//----------------------------------------------


#endif // LUX_PROP_PHONEBOX_H
