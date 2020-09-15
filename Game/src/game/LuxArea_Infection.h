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

#ifndef LUX_AREA_INFECTION_H
#define LUX_AREA_INFECTION_H

//----------------------------------------------

#include "LuxArea.h"
#include "LuxBase.h"

//----------------------------------------------

class cLuxArea_Infection_SaveData : public iLuxArea_SaveData
{
	kSerializableClassInit(cLuxArea_Infection_SaveData)
public:
	iLuxArea* CreateArea(cLuxMap *apMap);

	float
		mfIncrementSpeed,
		mfTotalIncrementApplied,
		mfTotalIncrementBound,
		mfInfectionLevelLowerBound,
		mfInfectionLevelUpperBound;
	bool
		mbDisableOnLevelBoundHit;
};

//----------------------------------------------

class cLuxArea_Infection : public iLuxArea
{
typedef iLuxArea super_class;
friend class cLuxAreaLoader_Infection;
public:	
	cLuxArea_Infection(const tString &asName, int alID, cLuxMap *apMap);
	virtual ~cLuxArea_Infection();

	//////////////////////
	//General
	void OnUpdate(float afTimeStep);
	
	//////////////////////
	//Connection callbacks
	void OnConnectionStateChange(iLuxEntity *apEntity, int alState){}

	//////////////////////
	//Save data stuff
	iLuxEntity_SaveData* CreateSaveData();
	virtual void SaveToSaveData(iLuxEntity_SaveData* apSaveData);
	virtual void LoadFromSaveData(iLuxEntity_SaveData* apSaveData);
	virtual void SetupSaveData(iLuxEntity_SaveData *apSaveData);
private:

	void UpdateInfection(float afTimeStep );
	/////////////////////////
	// Variables
	float
		mfCollisionCheckTimer,
		mfCheckTimeMin,
		mfCheckTimeMax,
		mfIncrementSpeed,
		mfTotalIncrementApplied,
		mfTotalIncrementBound,
		mfInfectionLevelLowerBound,
		mfInfectionLevelUpperBound;
	bool
		mbDisableOnLevelBoundHit,
		mbCollidesWithPlayer;
};

//----------------------------------------------

class cLuxAreaLoader_Infection : public iLuxAreaLoader
{
public:
	cLuxAreaLoader_Infection(const tString& asName);
	~cLuxAreaLoader_Infection();

	iLuxArea *CreateArea(const tString& asName, int alID, cLuxMap *apMap);
	
	void LoadVariables(iLuxArea *apArea, cWorld *apWorld);
	void SetupArea(iLuxArea *apArea, cWorld *apWorld);

private:

	float GetInfectionLevelMiddle(int aiInfectionLevel);

	int
		miNumberOfInfectionLevels;
	
};

//----------------------------------------------


#endif // LUX_AREA_INFECTION_H
