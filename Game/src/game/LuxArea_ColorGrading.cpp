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

#include "LuxArea_ColorGrading.h"

#include "LuxPlayer.h"
#include "LuxEffectHandler.h"

//////////////////////////////////////////////////////////////////////////
// LOADER
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cLuxAreaLoader_ColorGrading::cLuxAreaLoader_ColorGrading(const tString& asName) : iLuxAreaLoader(asName)
{

}

cLuxAreaLoader_ColorGrading::~cLuxAreaLoader_ColorGrading()
{

}

//-----------------------------------------------------------------------

iLuxArea *cLuxAreaLoader_ColorGrading::CreateArea(const tString& asName, int alID, cLuxMap *apMap)
{
	cLuxArea_ColorGrading *pArea = hplNew(cLuxArea_ColorGrading, (asName, alID, apMap));
	return pArea;
}

//-----------------------------------------------------------------------

void cLuxAreaLoader_ColorGrading::LoadVariables(iLuxArea *apArea, cWorld *apWorld)
{
	cLuxArea_ColorGrading *pColorGradingArea = static_cast<cLuxArea_ColorGrading*>(apArea);

	pColorGradingArea->mfCollisionCheckInterval = gpBase->mpGameCfg->GetFloat("Effects","GradingAreaCollisionCheckInterval",1.0f);

    pColorGradingArea->msLookupTable = GetVarString("LookUpTable","");
	pColorGradingArea->mfFadeTime = GetVarFloat("FadeTime",1.0f);
}

void cLuxAreaLoader_ColorGrading::SetupArea(iLuxArea *apArea, cWorld *apWorld)
{

}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cLuxArea_ColorGrading::cLuxArea_ColorGrading(const tString &asName, int alID, cLuxMap *apMap)  : iLuxArea(asName,alID,apMap, eLuxAreaType_ColorGrading)
{
	mfCollisionCheckTimer = mfCollisionCheckInterval;
    mbCollidesWithPlayer = false;
}

//-----------------------------------------------------------------------

cLuxArea_ColorGrading::~cLuxArea_ColorGrading()
{
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cLuxArea_ColorGrading::OnUpdate(float afTimeStep)
{
	//////////////////////////
	// Update collision check timer
	mfCollisionCheckTimer -= afTimeStep;
	if ( mfCollisionCheckTimer <= 0 )
	{
		mfCollisionCheckTimer = mfCollisionCheckInterval;

        bool bCollidesWithPlayer = CollidesWithPlayer();

        if ( bCollidesWithPlayer && ! mbCollidesWithPlayer )
        {
            gpBase->mpEffectHandler->GetColorGrading()->EnterLUTEnvironment( msLookupTable, mfFadeTime );
        }
        else if ( !bCollidesWithPlayer && mbCollidesWithPlayer )
        {
            gpBase->mpEffectHandler->GetColorGrading()->LeaveLUTEnvironment(msLookupTable);
        }
        
		mbCollidesWithPlayer = bCollidesWithPlayer;
	}
}

//////////////////////////////////////////////////////////////////////////
// SAVE DATA STUFF
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

kBeginSerialize(cLuxArea_ColorGrading_SaveData, iLuxArea_SaveData)

kSerializeVar(msLookupTable, eSerializeType_String)
kSerializeVar(mfFadeTime, eSerializeType_Float32)

kEndSerialize()

//-----------------------------------------------------------------------

iLuxArea* cLuxArea_ColorGrading_SaveData::CreateArea(cLuxMap *apMap)
{
	return hplNew(cLuxArea_ColorGrading, (msName, mlID, apMap));
}

//-----------------------------------------------------------------------

iLuxEntity_SaveData* cLuxArea_ColorGrading::CreateSaveData()
{
	return hplNew(cLuxArea_ColorGrading_SaveData, ());
}

//-----------------------------------------------------------------------

void cLuxArea_ColorGrading::SaveToSaveData(iLuxEntity_SaveData* apSaveData)
{
	super_class::SaveToSaveData(apSaveData);
	cLuxArea_ColorGrading_SaveData *pData = static_cast<cLuxArea_ColorGrading_SaveData*>(apSaveData);

    kCopyToVar(pData, msLookupTable);
    kCopyToVar(pData, mfFadeTime);
}

//-----------------------------------------------------------------------

void cLuxArea_ColorGrading::LoadFromSaveData(iLuxEntity_SaveData* apSaveData)
{
	super_class::LoadFromSaveData(apSaveData);
	cLuxArea_ColorGrading_SaveData *pData = static_cast<cLuxArea_ColorGrading_SaveData*>(apSaveData);

	kCopyFromVar(pData, msLookupTable);
    kCopyFromVar(pData, mfFadeTime);
}

//-----------------------------------------------------------------------

void cLuxArea_ColorGrading::SetupSaveData(iLuxEntity_SaveData *apSaveData)
{
	super_class::SetupSaveData(apSaveData);

}

//-----------------------------------------------------------------------

