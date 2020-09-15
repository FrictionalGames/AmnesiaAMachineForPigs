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

#include "LuxScriptHandler.h"

#include "LuxMap.h"
#include "LuxPlayer.h"
#include "LuxPlayerHelpers.h"
#include "LuxMapHandler.h"
#include "LuxInputHandler.h"
#include "LuxInventory.h"
#include "LuxMoveState_Normal.h"
#include "LuxSaveHandler.h"
#include "LuxSavedGameTypes.h"
#include "LuxPlayerState.h"
#include "LuxMusicHandler.h"
#include "LuxDebugHandler.h"
#include "LuxJournal.h"
#include "LuxEffectHandler.h"
#include "LuxHelpFuncs.h"
#include "LuxMessageHandler.h"
#include "LuxCompletionCountHandler.h"
#include "LuxGlobalDataHandler.h"
#include "LuxHintHandler.h"
#include "LuxProgressLogHandler.h"
#include "LuxLoadScreenHandler.h"
//#include "LuxInsanityHandler.h"
#include "LuxCredits.h"
#include "LuxDemoEnd.h"
#include "LuxProp_PhoneBox.h"
#include "LuxPlayerHands.h"
#include "LuxHandObject_LightSource.h"

#include "LuxProp_Object.h"
#include "LuxProp_SwingDoor.h"
#include "LuxProp_Lamp.h"
#include "LuxProp_Lever.h"
#include "LuxProp_Wheel.h"
#include "LuxProp_MoveObject.h"
#include "LuxProp_Item.h"
#include "LuxProp_LevelDoor.h"
#include "LuxProp_Button.h"
#include "LuxProp_NPC.h"
#include "LuxProp_MultiSlider.h"
#include "LuxEnemy_ManPig.h"

#include "LuxArea_Sticky.h"
#include "LuxArea.h"

#include "LuxEnemy.h"
#include "LuxEnemyPathfinder.h"
#include "LuxItemType.h"

#include "LuxInteractConnections.h"

#include "LuxAchievementHandler.h"


//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

int cLuxScriptHandler::mlRopeIdCount =0;


string gsScriptNull="";

//-----------------------------------------------------------------------

cLuxScriptHandler::cLuxScriptHandler() : iLuxUpdateable("LuxScriptHandler")
{
	mpLowLevelSystem = gpBase->mpEngine->GetSystem()->GetLowLevel();

	
	InitScriptFunctions();
}

//-----------------------------------------------------------------------

cLuxScriptHandler::~cLuxScriptHandler()
{
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------


void cLuxScriptHandler::OnStart()
{
	
}

//-----------------------------------------------------------------------


void cLuxScriptHandler::Reset()
{

}

//-----------------------------------------------------------------------

void cLuxScriptHandler::Update(float afTimeStep)
{
}

//-----------------------------------------------------------------------


void cLuxScriptHandler::OnDraw(float afFrameTime)
{
	
}


//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------



//-----------------------------------------------------------------------


//////////////////////////////////////////////////////////////////////////
// SCRIPT FUNCTION HELPERS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cLuxScriptHandler::AddFunc(const tString& asFunc, void *apFuncPtr)
{
	mpLowLevelSystem->AddScriptFunc(asFunc,apFuncPtr);
}

//-----------------------------------------------------------------------

static iLuxArea* ToArea(iLuxEntity *apEntity)
{
	return static_cast<iLuxArea*>(apEntity);
}

static cLuxArea_Sticky* ToStickyArea(iLuxEntity *apEntity)
{
	return static_cast<cLuxArea_Sticky*>(apEntity);
}

static iLuxProp* ToProp(iLuxEntity *apEntity)
{
	return static_cast<iLuxProp*>(apEntity);
}

static cLuxProp_Object* ToObject(iLuxEntity *apEntity)
{
	return static_cast<cLuxProp_Object*>(apEntity);
}

static cLuxProp_Lamp* ToLamp(iLuxEntity *apEntity)
{
	return static_cast<cLuxProp_Lamp*>(apEntity);
}
static cLuxProp_SwingDoor* ToSwingDoor(iLuxEntity *apEntity)
{
	return static_cast<cLuxProp_SwingDoor*>(apEntity);
}
static cLuxProp_Lever* ToLever(iLuxEntity *apEntity)
{
	return static_cast<cLuxProp_Lever*>(apEntity);
}
static cLuxProp_Wheel* ToWheel(iLuxEntity *apEntity)
{
	return static_cast<cLuxProp_Wheel*>(apEntity);
}
static cLuxProp_MoveObject* ToMoveObject(iLuxEntity *apEntity)
{
	return static_cast<cLuxProp_MoveObject*>(apEntity);
}
static iLuxEnemy* ToEnemy(iLuxEntity *apEntity)
{
	return static_cast<iLuxEnemy*>(apEntity);
}
static cLuxEnemy_ManPig * ToManPig(iLuxEntity *apEntity)
{
	return static_cast<cLuxEnemy_ManPig*>(apEntity);
}
static cLuxProp_LevelDoor* ToLevelDoor(iLuxEntity *apEntity)
{
	return static_cast<cLuxProp_LevelDoor*>(apEntity);
}
static cLuxProp_Button* ToButton(iLuxEntity *apEntity)
{
	return static_cast<cLuxProp_Button*>(apEntity);
}

static cLuxProp_NPC* ToNPC(iLuxEntity *apEntity)
{
	return static_cast<cLuxProp_NPC*>(apEntity);
}

static cLuxProp_MultiSlider* ToMultiSlider(iLuxEntity *apEntity)
{
	return static_cast<cLuxProp_MultiSlider*>(apEntity);
}


//-----------------------------------------------------------------------

bool cLuxScriptHandler::GetEntities(const tString& asName,tLuxEntityList &alstEntities, eLuxEntityType aType, int alSubType)
{
	cLuxMap *pMap = gpBase->mpMapHandler->GetCurrentMap();
	if(pMap==NULL)
	{
		Error("GetEntities(..) failed! No map was set!\n");
		return false;
	}

	///////////////////
	// Exact match
	if(cString::CountCharsInString(asName,"*")==0)
	{
		iLuxEntity *pEntity = pMap->GetEntityByName(asName,aType, alSubType);
		if(pEntity==NULL)
		{
			Warning("Entity '%s' with type %d and subtype %d does not exist!\n", asName.c_str(), aType, alSubType);
			return false;
		}
		        
		alstEntities.push_back(pEntity);
	}
	///////////////////
	// Wild card
	else
	{
		tStringVec vWantedStrings;
		tString sSepp = "*";
		cString::GetStringVec(asName,vWantedStrings,&sSepp);

        cLuxEntityIterator it = pMap->GetEntityIterator();
		while(it.HasNext())
		{
			iLuxEntity *pEntity = it.Next();
			if(LuxIsCorrectType(pEntity,aType, alSubType))
			{
				bool bContainsStrings = true;
				int lLastPos = -1;
				
				//Iterate wanted strings and name make sure they exist and show up in correct order.
				for(size_t i=0; i<vWantedStrings.size(); ++i)
				{
					int lPos = cString::GetFirstStringPos(pEntity->GetName(), vWantedStrings[i]);
                    if(lPos <= lLastPos) 
					{
						bContainsStrings = false;
						break;
					}
				}

				if(bContainsStrings) alstEntities.push_back(pEntity);	
			}
		}

		if(alstEntities.empty())
		{
			Warning("Could not find any entities with string '%s'\n", asName.c_str());
			return false;
		}
	}

	return true;
}

bool cLuxScriptHandler::GetParticleSystems(const tString& asName,std::list<cParticleSystem*> &alstParticleSystems)
{
	cLuxMap *pMap = gpBase->mpMapHandler->GetCurrentMap();
	if(pMap==NULL)
	{
		Error("GetParticleSystems(..) failed! No map was set!\n");
		return false;
	}

	///////////////////
	// Exact match
	if(cString::CountCharsInString(asName,"*")==0)
	{
		cParticleSystem *pPS = pMap->GetWorld()->GetParticleSystem(asName);
		if(pPS==NULL)
		{
			Warning("Particle system '%s' does not exist!\n", asName.c_str());
			return false;
		}
		        
		alstParticleSystems.push_back(pPS);
	}
	///////////////////
	// Wild card
	else
	{
		tStringVec vWantedStrings;
		tString sSepp = "*";
		cString::GetStringVec(asName,vWantedStrings,&sSepp);

        cParticleSystemIterator it = pMap->GetWorld()->GetParticleSystemIterator();
		while(it.HasNext())
		{
			cParticleSystem *pPS = it.Next();
			bool bContainsStrings = true;
			int lLastPos = -1;
				
			//Iterate wanted strings and name make sure they exist and show up in correct order.
			for(size_t i=0; i<vWantedStrings.size(); ++i)
			{
				int lPos = cString::GetFirstStringPos(pPS->GetName(), vWantedStrings[i]);
                if(lPos <= lLastPos) 
				{
					bContainsStrings = false;
					break;
				}
			}

			if(bContainsStrings) alstParticleSystems.push_back(pPS);	
		}

		if(alstParticleSystems.empty())
		{
			Warning("Could not find any particle systems with string '%s'\n", asName.c_str());
			return false;
		}
	}

	return true;
}

//-----------------------------------------------------------------------

iLuxEntity* cLuxScriptHandler::GetEntity(const tString& asName, eLuxEntityType aType, int alSubType)
{
	cLuxMap *pMap = gpBase->mpMapHandler->GetCurrentMap();
	if(pMap==NULL)
	{
		Error("GetEntity(..) failed! No map was set!\n");
		return NULL;
	}


	iLuxEntity *pEntity = pMap->GetEntityByName(asName);
	if(pEntity==NULL)
	{
		Warning("Entity '%s' does not exist!\n", asName.c_str());
		return NULL;
	}

	if(LuxIsCorrectType(pEntity,aType, alSubType)==false)
	{
		Warning("Entity '%s' is not correct type! (%d %d)\n", asName.c_str(), aType, alSubType);
		return NULL;
	}

	return pEntity;
}

//-----------------------------------------------------------------------

#define BEGIN_SET_PROPERTY(aType, aSubType)\
	tLuxEntityList lstEntities;\
	if(GetEntities(asName, lstEntities,aType, aSubType)==false) return;\
	for(tLuxEntityListIt it = lstEntities.begin(); it != lstEntities.end(); ++it)\
	{\
	iLuxEntity *pEntity = *it;

#define END_SET_PROPERTY }

#define BEGIN_ITERATE_PARTICLESYSTEM()\
	std::list<cParticleSystem *> lstParticleSystems;\
	if(GetParticleSystems(asName, lstParticleSystems)==false) return;\
	for(std::list<cParticleSystem *>::iterator it = lstParticleSystems.begin(); it != lstParticleSystems.end(); ++it)\
	{\
	cParticleSystem * pParticleSystem = *it;

#define END_ITERATE_PARTICLESYSTEM }

//-----------------------------------------------------------------------

void cLuxScriptHandler::InitScriptFunctions()
{
	AddFunc("void Print(string &in asString)", (void *)Print);
	AddFunc("void AddDebugMessage(string &in asString, bool abCheckForDuplicates)",(void *)AddDebugMessage);
	AddFunc("void ProgLog(string &in asLevel, string &in asMessage)", (void *)ProgLog);
	AddFunc("bool ScriptDebugOn()",(void *)ScriptDebugOn);

	AddFunc("float RandFloat(float afMin, float afMax)",(void *)RandFloat);
	AddFunc("int RandInt(int alMin, int alMax)",(void *)RandInt);
	AddFunc("bool StringContains(string &in asString, string &in asSubString)", (void *)StringContains);
	AddFunc("string& StringSub(string &in asString, int alStart, int alCount)", (void *)StringSub);

	AddFunc("void AddTimer(string &in asName, float afTime, string &in asFunction)",(void *)AddTimer);
	AddFunc("void RemoveTimer(string &in asName)",(void *)RemoveTimer);
	AddFunc("float GetTimerTimeLeft(string &in asName)",(void *)GetTimerTimeLeft);

	AddFunc("void SetLocalVarInt(string &in asName, int alVal)",(void *)SetLocalVarInt);
	AddFunc("void SetLocalVarFloat(string &in asName, float afVal)",(void *)SetLocalVarFloat);
	AddFunc("void SetLocalVarString(string &in asName, string &in asVal)",(void *)SetLocalVarString);
	
	AddFunc("void AddLocalVarInt(string &in asName, int alVal)",(void *)AddLocalVarInt);
	AddFunc("void AddLocalVarFloat(string &in asName, float afVal)",(void *)AddLocalVarFloat);
	AddFunc("void AddLocalVarString(string &in asName, string &in asVal)",(void *)AddLocalVarString);

	AddFunc("int GetLocalVarInt(string &in asName)",(void *)GetLocalVarInt);
	AddFunc("float GetLocalVarFloat(string &in asName)",(void *)GetLocalVarFloat);
	AddFunc("string& GetLocalVarString(string &in asName)",(void *)GetLocalVarString);

	AddFunc("void SetGlobalVarInt(string &in asName, int alVal)",(void *)SetGlobalVarInt);
	AddFunc("void SetGlobalVarFloat(string &in asName, float afVal)",(void *)SetGlobalVarFloat);
	AddFunc("void SetGlobalVarString(string &in asName, string &in asVal)",(void *)SetGlobalVarString);

	AddFunc("void AddGlobalVarInt(string &in asName, int alVal)",(void *)AddGlobalVarInt);
	AddFunc("void AddGlobalVarFloat(string &in asName, float afVal)",(void *)AddGlobalVarFloat);
	AddFunc("void AddGlobalVarString(string &in asName, string &in asVal)",(void *)AddGlobalVarString);

	AddFunc("int GetGlobalVarInt(string &in asName)",(void *)GetGlobalVarInt);
	AddFunc("float GetGlobalVarFloat(string &in asName)",(void *)GetGlobalVarFloat);
	AddFunc("string& GetGlobalVarString(string &in asName)",(void *)GetGlobalVarString);

	AddFunc("void StartCredits(string &in asMusic, bool abLoopMusic, string &in asTextCat, string &in asTextEntry, int alEndNum)",(void *)StartCredits);
	AddFunc("void AddKeyPart(int alKeyPart)", (void *)AddKeyPart);

	AddFunc("void StartDemoEnd()",(void *)StartDemoEnd);

	AddFunc("void AutoSave()", (void *)AutoSave);
	AddFunc("void CheckPoint(string &in asName,string &in asStartPos ,string &in asCallback, string &in asDeathHintCat, string &in asDeathHintEntry, bool abPlayerDark)", (void *)CheckPoint);
	AddFunc("void ReleasePlayerFromLimbo()", (void *)ReleasePlayerFromLimbo);

	AddFunc("void ChangeMap(string &in asMapName, string &in asStartPos, string &in asStartSound, string &in asEndSound)",(void *)ChangeMap);
	AddFunc("void ClearSavedMaps()",(void *)ClearSavedMaps);
	AddFunc("void CreateDataCache()",(void *)CreateDataCache);
	AddFunc("void DestroyDataCache()",(void *)DestroyDataCache);
	AddFunc("void SetMapDisplayNameEntry(string &in asNameEntry)",(void *)SetMapDisplayNameEntry);
	AddFunc("void SetSkyBoxActive(bool abActive)",(void *)SetSkyBoxActive);
	AddFunc("void SetSkyBoxTexture(string &in asTexture)",(void *)SetSkyBoxTexture);
	AddFunc("void SetSkyBoxColor(float afR, float afG, float afB, float afA)",(void *)SetSkyBoxColor);
      
	AddFunc("void UnlockAchievement(string &in asName)", (void *)UnlockAchievement);

	AddFunc("void SetFogActive(bool abActive)",(void *)SetFogActive);
	AddFunc("void SetFogColor(float afR, float afG, float afB, float afA)",(void *)SetFogColor);
	AddFunc("void SetFogProperties(float afStart, float afEnd, float afFalloffExp, bool abCulling)",(void *)SetFogProperties);

	AddFunc("void SetupLoadScreen(string &in asTextCat, string &in asTextEntry, int alRandomNum, string &in asImageFile)",(void *)SetupLoadScreen);
	
	AddFunc("void FadeIn(float afTime)",(void *)FadeIn);
	AddFunc("void FadeOut(float afTime)",(void *)FadeOut);
	AddFunc("void FadeImageTrailTo(float afAmount, float afSpeed)",(void *)FadeImageTrailTo);
	AddFunc("void FadeSepiaColorTo(float afAmount, float afSpeed)",(void *)FadeSepiaColorTo);
	AddFunc("void FadeRadialBlurTo(float afSize, float afSpeed)",(void *)FadeRadialBlurTo);
	AddFunc("void SetRadialBlurStartDist(float afStartDist)",(void *)SetRadialBlurStartDist);

	AddFunc("void StartEffectFlash(float afFadeIn, float afWhite, float afFadeOut)",(void *)StartEffectFlash);
	AddFunc("void StartEffectEmotionFlash(string &in asTextCat, string &in asTextEntry, string &in asSound)",(void *)StartEffectEmotionFlash);

	AddFunc("void AddEffectVoice(string &in asVoiceFile, string &in asEffectFile, string &in asTextCat, string &in asTextEntry, bool abUsePostion,  string &in asPosEnitity, float afMinDistance, float afMaxDistance)",(void *)AddEffectVoice);
	AddFunc("void AddEffectVoice(string &in asVoiceFile, string &in asEffectFile, string &in asTextCat, string &in asTextEntry, bool abUsePostion,  string &in asPosEnitity, float afMinDistance, float afMaxDistance, int alPriority, bool abStopInterrupted)",(void *)AddEffectVoiceExt);
	
	AddFunc("void AddEffectVoice2(string &in asVoiceFile, string &in asEffectFile, string &in asTextCat, string &in asTextEntry, float afTextDelay, string &in asText2Entry, float afText2Delay, bool abUsePostion,  string &in asPosEnitity, float afMinDistance, float afMaxDistance)",(void *)AddEffectVoice2);
	AddFunc("void AddEffectVoice3(string &in asVoiceFile, string &in asEffectFile, string &in asTextCat, string &in asTextEntry, float afTextDelay, string &in asText2Entry, float afText2Delay, string &in asText3Entry, float afText3Delay, bool abUsePostion,  string &in asPosEnitity, float afMinDistance, float afMaxDistance)",(void *)AddEffectVoice3);
	AddFunc("void AddEffectVoice4(string &in asVoiceFile, string &in asEffectFile, string &in asTextCat, string &in asTextEntry, float afTextDelay, string &in asText2Entry, float afText2Delay, string &in asText3Entry, float afText3Delay, string &in asText4Entry, float afText4Delay, bool abUsePostion, string &in asPosEnitity, float afMinDistance, float afMaxDistance)",(void *)AddEffectVoice4);
	AddFunc("void AddEffectVoice5(string &in asVoiceFile, string &in asEffectFile, string &in asTextCat, string &in asTextEntry, float afTextDelay, string &in asText2Entry, float afText2Delay, string &in asText3Entry, float afText3Delay, string &in asText4Entry, float afText4Delay, string &in asText5Entry, float afText5Delay, bool abUsePostion, string &in asPosEnitity, float afMinDistance, float afMaxDistance)",(void *)AddEffectVoice5);
	AddFunc("void AddEffectVoice6(string &in asVoiceFile, string &in asEffectFile, string &in asTextCat, string &in asTextEntry, float afTextDelay, string &in asText2Entry, float afText2Delay, string &in asText3Entry, float afText3Delay, string &in asText4Entry, float afText4Delay, string &in asText5Entry, float afText5Delay, string &in asText6Entry, float afText6Delay, bool abUsePostion, string &in asPosEnitity, float afMinDistance, float afMaxDistance)",(void *)AddEffectVoice6);
	AddFunc("void AddEffectVoice7(string &in asVoiceFile, string &in asEffectFile, string &in asTextCat, string &in asTextEntry, float afTextDelay, string &in asText2Entry, float afText2Delay, string &in asText3Entry, float afText3Delay, string &in asText4Entry, float afText4Delay, string &in asText5Entry, float afText5Delay, string &in asText6Entry, float afText6Delay, string &in asText7Entry, float afText7Delay, bool abUsePostion, string &in asPosEnitity, float afMinDistance, float afMaxDistance)",(void *)AddEffectVoice7);

	AddFunc("void StopAllEffectVoices(float afFadeOutTime)",(void *)StopAllEffectVoices);
	AddFunc("bool GetEffectVoiceActive()",(void *)GetEffectVoiceActive);
	AddFunc("void SetEffectVoiceOverCallback(string &in asFunc)", (void *)SetEffectVoiceOverCallback);
	AddFunc("void StartScreenShake(float afAmount, float afTime, float afFadeInTime,float afFadeOutTime)",(void *)StartScreenShake);
	AddFunc("bool GetFlashbackIsActive()", (void *)GetFlashbackIsActive);

	AddFunc("void ShowScreenImage(string &in asImageName, float afX, float afY, float afScale, bool abUseRelativeCoordinates, float afDuration, float afFadeIn, float afFadeOut)", (void *)ShowScreenImage);
	AddFunc("void HideScreenImageImmediately()", (void *)HideScreenImageImmediately);
	AddFunc("void HideScreenImageWithFade(float afFadeOut)", (void *)HideScreenImageWithFade);

//	AddFunc("void SetInsanitySetEnabled(string &in asSet, bool abX)", (void *)SetInsanitySetEnabled);
//	AddFunc("void StartRandomInsanityEvent()", (void *)StartRandomInsanityEvent);
//	AddFunc("void InsanityEventIsActive()", (void *)InsanityEventIsActive);

	AddFunc("void StartPlayerSpawnPS(string &in asSPSFile)", (void *)StartPlayerSpawnPS);
	AddFunc("void StopPlayerSpawnPS()", (void *)StartPlayerSpawnPS);

	AddFunc("void PlayGuiSound(string &in asSoundFile, float afVolume)",(void *)PlayGuiSound);

	AddFunc("void SetPlayerActive(bool abActive)",(void *)SetPlayerActive);
	AddFunc("void ChangePlayerStateToNormal()",(void *)ChangePlayerStateToNormal);
	AddFunc("void SetPlayerCrouching(bool abCrouch)",(void *)SetPlayerCrouching);
	AddFunc("void AddPlayerBodyForce(float afX, float afY, float afZ, bool abUseLocalCoords)",(void *)AddPlayerBodyForce);
	AddFunc("void ShowPlayerCrossHairIcons(bool abX)",(void *)ShowPlayerCrossHairIcons);
	
	AddFunc("void SetInfectionFauxMode(bool abFauxMode)",(void *)SetInfectionFauxMode);
	AddFunc("void SetPlayerInfection(float afInfection)",(void *)SetPlayerInfection);
	AddFunc("void AddPlayerInfection(float afInfection)",(void *)AddPlayerInfection);
	AddFunc("float GetPlayerInfection()",(void *)GetPlayerInfection);
	AddFunc("void SetPlayerHealth(float afHealth)",(void *)SetPlayerHealth);
	AddFunc("void AddPlayerHealth(float afHealth)",(void *)AddPlayerHealth);
	AddFunc("float GetPlayerHealth()",(void *)GetPlayerHealth);
	
	AddFunc("float GetPlayerSpeed()",(void *)GetPlayerSpeed);
	AddFunc("float GetPlayerYSpeed()",(void *)GetPlayerYSpeed);
	AddFunc("void MovePlayerForward(float afAmount)",(void *)MovePlayerForward);
	AddFunc("void SetPlayerPermaDeathSound(string &in asSound)",(void *)SetPlayerPermaDeathSound);

	AddFunc("void GiveInfectionDamage(float afAmount, bool abUseEffect)",(void *)GiveInfectionDamage);

	AddFunc("void GivePlayerDamage(float afAmount, string &in asType, bool abSpinHead, bool abLethal)",(void *)GivePlayerDamage);
	AddFunc("void FadePlayerFOVMulTo(float afX, float afSpeed)",(void *)FadePlayerFOVMulTo);
	AddFunc("void FadePlayerAspectMulTo(float afX, float afSpeed)",(void *)FadePlayerAspectMulTo);
	AddFunc("void FadePlayerRollTo(float afX, float afSpeedMul, float afMaxSpeed)",(void *)FadePlayerRollTo);
	AddFunc("void FadePlayerPitchTo(float afX, float afSpeedMul, float afMaxSpeed)",(void *)FadePlayerPitchTo);
	AddFunc("void MovePlayerHeadPos(float afX, float afY, float afZ, float afSpeed, float afSlowDownDist)",(void *)MovePlayerHeadPos);

	AddFunc("void StartPlayerLookAt(string &in asEntityName, float afSpeedMul, float afMaxSpeed,string &in asAtTargetCallback)",(void *)StartPlayerLookAt);
	AddFunc("void StopPlayerLookAt()",(void *)StopPlayerLookAt);

	AddFunc("void SetPlayerMoveSpeedMul(float afMul)",(void *)SetPlayerMoveSpeedMul);
	AddFunc("void SetPlayerRunSpeedMul(float afMul)",(void *)SetPlayerRunSpeedMul);
	AddFunc("void SetPlayerLookSpeedMul(float afMul)",(void *)SetPlayerLookSpeedMul);
	AddFunc("void SetPlayerJumpDisabled(bool abX)",(void *)SetPlayerJumpDisabled);
	AddFunc("void SetPlayerCrouchDisabled(bool abX)",(void *)SetPlayerCrouchDisabled);

	AddFunc("void TeleportPlayer(string &in asStartPosName)",(void *)TeleportPlayer);
	AddFunc("void SetLanternActive(bool abX, bool abUseEffects)",(void *)SetLanternActive);
	AddFunc("bool GetLanternActive()",(void *)GetLanternActive);
	AddFunc("void SetLanternDisabled(bool abX)",(void *)SetLanternDisabled);
	AddFunc("void SetLanternLitCallback(string &in asCallback)",(void *)SetLanternLitCallback);

	AddFunc("bool GetJournalDisabled()",(void *)GetJournalDisabled);
	AddFunc("void SetJournalDisabled(bool abX)",(void *)SetJournalDisabled);

	AddFunc("void SetLanternFlickerActive(bool abX)",(void *)SetLanternFlickerActive);
	
	AddFunc("void SetMessage(string &in asTextCategory, string &in asTextEntry, float afTime)",(void *)SetMessage);
	AddFunc("void SetDeathHint(string &in asTextCategory, string &in asTextEntry)",(void *)SetDeathHint);
	AddFunc("void DisableDeathStartSound()",(void *)DisableDeathStartSound);

	AddFunc("void AddNote(string &in asNameAndTextEntry, string &in asImage)",(void *)AddNote);
	AddFunc("void AddDiary(string &in asNameAndTextEntry, string &in asImage)",(void *)AddDiary);
	AddFunc("void AddHint(string &in asNameAndTextEntry, string &in asImage)",(void *)AddHint);
	AddFunc("void ReturnOpenJournal(bool abOpenJournal)",(void *)ReturnOpenJournal);

	AddFunc("void RemoveAllHints()",(void *)RemoveAllHints);
	AddFunc("void RemoveHint(string &in asNameAndTextEntry)",(void *)RemoveHint);

    AddFunc("void StartPhoneRinging(string &in asName)",(void *)StartPhoneRinging);
    AddFunc("void StopPhoneRinging(string &in asName)",(void *)StopPhoneRinging);
    AddFunc("void HangUpPhone(string &in asName)",(void *)HangUpPhone);

    AddFunc("void SetPlayerUsesDragFootsteps(bool abX)",(void *)SetPlayerUsesDragFootsteps);

	AddFunc("void AddQuest(string &in asName, string &in asNameAndTextEntry)",(void *)AddQuest);
	AddFunc("void CompleteQuest(string &in asName, string &in asNameAndTextEntry)",(void *)CompleteQuest);
	AddFunc("bool QuestIsCompleted(string &in asName)",(void *)QuestIsCompleted);
	AddFunc("bool QuestIsAdded(string &in asName)",(void *)QuestIsAdded);
	AddFunc("void SetNumberOfQuestsInMap(int alNumberOfQuests)",(void *)SetNumberOfQuestsInMap);

	AddFunc("void GiveHint(string &in asName, string &in asMessageCat, string &in asMessageEntry, float afTimeShown)",(void *)GiveHint);
	AddFunc("void BlockHint(string &in asName)", (void *)BlockHint);
	AddFunc("void UnBlockHint(string &in asName)", (void *)UnBlockHint);

	//AddFunc("void ExitInventory()",(void *)ExitInventory);
	//AddFunc("void SetInventoryDisabled(bool abX)",(void *)SetInventoryDisabled);
	//AddFunc("void SetInventoryMessage(string &in asTextCategory, string &in asTextEntry, float afTime)",(void *)SetInventoryMessage);
	
	AddFunc("void GiveItem(string &in asName, string &in asType, string &in asSubTypeName, string &in asImageName, float afAmount)",(void *)GiveItem);
	AddFunc("void GiveItemFromFile(string &in asName, string &in asFileName)",(void *)GiveItemFromFile);
	AddFunc("void RemoveItem(string &in asName)",(void *)RemoveItem);
	AddFunc("bool HasItem(string &in asName)",(void *)HasItem);

	AddFunc("void AddCombineCallback(string &in asName, string &in asItemA, string &in asItemB, string &in asFunction, bool abAutoDestroy)",(void *)AddCombineCallback);
	AddFunc("void RemoveCombineCallback(string &in asName)",(void *)RemoveCombineCallback);

	AddFunc("void AddUseItemCallback(string &in asName, string &in asItem, string &in asEntity, string &in asFunction, bool abAutoDestroy)",(void *)AddUseItemCallback);
	AddFunc("void RemoveUseItemCallback(string &in asName)",(void *)RemoveUseItemCallback);

	AddFunc("void PreloadParticleSystem(string &in asPSFile)",(void *)PreloadParticleSystem);
	AddFunc("void PreloadSound(string &in asSoundFile)",(void *)PreloadSound);

	AddFunc("void CreateParticleSystemAtEntity(string &in asPSName, string &in asPSFile, string &in asEntity, bool abSavePS)",(void *)CreateParticleSystemAtEntity);
	AddFunc("void CreateParticleSystemAtEntityExt(	string &in asPSName, string &in asPSFile, string &in asEntity, bool abSavePS, float afR, float afG, float afB, float afA, bool abFadeAtDistance, float afFadeMinEnd, float afFadeMinStart, float afFadeMaxStart, float afFadeMaxEnd)", (void *)CreateParticleSystemAtEntityExt);
	AddFunc("void DestroyParticleSystem(string &in asName)",(void *)DestroyParticleSystem); 
	AddFunc("void DestroyParticleSystemInstantly(string &in asName)",(void *)DestroyParticleSystemInstantly); 
	
	AddFunc("void PlaySoundAtEntity(string &in asSoundName, string &in asSoundFile, string &in asEntity, float afFadeSpeed, bool abSaveSound)",(void *)PlaySoundAtEntity);
	AddFunc("void PlaySoundAtPosition(string &in asSoundName, string &in asSoundFile, float afX, float afY, float afZ, float afFadeSpeed, bool abSaveSound)",(void *)PlaySoundAtPosition);

	AddFunc("void FadeInSound(string &in asSoundName, float afFadeTime, bool abPlayStart)",(void *)FadeInSound);
	AddFunc("void StopSound(string &in asSoundName, float afFadeTime)",(void *)StopSound);
	AddFunc("void PlayMusic(string &in asMusicFile, bool abLoop, float afVolume, float afFadeTime, int alPrio, bool abResume)",(void *)PlayMusic);
	AddFunc("void StopMusic(float afFadeTime, int alPrio)",(void *)StopMusic);
	AddFunc("void FadeGlobalSoundVolume(float afDestVolume, float afTime)",(void *)FadeGlobalSoundVolume);
	AddFunc("void FadeGlobalSoundSpeed(float afDestSpeed, float afTime)",(void *)FadeGlobalSoundSpeed);
	
	AddFunc("void SetLightVisible(string &in asLightName, bool abVisible)",(void *)SetLightVisible);
	AddFunc("void FadeLightTo(string &in asLightName, float afR, float afG, float afB, float afA, float afRadius, float afTime)",(void *)FadeLightTo);
	AddFunc("void SetLightFlickerActive(string &in asLightName, bool abActive)", (void *)SetLightFlickerActive);

	AddFunc("void FadeLampTo(string &in asName, float afR, float afG, float afB, float afA, float afRadius, float afTime)",(void *)FadeLampTo);
	AddFunc("void SetLampFlickerActive(string &in asName, bool abActive)", (void *)SetLampFlickerActive);

	AddFunc("void SetPhysicsAutoDisable(string &in asName, bool abAutoDisable)", (void *)SetPhysicsAutoDisable);

	AddFunc("void SetParticleSystemActive(string &in asName, bool abActive)", (void *)SetParticleSystemActive);

	AddFunc("void SetEntityActive(string &in asName, bool abActive)",(void *)SetEntityActive);
	AddFunc("bool GetEntityExists(string &in asName)",(void *)GetEntityExists);
	AddFunc("bool GetEntityActive(string &in asName)",(void *)GetEntityActive);
	AddFunc("void SetEntityCustomFocusCrossHair(string &in asName, string &in asCrossHair)",(void *)SetEntityCustomFocusCrossHair);
	AddFunc("void CreateEntityAtArea(string &in asEntityName, string &in asEntityFile, string &in asAreaName, bool abFullGameSave, float afPosX, float afPosY, float afPosZ, float afRotX, float afRotY, float afRotZ)",(void *)CreateEntityAtArea);
	AddFunc("void SetEntityPlayerLookAtCallback(string &in asName, string &in asCallback, bool abRemoveWhenLookedAt)",(void *)SetEntityPlayerLookAtCallback);
	AddFunc("void SetEntityPlayerInteractCallback(string &in asName, string &in asCallback, bool abRemoveOnInteraction)",(void *)SetEntityPlayerInteractCallback);
	AddFunc("void SetEntityCallbackFunc(string &in asName, string &in asCallback)", (void *)SetEntityCallbackFunc);
	AddFunc("void SetEntityConnectionStateChangeCallback(string &in asName, string &in asCallback)", (void *)SetEntityConnectionStateChangeCallback);
	AddFunc("void SetEntityInteractionDisabled(string &in asName, bool abDisabled)", (void *)SetEntityInteractionDisabled);
	AddFunc("bool GetEntitiesCollide(string &in asEntityA, string &in asEntityB)",(void *)GetEntitiesCollide);
	
	AddFunc("void SetPropEffectActive(string &in asName, bool abActive, bool abFadeAndPlaySounds)", (void *)SetPropEffectActive);
	AddFunc("void SetPropActiveAndFade(string &in asName, bool abActive, float afFadeTime)",(void *)SetPropActiveAndFade);
	AddFunc("void SetPropStaticPhysics(string &in asName, bool abX)", (void *)SetPropStaticPhysics);
	AddFunc("bool GetPropIsInteractedWith(string &in asName)", (void *)GetPropIsInteractedWith);
	AddFunc("void RotatePropToSpeed(string &in asName, float afAcc, float afGoalSpeed, float afAxisX, float afAxisY, float afAxisZ, bool abResetSpeed, string &in asOffsetArea)", (void *)RotatePropToSpeed);	
	AddFunc("void StopPropMovement(string &in asName)", (void *)StopPropMovement);	

	AddFunc("void AttachPropToBone(string &in asChildEntityName, string &in asParentEntityName, string &in asParentBoneName, float fPosX, float fPosY, float fPosZ, float fRotX, float fRotY, float fRotZ)",(void *)AttachPropToBone);
	AddFunc("void DetachPropFromBone(string &in asChildEntityName)",(void *)DetachPropFromBone);

	AddFunc("void AddAttachedPropToProp(string &in asPropName, string &in asAttachName, string &in asAttachFile, float fPosX, float fPosY, float fPosZ, float fRotX, float fRotY, float fRotZ)",(void *)AddAttachedPropToProp);
	AddFunc("void RemoveAttachedPropFromProp(string &in asPropName, string &in asAttachName)",(void *)RemoveAttachedPropFromProp);

	AddFunc("void SetLampCanBeLitByPlayer(string &in asName, bool abLit)",(void *)SetLampCanBeLitByPlayer); 

	AddFunc("void SetLampLit(string &in asName, bool abLit, bool abEffects)",(void *)SetLampLit); 
	AddFunc("void SetSwingDoorLocked(string &in asName, bool abLocked, bool abEffects)",(void *)SetSwingDoorLocked);
	AddFunc("void SetSwingDoorClosed(string &in asName, bool abClosed, bool abEffects)",(void *)SetSwingDoorClosed);
	AddFunc("void SetSwingDoorDisableAutoClose(string &in asName, bool abDisableAutoClose)",(void *)SetSwingDoorDisableAutoClose);
	AddFunc("void SetSwingDoorOpenAmount(string &in asName, float afOpenAmount, float afDuration, bool abOpenTowardsMaxAngle)",(void *)SetSwingDoorOpenAmount);
	AddFunc("void SetLevelDoorLocked(string &in asName, bool abLocked)", (void *)SetLevelDoorLocked);
	AddFunc("void SetLevelDoorLockedSound(string &in asName, string &in asSound)", (void *)SetLevelDoorLockedSound);
	AddFunc("void SetLevelDoorLockedText(string &in asName, string &in asTextCat, string &in asTextEntry)", (void *)SetLevelDoorLockedText);
	AddFunc("bool GetSwingDoorLocked(string &in asName)",(void *)GetSwingDoorLocked);
	AddFunc("bool GetSwingDoorClosed(string &in asName)",(void *)GetSwingDoorClosed);
	AddFunc("int GetSwingDoorState(string &in asName)",(void *)GetSwingDoorState);
	AddFunc("void SetPropObjectStuckState(string &in asName, int alState)",(void *)SetPropObjectStuckState);
	AddFunc("void SetWheelAngle(string &in asName, float afAngle, bool abAutoMove)",(void *)SetWheelAngle);
	AddFunc("void SetWheelStuckState(string &in asName, int alState, bool abEffects)",(void *)SetWheelStuckState);
	AddFunc("void SetLeverStuckState(string &in asName, int alState, bool abEffects)",(void *)SetLeverStuckState);
	AddFunc("void SetWheelInteractionDisablesStuck(string &in asName, bool abX)",(void *)SetWheelInteractionDisablesStuck);
	AddFunc("void SetLeverInteractionDisablesStuck(string &in asName, bool abX)",(void *)SetLeverInteractionDisablesStuck);
	AddFunc("int GetLeverState(string &in asName)",(void *)GetLeverState);
	
	AddFunc("void SetMultiSliderStuckState(string &in asName, int alStuckState, bool abEffects)",(void *)SetMultiSliderStuckState);
	AddFunc("void SetMultiSliderCallback(string &in asName, string &in asCallback)",(void *)SetMultiSliderCallback);
	
	AddFunc("void SetButtonSwitchedOn(string &in asName, bool abSwitchedOn, bool abEffects)",(void *)SetButtonSwitchedOn);
	AddFunc("void SetButtonCanBeSwitchedOn(string &in asName, bool abCanBeSwitchedOn)",(void *)SetButtonCanBeSwitchedOn);
	AddFunc("void SetButtonCanBeSwitchedOff(string &in asName, bool abCanBeSwitchedOff)",(void *)SetButtonCanBeSwitchedOff);

    AddFunc("void SetAllowStickyAreaAttachment(bool abX)", (void *)SetAllowStickyAreaAttachment);
	AddFunc("void AttachPropToStickyArea(string &in asAreaName, string &in asProp)", (void *)AttachPropToStickyArea);
	AddFunc("void AttachBodyToStickyArea(string &in asAreaName, string &in asBody)", (void *)AttachBodyToStickyArea);
	AddFunc("void DetachFromStickyArea(string &in asAreaName)", (void *)DetachFromStickyArea);
	AddFunc("void SetNPCAwake(string &in asName, bool abAwake, bool abEffects)",(void *)SetNPCAwake);
	AddFunc("void SetNPCFollowPlayer(string &in asName, bool abX)",(void *)SetNPCFollowPlayer);

	AddFunc("void AttachAreaToProp(string &in asAreaName, string &in asProp, int alBody)", (void*)AttachAreaToProp);

	AddFunc("void SetEnemyDisabled(string &in asName, bool abDisabled)",(void *)SetEnemyDisabled);
	AddFunc("void SetEnemyActivationDistance(string &in asName, float afX)",(void *)SetEnemyActivationDistance);
	AddFunc("void SetEnemyIsHallucination(string &in asName, bool abX)",(void *)SetEnemyIsHallucination);
	AddFunc("void FadeEnemyToSmoke(string &in asName, bool abPlaySound)",(void *)FadeEnemyToSmoke);
	AddFunc("void SetEnemyDisableTriggers(string &in asName, bool abX)",(void *)SetEnemyDisableTriggers);
	AddFunc("void ShowEnemyPlayerPosition(string &in asName)",(void *)ShowEnemyPlayerPosition);
	AddFunc("void ForceEnemyWaitState(string &in asName)",(void *)ForceEnemyWaitState);
	AddFunc("void AddEnemyPatrolNode(string &in asEnemyName, string &in asNodeName, float afWaitTime, string &in asAnimation, bool abLoopAnimation)",(void *)AddEnemyPatrolNode);
	AddFunc("void ClearEnemyPatrolNodes(string &in asEnemyName)",(void *)ClearEnemyPatrolNodes);
	AddFunc("void PlayEnemyAnimation(string &in asEnemyName, string &in asAnimName, bool abLoop, float afFadeTime)",(void *)PlayEnemyAnimation);
	AddFunc("void TeleportEnemyToNode(string &in asEnemyName, string &in asNodeName, bool abChangeY)",(void *)TeleportEnemyToNode);
	AddFunc("float GetEnemyPlayerDistance(string &in asEnemyName)",(void *)GetEnemyPlayerDistance);
	AddFunc("bool GetPlayerCanSeeEnemy(string &in asEnemyName)",(void *)GetPlayerCanSeeEnemy);

	AddFunc("void ChangeEnemyPose(string&in asName, string&in asPoseType)",(void *)ChangeEnemyPose);
	AddFunc("void SetEnemyMoveType(string &in asEnemyName, string &in asMoveType)",(void *)SetEnemyMoveType);
	AddFunc("void SetManPigType(string &in asManPigName, string &in asManPigType)",(void *)SetManPigType);
	AddFunc("void SendEnemyTimeOut(string &in asEnemyName, float afTimeOut)",(void *)SendEnemyTimeOut);
	AddFunc("void StopAnimationAndContinue(string &in asEnemyName, float afTimeOut)",(void *)StopAnimationAndContinue);
	AddFunc("void PlayScriptedAnimation(string &in asEnemyName, string &in asAnimationName, bool abLoopAnimation)",(void *)PlayScriptedAnimation);
	AddFunc("void SetEnemyBlind(string&in asName, bool abX)",(void *)SetEnemyBlind);
	AddFunc("void SetEnemyDeaf(string&in asName, bool abX)",(void *)SetEnemyDeaf);
	AddFunc("void SetTeslaPigFadeDisabled(string&in asName, bool abX)",(void *)SetTeslaPigFadeDisabled);
	AddFunc("void SetTeslaPigSoundDisabled(string&in asName, bool abX)",(void *)SetTeslaPigSoundDisabled);
	AddFunc("void SetTeslaPigEasyEscapeDisabled(string&in asName, bool abX)",(void *)SetTeslaPigEasyEscapeDisabled);
	AddFunc("void ForceTeslaPigSighting(string&in asName)",(void *)ForceTeslaPigSighting);
	AddFunc("int GetEnemyState(string &in asName)",(void *)GetEnemyState);
	AddFunc("string& GetEnemyStateName(string &in asName)",(void *)GetEnemyStateName);
	AddFunc("int StringToInt(string &in asString)",(void *)StringToInt);

	AddFunc("void SetPropHealth(string &in asName, float afHealth)",(void *)SetPropHealth);
	AddFunc("void AddPropHealth(string &in asName, float afHealth)",(void *)AddPropHealth);
	AddFunc("float GetPropHealth(string &in asName)",(void *)GetPropHealth);
	AddFunc("void ResetProp(string &in asName)",(void *)ResetProp);
	AddFunc("void PlayPropAnimation(string &in asProp, string &in asAnimation, float afFadeTime, bool abLoop, string &in asCallback)",(void *)PlayPropAnimation);
	AddFunc("void StopPropAnimation(string &in asProp)",(void *)StopPropAnimation);
	AddFunc("void PlayCurrentPropAnimation(string &in asProp, float afFadeTime, bool abLoop)",(void *)PlayCurrentAnimation);
	AddFunc("void PauseCurrentPropAnimation(string &in asProp, float afFadeTime)",(void *)PauseCurrentAnimation);
	AddFunc("void SetPropAnimationSpeed(string &in asProp, float afSpeed)",(void *)SetPropAnimationSpeed);
	AddFunc("void SetPropAnimationPosition(string &in asProp, float afPos)",(void *)SetPropAnimationPosition);

	AddFunc("void SetMoveObjectState(string &in asName, float afState)",(void *)SetMoveObjectState);
	AddFunc("void SetMoveObjectStateExt(string &in asName, float afState, float afAcc, float afMaxSpeed, float afSlowdownDist, bool abResetSpeed)",(void *)SetMoveObjectStateExt);

	AddFunc("void AddPropForce(string &in asName, float afX, float afY, float afZ, string &in asCoordSystem)",(void *)AddPropForce);
	AddFunc("void AddPropImpulse(string &in asName, float afX, float afY, float afZ, string &in asCoordSystem)",(void *)AddPropImpulse);
	AddFunc("void AddBodyForce(string &in asName, float afX, float afY, float afZ, string &in asCoordSystem)",(void *)AddBodyForce);
	AddFunc("void AddBodyImpulse(string &in asName, float afX, float afY, float afZ, string &in asCoordSystem)",(void *)AddBodyImpulse);
	AddFunc("void BreakJoint(string &in asName)", (void *)BreakJoint);


	AddFunc("void AddEntityCollideCallback(string &in asParentName, string &in asChildName, string &in asFunction, bool abDeleteOnCollide, int alStates)",(void *)AddEntityCollideCallback);
	AddFunc("void RemoveEntityCollideCallback(string &in asParentName, string &in asChildName)", (void *)RemoveEntityCollideCallback);

	//AddFunc("void CreateRope(string &in asName,string &in asStartArea, string &in asEndArea, string &in asStartBody, string &in asEndBody,float afMinTotalLength, float afMaxTotalLength,float afSegmentLength, float afDamping,float afStrength, float afStiffness, string &in asMaterial, float afRadius, float afLengthTileAmount, float afLengthTileSize, string &in asSound,float afSoundStartSpeed, float afSoundStopSpeed,bool abAutoMove, float afAutoMoveAcc, float afAutoMoveMaxSpeed)",(void *)CreateRope);

	AddFunc("void InteractConnectPropWithRope(string &in asName, string &in asLeverName, string &in asPropName, bool abInteractOnly, float afSpeedMul,float afMinSpeed, float afMaxSpeed, bool abInvert, int alStatesUsed)",(void *)InteractConnectPropWithRope);
	AddFunc("void InteractConnectPropWithMoveObject(string &in asName, string &in asPropName, string &in asMoveObjectName, bool abInteractOnly,bool abInvert, int alStatesUsed)",(void *)InteractConnectPropWithMoveObject);
	AddFunc("void ConnectEntities(string &in asName, string &in asMainEntity, string &in asConnectEntity, bool abInvertStateSent, int alStatesUsed, string &in asCallbackFunc)",(void *)ConnectEntities); 

}
//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// SCRIPT FUNCTION ENTITY PROPERTIES
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::Print(string& asString)
{
	Log("%s", asString.c_str());
}

void __stdcall cLuxScriptHandler::AddDebugMessage(string& asString, bool abCheckForDuplicates)
{
	gpBase->mpDebugHandler->AddMessage(cString::To16Char(asString),abCheckForDuplicates);
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::ProgLog(string& asLevel, string& asMessage)
{
	tString sLowType = cString::ToLowerCase(asLevel);
	eLuxProgressLogLevel level = eLuxProgressLogLevel_Low;
	if(sLowType == "medium") level = eLuxProgressLogLevel_Medium;
	if(sLowType == "high") level = eLuxProgressLogLevel_High;

    gpBase->mpProgressLogHandler->AddLog(level, asMessage);	
}

//-----------------------------------------------------------------------

bool __stdcall cLuxScriptHandler::ScriptDebugOn()
{
	return gpBase->mpDebugHandler->GetScriptDebugOn();
}

//-----------------------------------------------------------------------

float __stdcall cLuxScriptHandler::RandFloat(float afMin, float afMax)
{
	return cMath::RandRectf(afMin, afMax);
}

int __stdcall cLuxScriptHandler::RandInt(int alMin, int alMax)
{
	return cMath::RandRectl(alMin, alMax);
}

//-----------------------------------------------------------------------

bool __stdcall cLuxScriptHandler::StringContains(string& asString, string& asSubString)
{
	return cString::GetFirstStringPos(asString, asSubString)>=0;
}

//-----------------------------------------------------------------------

string gsGlobalTemp="";

string& __stdcall cLuxScriptHandler::StringSub(string& asString, int alStart, int alCount)
{
	gsGlobalTemp = cString::Sub(asString, alStart, alCount);
	return gsGlobalTemp;
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::AddTimer(string& asName, float afTime, string& asFunction)
{
	cLuxMap *pMap = gpBase->mpMapHandler->GetCurrentMap();
	if(pMap==NULL) return;

	pMap->AddTimer(asName,afTime, asFunction);
}
//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::RemoveTimer(string& asName)
{
	cLuxMap *pMap = gpBase->mpMapHandler->GetCurrentMap();
	if(pMap==NULL) return;

	pMap->RemoveTimer(asName);
}

//-----------------------------------------------------------------------

float __stdcall cLuxScriptHandler::GetTimerTimeLeft(string& asName)
{
	cLuxMap *pMap = gpBase->mpMapHandler->GetCurrentMap();
	if(pMap==NULL) return 0;

	cLuxEventTimer *pTimer = pMap->GetTimer(asName);
	if(pTimer)
		return pTimer->mfCount;
	else
		return 0;
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetLocalVarInt(string& asName, int alVal)
{
	SetLocalVarString(asName, cString::ToString(alVal));
}

void __stdcall cLuxScriptHandler::SetLocalVarFloat(string& asName, float afVal)
{
	SetLocalVarString(asName, cString::ToString(afVal));
}

void __stdcall cLuxScriptHandler::SetLocalVarString(string& asName, const string& asVal)
{
	cLuxMap *pMap = gpBase->mpMapHandler->GetCurrentMap();
	if(pMap==NULL) return;

	cLuxScriptVar* pVar = pMap->GetVar(asName);
	if(pVar==NULL)
	{
		Error("Couldn't find local var '%s'\n",asName.c_str());
		return;
	}
	pVar->msVal = asVal;
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::AddLocalVarInt(string& asName, int alVal)
{
	cLuxMap *pMap = gpBase->mpMapHandler->GetCurrentMap();
	if(pMap==NULL) return;

	cLuxScriptVar* pVar = pMap->GetVar(asName);
	if(pVar==NULL) {
		Error("Couldn't find local var '%s'\n",asName.c_str());
		return;
	}

	pVar->msVal = cString::ToString(cString::ToInt(pVar->msVal.c_str(),0)+alVal);
}

void __stdcall cLuxScriptHandler::AddLocalVarFloat(string& asName, float afVal)
{
	cLuxMap *pMap = gpBase->mpMapHandler->GetCurrentMap();
	if(pMap==NULL) return;

	cLuxScriptVar* pVar = pMap->GetVar(asName);
	if(pVar==NULL) {
		Error("Couldn't find local var '%s'\n",asName.c_str());
		return;
	}

	pVar->msVal = cString::ToString(cString::ToFloat(pVar->msVal.c_str(),0)+afVal);
}

void __stdcall cLuxScriptHandler::AddLocalVarString(string& asName, string& asVal)
{
	cLuxMap *pMap = gpBase->mpMapHandler->GetCurrentMap();
	if(pMap==NULL) return;

	cLuxScriptVar* pVar = pMap->GetVar(asName);
	if(pVar==NULL)
	{
		Error("Couldn't find local var '%s'\n",asName.c_str());
		return;
	}
	pVar->msVal += asVal;
}

//-----------------------------------------------------------------------

int __stdcall cLuxScriptHandler::GetLocalVarInt(string& asName)
{
	string sVal = GetLocalVarString(asName);
	if(sVal=="") return 0;

	return cString::ToInt(sVal.c_str(),0);
}

float __stdcall cLuxScriptHandler::GetLocalVarFloat(string& asName)
{
	string sVal = GetLocalVarString(asName);
	if(sVal=="") return 0;

	return cString::ToFloat(sVal.c_str(),0);
}

string gsGetLocalVarNullString = "";
string& __stdcall cLuxScriptHandler::GetLocalVarString(string& asName)
{
	cLuxMap *pMap = gpBase->mpMapHandler->GetCurrentMap();
	if(pMap==NULL) return gsGetLocalVarNullString;

	cLuxScriptVar* pVar = pMap->GetVar(asName);
	if(pVar==NULL)
	{
		Error("Couldn't find local var '%s'\n",asName.c_str());
		return gsScriptNull;
	}
	return pVar->msVal;
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetGlobalVarInt(string& asName, int alVal)
{
	SetGlobalVarString(asName, cString::ToString(alVal));
}

void __stdcall cLuxScriptHandler::SetGlobalVarFloat(string& asName, float afVal)
{
	string t =  cString::ToString(afVal);
	SetGlobalVarString(asName, t);
}

void __stdcall cLuxScriptHandler::SetGlobalVarString(string& asName, const string& asVal)
{
	cLuxScriptVar* pVar = gpBase->mpGlobalDataHandler->GetVar(asName);
	if(pVar==NULL)
	{
		Error("Couldn't find Global var '%s'\n",asName.c_str());
		return;
	}
	pVar->msVal = asVal;
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::AddGlobalVarInt(string& asName, int alVal)
{
	cLuxScriptVar* pVar = gpBase->mpGlobalDataHandler->GetVar(asName);
	if(pVar==NULL) {
		Error("Couldn't find Global var '%s'\n",asName.c_str());
		return;
	}

	pVar->msVal = cString::ToString(cString::ToInt(pVar->msVal.c_str(),0)+alVal);
}

void __stdcall cLuxScriptHandler::AddGlobalVarFloat(string& asName, float afVal)
{
	cLuxScriptVar* pVar = gpBase->mpGlobalDataHandler->GetVar(asName);
	if(pVar==NULL) {
		Error("Couldn't find Global var '%s'\n",asName.c_str());
		return;
	}

	pVar->msVal = cString::ToString(cString::ToFloat(pVar->msVal.c_str(),0)+afVal);
}

void __stdcall cLuxScriptHandler::AddGlobalVarString(string& asName, string& asVal)
{
	cLuxScriptVar* pVar = gpBase->mpGlobalDataHandler->GetVar(asName);
	if(pVar==NULL)
	{
		Error("Couldn't find global var '%s'\n",asName.c_str());
		return;
	}
	pVar->msVal += asVal;
}

//-----------------------------------------------------------------------

int __stdcall cLuxScriptHandler::GetGlobalVarInt(string& asName)
{
	string sVal = GetGlobalVarString(asName);
	if(sVal=="") return 0;

	return cString::ToInt(sVal.c_str(),0);
}

float __stdcall cLuxScriptHandler::GetGlobalVarFloat(string& asName)
{
	string sVal = GetGlobalVarString(asName);
	if(sVal=="") return 0;

	return cString::ToFloat(sVal.c_str(),0);
}

string& __stdcall cLuxScriptHandler::GetGlobalVarString(string& asName)
{
	cLuxScriptVar* pVar = gpBase->mpGlobalDataHandler->GetVar(asName);
	if(pVar==NULL)
	{
		Error("Couldn't find global var '%s'\n",asName.c_str());
		return gsScriptNull;
	}
	return pVar->msVal;
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::StartCredits(string& asMusic, bool abLoopMusic, string& asTextCat, string& asTextEntry, int alEndNum)
{
	gpBase->mpCredits->Setup(asMusic, abLoopMusic, asTextCat, asTextEntry, alEndNum);
	gpBase->mpEngine->GetUpdater()->SetContainer("Credits");
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::AddKeyPart(int alKeyPart)
{
	gpBase->mpCredits->AddKeyPart(alKeyPart);
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::StartDemoEnd()
{
	if(gpBase->mpDemoEnd)
		gpBase->mpEngine->GetUpdater()->SetContainer("DemoEnd");
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::AutoSave()
{
	gpBase->mpSaveHandler->AutoSave();
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::CheckPoint(string& asName,string& asStartPos ,string& asCallback, string &asDeathHintCat, string &asDeathHintEntry, bool abKeepPlayerInLimbo)
{
	gpBase->mpMapHandler->GetCurrentMap()->SetCheckPoint(asName, asStartPos, asCallback, abKeepPlayerInLimbo);
	gpBase->mpPlayer->GetHelperDeath()->SetHint(asDeathHintCat, asDeathHintEntry);
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::ReleasePlayerFromLimbo()
{
	gpBase->mpPlayer->ReleasePlayerFromLimbo();
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::ChangeMap(string& asMapName, string& asStartPos, string& asStartSound, string& asEndSound)
{
	gpBase->mpMapHandler->ChangeMap(asMapName, asStartPos, asStartSound, asEndSound);
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::ClearSavedMaps()
{
	gpBase->mpMapHandler->ClearSaveMapCollection();
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::CreateDataCache()
{
	gpBase->mpMapHandler->CreateDataCache();
}

void __stdcall cLuxScriptHandler::DestroyDataCache()
{
	gpBase->mpMapHandler->DestroyDataCache();
}
 
void __stdcall cLuxScriptHandler::UnlockAchievement(string& asName)
{
	if (asName == "TheHeart")
	{
		gpBase->mpAchievementHandler->UnlockAchievement(eLuxAchievement_TheHeart);
	}
}
//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetMapDisplayNameEntry(string& asNameEntry)
{
	cLuxMap *pMap = gpBase->mpMapHandler->GetCurrentMap();
	if(pMap)	pMap->SetDisplayNameEntry(asNameEntry);
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetSkyBoxActive(bool abActive)
{
	cWorld *pWorld = gpBase->mpMapHandler->GetCurrentMap()->GetWorld();

	pWorld->SetSkyBoxActive(abActive);
}

void __stdcall cLuxScriptHandler::SetSkyBoxTexture(string& asTexture)
{
	cWorld *pWorld = gpBase->mpMapHandler->GetCurrentMap()->GetWorld();

    iTexture *pTexture;
	if(asTexture != "")
		pTexture = gpBase->mpEngine->GetResources()->GetTextureManager()->CreateCubeMap(asTexture,true);
	else
		pTexture = NULL;
    
	pWorld->SetSkyBox(pTexture, true);
}

void __stdcall cLuxScriptHandler::SetSkyBoxColor(float afR, float afG, float afB, float afA)
{
	cWorld *pWorld = gpBase->mpMapHandler->GetCurrentMap()->GetWorld();

	pWorld->SetSkyBoxColor(cColor(afR, afG, afB, afA));
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetFogActive(bool abActive)
{
	cWorld *pWorld = gpBase->mpMapHandler->GetCurrentMap()->GetWorld();

	pWorld->SetFogActive(abActive);
}

void __stdcall cLuxScriptHandler::SetFogColor(float afR, float afG, float afB, float afA)
{
	cWorld *pWorld = gpBase->mpMapHandler->GetCurrentMap()->GetWorld();

	pWorld->SetFogColor(cColor(afR, afG, afB, afA));
}

void __stdcall cLuxScriptHandler::SetFogProperties(float afStart, float afEnd, float afFalloffExp, bool abCulling)
{
	cWorld *pWorld = gpBase->mpMapHandler->GetCurrentMap()->GetWorld();

	pWorld->SetFogStart(afStart);
	pWorld->SetFogEnd(afEnd);
	pWorld->SetFogFalloffExp(afFalloffExp);
	pWorld->SetFogCulling(abCulling);
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetupLoadScreen(string &asTextCat, string &asTextEntry, int alRandomNum, string &asImageFile)
{
	gpBase->mpLoadScreenHandler->SetupLoadText(asTextCat, asTextEntry, alRandomNum, asImageFile);
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::FadeIn(float afTime)
{
	gpBase->mpEffectHandler->GetFade()->FadeIn(afTime);
}

void __stdcall cLuxScriptHandler::FadeOut(float afTime)
{
	gpBase->mpEffectHandler->GetFade()->FadeOut(afTime);
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::FadeImageTrailTo(float afAmount, float afSpeed)
{
	gpBase->mpEffectHandler->GetImageTrail()->FadeTo(afAmount, afSpeed);
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::FadeSepiaColorTo(float afAmount, float afSpeed)
{
	gpBase->mpEffectHandler->GetSepiaColor()->FadeTo(afAmount, afSpeed);
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::FadeRadialBlurTo(float afSize, float afSpeed)
{
	gpBase->mpEffectHandler->GetRadialBlur()->FadeTo(afSize, afSpeed);
}

void __stdcall cLuxScriptHandler::SetRadialBlurStartDist(float afStartDist)
{
	gpBase->mpEffectHandler->GetRadialBlur()->SetBlurStartDist(afStartDist);
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::StartEffectFlash(float afFadeIn, float afWhite, float afFadeOut)
{
	gpBase->mpEffectHandler->GetFlash()->Start(afFadeOut, afWhite, afFadeOut);
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::StartEffectEmotionFlash(string &asTextCat, string &asTextEntry, string &asSound)
{
	gpBase->mpEffectHandler->GetEmotionFlash()->Start(asTextCat, asTextEntry, asSound);
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::ShowScreenImage(string & asImageName, float afX, float afY, float afScale, bool abUseRelativeCoordinates, float afDuration, float afFadeIn, float afFadeOut)
{
    gpBase->mpEffectHandler->GetScreenImage()->ShowImage(asImageName, afX, afY, afScale, abUseRelativeCoordinates, afDuration, afFadeIn, afFadeOut);
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::HideScreenImageImmediately()
{
    gpBase->mpEffectHandler->GetScreenImage()->HideImmediately();
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::HideScreenImageWithFade(float afFadeOut)
{
    gpBase->mpEffectHandler->GetScreenImage()->HideWithFade(afFadeOut);
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::AddEffectVoice(string& asVoiceFile, string& asEffectFile,
												string& asTextCat, string& asTextEntry, bool abUsePostion, 
												string& asPosEntity, float afMinDistance, float afMaxDistance)
{
	cVector3f vPos(0);
	if(abUsePostion)
	{
		iLuxEntity *pEntity = GetEntity(asPosEntity,eLuxEntityType_LastEnum,-1);
		if(pEntity && pEntity->GetBodyNum()>0)
		{
			vPos = pEntity->GetBody(0)->GetLocalPosition();
		}
	}
    
	gpBase->mpEffectHandler->GetPlayVoice()->AddVoice(asVoiceFile, asEffectFile, asTextCat, asTextEntry, abUsePostion, vPos, afMinDistance, afMaxDistance);
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::AddEffectVoice2(	string& asVoiceFile, string& asEffectFile,
											string& asTextCat,
											string& asTextEntry, float afTextDelay,
											string& asText2Entry, float afText2Delay,
											bool abUsePostion, string& asPosEntity, float afMinDistance, float afMaxDistance)
{
	cVector3f vPos(0);
	if(abUsePostion)
	{
		iLuxEntity *pEntity = GetEntity(asPosEntity,eLuxEntityType_LastEnum,-1);
		if(pEntity && pEntity->GetBodyNum()>0)
		{
			vPos = pEntity->GetBody(0)->GetLocalPosition();
		}
	}
    
	gpBase->mpEffectHandler->GetPlayVoice()->AddMultiSubbedVoice(
		asVoiceFile,
		asEffectFile,
		asTextCat,
		asTextEntry, afTextDelay,
		asText2Entry, afText2Delay,
		"", 0,
		"", 0,
		"", 0,
		"", 0,
		"", 0,
		abUsePostion,
		vPos,
		afMinDistance,
		afMaxDistance);
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::AddEffectVoice3(	string& asVoiceFile, string& asEffectFile,
										string& asTextCat,
										string& asTextEntry, float afTextDelay,
										string& asText2Entry, float afText2Delay,
										string& asText3Entry, float afText3Delay,
										bool abUsePostion, string& asPosEntity, float afMinDistance, float afMaxDistance)
{
	cVector3f vPos(0);
	if(abUsePostion)
	{
		iLuxEntity *pEntity = GetEntity(asPosEntity,eLuxEntityType_LastEnum,-1);
		if(pEntity && pEntity->GetBodyNum()>0)
		{
			vPos = pEntity->GetBody(0)->GetLocalPosition();
		}
	}
    
	gpBase->mpEffectHandler->GetPlayVoice()->AddMultiSubbedVoice(
		asVoiceFile,
		asEffectFile,
		asTextCat,
		asTextEntry, afTextDelay,
		asText2Entry, afText2Delay,
		asText3Entry, afText3Delay,
		"", 0,
		"", 0,
		"", 0,
		"", 0,
		abUsePostion,
		vPos,
		afMinDistance,
		afMaxDistance);
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::AddEffectVoice4(	string& asVoiceFile, string& asEffectFile,
										string& asTextCat,
										string& asTextEntry, float afTextDelay,
										string& asText2Entry, float afText2Delay,
										string& asText3Entry, float afText3Delay,
										string& asText4Entry, float afText4Delay,
										bool abUsePostion, string& asPosEntity, float afMinDistance, float afMaxDistance)
{
	cVector3f vPos(0);
	if(abUsePostion)
	{
		iLuxEntity *pEntity = GetEntity(asPosEntity,eLuxEntityType_LastEnum,-1);
		if(pEntity && pEntity->GetBodyNum()>0)
		{
			vPos = pEntity->GetBody(0)->GetLocalPosition();
		}
	}
    
	gpBase->mpEffectHandler->GetPlayVoice()->AddMultiSubbedVoice(
		asVoiceFile,
		asEffectFile,
		asTextCat,
		asTextEntry, afTextDelay,
		asText2Entry, afText2Delay,
		asText3Entry, afText3Delay,
		asText4Entry, afText4Delay,
		"", 0,
		"", 0,
		"", 0,
		abUsePostion,
		vPos,
		afMinDistance,
		afMaxDistance);
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::AddEffectVoice5(	string& asVoiceFile, string& asEffectFile,
										string& asTextCat,
										string& asTextEntry, float afTextDelay,
										string& asText2Entry, float afText2Delay,
										string& asText3Entry, float afText3Delay,
										string& asText4Entry, float afText4Delay,
										string& asText5Entry, float afText5Delay,
										bool abUsePostion, string& asPosEntity, float afMinDistance, float afMaxDistance)
{
	cVector3f vPos(0);
	if(abUsePostion)
	{
		iLuxEntity *pEntity = GetEntity(asPosEntity,eLuxEntityType_LastEnum,-1);
		if(pEntity && pEntity->GetBodyNum()>0)
		{
			vPos = pEntity->GetBody(0)->GetLocalPosition();
		}
	}
    
	gpBase->mpEffectHandler->GetPlayVoice()->AddMultiSubbedVoice(
		asVoiceFile,
		asEffectFile,
		asTextCat,
		asTextEntry, afTextDelay,
		asText2Entry, afText2Delay,
		asText3Entry, afText3Delay,
		asText4Entry, afText4Delay,
		asText5Entry, afText5Delay,
		"", 0,
		"", 0,
		abUsePostion,
		vPos,
		afMinDistance,
		afMaxDistance);
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::AddEffectVoice6(	string& asVoiceFile, string& asEffectFile,
										string& asTextCat,
										string& asTextEntry, float afTextDelay,
										string& asText2Entry, float afText2Delay,
										string& asText3Entry, float afText3Delay,
										string& asText4Entry, float afText4Delay,
										string& asText5Entry, float afText5Delay,
										string& asText6Entry, float afText6Delay,
										bool abUsePostion, string& asPosEntity, float afMinDistance, float afMaxDistance)
{
	cVector3f vPos(0);
	if(abUsePostion)
	{
		iLuxEntity *pEntity = GetEntity(asPosEntity,eLuxEntityType_LastEnum,-1);
		if(pEntity && pEntity->GetBodyNum()>0)
		{
			vPos = pEntity->GetBody(0)->GetLocalPosition();
		}
	}
    
	gpBase->mpEffectHandler->GetPlayVoice()->AddMultiSubbedVoice(
		asVoiceFile,
		asEffectFile,
		asTextCat,
		asTextEntry, afTextDelay,
		asText2Entry, afText2Delay,
		asText3Entry, afText3Delay,
		asText4Entry, afText4Delay,
		asText5Entry, afText5Delay,
		asText6Entry, afText6Delay,
		"", 0,
		abUsePostion,
		vPos,
		afMinDistance,
		afMaxDistance);
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::AddEffectVoice7(	string& asVoiceFile, string& asEffectFile,
										string& asTextCat,
										string& asTextEntry, float afTextDelay,
										string& asText2Entry, float afText2Delay,
										string& asText3Entry, float afText3Delay,
										string& asText4Entry, float afText4Delay,
										string& asText5Entry, float afText5Delay,
										string& asText6Entry, float afText6Delay,
										string& asText7Entry, float afText7Delay,
										bool abUsePostion, string& asPosEntity, float afMinDistance, float afMaxDistance)
{
	cVector3f vPos(0);
	if(abUsePostion)
	{
		iLuxEntity *pEntity = GetEntity(asPosEntity,eLuxEntityType_LastEnum,-1);
		if(pEntity && pEntity->GetBodyNum()>0)
		{
			vPos = pEntity->GetBody(0)->GetLocalPosition();
		}
	}
    
	gpBase->mpEffectHandler->GetPlayVoice()->AddMultiSubbedVoice(
		asVoiceFile,
		asEffectFile,
		asTextCat,
		asTextEntry, afTextDelay,
		asText2Entry, afText2Delay,
		asText3Entry, afText3Delay,
		asText4Entry, afText4Delay,
		asText5Entry, afText5Delay,
		asText6Entry, afText6Delay,
		asText7Entry, afText7Delay,
		abUsePostion,
		vPos,
		afMinDistance,
		afMaxDistance);
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::AddEffectVoiceExt(string& asVoiceFile, string& asEffectFile,
												string& asTextCat, string& asTextEntry, bool abUsePostion, 
												string& asPosEntity, float afMinDistance, float afMaxDistance, int alPriority, bool abStopInterrupt)
{
	cVector3f vPos(0);
	if(abUsePostion)
	{
		iLuxEntity *pEntity = GetEntity(asPosEntity,eLuxEntityType_LastEnum,-1);
		if(pEntity && pEntity->GetBodyNum()>0)
		{
			vPos = pEntity->GetBody(0)->GetLocalPosition();
		}
	}
    
	gpBase->mpEffectHandler->GetPlayVoice()->AddVoice(asVoiceFile, asEffectFile, asTextCat, asTextEntry, abUsePostion, vPos, afMinDistance, afMaxDistance, alPriority, abStopInterrupt);
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::StopAllEffectVoices(float afFadeOutTime)
{
	gpBase->mpEffectHandler->GetPlayVoice()->StopVoices(1.0f/afFadeOutTime);
}

//-----------------------------------------------------------------------

bool __stdcall cLuxScriptHandler::GetEffectVoiceActive()
{
	return gpBase->mpEffectHandler->GetPlayVoice()->IsActive();
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::StartPlayerSpawnPS(string& asSPSFile)
{
	gpBase->mpPlayer->GetHelperSpawnPS()->Start(asSPSFile);
}

void __stdcall cLuxScriptHandler::StopPlayerSpawnPS()
{
	gpBase->mpPlayer->GetHelperSpawnPS()->Stop();
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetEffectVoiceOverCallback(string& asFunc)
{
	gpBase->mpEffectHandler->GetPlayVoice()->SetOverCallback(asFunc);
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::StartScreenShake(float afAmount, float afTime, float afFadeInTime,float afFadeOutTime)
{
	gpBase->mpEffectHandler->GetScreenShake()->Start(afAmount, afTime, afFadeInTime, afFadeOutTime);
}

//-----------------------------------------------------------------------

bool __stdcall cLuxScriptHandler::GetFlashbackIsActive()
{
	return gpBase->mpPlayer->GetHelperFlashback()->IsActive();
}

//-----------------------------------------------------------------------

//void __stdcall cLuxScriptHandler::SetInsanitySetEnabled(string& asSet, bool abX)
//{
//	if(abX)	gpBase->mpInsanityHandler->EnableSet(asSet);
//	else	gpBase->mpInsanityHandler->DisableSet(asSet);
//
//}
//
//void __stdcall cLuxScriptHandler::StartRandomInsanityEvent()
//{
//	gpBase->mpInsanityHandler->StartEvent();
//}
//
//bool __stdcall cLuxScriptHandler::InsanityEventIsActive()
//{
//	return gpBase->mpInsanityHandler->GetCurrentEvent() >= 0;
//}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::PlayGuiSound(string& asSoundEntFile, float afVolume)
{
	tString sExt = cString::GetFileExt(asSoundEntFile);

	//Sound entity!
	if(sExt == "" || sExt == "snt")
	{
		gpBase->mpHelpFuncs->PlayGuiSoundData(asSoundEntFile, eSoundEntryType_Gui, afVolume);
	}
	//Normal sound
	else
	{
		cSoundHandler *pSoundHandler = gpBase->mpEngine->GetSound()->GetSoundHandler();
		pSoundHandler->PlayGui(asSoundEntFile,false, afVolume);
		
	}
	
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetPlayerActive(bool abActive)
{
	gpBase->mpPlayer->SetActive(abActive);
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::ChangePlayerStateToNormal()
{
	gpBase->mpPlayer->ChangeState(eLuxPlayerState_Normal);
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetPlayerCrouching(bool abCrouch)
{
	gpBase->mpPlayer->ChangeMoveState(eLuxMoveState_Normal);

	cLuxMoveState_Normal *pState = static_cast<cLuxMoveState_Normal*>(gpBase->mpPlayer->GetMoveStateData(eLuxMoveState_Normal));
	pState->SetCrouch(abCrouch);
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::AddPlayerBodyForce(float afX, float afY, float afZ, bool abUseLocalCoords)
{
	iCharacterBody *pBody = gpBase->mpPlayer->GetCharacterBody();

	cVector3f vForce;
	if(abUseLocalCoords)
	{
		vForce = pBody->GetForward()*afZ + pBody->GetRight()*afY + pBody->GetUp()*afY;
	}
	else
	{
		vForce = cVector3f(afX, afY, afZ);
	}

	pBody->AddForce(vForce);
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::ShowPlayerCrossHairIcons(bool abX)
{
	gpBase->mpPlayer->SetScriptShowFocusIconAndCrossHair(abX);
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetInfectionFauxMode(bool abFauxMode)
{
    if ( gpBase->mpPlayer->GetHelperInfection() )
    {
        gpBase->mpPlayer->GetHelperInfection()->SetFauxMode( abFauxMode );
    }
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetPlayerInfection(float afInfection)
{
	gpBase->mpPlayer->SetInfection(afInfection);
}

void __stdcall cLuxScriptHandler::AddPlayerInfection(float afInfection)
{
	gpBase->mpPlayer->AddInfection(afInfection);
}

float __stdcall cLuxScriptHandler::GetPlayerInfection()
{
	return gpBase->mpPlayer->GetInfection();
}

void __stdcall cLuxScriptHandler::SetPlayerHealth(float afHealth)
{
	gpBase->mpPlayer->SetHealth(afHealth);
}

void __stdcall cLuxScriptHandler::AddPlayerHealth(float afHealth)
{
	gpBase->mpPlayer->AddHealth(afHealth);
}

float __stdcall cLuxScriptHandler::GetPlayerHealth()
{
	return gpBase->mpPlayer->GetHealth();
}

//-----------------------------------------------------------------------

float __stdcall cLuxScriptHandler::GetPlayerSpeed()
{
	return gpBase->mpPlayer->GetCharacterBody()->GetVelocity(1.0f/60.0f).Length();
}

//-----------------------------------------------------------------------

float __stdcall cLuxScriptHandler::GetPlayerYSpeed()
{
	return gpBase->mpPlayer->GetCharacterBody()->GetVelocity(1.0f/60.0f).y;
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::MovePlayerForward(float afAmount)
{
	gpBase->mpPlayer->GetCharacterBody()->Move(eCharDir_Forward, afAmount);
}

void __stdcall cLuxScriptHandler::SetPlayerPermaDeathSound(string& asSound)
{
	gpBase->mpPlayer->SetCurrentPermaDeathSound(asSound);
}

//-----------------------------------------------------------------------

 void __stdcall cLuxScriptHandler::GiveInfectionDamage(float afAmount, bool abUseEffect)
{
	if(abUseEffect)
		gpBase->mpPlayer->GiveInfectionDamage(afAmount);
	else
		gpBase->mpPlayer->IncreaseInfection(afAmount, false);
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::GivePlayerDamage(float afAmount, string& asType, bool abSpinHead, bool abLethal)
{
	tString sLowType = cString::ToLowerCase(asType);
	eLuxDamageType type = eLuxDamageType_BloodSplat;
	if(sLowType == "claws") type = eLuxDamageType_Claws;
	if(sLowType == "slash") type = eLuxDamageType_Slash;
	
	gpBase->mpPlayer->GiveDamage(afAmount, 1, type, abSpinHead,abLethal);
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::FadePlayerFOVMulTo(float afX, float afSpeed)
{
	gpBase->mpPlayer->FadeFOVMulTo(afX, afSpeed);
}

void __stdcall cLuxScriptHandler::FadePlayerAspectMulTo(float afX, float afSpeed)
{
	gpBase->mpPlayer->FadeAspectMulTo(afX, afSpeed);
}

void __stdcall cLuxScriptHandler::FadePlayerRollTo(float afX, float afSpeedMul, float afMaxSpeed)
{
	gpBase->mpPlayer->FadeRollTo(cMath::ToRad(afX), afSpeedMul, cMath::ToRad(afMaxSpeed));
}

void __stdcall cLuxScriptHandler::FadePlayerPitchTo(float afX, float afSpeedMul, float afMaxSpeed)
{
	gpBase->mpPlayer->FadePitchTo(cMath::ToRad(afX), afSpeedMul, cMath::ToRad(afMaxSpeed));
}

void __stdcall cLuxScriptHandler::MovePlayerHeadPos(float afX, float afY, float afZ, float afSpeed, float afSlowDownDist)
{
	gpBase->mpPlayer->MoveHeadPosAdd(eLuxHeadPosAdd_Script, cVector3f(afX, afY, afZ), afSpeed, afSlowDownDist);
}


//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::StartPlayerLookAt(string& asEntityName, float afSpeedMul, float afMaxSpeed, string & asAtTargetCallback)
{
	iLuxEntity *pEntity = GetEntity(asEntityName, eLuxEntityType_LastEnum, -1);
	if(pEntity==NULL) return;

	cVector3f vPos =0;
	if(pEntity->GetBodyNum()>0)
		vPos = pEntity->GetBody(0)->GetLocalPosition();
	else if(pEntity->GetMeshEntity())
		vPos = pEntity->GetMeshEntity()->GetBoundingVolume()->GetWorldCenter();
	else
	{
		Error("Entity %s does not have body or mesh entity! Cannot make player look at it!", asEntityName.c_str());
		return;
	}

	gpBase->mpPlayer->GetHelperLookAt()->SetTarget(vPos, afSpeedMul, afMaxSpeed, asAtTargetCallback);
	gpBase->mpPlayer->GetHelperLookAt()->SetActive(true);
}

void __stdcall cLuxScriptHandler::StopPlayerLookAt()
{
	gpBase->mpPlayer->GetHelperLookAt()->SetActive(false);
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetPlayerMoveSpeedMul(float afMul)
{
	gpBase->mpPlayer->SetScriptMoveSpeedMul(afMul);
}

void __stdcall cLuxScriptHandler::SetPlayerRunSpeedMul(float afMul)
{
	gpBase->mpPlayer->SetScriptRunSpeedMul(afMul);
}

void __stdcall cLuxScriptHandler::SetPlayerLookSpeedMul(float afMul)
{
	gpBase->mpPlayer->SetLookSpeedMul(afMul);
}

void __stdcall cLuxScriptHandler::SetPlayerJumpDisabled(bool abX)
{
	gpBase->mpPlayer->SetJumpDisabled(abX);
}

void __stdcall cLuxScriptHandler::SetPlayerCrouchDisabled(bool abX)
{
	gpBase->mpPlayer->SetCrouchDisabled(abX);
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::TeleportPlayer(string &asStartPosName)
{
	cLuxNode_PlayerStart *pNode = gpBase->mpMapHandler->GetCurrentMap()->GetPlayerStart(asStartPosName);
	if(pNode==NULL)
	{
		Error("Start pos '%s' could not be found!\n", asStartPosName.c_str());
		return;
	}

    gpBase->mpPlayer->PlaceAtStartNode(pNode);
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetLanternActive(bool abX, bool abUseEffects)
{
	gpBase->mpPlayer->GetHelperLantern()->SetActive(abX, abUseEffects);
}

//-----------------------------------------------------------------------

bool __stdcall cLuxScriptHandler::GetLanternActive()
{
	return gpBase->mpPlayer->GetHelperLantern()->IsActive();
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetLanternDisabled(bool abX)
{
	gpBase->mpPlayer->GetHelperLantern()->SetDisabled(abX);
}

//-----------------------------------------------------------------------

bool __stdcall cLuxScriptHandler::GetJournalDisabled()
{
	return gpBase->mpJournal->GetDisabled();
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetJournalDisabled(bool abX)
{
	gpBase->mpJournal->SetDisabled(abX);
}
//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetLanternLitCallback(string &asCallback)
{
	cLuxMap *pMap = gpBase->mpMapHandler->GetCurrentMap();

	pMap->SetLanternLitCallback(asCallback);
}

//-----------------------------------------------------------------------
void __stdcall cLuxScriptHandler::SetLanternFlickerActive(bool abX)
{
    iLuxHandObject * object = gpBase->mpPlayer->GetHands()->GetHandObject("lantern");

    if ( object )
    {
        cLuxHandObject_LightSource * lantern = (cLuxHandObject_LightSource *)object;

        lantern->SetFlickering( abX );
    }
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetMessage(string &asTextCategory, string &asTextEntry, float afTime)
{
	gpBase->mpMessageHandler->SetMessage(kTranslate(asTextCategory, asTextEntry), afTime);
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetDeathHint(string &asTextCategory, string &asTextEntry)
{
	gpBase->mpPlayer->GetHelperDeath()->SetHint(asTextCategory, asTextEntry);
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::DisableDeathStartSound()
{
	gpBase->mpPlayer->GetHelperDeath()->DisableStartSound();
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::AddNote(string& asNameAndTextEntry, string& asImage)
{
	gpBase->mpJournal->AddNote(asNameAndTextEntry, asImage);
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::AddDiary(string& asNameAndTextEntry, string& asImage)
{
	int lReturnNum=0;
	gpBase->mpJournal->AddDiary(asNameAndTextEntry, asImage,lReturnNum);
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::AddHint(string& asNameAndTextEntry, string& asImage)
{
	if(gpBase->mpJournal->AddHint(asNameAndTextEntry, asImage))
	{
		gpBase->mpMessageHandler->StarQuestAddedMessage();
		gpBase->mpHintHandler->Add("QuestAdded", kTranslate("Hints", "QuestAdded"), 0);
		
	}
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::RemoveAllHints()
{
	gpBase->mpJournal->RemoveAllHints();
}

void __stdcall cLuxScriptHandler::RemoveHint(string& asNameAndTextEntry)
{
	gpBase->mpJournal->RemoveHint(asNameAndTextEntry);
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::ReturnOpenJournal(bool abOpenJournal)
{
	cLuxItemType_Diary::mbShowJournalOnPickup = abOpenJournal;
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::AddQuest(string& asName, string& asNameAndTextEntry)
{
	tString sTextEntry = "Quest_"+asNameAndTextEntry+"_Text";

	if(gpBase->mpJournal->AddQuestNote(asName, asNameAndTextEntry))
	{
		gpBase->mpProgressLogHandler->AddLog(eLuxProgressLogLevel_High, "Added Memento "+ asName);
		
		gpBase->mpHintHandler->Add("QuestAdded", kTranslate("Hints", "QuestAdded"), 0);
		
		gpBase->mpMessageHandler->StarQuestAddedMessage();
		
		//tWString sMess = kTranslate("Quest", "QuestAdded") + _W("\n") + kTranslate("Journal", sTextEntry);
		//gpBase->mpMessageHandler->SetMessage(sMess, 0);	
		//gpBase->mpHelpFuncs->PlayGuiSoundData("quest_added", eSoundEntryType_Gui);
	}
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::CompleteQuest(string& asName, string& asNameAndTextEntry)
{
	tString sTextEntry = "Quest_"+asNameAndTextEntry+"_Text";

	gpBase->mpJournal->AddQuestNote(asName, asNameAndTextEntry);
	if(gpBase->mpJournal->DisableQuestNote(asName))
	{
		gpBase->mpProgressLogHandler->AddLog(eLuxProgressLogLevel_High, "Completed Quest "+ asName);

		//tWString sMess = kTranslate("Quest", "QuestCompleted") + _W("\n") + kTranslate("Journal", sTextEntry);
		//gpBase->mpMessageHandler->SetMessage(sMess, 0);
		//gpBase->mpHelpFuncs->PlayGuiSoundData("quest_completed", eSoundEntryType_Gui);
		
		cLuxMap *pMap = gpBase->mpMapHandler->GetCurrentMap();
		if(pMap) pMap->AddCompletionAmount(gpBase->mpCompletionCountHandler->mlQuestCompletionValue, 6.0f);
	}
}

//-----------------------------------------------------------------------

bool __stdcall cLuxScriptHandler::QuestIsCompleted(string& asName)
{
	cLuxQuestNote *pQuest = gpBase->mpJournal->GetQuestNote(asName);
	if(pQuest==NULL) return false;

	return pQuest->mbActive==false;
}

bool __stdcall cLuxScriptHandler::QuestIsAdded(string& asName)
{
	cLuxQuestNote *pQuest = gpBase->mpJournal->GetQuestNote(asName);
	if(pQuest==NULL) return false;
	return true;
}

void __stdcall cLuxScriptHandler::SetNumberOfQuestsInMap(int alNumberOfQuests)
{
	cLuxMap *pMap = gpBase->mpMapHandler->GetCurrentMap();
	if(pMap==NULL) return;

	pMap->SetNumberOfQuests(alNumberOfQuests);
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::GiveHint(string& asName, string& asMessageCat, string& asMessageEntry, float afTimeShown)
{
	gpBase->mpHintHandler->Add(asName, kTranslate(asMessageCat, asMessageEntry), afTimeShown);
}

void __stdcall cLuxScriptHandler::BlockHint(string& asName)
{
	gpBase->mpHintHandler->Block(asName);
}

void __stdcall cLuxScriptHandler::UnBlockHint(string& asName)
{
	gpBase->mpHintHandler->UnBlock(asName);
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::ExitInventory()
{
	if(gpBase->mpInputHandler->GetState() != eLuxInputState_Inventory) return;

	gpBase->mpInventory->ExitPressed();
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetInventoryDisabled(bool abX)
{
	gpBase->mpInventory->SetDisabled(abX);
}

void __stdcall cLuxScriptHandler::SetInventoryMessage(string &asTextCategory, string &asTextEntry, float afTime)
{
	gpBase->mpInventory->SetMessageText(kTranslate(asTextCategory, asTextEntry), afTime);
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::GiveItem(string& asName, string& asType, string& asSubTypeName, string& asImageName, float afAmount)
{
	eLuxItemType type = gpBase->mpInventory->GetItemTypeFromString(asType);
	gpBase->mpInventory->AddItem(asName,type,asSubTypeName,asImageName, afAmount, "", "");
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::GiveItemFromFile(string& asName, string& asFileName)
{
	cLuxMap *pMap = gpBase->mpMapHandler->GetCurrentMap();
	if(pMap==NULL) return;

	pMap->ResetLatestEntity();
	pMap->CreateEntity(asName,asFileName, cMatrixf::Identity,1);
	iLuxEntity *pEntity = pMap->GetLatestEntity();
	if(pEntity)
	{
		if(pEntity->GetEntityType() == eLuxEntityType_Prop)
		{
			iLuxProp *pProp = static_cast<iLuxProp*>(pEntity);
			if(pProp->GetPropType() == eLuxPropType_Item)
			{
				cLuxProp_Item *pItem = static_cast<cLuxProp_Item*>(pProp);
				gpBase->mpInventory->AddItem(	asName, pItem->GetItemType(), pItem->GetSubItemTypeName(), pItem->GetImageFile(), pItem->GetAmount(), "", "");
			}
		}
		
		pMap->DestroyEntity(pEntity);
	}
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::RemoveItem(string& asName)
{
	gpBase->mpInventory->RemoveItem(asName);
}

bool __stdcall cLuxScriptHandler::HasItem(string& asName)
{
	return gpBase->mpInventory->GetItem(asName)!=NULL;
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::AddCombineCallback(string& asName, string& asItemA, string& asItemB, string& asFunction, bool abAutoDestroy)
{
	gpBase->mpInventory->AddCombineCallback(asName, asItemA, asItemB, asFunction, abAutoDestroy);
}

void __stdcall cLuxScriptHandler::RemoveCombineCallback(string& asName)
{
	gpBase->mpInventory->RemoveCombineCallback(asName);
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::AddUseItemCallback(string& asName, string& asItem, string& asEntity, string& asFunction, bool abAutoDestroy)
{
	cLuxMap *pMap = gpBase->mpMapHandler->GetCurrentMap();
	if(pMap==NULL) return;

    pMap->AddUseItemCallback(asName, asItem, asEntity, asFunction, abAutoDestroy);
}

void __stdcall cLuxScriptHandler::RemoveUseItemCallback(string& asName)
{
	cLuxMap *pMap = gpBase->mpMapHandler->GetCurrentMap();
	if(pMap==NULL) return;

	pMap->RemoveUseItemCallback(asName);
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::PreloadParticleSystem(string& asPSFile)
{
	cResources *pResources = gpBase->mpEngine->GetResources();
	pResources->GetParticleManager()->Preload(asPSFile);	
}

void __stdcall cLuxScriptHandler::PreloadSound(string& asSoundFile)
{
	cResources *pResources = gpBase->mpEngine->GetResources();
	pResources->GetSoundEntityManager()->Preload(asSoundFile);
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::CreateParticleSystemAtEntity(string& asPSName, string& asPSFile, string& asEntity, bool abSavePS)
{
	CreateParticleSystemAtEntityExt(asPSName, asPSFile, asEntity, abSavePS, 1,1,1,1, false, 1,2,100,110);
}

void __stdcall cLuxScriptHandler::CreateParticleSystemAtEntityExt(	string& asPSName, string& asPSFile, string& asEntity, bool abSavePS, 
																	float afR, float afG, float afB, float afA,
																	bool abFadeAtDistance, float afFadeMinEnd, float afFadeMinStart, 
																	float afFadeMaxStart, float afFadeMaxEnd)
{
	cLuxMap *pMap = gpBase->mpMapHandler->GetCurrentMap();
	if(pMap==NULL) return;

	cParticleSystem *pPS=NULL;

	//////////////////////////
	// Player
	if(asEntity == "Player")
	{
		pPS = pMap->GetWorld()->CreateParticleSystem(asPSName,asPSFile,1.0f);
		if(pPS)
		{
			pPS->SetPosition(gpBase->mpPlayer->GetCharacterBody()->GetPosition());
			pPS->SetIsSaved(abSavePS);
			//gpBase->mpPlayer->GetCharacterBody()->
		}
	}
	//////////////////////////
	// Normal entity
	else
	{
		iLuxEntity* pEntity = GetEntity(asEntity, eLuxEntityType_LastEnum, -1);
		if(pEntity==NULL) return;	

		pPS = pMap->GetWorld()->CreateParticleSystem(asPSName,asPSFile,1.0f);
		if(pPS)
		{
			if(abSavePS==false) pEntity->GetAttachEntity()->AddChild(pPS);
			else				pPS->SetMatrix(pEntity->GetAttachEntity()->GetWorldMatrix());
			pPS->SetIsSaved(abSavePS);
		}
	}

	//////////////////////////
	// Settings
	if(pPS)
	{
		pPS->SetColor(cColor(afR, afG, afB, afA));
		pPS->SetFadeAtDistance(abFadeAtDistance);
		pPS->SetMinFadeDistanceEnd(afFadeMinEnd);
		pPS->SetMinFadeDistanceStart(afFadeMinStart);
		pPS->SetMaxFadeDistanceStart(afFadeMaxStart);
		pPS->SetMaxFadeDistanceEnd(afFadeMaxEnd);
	}
}

void __stdcall cLuxScriptHandler::DestroyParticleSystem(string& asName)
{
	cLuxMap *pMap = gpBase->mpMapHandler->GetCurrentMap();
	if(pMap==NULL) return;

	bool bFound = false;

	cParticleSystemIterator it = pMap->GetWorld()->GetParticleSystemIterator();
	while(it.HasNext())
	{
		cParticleSystem *pPS = it.Next();
		if(pPS->GetName() == asName)
		{
			pPS->Kill();

			bFound = true;
		}
	}
	if(bFound==false) Error("Could not find particle system '%s'\n", asName.c_str());
}

void __stdcall cLuxScriptHandler::DestroyParticleSystemInstantly(string& asName)
{
	cLuxMap *pMap = gpBase->mpMapHandler->GetCurrentMap();
	if(pMap==NULL) return;

	bool bFound = false;

	cParticleSystemIterator it = pMap->GetWorld()->GetParticleSystemIterator();
	while(it.HasNext())
	{
		cParticleSystem *pPS = it.Next();
		if(pPS->GetName() == asName)
		{
			pPS->KillInstantly();

			bFound = true;
		}
	}
	if(bFound==false) Error("Could not find particle system '%s'\n", asName.c_str());
}

void __stdcall cLuxScriptHandler::SetParticleSystemActive(string& asName, bool bActive)
{
	BEGIN_ITERATE_PARTICLESYSTEM()
			pParticleSystem->SetActive(bActive);
	END_ITERATE_PARTICLESYSTEM
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::PlaySoundAtPosition(string& asSoundName, string& asSoundFile, float afX, float afY, float afZ, float afFadeTime, bool abSaveSound)
{
	float fFadeSpeed = afFadeTime ==0 ? 0 : 1.0f/afFadeTime;
	bool bRemoveWhenOver = true;

	cLuxMap *pMap = gpBase->mpMapHandler->GetCurrentMap();
	if(pMap==NULL) return;
	cSoundEntity *pSound= pMap->GetWorld()->CreateSoundEntity(asSoundName, asSoundFile,bRemoveWhenOver);
	
	if(pSound)
	{
		cVector3f position = cVector3f(afX, afY, afZ);
		pSound->SetPosition(position);
		pSound->SetIsSaved(abSaveSound);
		if(afFadeTime >0) pSound->FadeIn(fFadeSpeed);
	}
}

void __stdcall cLuxScriptHandler::PlaySoundAtEntity(string& asSoundName, string& asSoundFile, string& asEntity, float afFadeTime, bool abSaveSound)
{
	float fFadeSpeed = afFadeTime ==0 ? 0 : 1.0f/afFadeTime;

	bool bRemoveWhenOver = true;//abSaveSound ? false : true;

	//////////////////////////
	// Player
	if(asEntity == "Player")
	{
		cLuxMap *pMap = gpBase->mpMapHandler->GetCurrentMap();
		if(pMap==NULL) return;

		cSoundEntity *pSound= pMap->GetWorld()->CreateSoundEntity(asSoundName, asSoundFile,bRemoveWhenOver);
		if(pSound)
		{
			pSound->SetForcePlayAsGUISound(true);
			
			pSound->SetIsSaved(abSaveSound);
			if(afFadeTime >0) pSound->FadeIn(fFadeSpeed);
		}
	}
	//////////////////////////
	// Normal entity
	else
	{
		iLuxEntity* pEntity = GetEntity(asEntity, eLuxEntityType_LastEnum, -1);
		if(pEntity==NULL) return;

		cLuxMap *pMap = gpBase->mpMapHandler->GetCurrentMap();
		if(pMap==NULL) return;

		cSoundEntity *pSound= pMap->GetWorld()->CreateSoundEntity(asSoundName, asSoundFile,bRemoveWhenOver);
		if(pSound)
		{
			if(abSaveSound==false)
			{
				pEntity->GetAttachEntity()->AddChild(pSound);
			}
			else
			{
				pSound->SetPosition(pEntity->GetAttachEntity()->GetWorldPosition());
			}
			
			pSound->SetIsSaved(abSaveSound);
			if(afFadeTime >0) pSound->FadeIn(fFadeSpeed);
		}
	}
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::FadeInSound(string& asSoundName, float afFadeTime, bool abPlayStart)
{
	cLuxMap *pMap = gpBase->mpMapHandler->GetCurrentMap();
	if(pMap==NULL) return;

	float fFadeSpeed = afFadeTime ==0 ? 0 : 1.0f/afFadeTime;
	
	cSoundEntity *pSound = pMap->GetWorld()->GetSoundEntity(asSoundName);
	if(pSound)
	{
		if(fFadeSpeed <=0)	pSound->Play(abPlayStart);
		else				pSound->FadeIn(fFadeSpeed);
	}
	else
	{
		Error("Could not find sound entity '%s'\n", asSoundName.c_str());
	}
}

//-----------------------------------------------------------------------


void __stdcall cLuxScriptHandler::StopSound(string& asSoundName, float afFadeTime)
{
	cLuxMap *pMap = gpBase->mpMapHandler->GetCurrentMap();
	if(pMap==NULL) return;

	float fFadeSpeed = afFadeTime ==0 ? 0 : 1.0f/afFadeTime;
	bool bFound = false;

	cSoundEntityIterator it = pMap->GetWorld()->GetSoundEntityIterator();
    while(it.HasNext())
	{
		cSoundEntity *pSound = it.Next();
        if(pSound->GetName() == asSoundName)
		{
			if(fFadeSpeed <=0)	pSound->Stop(true);
			else				pSound->FadeOut(fFadeSpeed);

			bFound = true;
		}
	}
	if(bFound==false) Error("Could not find sound entity '%s'\n", asSoundName.c_str());
}


//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetLightVisible(string& asLightName, bool abVisible)
{
	cLuxMap *pMap = gpBase->mpMapHandler->GetCurrentMap();
	if(pMap==NULL) return;

	iLight *pLight = pMap->GetWorld()->GetLight(asLightName);
	if(pLight==NULL)
	{
		Error("Could not find light '%s'\n", asLightName.c_str());
		return;
	}

	pLight->SetVisible(abVisible);
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::FadeLightTo(string& asLightName, float afR, float afG, float afB, float afA, float afRadius, float afTime)
{
	cLuxMap *pMap = gpBase->mpMapHandler->GetCurrentMap();
	if(pMap==NULL) return;

	iLight *pLight = pMap->GetWorld()->GetLight(asLightName);
	if(pLight==NULL)
	{
		Error("Could not find light '%s'\n", asLightName.c_str());
		return;
	}

	pLight->SetFlickerActive(false);
	
    cColor newColor(
		afR >=0 ? afR : pLight->GetDiffuseColor().r,	
		afG >=0 ? afG : pLight->GetDiffuseColor().g,
		afB >=0 ? afB : pLight->GetDiffuseColor().b,
		afA >=0 ? afA : pLight->GetDiffuseColor().a);
	
	float fNewRadius = afRadius >=0 ? afRadius : pLight->GetRadius();

	pLight->SetVisible(true);
    pLight->FadeTo(newColor, fNewRadius, afTime);
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetLightFlickerActive(string& asLightName, bool abActive)
{
	cLuxMap *pMap = gpBase->mpMapHandler->GetCurrentMap();
	if(pMap==NULL) return;

	iLight *pLight = pMap->GetWorld()->GetLight(asLightName);
	if(pLight==NULL)
	{
		Error("Could not find light '%s'\n", asLightName.c_str());
		return;
	}

	pLight->SetFlickerActive(abActive);
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetLampFlickerActive(string& asName, bool abActive)
{
	cLuxMap *pMap = gpBase->mpMapHandler->GetCurrentMap();
	if(pMap==NULL) return;

	BEGIN_SET_PROPERTY(eLuxEntityType_Prop,eLuxPropType_Lamp)

		cLuxProp_Lamp *pLamp = ToLamp(pEntity);
		pLamp->SetFlickerActive(abActive);

	END_SET_PROPERTY
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetPhysicsAutoDisable(string& asName, bool abAutoDisable)
{
	cLuxMap *pMap = gpBase->mpMapHandler->GetCurrentMap();
	if(pMap==NULL) return;

	BEGIN_SET_PROPERTY(eLuxEntityType_Prop,-1)

		iLuxProp *pProp = ToProp(pEntity);
		
		pProp->GetMainBody()->SetAutoDisable( abAutoDisable );

	END_SET_PROPERTY
}

//-----------------------------------------------------------------------


void __stdcall cLuxScriptHandler::PlayMusic(string& asMusicFile, bool abLoop, float afVolume, float afFadeTime, int alPrio, bool abResume)
{
	gpBase->mpMusicHandler->Play(asMusicFile, abLoop, afVolume, afFadeTime,alPrio,abResume, false);
}

void __stdcall cLuxScriptHandler::StopMusic(float afFadeTime, int alPrio)
{
	gpBase->mpMusicHandler->Stop(afFadeTime, alPrio);
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::FadeGlobalSoundVolume(float afDestVolume, float afTime)
{
	bool abDestroy = afDestVolume==1;
	cSoundHandler *pSoundHandler = gpBase->mpEngine->GetSound()->GetSoundHandler();

	cMultipleSettingsHandler* pHandler = pSoundHandler->GetGlobalVolumeSettingsHandler();
	cMultipleSettingsHandler::cGSEntry* pEntry = pHandler->GetEntry(eLuxGlobalVolumeType_Script, true);

	float fSpeed = 1000.0f;
	if(afTime > 0) fSpeed = fabs(afDestVolume-pEntry->GetVal()) / afTime;

	pSoundHandler->FadeGlobalVolume(afDestVolume, fSpeed, eSoundEntryType_World, eLuxGlobalVolumeType_Script, abDestroy);
}

void __stdcall cLuxScriptHandler::FadeGlobalSoundSpeed(float afDestSpeed, float afTime)
{
	bool abDestroy = afDestSpeed==1;
	cSoundHandler *pSoundHandler = gpBase->mpEngine->GetSound()->GetSoundHandler();

	cMultipleSettingsHandler* pHandler = pSoundHandler->GetGlobalSpeedSettingsHandler();
	cMultipleSettingsHandler::cGSEntry* pEntry = pHandler->GetEntry(eLuxGlobalVolumeType_Script, true);

	float fSpeed = 1000.0f;
	if(afTime > 0) fSpeed = fabs(afDestSpeed-pEntry->GetVal()) / afTime;
	pSoundHandler->FadeGlobalSpeed(afDestSpeed, fSpeed, eSoundEntryType_World, eLuxGlobalVolumeType_Script, abDestroy);
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetEntityActive(string& asName, bool abActive)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_LastEnum,-1)

		pEntity->SetActive(abActive);

	END_SET_PROPERTY
}

//-----------------------------------------------------------------------

bool __stdcall cLuxScriptHandler::GetEntityExists(string& asName)
{
	return gpBase->mpMapHandler->GetCurrentMap()->GetEntityByName(asName)!=NULL;
}

//-----------------------------------------------------------------------

bool __stdcall cLuxScriptHandler::GetEntityActive(string& asName)
{
    iLuxEntity * entity = gpBase->mpMapHandler->GetCurrentMap()->GetEntityByName(asName);

    if ( entity == NULL ) return false;
    return entity->IsActive();
}

//-----------------------------------------------------------------------

static eLuxFocusCrosshair StringToCrossHair(const tString &asCrossHair)
{
	tString sLowCross = cString::ToLowerCase(asCrossHair);
	
	if(sLowCross=="default")	return eLuxFocusCrosshair_Default;
	if(sLowCross=="grab")		return eLuxFocusCrosshair_Grab;
	if(sLowCross=="push")		return eLuxFocusCrosshair_Push;
	if(sLowCross=="ignite")		return eLuxFocusCrosshair_Ignite;
	if(sLowCross=="pick")		return eLuxFocusCrosshair_Pick;
	if(sLowCross=="leveldoor")	return eLuxFocusCrosshair_LevelDoor;
	if(sLowCross=="ladder")		return eLuxFocusCrosshair_Ladder;
	if(sLowCross=="voiceover")		return eLuxFocusCrosshair_VoiceOver;
	if(sLowCross=="phonebox")		return eLuxFocusCrosshair_PhoneBox;
	if(sLowCross=="note")		return eLuxFocusCrosshair_Note;

    Error("CrossHair type '%s' does not exist!\n", asCrossHair.c_str());
	return eLuxFocusCrosshair_Default;
}

void __stdcall cLuxScriptHandler::SetEntityCustomFocusCrossHair(string& asName, string &asCrossHair)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_LastEnum,-1)

		pEntity->SetCustomFocusCrossHair(StringToCrossHair(asCrossHair));

	END_SET_PROPERTY
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::CreateEntityAtArea(string& asEntityName, string& asEntityFile, string& asAreaName, bool abFullGameSave, float afPosX, float afPosY, float afPosZ, float afRotX, float afRotY, float afRotZ)
{
	cLuxMap *pMap = gpBase->mpMapHandler->GetCurrentMap();

	iLuxArea *pArea = ToArea(GetEntity(asAreaName,eLuxEntityType_Area, -1));
	if(pArea == NULL) return;

	cMatrixf mtxTransform = cMath::MatrixRotate(cMath::Vector3ToRad(cVector3f(afRotX, afRotY, afRotZ)), eEulerRotationOrder_XYZ);
	mtxTransform.SetTranslation(cVector3f(afPosX, afPosY, afPosZ));

	pMap->ResetLatestEntity();
	pMap->CreateEntity(asEntityName, asEntityFile, cMath::MatrixMul( pArea->GetBody()->GetWorldMatrix(), mtxTransform ),1);

	iLuxEntity *pEntity = pMap->GetLatestEntity();
	if(pEntity && pEntity->GetName() == asEntityName)
	{
		pEntity->SetFullGameSave(abFullGameSave);	
	}
	else
	{
		Error("Could not create entity '%s' from file '%s'!\n", asEntityName.c_str(), asEntityFile.c_str());
	}
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetEntityPlayerLookAtCallback(string& asName, string& asCallback, bool abRemoveWhenLookedAt)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_LastEnum,-1)

		pEntity->SetPlayerLookAtCallback(asCallback, abRemoveWhenLookedAt);

	END_SET_PROPERTY
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetEntityPlayerInteractCallback(string& asName, string& asCallback, bool abRemoveOnInteraction)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_LastEnum,-1)

		pEntity->SetPlayerInteractCallback(asCallback, abRemoveOnInteraction);

	END_SET_PROPERTY
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetEntityCallbackFunc(string& asName, string& asCallback)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_LastEnum,-1)

		pEntity->SetCallbackFunc(asCallback);

	END_SET_PROPERTY
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetEntityConnectionStateChangeCallback(string& asName, string& asCallback)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_LastEnum,-1)

		pEntity->SetConnectionStateChangeCallback(asCallback);

	END_SET_PROPERTY
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetEntityInteractionDisabled(string& asName, bool abDisabled)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_Prop,-1)

		iLuxProp *pProp = ToProp(pEntity);
		pProp->SetInteractionDisabled(abDisabled);
	
	END_SET_PROPERTY

}

//-----------------------------------------------------------------------

bool __stdcall cLuxScriptHandler::GetEntitiesCollide(string& asEntityA, string& asEntityB)
{
	iLuxEntity *pEntityA = GetEntity(asEntityA, eLuxEntityType_LastEnum,-1);
	if(pEntityA==NULL) return false;

	iLuxEntity *pEntityB = GetEntity(asEntityB, eLuxEntityType_LastEnum,-1);
	if(pEntityB==NULL) return false;

	return pEntityA->CheckEntityCollision(pEntityB, gpBase->mpMapHandler->GetCurrentMap());
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetPropEffectActive(string& asName, bool abActive, bool abFadeAndPlaySounds)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_Prop,-1)

		iLuxProp *pProp = ToProp(pEntity);
		pProp->SetEffectsActive(abActive, abFadeAndPlaySounds);
	
	END_SET_PROPERTY
}

//-----------------------------------------------------------------------


void __stdcall cLuxScriptHandler::SetPropActiveAndFade(string& asName, bool abActive, float afFadeTime)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_Prop,-1)

		iLuxProp *pProp = ToProp(pEntity);
		pProp->SetActive(abActive);
		if(abActive)
		{
			pProp->FadeInMeshEntity(afFadeTime);
		}
		else if(pProp->GetMeshEntity())
		{
			gpBase->mpMapHandler->GetCurrentMap()->AddDissolveEntity(pProp->GetMeshEntity(), afFadeTime);
		}
			

	END_SET_PROPERTY
}

void __stdcall cLuxScriptHandler::SetPropStaticPhysics(string& asName, bool abX)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_Prop,-1)

		iLuxProp *pProp = ToProp(pEntity);
		pProp->SetStaticPhysics(abX);

	END_SET_PROPERTY
}

//-----------------------------------------------------------------------

bool __stdcall cLuxScriptHandler::GetPropIsInteractedWith(string& asName)
{
	iLuxProp *pProp = ToProp(GetEntity(asName, eLuxEntityType_Prop, -1));
	if(pProp==NULL) return false;

    return pProp->IsInteractedWith();
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::RotatePropToSpeed(string& asName, float afAcc, float afGoalSpeed, float afAxisX, float afAxisY, float afAxisZ, bool abResetSpeed, string& asOffsetArea)
{
	iLuxArea *pArea = NULL;
	if(asOffsetArea != "")
	{
		pArea = ToArea(GetEntity(asOffsetArea,eLuxEntityType_Area, -1));
	}


	BEGIN_SET_PROPERTY(eLuxEntityType_Prop,-1)

		iLuxProp *pProp = ToProp(pEntity);
		pProp->SetFullGameSave(true);
		if(pArea)
		{
			cMatrixf mtxInvLoad = cMath::MatrixInverse(pProp->GetMainBody()->GetWorldMatrix());
			cVector3f vLocalOffset = cMath::MatrixMul(mtxInvLoad, pArea->GetPosition());

			pProp->RotateAtSpeed(afAcc, afGoalSpeed, cVector3f(afAxisX, afAxisY, afAxisZ), abResetSpeed, true, 
								pArea->GetPosition(), vLocalOffset);
		}
		else
		{
			pProp->RotateAtSpeed(afAcc, afGoalSpeed, cVector3f(afAxisX, afAxisY, afAxisZ), abResetSpeed, false, 0,0);
		}

	END_SET_PROPERTY
}

//-----------------------------------------------------------------------

void __stdcall  cLuxScriptHandler::StopPropMovement(string& asName)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_Prop,-1)

		iLuxProp *pProp = ToProp(pEntity);
		pProp->StopMove();
		

	END_SET_PROPERTY
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::DetachPropFromBone(string& asChildEntityName)
{
    iLuxProp *pChildProp = ToProp(GetEntity(asChildEntityName,eLuxEntityType_Prop,-1));
	if ( pChildProp == NULL )
	{
		Error("Could not find child prop '%s'\n", asChildEntityName.c_str());
		return;
	}

    pChildProp->DetachFromParentBone();
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::AttachPropToBone(string& asChildEntityName, string& asParentEntityName, string& asParentBoneName, float fPosX, float fPosY, float fPosZ, float fRotX, float fRotY, float fRotZ)
{
	iLuxProp *pChildProp = ToProp(GetEntity(asChildEntityName,eLuxEntityType_Prop,-1));
	if ( pChildProp == NULL )
	{
		Error("Could not find child prop '%s'\n", asChildEntityName.c_str());
		return;
	}

	iLuxEntity * pParentEntity = GetEntity(asParentEntityName, eLuxEntityType_LastEnum, -1);

	if ( pParentEntity == NULL )
	{
		Error("Could not find parent entity '%s'\n", asParentEntityName.c_str());
		return;
	}
	
	cMeshEntity * pChildMeshEntity = pChildProp->GetMeshEntity();

	if ( pChildMeshEntity == NULL )
	{
		Error("Child entity '%s' does not have a mesh entity\n", asChildEntityName.c_str());
		return;
	}

	cMeshEntity * pParentMeshEntity = pParentEntity->GetMeshEntity();

	if ( pParentMeshEntity == NULL )
	{
		Error("Parent entity '%s' does not have a mesh entity\n", asParentEntityName.c_str());
		return;
	}

	cBoneState * pParentBone = pParentMeshEntity->GetBoneStateFromName( asParentBoneName );

	if ( pParentBone == NULL )
	{
		Error("Did not find bone '%s' in parent entity '%s'\n", asParentBoneName.c_str(), asParentEntityName.c_str());
		return;
	}

    cMatrixf mtxTransform = cMath::MatrixRotate(cMath::Vector3ToRad(cVector3f(fRotX, fRotY, fRotZ)), eEulerRotationOrder_XYZ);
	mtxTransform.SetTranslation(cVector3f(fPosX, fPosY,fPosZ));

	pChildProp->SetParentBone(pParentBone, mtxTransform, asParentEntityName, asParentBoneName);
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::AddAttachedPropToProp(string& asPropName, string& asAttachName, string& asAttachFile, float afPosX, float afPosY, float afPosZ, float afRotX, float afRotY, float afRotZ)
{
	tString asName = asPropName;

	cMatrixf mtxTransform = cMath::MatrixRotate(cMath::Vector3ToRad(cVector3f(afRotX, afRotY, afRotZ)), eEulerRotationOrder_XYZ);
	mtxTransform.SetTranslation(cVector3f(afPosX, afPosY, afPosZ));

	BEGIN_SET_PROPERTY(eLuxEntityType_Prop,-1)

		iLuxProp *pProp = ToProp(pEntity);
		pProp->AddAndAttachProp(asAttachName, asAttachFile, mtxTransform);

	END_SET_PROPERTY
}

void __stdcall cLuxScriptHandler::RemoveAttachedPropFromProp(string& asPropName, string& asAttachName)
{
	tString asName = asPropName;

	BEGIN_SET_PROPERTY(eLuxEntityType_Prop,-1)

		iLuxProp *pProp = ToProp(pEntity);
		if(pProp->DestroyAttachedProp(asAttachName)==false)
		{
			Error("Could not find attached prop '%s' in '%s'\n", asPropName.c_str(), asAttachName.c_str());
		}

	END_SET_PROPERTY
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetLampCanBeLitByPlayer(string& asName, bool abLit)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_Prop,eLuxPropType_Lamp)

		cLuxProp_Lamp *pLamp = ToLamp(pEntity);
		pLamp->SetCanBeLitByPlayer(abLit);

	END_SET_PROPERTY
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::FadeLampTo(string& asName, float afR, float afG, float afB, float afA, float afRadius, float afTime)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_Prop,eLuxPropType_Lamp)

		cLuxProp_Lamp *pLamp = ToLamp(pEntity);

		pLamp->FadeTo(afR, afG, afB, afA, afRadius, afTime);

	END_SET_PROPERTY
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetLampLit(string& asName, bool abLit, bool abEffects)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_Prop,eLuxPropType_Lamp)

		cLuxProp_Lamp *pLamp = ToLamp(pEntity);
		pLamp->SetLit(abLit, abEffects);

	END_SET_PROPERTY
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetSwingDoorLocked(string& asName, bool abLocked, bool abEffects)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_Prop,eLuxPropType_SwingDoor)
    
		cLuxProp_SwingDoor *pSwingDoor = ToSwingDoor(pEntity);
		pSwingDoor->SetLocked(abLocked, abEffects);

	END_SET_PROPERTY
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetSwingDoorClosed(string& asName, bool abClosed, bool abEffects)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_Prop,eLuxPropType_SwingDoor)

		cLuxProp_SwingDoor *pSwingDoor = ToSwingDoor(pEntity);
		pSwingDoor->SetClosed(abClosed, abEffects);

	END_SET_PROPERTY
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetSwingDoorDisableAutoClose(string& asName, bool abDisableAutoClose)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_Prop,eLuxPropType_SwingDoor)

		cLuxProp_SwingDoor *pSwingDoor = ToSwingDoor(pEntity);
		pSwingDoor->SetDisableAutoClose(abDisableAutoClose);

	END_SET_PROPERTY
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetSwingDoorOpenAmount(string& asName, float afOpenAmount, float afDuration, bool abOpenTowardsMaxAngle)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_Prop,eLuxPropType_SwingDoor)

		cLuxProp_SwingDoor *pSwingDoor = ToSwingDoor(pEntity);
		pSwingDoor->SetOpenAmount(afOpenAmount, afDuration, abOpenTowardsMaxAngle);

	END_SET_PROPERTY
}

//-----------------------------------------------------------------------

bool __stdcall cLuxScriptHandler::GetSwingDoorLocked(string &asName)
{
	cLuxProp_SwingDoor *pSwingDoor = ToSwingDoor(GetEntity(asName,eLuxEntityType_Prop,eLuxPropType_SwingDoor));
	if(pSwingDoor==NULL) return false;

    return pSwingDoor->GetLocked();
}

bool __stdcall cLuxScriptHandler::GetSwingDoorClosed(string &asName)
{
	cLuxProp_SwingDoor *pSwingDoor = ToSwingDoor(GetEntity(asName,eLuxEntityType_Prop,eLuxPropType_SwingDoor));
	if(pSwingDoor==NULL) return false;

	return pSwingDoor->GetClosed();
}

int __stdcall cLuxScriptHandler::GetSwingDoorState(string &asName)
{
	cLuxProp_SwingDoor *pSwingDoor = ToSwingDoor(GetEntity(asName,eLuxEntityType_Prop,eLuxPropType_SwingDoor));
	if(pSwingDoor==NULL) return 0;
	
	return pSwingDoor->GetDoorState();
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetLevelDoorLocked(string& asName, bool abLocked)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_Prop,eLuxPropType_LevelDoor)

		cLuxProp_LevelDoor *pLevelDoor = ToLevelDoor(pEntity);
		pLevelDoor->SetLocked(abLocked);

	END_SET_PROPERTY
}

void __stdcall cLuxScriptHandler::SetLevelDoorLockedSound(string& asName, string& asSound)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_Prop,eLuxPropType_LevelDoor)

		cLuxProp_LevelDoor *pLevelDoor = ToLevelDoor(pEntity);
		pLevelDoor->SetLockedSound(asSound);

	END_SET_PROPERTY
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetLevelDoorLockedText(string& asName, string& asTextCat, string& asTextEntry)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_Prop,eLuxPropType_LevelDoor)

		cLuxProp_LevelDoor *pLevelDoor = ToLevelDoor(pEntity);
		pLevelDoor->SetLockedText(asTextCat, asTextEntry);

	END_SET_PROPERTY
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetPropObjectStuckState(string& asName, int alState)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_Prop,eLuxPropType_Object)

		cLuxProp_Object *pObject = ToObject(pEntity);
		pObject->SetStuckState(alState);

	END_SET_PROPERTY
}

void __stdcall cLuxScriptHandler::SetWheelAngle(string& asName, float afAngle, bool abAutoMove)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_Prop,eLuxPropType_Wheel)

		cLuxProp_Wheel *pWheel = ToWheel(pEntity);
		pWheel->SetAngle(afAngle, abAutoMove);

	END_SET_PROPERTY
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetWheelStuckState(string& asName, int alState, bool afEffects)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_Prop,eLuxPropType_Wheel)

		cLuxProp_Wheel *pWheel = ToWheel(pEntity);
		pWheel->SetStuckState(alState, afEffects);

	END_SET_PROPERTY
}

//-----------------------------------------------------------------------


void __stdcall cLuxScriptHandler::SetLeverStuckState(string& asName, int alState, bool afEffects)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_Prop,eLuxPropType_Lever)

		cLuxProp_Lever *pLever = ToLever(pEntity);
		pLever->SetStuckState(alState, afEffects);

	END_SET_PROPERTY
}
//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetWheelInteractionDisablesStuck(string& asName, bool abX)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_Prop,eLuxPropType_Wheel)

		cLuxProp_Wheel *pWheel = ToWheel(pEntity);
		pWheel->SetInteractionDisablesStuck(abX);

	END_SET_PROPERTY
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetLeverInteractionDisablesStuck(string& asName, bool abX)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_Prop,eLuxPropType_Lever)

		cLuxProp_Lever *pLever = ToLever(pEntity);
		pLever->SetInteractionDisablesStuck(abX);

	END_SET_PROPERTY
}

//-----------------------------------------------------------------------

int __stdcall cLuxScriptHandler::GetLeverState(string& asName)
{
	cLuxProp_Lever *pLever = ToLever(GetEntity(asName, eLuxEntityType_Prop, eLuxPropType_Lever));
	if(pLever==NULL) return 0;

	return pLever->GetLeverState();
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetMultiSliderStuckState(string& asName, int alStuckState, bool abEffects)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_Prop,eLuxPropType_MultiSlider)

		cLuxProp_MultiSlider *pSlider = ToMultiSlider(pEntity);
		pSlider->SetStuckState(alStuckState, abEffects);

	END_SET_PROPERTY
}

void __stdcall cLuxScriptHandler::SetMultiSliderCallback(string& asName, string& asCallback)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_Prop,eLuxPropType_MultiSlider)

		cLuxProp_MultiSlider *pSlider = ToMultiSlider(pEntity);
		pSlider->SetChangeStateCallback(asCallback);

	END_SET_PROPERTY
}


//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetButtonSwitchedOn(string& asName, bool abSwitchedOn, bool abEffects)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_Prop,eLuxPropType_Button)

		cLuxProp_Button *pButton = ToButton(pEntity);
		pButton->SetSwitchedOn(abSwitchedOn, abEffects);

	END_SET_PROPERTY
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetButtonCanBeSwitchedOn(string& asName, bool abCanBeSwitchedOn)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_Prop,eLuxPropType_Button)

		cLuxProp_Button *pButton = ToButton(pEntity);
		pButton->SetCanBeSwitchedOn(abCanBeSwitchedOn);

	END_SET_PROPERTY
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetButtonCanBeSwitchedOff(string& asName, bool abCanBeSwitchedOff)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_Prop,eLuxPropType_Button)

		cLuxProp_Button *pButton = ToButton(pEntity);
		pButton->SetCanBeSwitchedOff(abCanBeSwitchedOff);

	END_SET_PROPERTY
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetAllowStickyAreaAttachment(bool abX)
{
	cLuxArea_Sticky::SetAllowAttachment(abX);
}

void __stdcall cLuxScriptHandler::AttachPropToStickyArea(string& asAreaName, string& asProp)
{
	cLuxArea_Sticky *pStickyArea = ToStickyArea(GetEntity(asAreaName,eLuxEntityType_Area,eLuxAreaType_Sticky));
	iLuxProp *pProp = ToProp(GetEntity(asProp,eLuxEntityType_Prop,-1));
	
	if(pProp==NULL || pStickyArea==NULL) return;

	pStickyArea->AttachBody(pProp->GetMainBody() ? pProp->GetMainBody() : pProp->GetBody(0));	
}

void __stdcall cLuxScriptHandler::AttachAreaToProp(string& asAreaName, string& asProp, int alBody)
{
	iLuxArea *pArea = ToArea(GetEntity(asAreaName,eLuxEntityType_Area,-1));
	
	if(pArea==NULL)
	{
#if 1
		Error("WARNING: AttachAreaToProp Area Not Found:'%s'\n", asAreaName.c_str());
#endif
		return;
	}

	pArea->AttachToBody(asProp, alBody);
}

void __stdcall cLuxScriptHandler::AttachBodyToStickyArea(string& asAreaName, string& asBody)
{
	cLuxArea_Sticky *pStickyArea = ToStickyArea(GetEntity(asAreaName,eLuxEntityType_Area,eLuxAreaType_Sticky));
	if(pStickyArea==NULL) return;
	
	cLuxMap *pMap = gpBase->mpMapHandler->GetCurrentMap();
	iPhysicsBody *pBody = pMap->GetPhysicsWorld()->GetBody(asBody);
	if(pBody==NULL)
	{
		Error("Could not find body '%s'\n", asBody.c_str());
		return;
	}

	pStickyArea->AttachBody(pBody);
}

void __stdcall cLuxScriptHandler::DetachFromStickyArea(string& asAreaName)
{
	cLuxArea_Sticky *pStickyArea = ToStickyArea(GetEntity(asAreaName,eLuxEntityType_Area,eLuxAreaType_Sticky));
	
	if(pStickyArea) pStickyArea->DetachBody();
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetNPCAwake(string& asName, bool abAwake, bool abEffects)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_Prop,eLuxPropType_NPC)

		cLuxProp_NPC *pNPC = ToNPC(pEntity);
		pNPC->SetAwake(abAwake, abEffects);

	END_SET_PROPERTY
}

void __stdcall cLuxScriptHandler::SetNPCFollowPlayer(string& asName, bool abX)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_Prop,eLuxPropType_NPC)

		cLuxProp_NPC *pNPC = ToNPC(pEntity);
		pNPC->SetFollowPlayer(abX);

	END_SET_PROPERTY
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::ChangeEnemyPose(string& asName, string& asPoseType)
{
	eLuxEnemyPoseType pose = eLuxEnemyPoseType_LastEnum;
	if(asPoseType == "Biped")			pose =eLuxEnemyPoseType_Biped;
	else if(asPoseType == "Quadruped")	pose =eLuxEnemyPoseType_Quadruped;

	if(pose == eLuxEnemyPoseType_LastEnum)
	{
		Error("Could not set pose '%s' for enemy '%s'. Pose does not exist!\n", asPoseType.c_str(), asName.c_str());
		return;
	}

	BEGIN_SET_PROPERTY(eLuxEntityType_Enemy,-1)

		iLuxEnemy *pEnemy = ToEnemy(pEntity);
		pEnemy->ChangePose(pose);

	END_SET_PROPERTY
}

void __stdcall cLuxScriptHandler::SetEnemyMoveType(string& asName, string& asMoveType)
{
	/*eLuxEnemyMoveType moveType = eLuxEnemyMoveType_WalkBiped;

	if ( asMoveType == "WalkBiped" )
	{
		moveType = eLuxEnemyMoveType_WalkBiped;
	}
	else if ( asMoveType == "RunBiped" )
	{
		moveType = eLuxEnemyMoveType_RunBiped;
	}
	else if ( asMoveType == "WalkQuadruped" )
	{
		moveType = eLuxEnemyMoveType_WalkQuadruped;
	}
	else if ( asMoveType == "RunQuadruped" )
	{
		moveType = eLuxEnemyMoveType_RunQuadruped;
	}
	else if ( asMoveType == "JogQuadruped" )
	{
		moveType = eLuxEnemyMoveType_JogQuadruped;
	}
	else if ( asMoveType == "ChargeBiped" )
	{
		moveType = eLuxEnemyMoveType_ChargeBiped;
	}
	else if ( asMoveType == "FleeBiped" )
	{
		moveType = eLuxEnemyMoveType_FleeBiped;
	}

	BEGIN_SET_PROPERTY(eLuxEntityType_Enemy,-1)

		iLuxEnemy *pEnemy = ToEnemy(pEntity);
		pEnemy->SetNativeMoveType(moveType);
	
	END_SET_PROPERTY*/
}

//-----------------------------------------------------------------------

int __stdcall cLuxScriptHandler::GetEnemyState(string& asName)
{
	iLuxEnemy *pEnemy = ToEnemy(GetEntity(asName, eLuxEntityType_Enemy, -1));
	if( pEnemy==NULL )
	{
		Error("Can't find enemy '%s'!\n", asName.c_str());
		return 0;
	}

	return pEnemy->GetCurrentEnemyState();
}

//-----------------------------------------------------------------------

int __stdcall cLuxScriptHandler::StringToInt(string& asString)
{
	return atoi(asString.c_str());
}

//-----------------------------------------------------------------------

string& __stdcall cLuxScriptHandler::GetEnemyStateName(string& asName)
{
	iLuxEnemy *pEnemy = ToEnemy(GetEntity(asName, eLuxEntityType_Enemy, -1));
	if( pEnemy==NULL )
	{
		Error("Can't find enemy '%s'!\n", asName.c_str());
		return gsScriptNull;
	}

	return pEnemy->GetCurrentEnemyStateName();
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetManPigType(string& asName, string& asManPigType)
{
	/*eLuxEnemyPigType manPigType = eLuxEnemyPigType_Rod;
	
	if ( asManPigType == "Rod" )
	{
		manPigType = eLuxEnemyPigType_Rod;
	}
	else if ( asManPigType == "Jane" )
	{
		manPigType = eLuxEnemyPigType_Jane;
	}
	else if ( asManPigType == "Freddy" )
	{
		manPigType = eLuxEnemyPigType_Freddy;
	}

	BEGIN_SET_PROPERTY(eLuxEntityType_Enemy,-1)

		cLuxEnemy_ManPig *pEnemy = ToManPig(pEntity);
		pEnemy->SetManPigType( manPigType );
	
	END_SET_PROPERTY*/
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SendEnemyTimeOut(string& asName, float afTimeOut )
{
	BEGIN_SET_PROPERTY(eLuxEntityType_Enemy,-1)

		iLuxEnemy *pEnemy = ToEnemy(pEntity);
		pEnemy->SendMessage(eLuxEnemyMessage_TimeOut, afTimeOut);
	
	END_SET_PROPERTY
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::StopAnimationAndContinue(string& asName, float afTimeOut )
{
	BEGIN_SET_PROPERTY(eLuxEntityType_Enemy,-1)

		iLuxEnemy *pEnemy = ToEnemy(pEntity);
		pEnemy->SendMessage(eLuxEnemyMessage_StopPatrolAnimation, afTimeOut);
	
	END_SET_PROPERTY
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::PlayScriptedAnimation(string& asName, string& asAnimationName, bool abLoopAnimation)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_Enemy,-1)

		iLuxEnemy *pEnemy = ToEnemy(pEntity);
		pEnemy->PlayScriptedAnimation(asAnimationName, abLoopAnimation);
	
	END_SET_PROPERTY
}

void __stdcall cLuxScriptHandler::SetEnemyBlind(string& asName, bool abX)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_Enemy,-1)

		iLuxEnemy *pEnemy = ToEnemy(pEntity);
		pEnemy->SetBlind(abX);

	END_SET_PROPERTY
}

void __stdcall cLuxScriptHandler::SetEnemyDeaf(string& asName, bool abX)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_Enemy,-1)

		iLuxEnemy *pEnemy = ToEnemy(pEntity);
		pEnemy->SetDeaf(abX);

	END_SET_PROPERTY
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetTeslaPigFadeDisabled(string& asName, bool abX)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_Enemy,-1)

		cLuxEnemy_ManPig *pEnemy = ToManPig(pEntity);
		if (!pEnemy) continue;
		pEnemy->SetTeslaFadeDisabled(abX);
	
	END_SET_PROPERTY
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetTeslaPigSoundDisabled(string& asName, bool abX)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_Enemy,-1)

		cLuxEnemy_ManPig *pEnemy = ToManPig(pEntity);
		if (!pEnemy) continue;
		pEnemy->SetTeslaSoundDisabled(abX);
	
	END_SET_PROPERTY
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetTeslaPigEasyEscapeDisabled(string& asName, bool abX)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_Enemy,-1)

		cLuxEnemy_ManPig *pEnemy = ToManPig(pEntity);
		if (!pEnemy) continue;
		pEnemy->SetTeslaEasyEscapeDisabled(abX);
	
	END_SET_PROPERTY
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::ForceTeslaPigSighting(string& asName)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_Enemy,-1)

		cLuxEnemy_ManPig *pEnemy = ToManPig(pEntity);
		if (!pEnemy) continue;
		pEnemy->ForceTeslaSighting();
	
	END_SET_PROPERTY
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetEnemyDisabled(string& asName, bool abDisabled)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_Enemy,-1)

		iLuxEnemy *pEnemy = ToEnemy(pEntity);
		pEnemy->SetDisabled(abDisabled);
	
	END_SET_PROPERTY
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetEnemyActivationDistance(string& asName, float afX)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_Enemy,-1)

		iLuxEnemy *pEnemy = ToEnemy(pEntity);
		pEnemy->SetActivationDistance(afX);
	
	END_SET_PROPERTY
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetEnemyIsHallucination(string& asName, bool abX)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_Enemy,-1)

		iLuxEnemy *pEnemy = ToEnemy(pEntity);
		pEnemy->SetHallucination(abX);

	END_SET_PROPERTY
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::FadeEnemyToSmoke(string& asName, bool abPlaySound)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_Enemy,-1)

		iLuxEnemy *pEnemy = ToEnemy(pEntity);
		pEnemy->FadeToSmoke(abPlaySound);

	END_SET_PROPERTY
}

void __stdcall cLuxScriptHandler::ShowEnemyPlayerPosition(string& asName)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_Enemy,-1)

		iLuxEnemy *pEnemy = ToEnemy(pEntity);
		pEnemy->ShowPlayerPosition();
		
		eLuxEnemyState state = pEnemy->GetCurrentEnemyState();
		if(	state != eLuxEnemyState_Hunt &&
			state != eLuxEnemyState_HuntPause &&
			state != eLuxEnemyState_AttackMeleeLong &&
			state != eLuxEnemyState_AttackMeleeShort &&
			state != eLuxEnemyState_BreakDoor)
		{
			pEnemy->ChangeState(eLuxEnemyState_Hunt);
		}
	
	END_SET_PROPERTY
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::ForceEnemyWaitState(string& asName)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_Enemy,-1)

		iLuxEnemy *pEnemy = ToEnemy(pEntity);
		pEnemy->ChangeState(eLuxEnemyState_Wait);
	
	END_SET_PROPERTY
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetEnemyDisableTriggers(string& asName, bool abX)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_Enemy,-1)

		iLuxEnemy *pEnemy = ToEnemy(pEntity);
		pEnemy->SetDisableTriggers(abX);

	END_SET_PROPERTY
}


//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::AddEnemyPatrolNode(string& asName, string& asNodeName, float afWaitTime, string& asAnimation, bool abLoopAnimation)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_Enemy,-1)
		
		iLuxEnemy *pEnemy = ToEnemy(pEntity);
		cAINodeContainer *pAINodeCont = pEnemy->GetPathFinder()->GetNodeContainer();
		if(pAINodeCont==NULL)
		{
			Error("There is no node container in enemy '%s'! (probably no nodes in map!)\n", pEnemy->GetName().c_str());
			continue;
		}

		cAINode *pNode = pAINodeCont->GetNodeFromName(asNodeName);
		
		if(pNode==NULL)
		{
			Error("Could not find node '%s' for enemy '%s'\n", asNodeName.c_str(), pEntity->GetName().c_str());
			continue;
		}

		pEnemy->AddPatrolNode(pNode, afWaitTime, asAnimation, abLoopAnimation);
	
	END_SET_PROPERTY
}

void __stdcall cLuxScriptHandler::ClearEnemyPatrolNodes(string& asName)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_Enemy,-1)
		
		iLuxEnemy *pEnemy = ToEnemy(pEntity);
		pEnemy->ClearPatrolNodes();

	END_SET_PROPERTY
}

void __stdcall cLuxScriptHandler::PlayEnemyAnimation(string & asName, string & asAnimName, bool abLoop, float afFadeTime)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_Enemy,-1)

		iLuxEnemy *pEnemy = ToEnemy(pEntity);
		pEnemy->ClearPatrolNodes();
		pEnemy->PlayAnim(asAnimName, abLoop, afFadeTime);

	END_SET_PROPERTY
}

void __stdcall cLuxScriptHandler::TeleportEnemyToNode(string & asName, string & asNodeName, bool abChangeY)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_Enemy,-1)

		iLuxEnemy *pEnemy = ToEnemy(pEntity);
		cAINode *pNode = pEnemy->GetPathFinder()->GetNodeContainer()->GetNodeFromName(asNodeName);
		if(pNode==NULL){
			Error("Could not find node '%s' for enemy '%s'\n", asNodeName.c_str(), pEnemy->GetName().c_str());
			continue;
		}
		
		cVector3f vNodePos = pNode->GetPosition();
		if(abChangeY==false) vNodePos.y = pEnemy->GetCharacterBody()->GetFeetPosition().y;

		pEnemy->GetCharacterBody()->SetFeetPosition(vNodePos);

	END_SET_PROPERTY
}

//-----------------------------------------------------------------------

float __stdcall cLuxScriptHandler::GetEnemyPlayerDistance(string & asEnemyName)
{
	iLuxEnemy *pEnemy = ToEnemy(GetEntity(asEnemyName, eLuxEntityType_Enemy, -1));
	if( pEnemy==NULL )
	{
		Error("Can't find enemy '%s'!\n", asEnemyName.c_str());
		return 0;
	}

	return pEnemy->DistToPlayer();
}


bool __stdcall cLuxScriptHandler::GetPlayerCanSeeEnemy(string & asEnemyName)
{
	iLuxEnemy *pEnemy = ToEnemy(GetEntity(asEnemyName, eLuxEntityType_Enemy, -1));
	if( pEnemy==NULL )
	{
		Error("Can't find enemy '%s'!\n", asEnemyName.c_str());
		return false;
	}

	return pEnemy->GetIsSeenByPlayer();
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetMoveObjectState(string& asName, float afState)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_Prop, eLuxPropType_MoveObject)

		cLuxProp_MoveObject *pMoveObj = ToMoveObject(pEntity);
		pMoveObj->MoveToState(afState);

	END_SET_PROPERTY
}

void __stdcall cLuxScriptHandler::SetMoveObjectStateExt(string& asName, float afState, float afAcc, float afMaxSpeed, float afSlowdownDist, bool abResetSpeed)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_Prop, eLuxPropType_MoveObject)

		cLuxProp_MoveObject *pMoveObj = ToMoveObject(pEntity);
		pMoveObj->MoveToState(afState, afAcc, afMaxSpeed, afSlowdownDist, abResetSpeed);

	END_SET_PROPERTY
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetPropHealth(string& asName, float afHealth)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_Prop, -1)

		iLuxProp *pProp = ToProp(pEntity);
		pProp->SetHealth(afHealth);	

	END_SET_PROPERTY
}

void __stdcall cLuxScriptHandler::AddPropHealth(string& asName, float afHealth)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_Prop, -1)

		iLuxProp *pProp = ToProp(pEntity);
		pProp->SetHealth(pProp->GetHealth() + afHealth);	

	END_SET_PROPERTY
}

float __stdcall cLuxScriptHandler::GetPropHealth(string& asName)
{
	iLuxProp *pProp = ToProp(GetEntity(asName, eLuxEntityType_Prop, -1));
	if(pProp==NULL) return 0;

	return pProp->GetHealth();
}

void __stdcall cLuxScriptHandler::SetPlayerUsesDragFootsteps(bool abX)
{
    gpBase->mpPlayer->SetUsesDragFootsteps( abX );
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::ResetProp(string& asName)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_Prop, -1)

		iLuxProp *pProp = ToProp(pEntity);
		pProp->ResetProperties();	

	END_SET_PROPERTY
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::PlayPropAnimation(string& asProp, string& asAnimation, float afFadeTime, bool abLoop, string &asCallback)
{
	tString asName = asProp;
	BEGIN_SET_PROPERTY(eLuxEntityType_Prop, -1)

		iLuxProp *pProp = ToProp(pEntity);
		pProp->PlayAnimation(asAnimation, afFadeTime, abLoop, asCallback);

	END_SET_PROPERTY
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::StopPropAnimation(string& asProp)
{
	tString asName = asProp;
	BEGIN_SET_PROPERTY(eLuxEntityType_Prop, -1)

		iLuxProp *pProp = ToProp(pEntity);
		pProp->StopAnimation();

	END_SET_PROPERTY
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::PlayCurrentAnimation(string& asProp, float afFadeTime, bool abLoop)
{
	tString asName = asProp;
	BEGIN_SET_PROPERTY(eLuxEntityType_Prop, -1)

		iLuxProp *pProp = ToProp(pEntity);
		pProp->PlayCurrentAnimation(afFadeTime, abLoop);

	END_SET_PROPERTY
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::PauseCurrentAnimation(string& asProp, float afFadeTime)
{
	tString asName = asProp;
	BEGIN_SET_PROPERTY(eLuxEntityType_Prop, -1)

		iLuxProp *pProp = ToProp(pEntity);
		pProp->PauseCurrentAnimation(afFadeTime);

	END_SET_PROPERTY
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::SetPropAnimationSpeed(string& asProp, float afSpeed)
{
	tString asName = asProp;
	BEGIN_SET_PROPERTY(eLuxEntityType_Prop, -1)

		iLuxProp *pProp = ToProp(pEntity);
		pProp->SetAnimationSpeed(afSpeed);

	END_SET_PROPERTY
}

void __stdcall cLuxScriptHandler::SetPropAnimationPosition(string& asProp, float afPos)
{
	tString asName = asProp;
	BEGIN_SET_PROPERTY(eLuxEntityType_Prop, -1)

		iLuxProp *pProp = ToProp(pEntity);
		pProp->SetAnimationPosition(afPos);

	END_SET_PROPERTY
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::AddEntityCollideCallback(string& asName, string& asChildName, string& asFunction, bool abDeleteOnCollide, int alStates)
{
	iLuxEntity* pChild = GetEntity(asChildName, eLuxEntityType_LastEnum, -1);
	if(pChild==NULL) return;
	if(asName == "Player")
	{
		gpBase->mpPlayer->AddCollideCallback(pChild, asFunction, abDeleteOnCollide,alStates);	
	}
	else
	{
		BEGIN_SET_PROPERTY(eLuxEntityType_LastEnum, -1)

			pEntity->AddCollideCallback(pChild, asFunction, abDeleteOnCollide, alStates);
		
		END_SET_PROPERTY
	}
}

void __stdcall cLuxScriptHandler::RemoveEntityCollideCallback(string& asName, string& asChildName)
{
	//Just to check so entity exist!
	GetEntity(asChildName, eLuxEntityType_LastEnum, -1);
	
	if(asName == "Player")
	{
		gpBase->mpPlayer->RemoveCollideCallback(asChildName);
	}
	else
	{
		BEGIN_SET_PROPERTY(eLuxEntityType_LastEnum, -1)

			pEntity->RemoveCollideCallback(asChildName);

		END_SET_PROPERTY
	}
}

//-----------------------------------------------------------------------

cVector3f VecToCoordSystem(iPhysicsBody *apBody, const cVector3f& avVec, const tString& asCoordSystem)
{
	if(cString::ToLowerCase(asCoordSystem)=="local")
	{
		return cMath::MatrixMul(apBody->GetLocalMatrix().GetRotation(),avVec);
	}
	else
	{
		return avVec;
	}
}

void __stdcall cLuxScriptHandler::AddPropForce(string& asName, float afX, float afY, float afZ, string& asCoordSystem)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_Prop,-1)

		iLuxProp* pProp = ToProp(pEntity);
		if(pProp==NULL) return;
		cVector3f vVec(afX, afY, afZ);
    
		for(int i=0; i<pProp->GetBodyNum(); ++i)
		{
			iPhysicsBody *pBody = pProp->GetBody(i);
			pBody->AddForce(VecToCoordSystem(pBody, vVec, asCoordSystem));
		}

	END_SET_PROPERTY
}

void __stdcall cLuxScriptHandler::AddPropImpulse(string& asName, float afX, float afY, float afZ, string& asCoordSystem)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_Prop,-1)

		iLuxProp* pProp = ToProp(pEntity);
		if(pProp==NULL) return;
		cVector3f vVec(afX, afY, afZ);

		for(int i=0; i<pProp->GetBodyNum(); ++i)
		{
			iPhysicsBody *pBody = pProp->GetBody(i);
			pBody->AddImpulse(VecToCoordSystem(pBody, vVec, asCoordSystem));
		}
	END_SET_PROPERTY
}

void __stdcall cLuxScriptHandler::StartPhoneRinging(string & asName)
{
	BEGIN_SET_PROPERTY(eLuxEntityType_Prop,-1)
        
        cLuxProp_PhoneBox * phone = static_cast<cLuxProp_PhoneBox*>(pEntity);

        if ( phone )
        {
            phone->StartRinging();
        }

	END_SET_PROPERTY
}

void __stdcall cLuxScriptHandler::StopPhoneRinging(string & asName)
{
    BEGIN_SET_PROPERTY(eLuxEntityType_Prop,-1)
        
        cLuxProp_PhoneBox * phone = static_cast<cLuxProp_PhoneBox*>(pEntity);

        if ( phone )
        {
            phone->StopRinging();
        }

	END_SET_PROPERTY
}

void __stdcall cLuxScriptHandler::HangUpPhone(string & asName)
{
    BEGIN_SET_PROPERTY(eLuxEntityType_Prop,-1)
        
        cLuxProp_PhoneBox * phone = static_cast<cLuxProp_PhoneBox*>(pEntity);

        if ( phone )
        {
            phone->HangUp();
        }

	END_SET_PROPERTY
}

void __stdcall cLuxScriptHandler::AddBodyForce(string& asName, float afX, float afY, float afZ, string& asCoordSystem)
{
	cLuxMap *pMap = gpBase->mpMapHandler->GetCurrentMap();
	iPhysicsBody *pBody = pMap->GetPhysicsWorld()->GetBody(asName);
	if(pBody==NULL){
		Error("Could not find body '%s'!\n", asName.c_str());
		return;
	}
	cVector3f vVec(afX, afY, afZ);

	pBody->AddForce(VecToCoordSystem(pBody, vVec, asCoordSystem));

}

void __stdcall cLuxScriptHandler::AddBodyImpulse(string& asName, float afX, float afY, float afZ, string& asCoordSystem)
{
	cLuxMap *pMap = gpBase->mpMapHandler->GetCurrentMap();
	iPhysicsBody *pBody = pMap->GetPhysicsWorld()->GetBody(asName);
	if(pBody==NULL){
		Error("Could not find body '%s'!\n", asName.c_str());
		return;
	}
	cVector3f vVec(afX, afY, afZ);
	
	pBody->AddImpulse(VecToCoordSystem(pBody, vVec, asCoordSystem));
}

void __stdcall cLuxScriptHandler::BreakJoint(string& asName)
{
	cLuxMap *pMap = gpBase->mpMapHandler->GetCurrentMap();
	iPhysicsJoint *pJoint = pMap->GetPhysicsWorld()->GetJoint(asName);
	if(pJoint == NULL)
	{
		Error("Could not find joint '%s'!\n", asName.c_str());
		return;
	}

	pJoint->Break();
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::InteractConnectPropWithRope(	string& asName, string& asPropName, string& asRopeName, bool abInteractOnly, 
																float afSpeedMul,float afMinSpeed, float afMaxSpeed,
																bool abInvert, int alStatesUsed)
{
	cLuxMap *pMap = gpBase->mpMapHandler->GetCurrentMap();
	iPhysicsWorld *pPhysicsWorld = pMap->GetPhysicsWorld();

	iLuxProp *pProp = ToProp(GetEntity(asPropName, eLuxEntityType_Prop, -1));
	if(pProp==NULL) return;
	
    iPhysicsRope *pRope = pPhysicsWorld->GetRope(asRopeName);
	if(pRope == NULL)
	{
		Error("Could not find rope '%s'!\n", asRopeName.c_str());
		return;
	}
	
	cLuxInteractConnection_Rope *pConnection = hplNew(cLuxInteractConnection_Rope, (asName, pProp,pRope, afSpeedMul,
														-cMath::Abs(afMinSpeed), cMath::Abs(afMaxSpeed),
														abInvert, alStatesUsed));
	pConnection->SetInteractionOnly(abInteractOnly);

	pProp->AddInteractConnection(pConnection);
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::InteractConnectPropWithMoveObject(	string& asName, string& asPropName, string& asMoveObjectName, bool abInteractOnly,
																		bool abInvert, int alStatesUsed)
{
	iLuxProp *pProp = ToProp(GetEntity(asPropName, eLuxEntityType_Prop, -1));
	if(pProp==NULL)
	{
		return;
	}

	cLuxProp_MoveObject *pSwingDoor = ToMoveObject(GetEntity(asMoveObjectName, eLuxEntityType_Prop, eLuxPropType_MoveObject));
	if(pSwingDoor==NULL)
	{
		return;
	}

	cLuxInteractConnection_MoveObject *pConnection = hplNew(cLuxInteractConnection_MoveObject, (asName, pProp, pSwingDoor, abInvert, alStatesUsed));
	pConnection->SetInteractionOnly(abInteractOnly);

	pProp->AddInteractConnection(pConnection);
}

//-----------------------------------------------------------------------

void __stdcall cLuxScriptHandler::ConnectEntities(string& asName, string& asMainEntity, string& asConnectEntity, bool abInvertStateSent, int alStatesUsed, string& asCallbackFunc)
{
	iLuxEntity *pMainEntity = GetEntity(asMainEntity, eLuxEntityType_LastEnum, -1);
	if(pMainEntity==NULL) return;	

	iLuxEntity *pConnectEntity = GetEntity(asConnectEntity, eLuxEntityType_LastEnum, -1);
	if(pConnectEntity==NULL) return;

	pMainEntity->AddConnection(asName,pConnectEntity, abInvertStateSent, alStatesUsed, asCallbackFunc);
}

//-----------------------------------------------------------------------

/*void __stdcall cLuxScriptHandler::CreateRope(string& asName, 
											string& asStartArea, string& asEndArea, 
											string& asStartBody, string& asEndBody,
											float afMinTotalLength, float afMaxTotalLength,
											float afSegmentLength, float afDamping,
											float afStrength, float afStiffness,
											string& asMaterial, float afRadius,
											float afLengthTileAmount, float afLengthTileSize,
											string& asSound,
											float afSoundStartSpeed, float afSoundStopSpeed,
											bool abAutoMove, float afAutoMoveAcc, float afAutoMoveMaxSpeed)
{
	iLuxArea *pStartArea =	ToArea(GetEntity(asStartArea, eLuxEntityType_Area, -1));
	iLuxArea *pEndArea =	ToArea(GetEntity(asEndArea, eLuxEntityType_Area, -1));

	if(pStartArea==NULL || pEndArea==NULL) return;

	cLuxMap *pMap = gpBase->mpMapHandler->GetCurrentMap();
	iPhysicsWorld *pPhysicsWorld = pMap->GetPhysicsWorld();
	cResources *pResources = gpBase->mpEngine->GetResources();

	iPhysicsRope *pRope = pPhysicsWorld->CreateRope(asName, pStartArea->GetPosition(), pEndArea->GetPosition());

	pRope->SetMinTotalLength(afMinTotalLength);
	pRope->SetMaxTotalLength(afMaxTotalLength);
	pRope->SetSegmentLength(afSegmentLength);
	pRope->SetDamping(afDamping);
	pRope->SetStrength(afStrength);
	pRope->SetStiffness(afStiffness);
	pRope->SetMotorSound(asSound);
	pRope->SetMotorSoundStartSpeed(afSoundStartSpeed);
	pRope->SetMotorSoundStopSpeed(afSoundStopSpeed);
	pRope->SetAutoMoveActive(abAutoMove);
	pRope->SetAutoMoveAcc(afAutoMoveAcc);
	pRope->SetAutoMoveMaxSpeed(afAutoMoveMaxSpeed);

	pRope->SetUniqueID(mlRopeIdCount++);

	//Log("Total Length: %f\n", pRope->GetTotalLength());
	
	//////////////////////////
	//Create graphical entity

	int lMaxSegments = (int)(afMaxTotalLength / afSegmentLength)+4;
	cRopeEntity *pRopeGfx = pMap->GetWorld()->CreateRopeEntity(asName, pRope, lMaxSegments);

	pRopeGfx->SetMaterial(pResources->GetMaterialManager()->CreateMaterial(asMaterial));
	pRopeGfx->SetRadius(afRadius);
	pRopeGfx->SetLengthTileAmount(afLengthTileAmount);
	pRopeGfx->SetLengthTileSize(afLengthTileSize);

	pRopeGfx->SetUniqueID(mlRopeIdCount++);

	//////////////////////////
	//Start body attachment
	if(asStartBody != "")
	{
		iPhysicsBody *pBody = pPhysicsWorld->GetBody(asStartBody);
		if(pBody!=NULL)
		{
			pRope->SetAttachedStartBody(pBody);
			pRope->GetStartParticle()->SetInvMass(1.0f);
		}
		else
		{
			Warning("Could not find body '%s'", asStartBody.c_str());
		}
	}

	//////////////////////////
	//End body attachment
	if(asEndBody != "")
	{
		iPhysicsBody *pBody = pPhysicsWorld->GetBody(asEndBody);
		if(pBody!=NULL)
		{
			pRope->SetAttachedEndBody(pBody);
		}
		else
		{
			Warning("Could not find body '%s'", asEndBody.c_str());
		}
	}

	
}*/

//-----------------------------------------------------------------------


