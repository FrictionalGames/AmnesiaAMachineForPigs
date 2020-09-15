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

#include "LuxProp_PhoneBox.h"

#include "LuxPlayer.h"
#include "LuxMap.h"
#include "LuxMapHandler.h"
#include "LuxHelpFuncs.h"
#include "LuxInventory.h"
#include "LuxItemType.h"
#include "LuxMessageHandler.h"
#include "LuxEffectRenderer.h"
#include "LuxPlayerHelpers.h"

//////////////////////////////////////////////////////////////////////////
// LOADER
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cLuxPropLoader_PhoneBox::cLuxPropLoader_PhoneBox(const tString& asName) : iLuxPropLoader(asName)
{
	mfDefaultMaxFocusDistance = gpBase->mpGameCfg->GetFloat("Player_Interaction","Item_DefaultMaxFocusDist",0);
}

//-----------------------------------------------------------------------

iLuxProp *cLuxPropLoader_PhoneBox::CreateProp(const tString& asName, int alID, cLuxMap *apMap)
{
	return hplNew(cLuxProp_PhoneBox, (asName, alID,apMap) );
}

//-----------------------------------------------------------------------

void cLuxPropLoader_PhoneBox::LoadVariables(iLuxProp *apProp, cXmlElement *apRootElem)
{
}

//-----------------------------------------------------------------------

void cLuxPropLoader_PhoneBox::LoadInstanceVariables(iLuxProp *apProp, cResourceVarsObject *apInstanceVars)
{
	cLuxProp_PhoneBox  *pPhoneBox = static_cast<cLuxProp_PhoneBox*>(apProp);

    tString voiceFile, textEntry;

	pPhoneBox->msVoiceFile = apInstanceVars->GetVarString("VoiceFile1", "");;
	pPhoneBox->msTextEntry1 = apInstanceVars->GetVarString("TextEntry1", "");
	pPhoneBox->mfDelay1 = apInstanceVars->GetVarFloat("Delay1", 0);
	pPhoneBox->msTextEntry2 = apInstanceVars->GetVarString("TextEntry2", "");
	pPhoneBox->mfDelay2 = apInstanceVars->GetVarFloat("Delay2", 0);
	pPhoneBox->msTextEntry3 = apInstanceVars->GetVarString("TextEntry3", "");
	pPhoneBox->mfDelay3 = apInstanceVars->GetVarFloat("Delay3", 0);
	pPhoneBox->msTextEntry4 = apInstanceVars->GetVarString("TextEntry4", "");
	pPhoneBox->mfDelay4 = apInstanceVars->GetVarFloat("Delay4", 0);
	pPhoneBox->msTextEntry5 = apInstanceVars->GetVarString("TextEntry5", "");
	pPhoneBox->mfDelay5 = apInstanceVars->GetVarFloat("Delay5", 0);
	pPhoneBox->msTextEntry6 = apInstanceVars->GetVarString("TextEntry6", "");
	pPhoneBox->mfDelay6 = apInstanceVars->GetVarFloat("Delay6", 0);
	pPhoneBox->msTextEntry7 = apInstanceVars->GetVarString("TextEntry7", "");
	pPhoneBox->mfDelay7 = apInstanceVars->GetVarFloat("Delay7", 0);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cLuxProp_PhoneBox::cLuxProp_PhoneBox(const tString &asName,int alID, cLuxMap *apMap) : iLuxProp(asName,alID,apMap, eLuxPropType_Item)
{
	mlSpawnContainerID =-1;
	mfFlashAlpha =0;
	mbPlayerHasInteracted = false;
    mbIsRinging = false;
    mpRingingSound = NULL;
    mbIsPlayingVoice = false;

    mfMoveSpeedMultiplier = gpBase->mpGameCfg->GetFloat("VoiceFlashback","PhoneBoxMoveSpeedMultiplier",0);
	mfMouseSensitivityModifier = gpBase->mpGameCfg->GetFloat("VoiceFlashback","PhoneBoxMouseSensitivityModifier",0);
}

//-----------------------------------------------------------------------

cLuxProp_PhoneBox::~cLuxProp_PhoneBox()
{
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

bool cLuxProp_PhoneBox::CanInteract(iPhysicsBody *apBody)
{
	return !mbPlayerHasInteracted && mbIsRinging;
}

bool cLuxProp_PhoneBox::OnInteract(iPhysicsBody *apBody, const cVector3f &avPos)
{
    StopRinging();

    PlayAnimation("switch", 0.0f, false, "");
    gpBase->mpHelpFuncs->PlayGuiSoundData("telephone_up", eSoundEntryType_Gui);

	cLuxPlayerVoiceFlashback * pVoiceFlashback = gpBase->mpPlayer->GetHelperVoiceFlashback();

	pVoiceFlashback->StopPlaying();
    pVoiceFlashback->SetMoveSpeedMultiplier( mfMoveSpeedMultiplier );
    pVoiceFlashback->SetMouseSensitivityModifier( mfMouseSensitivityModifier );
    
    pVoiceFlashback->SetStopSound( "telephone_down" );

	pVoiceFlashback->Start( msVoiceFile,
		msTextEntry1, mfDelay1,
		msTextEntry2, mfDelay2,
		msTextEntry3, mfDelay3,
		msTextEntry4, mfDelay4,
		msTextEntry5, mfDelay5,
		msTextEntry6, mfDelay6,
		msTextEntry7, mfDelay7,
		false, true
		);

    mbPlayerHasInteracted = true;
    mbIsPlayingVoice = true;

	return true;
}

//-----------------------------------------------------------------------

void cLuxProp_PhoneBox::OnResetProperties()
{

}

//-----------------------------------------------------------------------

void cLuxProp_PhoneBox::OnSetupAfterLoad(cWorld *apWorld)
{
	
}

//-----------------------------------------------------------------------

void cLuxProp_PhoneBox::UpdatePropSpecific(float afTimeStep)
{
	cLuxPlayerVoiceFlashback * pVoiceFlashback = gpBase->mpPlayer->GetHelperVoiceFlashback();

    if ( mbIsPlayingVoice && pVoiceFlashback && !pVoiceFlashback->GetIsPlaying() )
    {
        mbIsPlayingVoice = false;
        VoiceStopped();
    }

	if(mbPlayerHasInteracted) return;

    FlashIfNearPlayer(afTimeStep);
}

//-----------------------------------------------------------------------

void cLuxProp_PhoneBox::BeforePropDestruction()
{
}

//-----------------------------------------------------------------------

eLuxFocusCrosshair cLuxProp_PhoneBox::GetFocusCrosshair(iPhysicsBody *apBody, const cVector3f &avPos)
{
    if ( CanInteract(apBody) )
    {
	    return eLuxFocusCrosshair_PhoneBox;
    }

    return eLuxFocusCrosshair_Default;
}

//-----------------------------------------------------------------------

void cLuxProp_PhoneBox::OnConnectionStateChange(iLuxEntity *apEntity, int alState)
{
	//TODO!
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

kBeginSerialize(cLuxProp_PhoneBox_SaveData, iLuxProp_SaveData)
kSerializeVar(msVoiceFile, eSerializeType_String)
kSerializeVar(msTextEntry1, eSerializeType_String)
kSerializeVar(mfDelay1, eSerializeType_Float32)
kSerializeVar(msTextEntry2, eSerializeType_String)
kSerializeVar(mfDelay2, eSerializeType_Float32)
kSerializeVar(msTextEntry3, eSerializeType_String)
kSerializeVar(mfDelay3, eSerializeType_Float32)
kSerializeVar(msTextEntry4, eSerializeType_String)
kSerializeVar(mfDelay4, eSerializeType_Float32)
kSerializeVar(msTextEntry5, eSerializeType_String)
kSerializeVar(mfDelay5, eSerializeType_Float32)
kSerializeVar(msTextEntry6, eSerializeType_String)
kSerializeVar(mfDelay6, eSerializeType_Float32)
kSerializeVar(msTextEntry7, eSerializeType_String)
kSerializeVar(mfDelay7, eSerializeType_Float32)
kSerializeVar(mbIsRinging, eSerializeType_Bool)
kSerializeVar(mbPlayerHasInteracted, eSerializeType_Bool)
kSerializeVar(mbIsPlayingVoice, eSerializeType_Bool)
kEndSerialize()

//-----------------------------------------------------------------------

iLuxEntity_SaveData* cLuxProp_PhoneBox::CreateSaveData()
{
	return hplNew(cLuxProp_PhoneBox_SaveData, ());
}

//-----------------------------------------------------------------------

void cLuxProp_PhoneBox::SaveToSaveData(iLuxEntity_SaveData* apSaveData)
{
	//////////////////
	//Init
	super_class::SaveToSaveData(apSaveData);
	cLuxProp_PhoneBox_SaveData *pData = static_cast<cLuxProp_PhoneBox_SaveData*>(apSaveData);

	kCopyToVar(pData,msVoiceFile);
	kCopyToVar(pData,msTextEntry1);
	kCopyToVar(pData,mfDelay1);
	kCopyToVar(pData,msTextEntry2);
	kCopyToVar(pData,mfDelay2);
	kCopyToVar(pData,msTextEntry3);
	kCopyToVar(pData,mfDelay3);
	kCopyToVar(pData,msTextEntry4);
	kCopyToVar(pData,mfDelay4);
	kCopyToVar(pData,msTextEntry5);
	kCopyToVar(pData,mfDelay5);
	kCopyToVar(pData,msTextEntry6);
	kCopyToVar(pData,mfDelay6);
	kCopyToVar(pData,msTextEntry7);
	kCopyToVar(pData,mfDelay7);
	kCopyToVar(pData,mbIsRinging);
	kCopyToVar(pData,mbPlayerHasInteracted);
	kCopyToVar(pData,mbIsPlayingVoice);
}

//-----------------------------------------------------------------------

void cLuxProp_PhoneBox::LoadFromSaveData(iLuxEntity_SaveData* apSaveData)
{
	//////////////////
	//Init
	super_class::LoadFromSaveData(apSaveData);
	cLuxProp_PhoneBox_SaveData *pData = static_cast<cLuxProp_PhoneBox_SaveData*>(apSaveData);
	
	kCopyFromVar(pData,msVoiceFile);
	kCopyFromVar(pData,msTextEntry1);
	kCopyFromVar(pData,mfDelay1);
	kCopyFromVar(pData,msTextEntry2);
	kCopyFromVar(pData,mfDelay2);
	kCopyFromVar(pData,msTextEntry3);
	kCopyFromVar(pData,mfDelay3);
	kCopyFromVar(pData,msTextEntry4);
	kCopyFromVar(pData,mfDelay4);
	kCopyFromVar(pData,msTextEntry5);
	kCopyFromVar(pData,mfDelay5);
	kCopyFromVar(pData,msTextEntry6);
	kCopyFromVar(pData,mfDelay6);
	kCopyFromVar(pData,msTextEntry7);
	kCopyFromVar(pData,mfDelay7);
	kCopyFromVar(pData,mbIsRinging);
	kCopyFromVar(pData,mbPlayerHasInteracted);
	kCopyFromVar(pData,mbIsPlayingVoice);
}

//-----------------------------------------------------------------------

void cLuxProp_PhoneBox::SetupSaveData(iLuxEntity_SaveData *apSaveData)
{
	super_class::SetupSaveData(apSaveData);
}

//-----------------------------------------------------------------------

void cLuxProp_PhoneBox::StartRinging()
{
    if ( mbPlayerHasInteracted || mbIsRinging )
    {
        return;
    }

    PlayAnimation("hammer", 0.0f, true, "");
    mbIsRinging = true;
    
 //   mpRingingSound = gpBase->mpMapHandler->GetCurrentMap()->GetWorld()->CreateSoundEntity("zephoneizringing", "telephone_ring_loop",false);
	//
 //   if( mpRingingSound )
	//{
	//	//mpRingingSound->SetForcePlayAsGUISound(true);
	//	mpRingingSound->SetPosition(GetWorldCenterPos());
	//	mpRingingSound->SetIsSaved(true);
	//}
}

//-----------------------------------------------------------------------

void cLuxProp_PhoneBox::StopRinging()
{
    /*if ( mpRingingSound )
    {
        mpRingingSound->Stop(false);
    }*/

    StopAnimation();
	
	cLuxPlayerVoiceFlashback * pVoiceFlashback = gpBase->mpPlayer->GetHelperVoiceFlashback();
    pVoiceFlashback->StopPlaying();

    mbIsRinging = false;
}

//-----------------------------------------------------------------------

void cLuxProp_PhoneBox::HangUp()
{
    cLuxPlayerVoiceFlashback * pVoiceFlashback = gpBase->mpPlayer->GetHelperVoiceFlashback();

    pVoiceFlashback->StopPlaying();

    PlayAnimation("off", 0.0f, false, "");
    gpBase->mpHelpFuncs->PlayGuiSoundData("telephone_down", eSoundEntryType_Gui);
    mbIsPlayingVoice = false;
}

//-----------------------------------------------------------------------

void cLuxProp_PhoneBox::VoiceStopped()
{
    PlayAnimation("off", 0.0f, false, "");
    //gpBase->mpHelpFuncs->PlayGuiSoundData("telephone_down", eSoundEntryType_Gui);
}