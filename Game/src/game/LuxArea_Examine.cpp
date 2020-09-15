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

#include "LuxArea_Examine.h"

#include "LuxMap.h"
#include "LuxPlayer.h"
#include "LuxPlayerState.h"
#include "LuxEffectHandler.h"
#include "LuxHelpFuncs.h"
#include "LuxCompletionCountHandler.h"
#include "LuxMessageHandler.h"
#include "LuxMapHandler.h"
#include "LuxMap.h"


//////////////////////////////////////////////////////////////////////////
// LOADER
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cLuxAreaLoader_Examine::cLuxAreaLoader_Examine(const tString& asName) : iLuxAreaLoader(asName)
{

}

cLuxAreaLoader_Examine::~cLuxAreaLoader_Examine()
{

}

//-----------------------------------------------------------------------

iLuxArea *cLuxAreaLoader_Examine::CreateArea(const tString& asName, int alID, cLuxMap *apMap)
{
	cLuxArea_Examine *pArea = hplNew(cLuxArea_Examine, (asName, alID, apMap));
	return pArea;
}

//-----------------------------------------------------------------------

void cLuxAreaLoader_Examine::LoadVariables(iLuxArea *apArea, cWorld *apWorld)
{
	cLuxArea_Examine *pExamineArea = static_cast<cLuxArea_Examine*>(apArea);

	pExamineArea->msDescCat = GetVarString("DescCat","");
	pExamineArea->msDescEntry = GetVarString("DescEntry","");

	pExamineArea->msDescInfectedCat = GetVarString("DescInfectedCat","");
	pExamineArea->msDescInfectedEntry = GetVarString("DescInfectedEntry","");

	pExamineArea->msSound = GetVarString("Sound","");
	pExamineArea->msInfectedSound = GetVarString("InfectedSound","");
}

void cLuxAreaLoader_Examine::SetupArea(iLuxArea *apArea, cWorld *apWorld)
{

}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cLuxArea_Examine::cLuxArea_Examine(const tString &asName, int alID, cLuxMap *apMap)  : iLuxArea(asName,alID,apMap, eLuxAreaType_Examine)
{
	mfMaxFocusDistance = gpBase->mpGameCfg->GetFloat("Player_Interaction","Examine_MaxFocusDist",0);
	mfInfectedLimit = gpBase->mpGameCfg->GetFloat("Player_Interaction","MinExamineInfection",0);

	mfPlaySoundCount =0;

}

//-----------------------------------------------------------------------

cLuxArea_Examine::~cLuxArea_Examine()
{
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cLuxArea_Examine::SetupAfterLoad(cWorld *apWorld)
{
	
}

//-----------------------------------------------------------------------

void cLuxArea_Examine::OnUpdate(float afTimeStep)
{
	if(mfPlaySoundCount>0) mfPlaySoundCount -= afTimeStep;
}

//-----------------------------------------------------------------------

bool cLuxArea_Examine::CanInteract(iPhysicsBody *apBody)
{
	return true;
}

//-----------------------------------------------------------------------

bool cLuxArea_Examine::OnInteract(iPhysicsBody *apBody, const cVector3f &avPos)
{
	float fInfection = gpBase->mpPlayer->GetInfection();
	
	////////////////////////////////
	// Show text
	if(msDescCat != "")
	{
		tString sCat = msDescCat;
		tString sEntry = msDescEntry;
		if(fInfection > mfInfectedLimit && msDescInfectedCat != "" && msDescInfectedEntry != "")
		{
			sCat = msDescInfectedCat;
			sEntry = msDescInfectedEntry;
		}

		gpBase->mpMessageHandler->SetMessage(kTranslate(sCat, sEntry), 0);
	}

	////////////////////////////////
	// Play sound
	if(mfPlaySoundCount <=0 && msSound != "")
	{
		tString sSound = msSound;
		if(fInfection > mfInfectedLimit && msInfectedSound != "")
			sSound = msInfectedSound;

		if(sSound != "")
			gpBase->mpHelpFuncs->PlayGuiSoundData(sSound, eSoundEntryType_Gui);
	}


	mfPlaySoundCount = 0.5f;
	return true;
}

//-----------------------------------------------------------------------

eLuxFocusCrosshair cLuxArea_Examine::GetFocusCrosshair(iPhysicsBody *apBody, const cVector3f &avPos)
{
	return eLuxFocusCrosshair_Grab;	
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// SAVE DATA STUFF
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

kBeginSerialize(cLuxArea_Examine_SaveData, iLuxArea_SaveData)

kSerializeVar(msDescCat, eSerializeType_String)
kSerializeVar(msDescEntry, eSerializeType_String)

kSerializeVar(msDescInfectedCat, eSerializeType_String)
kSerializeVar(msDescInfectedEntry, eSerializeType_String)

kSerializeVar(msSound, eSerializeType_String)
kSerializeVar(msInfectedSound, eSerializeType_String)

kEndSerialize()

//-----------------------------------------------------------------------

iLuxArea* cLuxArea_Examine_SaveData::CreateArea(cLuxMap *apMap)
{
	return hplNew(cLuxArea_Examine, (msName, mlID, apMap));
}

//-----------------------------------------------------------------------

iLuxEntity_SaveData* cLuxArea_Examine::CreateSaveData()
{
	return hplNew(cLuxArea_Examine_SaveData, ());
}

//-----------------------------------------------------------------------

void cLuxArea_Examine::SaveToSaveData(iLuxEntity_SaveData* apSaveData)
{
	super_class::SaveToSaveData(apSaveData);
	cLuxArea_Examine_SaveData *pData = static_cast<cLuxArea_Examine_SaveData*>(apSaveData);

    kCopyToVar(pData, msDescCat);
	kCopyToVar(pData, msDescEntry);

	kCopyToVar(pData, msDescInfectedCat);
	kCopyToVar(pData, msDescInfectedEntry);

	kCopyToVar(pData, msSound);
	kCopyToVar(pData, msInfectedSound);
}

//-----------------------------------------------------------------------

void cLuxArea_Examine::LoadFromSaveData(iLuxEntity_SaveData* apSaveData)
{
	super_class::LoadFromSaveData(apSaveData);
	cLuxArea_Examine_SaveData *pData = static_cast<cLuxArea_Examine_SaveData*>(apSaveData);

	kCopyFromVar(pData, msDescCat);
	kCopyFromVar(pData, msDescEntry);

	kCopyFromVar(pData, msDescInfectedCat);
	kCopyFromVar(pData, msDescInfectedEntry);

	kCopyFromVar(pData, msSound);
	kCopyFromVar(pData, msInfectedSound);
}

//-----------------------------------------------------------------------

void cLuxArea_Examine::SetupSaveData(iLuxEntity_SaveData *apSaveData)
{
	super_class::SetupSaveData(apSaveData);

}

//-----------------------------------------------------------------------

