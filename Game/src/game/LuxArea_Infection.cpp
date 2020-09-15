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

#include "LuxArea_Infection.h"

#include "LuxInfectionHandler.h"
#include "LuxPlayer.h"

//////////////////////////////////////////////////////////////////////////
// LOADER
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cLuxAreaLoader_Infection::cLuxAreaLoader_Infection(const tString& asName) : iLuxAreaLoader(asName)
{

}

cLuxAreaLoader_Infection::~cLuxAreaLoader_Infection()
{

}

//-----------------------------------------------------------------------

iLuxArea *cLuxAreaLoader_Infection::CreateArea(const tString& asName, int alID, cLuxMap *apMap)
{
	cLuxArea_Infection *pArea = hplNew(cLuxArea_Infection, (asName, alID, apMap));
	return pArea;
}

//-----------------------------------------------------------------------

void cLuxAreaLoader_Infection::LoadVariables(iLuxArea *apArea, cWorld *apWorld)
{
	miNumberOfInfectionLevels = gpBase->mpGameCfg->GetFloat("Player_Infection","NumberOfInfectionLevels",0);

	cLuxArea_Infection *pInfectionArea = static_cast<cLuxArea_Infection*>(apArea);

	pInfectionArea->mfIncrementSpeed = GetVarFloat("IncrementSpeed",0.0f);;
	pInfectionArea->mfTotalIncrementBound = GetVarFloat("TotalIncrementBound",0.0f);
	pInfectionArea->mfInfectionLevelLowerBound = GetInfectionLevelMiddle(GetVarInt("InfectionLevelLowerBound",-1));
	pInfectionArea->mfInfectionLevelUpperBound = GetInfectionLevelMiddle(GetVarInt("InfectionLevelUpperBound",5));
	pInfectionArea->mbDisableOnLevelBoundHit = GetVarBool("DisableOnLevelBoundHit",false);
}

float cLuxAreaLoader_Infection::GetInfectionLevelMiddle(int aiInfectionLevel)
{
	if ( aiInfectionLevel == 0 )
	{
		return 0.0f;
	}
	else 
	{
		return ( 100.0f * ( (float)(aiInfectionLevel - 1 ) + 0.5f ) ) / (float)miNumberOfInfectionLevels;
	}
}

void cLuxAreaLoader_Infection::SetupArea(iLuxArea *apArea, cWorld *apWorld)
{

}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cLuxArea_Infection::cLuxArea_Infection(const tString &asName, int alID, cLuxMap *apMap)  : iLuxArea(asName,alID,apMap, eLuxAreaType_Infection)
{
	mfCheckTimeMin = gpBase->mpGameCfg->GetFloat("Infection", "InfectionArea_CheckTimeMin",0);
	mfCheckTimeMax = gpBase->mpGameCfg->GetFloat("Infection", "InfectionArea_CheckTimeMax",0);

	mfCollisionCheckTimer = cMath::RandRectf(mfCheckTimeMin, mfCheckTimeMax);
	mbCollidesWithPlayer = false;
}

//-----------------------------------------------------------------------

cLuxArea_Infection::~cLuxArea_Infection()
{
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cLuxArea_Infection::OnUpdate(float afTimeStep)
{
	//////////////////////////
	// Update collision check timer
	mfCollisionCheckTimer -= afTimeStep;
	if ( mfCollisionCheckTimer <= 0 )
	{
		mfCollisionCheckTimer = cMath::RandRectf(mfCheckTimeMin, mfCheckTimeMax);
		mbCollidesWithPlayer = CollidesWithPlayer();
	}
	
	//////////////////////////
	// Update infection
	UpdateInfection( afTimeStep );
}

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cLuxArea_Infection::UpdateInfection(float afTimeStep )
{
	if ( mbCollidesWithPlayer )
	{
		float fIncrementThisFrame = mfIncrementSpeed * afTimeStep;

		// check if total increment bound is set & hit
		if ( ( mfTotalIncrementBound < 0 && ( mfTotalIncrementApplied + fIncrementThisFrame <= mfTotalIncrementBound ) )
			|| ( mfTotalIncrementBound > 0 && ( mfTotalIncrementApplied + fIncrementThisFrame >= mfTotalIncrementBound ) ) )
		{
			fIncrementThisFrame = mfTotalIncrementBound - mfTotalIncrementApplied;
			SetActive(false);
		}

		float currentInfection = gpBase->mpPlayer->GetInfection();
		
		if ( mfIncrementSpeed < 0 && ( currentInfection + fIncrementThisFrame ) <= mfInfectionLevelLowerBound )
		{
			fIncrementThisFrame = mfInfectionLevelLowerBound - currentInfection;

			if ( fIncrementThisFrame > 0.0f ) fIncrementThisFrame = 0.0f;    // If the area is meant to heal up to a certain level, it can't damage if the player isn't at that level yet

			if ( mbDisableOnLevelBoundHit )
			{
				SetActive(false);
			}
		}
		else if ( mfIncrementSpeed > 0 && ( currentInfection + fIncrementThisFrame ) >= mfInfectionLevelUpperBound )
		{
			fIncrementThisFrame = mfInfectionLevelUpperBound - currentInfection;

			if ( fIncrementThisFrame < 0.0f ) fIncrementThisFrame = 0.0f;    // If the area is meant to damage up to a certain level, it can't heal if the player is at a higher level

			if ( mbDisableOnLevelBoundHit )
			{
				SetActive(false);
			}
		}

		mfTotalIncrementApplied += fIncrementThisFrame;
		gpBase->mpPlayer->AddInfection( fIncrementThisFrame );
	}
}
//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// SAVE DATA STUFF
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

kBeginSerialize(cLuxArea_Infection_SaveData, iLuxArea_SaveData)

kSerializeVar(mfIncrementSpeed, eSerializeType_Float32)
kSerializeVar(mfTotalIncrementApplied, eSerializeType_Float32)
kSerializeVar(mfTotalIncrementBound, eSerializeType_Float32)
kSerializeVar(mfInfectionLevelLowerBound, eSerializeType_Float32)
kSerializeVar(mfInfectionLevelUpperBound, eSerializeType_Float32)
kSerializeVar(mbDisableOnLevelBoundHit, eSerializeType_Bool)

kEndSerialize()

//-----------------------------------------------------------------------

iLuxArea* cLuxArea_Infection_SaveData::CreateArea(cLuxMap *apMap)
{
	return hplNew(cLuxArea_Infection, (msName, mlID, apMap));
}

//-----------------------------------------------------------------------

iLuxEntity_SaveData* cLuxArea_Infection::CreateSaveData()
{
	return hplNew(cLuxArea_Infection_SaveData, ());
}

//-----------------------------------------------------------------------

void cLuxArea_Infection::SaveToSaveData(iLuxEntity_SaveData* apSaveData)
{
	super_class::SaveToSaveData(apSaveData);
	cLuxArea_Infection_SaveData *pData = static_cast<cLuxArea_Infection_SaveData*>(apSaveData);

	kCopyToVar(pData, mfIncrementSpeed);
	kCopyToVar(pData, mfTotalIncrementApplied);
	kCopyToVar(pData, mfTotalIncrementBound);
	kCopyToVar(pData, mfInfectionLevelLowerBound);
	kCopyToVar(pData, mfInfectionLevelUpperBound);
	kCopyToVar(pData, mbDisableOnLevelBoundHit);
}

//-----------------------------------------------------------------------

void cLuxArea_Infection::LoadFromSaveData(iLuxEntity_SaveData* apSaveData)
{
	super_class::LoadFromSaveData(apSaveData);
	cLuxArea_Infection_SaveData *pData = static_cast<cLuxArea_Infection_SaveData*>(apSaveData);

	kCopyFromVar(pData, mfIncrementSpeed);
	kCopyFromVar(pData, mfTotalIncrementApplied);
	kCopyFromVar(pData, mfTotalIncrementBound);
	kCopyFromVar(pData, mfInfectionLevelLowerBound);
	kCopyFromVar(pData, mfInfectionLevelUpperBound);
	kCopyFromVar(pData, mbDisableOnLevelBoundHit);
}

//-----------------------------------------------------------------------

void cLuxArea_Infection::SetupSaveData(iLuxEntity_SaveData *apSaveData)
{
	super_class::SetupSaveData(apSaveData);

}

//-----------------------------------------------------------------------

