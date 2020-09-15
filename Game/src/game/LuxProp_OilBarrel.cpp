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

#include "LuxProp_OilBarrel.h"

#include "LuxPlayer.h"
#include "LuxMessageHandler.h"
#include "LuxHelpFuncs.h"
#include "LuxInventory.h"

#include "LuxMap.h"

//////////////////////////////////////////////////////////////////////////
// LOADER
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cLuxPropLoader_OilBarrel::cLuxPropLoader_OilBarrel(const tString& asName) : iLuxPropLoader(asName)
{
}

//-----------------------------------------------------------------------

iLuxProp *cLuxPropLoader_OilBarrel::CreateProp(const tString& asName, int alID, cLuxMap *apMap)
{
	return hplNew(cLuxProp_OilBarrel, (asName, alID,apMap) );
}

//-----------------------------------------------------------------------

void cLuxPropLoader_OilBarrel::LoadVariables(iLuxProp *apProp, cXmlElement *apRootElem)
{
	cLuxProp_OilBarrel  *pOilBarrel = static_cast<cLuxProp_OilBarrel*>(apProp);

	///////////////////////////
	// General
	//pOilBarrel->mSwingDoorData.mfMoveMaxSpeed = GetVarFloat("MoveMaxSpeed", 13.5f);
	pOilBarrel->msFillSound = GetVarString("FillSound", "");
	pOilBarrel->msEmptySound = GetVarString("EmptySound", "");
}

//-----------------------------------------------------------------------

void cLuxPropLoader_OilBarrel::LoadInstanceVariables(iLuxProp *apProp, cResourceVarsObject *apInstanceVars)
{
	cLuxProp_OilBarrel  *pOilBarrel = static_cast<cLuxProp_OilBarrel*>(apProp);

	//pOilBarrel->mlCoinsNeeded = apInstanceVars->GetVarInt("CoinsNeeded",0);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cLuxProp_OilBarrel::cLuxProp_OilBarrel(const tString &asName, int alID, cLuxMap *apMap) : iLuxProp(asName,alID,apMap, eLuxPropType_OilBarrel)
{
	mbAmountCalculated = false;
}

//-----------------------------------------------------------------------

cLuxProp_OilBarrel::~cLuxProp_OilBarrel()
{
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

bool cLuxProp_OilBarrel::CanInteract(iPhysicsBody *apBody)
{
	return false;
}

//-----------------------------------------------------------------------

void cLuxProp_OilBarrel::OnSetupAfterLoad(cWorld *apWorld)
{
	
}

//-----------------------------------------------------------------------

void cLuxProp_OilBarrel::OnResetProperties()
{

}

//-----------------------------------------------------------------------

void cLuxProp_OilBarrel::UpdatePropSpecific(float afTimeStep)
{
	
}

//-----------------------------------------------------------------------

void cLuxProp_OilBarrel::BeforePropDestruction()
{

}

//-----------------------------------------------------------------------

eLuxFocusCrosshair cLuxProp_OilBarrel::GetFocusCrosshair(iPhysicsBody *apBody, const cVector3f &avPos)
{
	return eLuxFocusCrosshair_Grab;
}

//-----------------------------------------------------------------------

tWString cLuxProp_OilBarrel::GetFocusText()
{
	CalculateOilAmount();

	tWString sText;
	if(mfOilAmount <=0)
	{
		sText = kTranslate("Game", "OilBarrel_FocusText_Empty");
	}
	else
	{
		if(mfOilAmount < 25)
			sText = kTranslate("Game", "OilBarrel_FocusText_025");
		else if(mfOilAmount < 50)
			sText = kTranslate("Game", "OilBarrel_FocusText_050");
		else if(mfOilAmount < 75)
			sText = kTranslate("Game", "OilBarrel_FocusText_075");
		else 
			sText = kTranslate("Game", "OilBarrel_FocusText_100");
	}
		
	return sText;
}

//-----------------------------------------------------------------------

void cLuxProp_OilBarrel::OnConnectionStateChange(iLuxEntity *apEntity, int alState)
{
	
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// SAVE DATA STUFF
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

kBeginSerialize(cLuxProp_OilBarrel_SaveData, iLuxProp_SaveData)

kSerializeVar(mbAmountCalculated, eSerializeType_Bool)
kSerializeVar(mfOilAmount, eSerializeType_Float32)

kEndSerialize()

//-----------------------------------------------------------------------

iLuxEntity_SaveData* cLuxProp_OilBarrel::CreateSaveData()
{
	return hplNew(cLuxProp_OilBarrel_SaveData, ());
}

//-----------------------------------------------------------------------

void cLuxProp_OilBarrel::SaveToSaveData(iLuxEntity_SaveData* apSaveData)
{
	//////////////////
	//Init
	super_class::SaveToSaveData(apSaveData);
	cLuxProp_OilBarrel_SaveData *pData = static_cast<cLuxProp_OilBarrel_SaveData*>(apSaveData);

	//////////////////
	//Set variables
	kCopyToVar(pData, mbAmountCalculated);
	kCopyToVar(pData, mfOilAmount);
	
}

//-----------------------------------------------------------------------

void cLuxProp_OilBarrel::LoadFromSaveData(iLuxEntity_SaveData* apSaveData)
{
	//////////////////
	//Init
	super_class::LoadFromSaveData(apSaveData);
	cLuxProp_OilBarrel_SaveData *pData = static_cast<cLuxProp_OilBarrel_SaveData*>(apSaveData);
	
	//////////////////
	//Set variables
	kCopyFromVar(pData, mbAmountCalculated);
	kCopyFromVar(pData, mfOilAmount);
}

//-----------------------------------------------------------------------

void cLuxProp_OilBarrel::SetupSaveData(iLuxEntity_SaveData *apSaveData)
{
	super_class::SetupSaveData(apSaveData);
}

//-----------------------------------------------------------------------
