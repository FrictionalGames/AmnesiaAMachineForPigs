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

#ifndef LUX_SCRIPT_HANDLER_H
#define LUX_SCRIPT_HANDLER_H

//----------------------------------------------

#include "LuxBase.h"

//----------------------------------------------

using namespace std;

class cLuxScriptHandler : public iLuxUpdateable
{
public:	
	cLuxScriptHandler();
	~cLuxScriptHandler();

	void OnStart();
	void Update(float afTimeStep);
	void Reset();

	void OnDraw(float afFrameTime);

private:
	iLowLevelSystem *mpLowLevelSystem;

	//Temp:
	static int mlRopeIdCount;

	//////////////////////////////////
	// Script functions

	// Helpers
	void InitScriptFunctions();
	void AddFunc(const tString& asFunc, void *apFuncPtr);

	static bool GetEntities(const tString& asName,tLuxEntityList &alstEntities, eLuxEntityType aType, int alSubType);
	static iLuxEntity* GetEntity(const tString& asName, eLuxEntityType aType, int alSubType);

	static bool GetParticleSystems(const tString& asName,std::list<cParticleSystem*> &alstParticleSystems);

	// Debug
	static void __stdcall Print(string& asString);
	static void __stdcall AddDebugMessage(string& asString, bool abCheckForDuplicates);
	/**
	 * Type can be "Low", "Medium" or "High"
	 */
	static void __stdcall ProgLog(string& asLevel, string& asMessage);
	static bool __stdcall ScriptDebugOn();


	// General
	static float __stdcall RandFloat(float afMin, float afMax);
	static int __stdcall RandInt(int alMin, int alMax);

	static bool __stdcall StringContains(string& asString, string& asSubString);
	static string& __stdcall StringSub(string& asString, int alStart, int alCount);

	//Function syntax: Func(string &in asTimer)
	static void __stdcall AddTimer(string& asName, float afTime, string& asFunction);
	static void __stdcall RemoveTimer(string& asName);
	static float __stdcall GetTimerTimeLeft(string& asName);

    static void __stdcall StartPhoneRinging(string & asName);
    static void __stdcall StopPhoneRinging(string & asName);
    static void __stdcall HangUpPhone(string & asName);

	static void __stdcall SetLocalVarInt(string& asName, int alVal);
	static void __stdcall SetLocalVarFloat(string& asName, float afVal);
	static void __stdcall SetLocalVarString(string& asName, const string& asVal);
	
	static void __stdcall AddLocalVarInt(string& asName, int alVal);
	static void __stdcall AddLocalVarFloat(string& asName, float afVal);
	static void __stdcall AddLocalVarString(string& asName, string& asVal);
	
	static int __stdcall GetLocalVarInt(string& asName);
	static float __stdcall GetLocalVarFloat(string& asName);
	static string& __stdcall GetLocalVarString(string& asName);

	static void __stdcall SetGlobalVarInt(string& asName, int alVal);
	static void __stdcall SetGlobalVarFloat(string& asName, float afVal);
	static void __stdcall SetGlobalVarString(string& asName, const string& asVal);

	static void __stdcall AddGlobalVarInt(string& asName, int alVal);
	static void __stdcall AddGlobalVarFloat(string& asName, float afVal);
	static void __stdcall AddGlobalVarString(string& asName, string& asVal);

	static int __stdcall GetGlobalVarInt(string& asName);
	static float __stdcall GetGlobalVarFloat(string& asName);
	static string& __stdcall GetGlobalVarString(string& asName);

	// Game
	static void __stdcall StartCredits(string& asMusic, bool abLoopMusic, string& asTextCat, string& asTextEntry, int alEndNum);
	static void __stdcall AddKeyPart(int alKeyPart);

	static void __stdcall StartDemoEnd();

	// Saving
	static void __stdcall AutoSave();
	/**
	 * Callback syntax: MyFunc(string &in asName, int alCount), Count is 0 on first checkpoint load!
	 */
	static void __stdcall CheckPoint(string& asName,string& asStartPos ,string& asCallback, string &asDeathHintCat, string &asDeathHintEntry, bool abKeepPlayerInLimbo);
	static void __stdcall ReleasePlayerFromLimbo();

	// Map
	static void __stdcall ChangeMap(string& asMapName, string& asStartPos, string& asStartSound, string& asEndSound);
	static void __stdcall ClearSavedMaps();
	
	/**
	 * This caches all current textures and models and they are not released until destroy is called. If there is already cached data it is destroyed.
	 */
	static void __stdcall CreateDataCache();
	static void __stdcall DestroyDataCache();
	
	static void __stdcall UnlockAchievement(string& asName);

	/**
	 * Looked up in lang as ("Levels", asNameEntry)
	 */
	static void __stdcall SetMapDisplayNameEntry(string& asNameEntry);

	static void __stdcall SetSkyBoxActive(bool abActive);
	static void __stdcall SetSkyBoxTexture(string& asTexture);
	static void __stdcall SetSkyBoxColor(float afR, float afG, float afB, float afA);

	static void __stdcall SetFogActive(bool abActive);
	static void __stdcall SetFogColor(float afR, float afG, float afB, float afA);
	static void __stdcall SetFogProperties(float afStart, float afEnd, float afFalloffExp, bool abCulling);
	
	/**
	 * If alRandomNum > 1, then it will randomize between 1 and alRandom for each LoadScreen giving entry the suffix XX (eg 01). If <=1 then no suffix is added
	 */
	static void __stdcall SetupLoadScreen(string &asTextCat, string &asTextEntry, int alRandomNum, string &asImageFile);
	
	
	// Effect
	static void __stdcall FadeIn(float afTime);
	static void __stdcall FadeOut(float afTime);
	static void __stdcall FadeImageTrailTo(float afAmount, float afSpeed);
	static void __stdcall FadeSepiaColorTo(float afAmount, float afSpeed);
	static void __stdcall FadeRadialBlurTo(float afSize, float afSpeed);
	static void __stdcall SetRadialBlurStartDist(float afStartDist);
	static void __stdcall StartEffectFlash(float afFadeIn, float afWhite, float afFadeOut);
	static void __stdcall StartEffectEmotionFlash(string &asTextCat, string &asTextEntry, string &asSound);

    static void __stdcall ShowScreenImage(string & asImageName, float afX, float afY, float afScale, bool abUseRelativeCoordinates, float afDuration, float afFadeIn, float afFadeOut);
	static void __stdcall HideScreenImageImmediately();
	static void __stdcall HideScreenImageWithFade(float afFadeOut);
	
	//This adds a voice + effect to be played. It is okay to call this many times in order to play many voices in a row. The EffectVoiceOverCallback is not called until ALL voices have finished.
	static void __stdcall AddEffectVoice(	string& asVoiceFile, string& asEffectFile,
											string& asTextCat, string& asTextEntry, bool abUsePostion, 
											string& asPosEnitity, float afMinDistance, float afMaxDistance);

	static void __stdcall AddEffectVoice2(	string& asVoiceFile, string& asEffectFile,
											string& asTextCat,
											string& asTextEntry, float afTextDelay,
											string& asText2Entry, float afText2Delay,
											bool abUsePostion, string& asPosEnitity, float afMinDistance, float afMaxDistance);
	
	static void __stdcall AddEffectVoice3(	string& asVoiceFile, string& asEffectFile,
											string& asTextCat,
											string& asTextEntry, float afTextDelay,
											string& asText2Entry, float afText2Delay,
											string& asText3Entry, float afText3Delay,
											bool abUsePostion, string& asPosEnitity, float afMinDistance, float afMaxDistance);

	static void __stdcall AddEffectVoice4(	string& asVoiceFile, string& asEffectFile,
											string& asTextCat,
											string& asTextEntry, float afTextDelay,
											string& asText2Entry, float afText2Delay,
											string& asText3Entry, float afText3Delay,
											string& asText4Entry, float afText4Delay,
											bool abUsePostion, string& asPosEnitity, float afMinDistance, float afMaxDistance);

	static void __stdcall AddEffectVoice5(	string& asVoiceFile, string& asEffectFile,
											string& asTextCat,
											string& asTextEntry, float afTextDelay,
											string& asText2Entry, float afText2Delay,
											string& asText3Entry, float afText3Delay,
											string& asText4Entry, float afText4Delay,
											string& asText5Entry, float afText5Delay,
											bool abUsePostion, string& asPosEnitity, float afMinDistance, float afMaxDistance);

	static void __stdcall AddEffectVoice6(	string& asVoiceFile, string& asEffectFile,
											string& asTextCat,
											string& asTextEntry, float afTextDelay,
											string& asText2Entry, float afText2Delay,
											string& asText3Entry, float afText3Delay,
											string& asText4Entry, float afText4Delay,
											string& asText5Entry, float afText5Delay,
											string& asText6Entry, float afText6Delay,
											bool abUsePostion, string& asPosEnitity, float afMinDistance, float afMaxDistance);

	static void __stdcall AddEffectVoice7(	string& asVoiceFile, string& asEffectFile,
											string& asTextCat,
											string& asTextEntry, float afTextDelay,
											string& asText2Entry, float afText2Delay,
											string& asText3Entry, float afText3Delay,
											string& asText4Entry, float afText4Delay,
											string& asText5Entry, float afText5Delay,
											string& asText6Entry, float afText6Delay,
											string& asText7Entry, float afText7Delay,
											bool abUsePostion, string& asPosEnitity, float afMinDistance, float afMaxDistance);

	static void __stdcall AddEffectVoiceExt(	string& asVoiceFile, string& asEffectFile,
											string& asTextCat, string& asTextEntry, bool abUsePostion, 
											string& asPosEnitity, float afMinDistance, float afMaxDistance, int alPriority, bool abStopInterrupted);
	static void __stdcall StopAllEffectVoices(float afFadeOutTime);
	static bool __stdcall GetEffectVoiceActive();
	/**
	* Syntax: void Func();
	*/
	static void __stdcall SetEffectVoiceOverCallback(string& asFunc);
	static bool __stdcall GetFlashbackIsActive();
		
	static void __stdcall StartPlayerSpawnPS(string& asSPSFile);
	static void __stdcall StopPlayerSpawnPS();

	static void __stdcall PlayGuiSound(string& asSoundFile, float afVolume);

	static void __stdcall StartScreenShake(float afAmount, float afTime, float afFadeInTime,float afFadeOutTime);
	

	// Insanity
	//static void __stdcall SetInsanitySetEnabled(string& asSet, bool abX);
	//static void __stdcall StartRandomInsanityEvent();
	//static bool __stdcall InsanityEventIsActive();
	

	// Player
	static void __stdcall SetPlayerActive(bool abActive);
	static void __stdcall ChangePlayerStateToNormal();
	static void __stdcall SetPlayerCrouching(bool abCrouch);
	static void __stdcall AddPlayerBodyForce(float afX, float afY, float afZ, bool abUseLocalCoords);
	static void __stdcall ShowPlayerCrossHairIcons(bool abX);
	
    static void __stdcall SetInfectionFauxMode(bool abFauxMode);
	static void __stdcall SetPlayerInfection(float afInfection);
	static void __stdcall AddPlayerInfection(float afInfection);
	static float __stdcall GetPlayerInfection();
	static void __stdcall SetPlayerHealth(float afHealth);
	static void __stdcall AddPlayerHealth(float afHealth);
	static float __stdcall GetPlayerHealth();

	static float __stdcall GetPlayerSpeed();
	static float __stdcall GetPlayerYSpeed();

	static void __stdcall MovePlayerForward(float afAmount);

	static void __stdcall SetPlayerPermaDeathSound(string& asSound);

	static void __stdcall GiveInfectionDamage(float afAmount, bool abUseEffect);
	/**
	 * Type can be: BloodSplat, Claws or Slash.
	 */
	static void __stdcall GivePlayerDamage(float afAmount, string& asType, bool abSpinHead, bool abLethal);
	static void __stdcall FadePlayerFOVMulTo(float afX, float afSpeed);
	static void __stdcall FadePlayerAspectMulTo(float afX, float afSpeed);
	static void __stdcall FadePlayerRollTo(float afX, float afSpeedMul, float afMaxSpeed);
	static void __stdcall FadePlayerPitchTo(float afX, float afSpeedMul, float afMaxSpeed);
	static void __stdcall MovePlayerHeadPos(float afX, float afY, float afZ, float afSpeed, float afSlowDownDist);
	static void __stdcall StartPlayerLookAt(string& asEntityName, float afSpeedMul, float afMaxSpeed, string &asAtTargetCallback);
	static void __stdcall StopPlayerLookAt();
	static void __stdcall SetPlayerMoveSpeedMul(float afMul);
	static void __stdcall SetPlayerRunSpeedMul(float afMul);
	static void __stdcall SetPlayerLookSpeedMul(float afMul);
	static void __stdcall SetPlayerJumpDisabled(bool abX);
	static void __stdcall SetPlayerCrouchDisabled(bool abX);
	static void __stdcall TeleportPlayer(string &asStartPosName);

	static void __stdcall SetLanternActive(bool abX, bool abUseEffects);
	static bool __stdcall GetLanternActive();
	static void __stdcall SetLanternDisabled(bool abX);

	static bool __stdcall GetJournalDisabled();
	static void __stdcall SetJournalDisabled(bool abX);

	static void __stdcall SetLanternFailureActive(bool abX);
    
	static void __stdcall SetLanternFailureChances(float afFlickerFailureChance, float afDeadFailureChance);
	static void __stdcall SetLanternDefaultFailureChances();

    static void __stdcall SetLanternTimeBetweenFailures(float afMinTimeBetweenFailures, float afMaxTimeBetweenFailures);
	static void __stdcall SetLanternDefaultTimesBetweenFailure();

    static void __stdcall SetLanternFlickerTimes(float afMinTimeInFailureFlickering, float afMaxTimeInFailureFlickering);
	static void __stdcall SetLanternDefaultFlickerTimes();

    static void __stdcall SetLanternFailureToFailureTransitionChances( float afChanceFlickerComesAfterDeadFailure, float afChanceDeadComesAfterFlickerFailure );
    static void __stdcall SetLanternDefaultFailureToFailureTransitionChances();

    static void __stdcall SetLanternDeadTimes(float afMinTimeInFailureDead, float afMaxTimeInFailureDead);
	static void __stdcall SetLanternDefaultDeadTimes();

	static void __stdcall StartLanternNoFailure();
	static void __stdcall StartLanternFailureDead();
	static void __stdcall StartLanternFailureFlickering();

    static void __stdcall StartLanternNoFailureWithDuration(float afDuration);
	static void __stdcall StartLanternFailureDeadWithDuration(float afDuration);
	static void __stdcall StartLanternFailureFlickeringWithDuration(float afDuration);

	/**
	 * Syntax: MyFun(bool abLit)
	 */
	static void __stdcall SetLanternLitCallback(string &asCallback);
	static void __stdcall SetLanternFlickerActive(bool abX);
	/**
	* if time is <=0 then the life time is calculated based on string length.
	*/
	static void __stdcall SetMessage(string &asTextCategory, string &asTextEntry, float afTime);
	static void __stdcall SetDeathHint(string &asTextCategory, string &asTextEntry);
	/**
	 * This must be called directly before player is killed! The variable as soon as player dies too.
	 */
	static void __stdcall DisableDeathStartSound();


	// Journal
	static void __stdcall AddNote(string& asNameAndTextEntry, string& asImage);
	static void __stdcall AddDiary(string& asNameAndTextEntry, string& asImage);
	static void __stdcall AddHint(string& asNameAndTextEntry, string& asImage);

	static void __stdcall RemoveAllHints();
	static void __stdcall RemoveHint(string& asNameAndTextEntry);
	/**
	 * Only called in the pickup diary callback! If true the journal displays the entry else not.
	 */
	static void __stdcall ReturnOpenJournal(bool abOpenJournal);
	

	// Quest
	static void __stdcall AddQuest(string& asName, string& asNameAndTextEntry);
	static void __stdcall CompleteQuest(string& asName, string& asNameAndTextEntry);
	static bool __stdcall QuestIsCompleted(string& asName);
	static bool __stdcall QuestIsAdded(string& asName);
	static void __stdcall SetNumberOfQuestsInMap(int alNumberOfQuests);

	// Hint
	/**
	* if time is <=0 then the life time is calculated based on string length.
	*/
	static void __stdcall GiveHint(string& asName, string& asMessageCat, string& asMessageEntry, float afTimeShown);
	//static void __stdcall RemoveHint(string& asName);
	static void __stdcall BlockHint(string& asName);
	static void __stdcall UnBlockHint(string& asName);
	
	// Inventory
	static void __stdcall ExitInventory();
	static void __stdcall SetInventoryDisabled(bool abX);
	/**
	* if life time is <=0 then the life time is calculated based on string length.
	*/
	static void __stdcall SetInventoryMessage(string &asTextCategory, string &asTextEntry, float afTime);
	
	static void __stdcall GiveItem(string& asName, string& asType, string& asSubTypeName, string& asImageName, float afAmount);
	static void __stdcall RemoveItem(string& asName);
	static bool __stdcall HasItem(string& asName);

	//This is meant to be used for debug mostly as it creates the actual item and then destroys i.
	static void __stdcall GiveItemFromFile(string& asName, string& asFileName);
	
	/**
	* Callback syntax: MyFunc(string &in asItemA, string &in asItemB)
	*/
	static void __stdcall AddCombineCallback(string& asName, string& asItemA, string& asItemB, string& asFunction, bool abAutoDestroy);
	static void __stdcall RemoveCombineCallback(string& asName);

	/**
	 * Callback syntax: MyFunc(string &in asItem, string &in asEntity)
	 */
	static void __stdcall AddUseItemCallback(string& asName, string& asItem, string& asEntity, string& asFunction, bool abAutoDestroy);
	static void __stdcall RemoveUseItemCallback(string& asName);

	// Engine data
	static void __stdcall PreloadParticleSystem(string& asPSFile);
	static void __stdcall PreloadSound(string& asSoundFile);

	static void __stdcall CreateParticleSystemAtEntity(string& asPSName, string& asPSFile, string& asEntity, bool abSavePS);
	static void __stdcall CreateParticleSystemAtEntityExt(	string& asPSName, string& asPSFile, string& asEntity, bool abSavePS, float afR, float afG, float afB, float afA,
															bool abFadeAtDistance, float afFadeMinEnd, float afFadeMinStart, float afFadeMaxStart, float afFadeMaxEnd);
	static void __stdcall DestroyParticleSystem(string& asName);
	static void __stdcall DestroyParticleSystemInstantly(string& asName);

	//asEntity can be "Player". If abSaveSound = true the sound is never attached to the entity! Also note that saving should on be used on looping sounds!
	static void __stdcall PlaySoundAtEntity(string& asSoundName, string& asSoundFile, string& asEntity, float afFadeTime, bool abSaveSound);
	static void __stdcall PlaySoundAtPosition(string& asSoundName, string& asSoundFile, float afX, float afY, float afZ, float afFadeTime, bool abSaveSound);
	static void __stdcall FadeInSound(string& asSoundName, float afFadeTime, bool abPlayStart);
	static void __stdcall StopSound(string& asSoundName, float afFadeTime);
	
	static void __stdcall PlayMusic(string& asMusicFile, bool abLoop, float afVolume, float afFadeTime, int alPrio, bool abResume);
	static void __stdcall StopMusic(float afFadeTime, int alPrio);
	
	static void __stdcall FadeGlobalSoundVolume(float afDestVolume, float afTime);
	static void __stdcall FadeGlobalSoundSpeed(float afDestSpeed, float afTime);
	
	static void __stdcall SetLightVisible(string& asLightName, bool abVisible);
	/**
	 * -1 for color or radius means keeping the value.
	 */
	static void __stdcall FadeLightTo(string& asLightName, float afR, float afG, float afB, float afA, float afRadius, float afTime);
	static void __stdcall SetLightFlickerActive(string& asLightName, bool abActive);

	static void __stdcall FadeLampTo(string& asName, float afR, float afG, float afB, float afA, float afRadius, float afTime);
	static void __stdcall SetLampFlickerActive(string& asName, bool abActive);

	static void __stdcall SetPhysicsAutoDisable(string& asName, bool abAutoDisable);

	static void __stdcall SetParticleSystemActive(string& asName, bool abActive);


	static void __stdcall SetEnemyActivationDistance(string& asName, float afX);
	// Entity properties
	static void __stdcall SetEntityActive(string& asName, bool abActive);
	static bool __stdcall GetEntityExists(string& asName);
	static bool __stdcall GetEntityActive(string& asName);
	/**
	* CrossHair can be: Default (uses default), Grab, Push, Ignite, Pick, LevelDoor, Ladder
	 */
	static void __stdcall SetEntityCustomFocusCrossHair(string& asName, string &asCrossHair);
	static void __stdcall CreateEntityAtArea(string& asEntityName, string& asEntityFile, string& asAreaName, bool abFullGameSave, float afPosX, float afPosY, float afPosZ, float afRotX, float afRotY, float afRotZ);
	/**
	* Callback syntax: MyFunc(string &in entity, int alState) state: 1=looking, -1=not looking
	*/
	static void __stdcall SetEntityPlayerLookAtCallback(string& asName, string& asCallback, bool abRemoveWhenLookedAt);
	/**
	* Callback syntax: MyFunc(string &in entity)
	*/
	static void __stdcall SetEntityPlayerInteractCallback(string& asName, string& asCallback, bool abRemoveOnInteraction);
	/**
	 * Callback syntax: MyFunc(string &in entity, string &in type). Type depends on entity type and includes: "OnPickup", "Break", "OnIgnite", etc
	 */
	static void __stdcall SetEntityCallbackFunc(string& asName, string& asCallback);
	/**
	* A callback called when ever the connection state changes (button being switched on). Syntax: void Func(string &in EntityName, int alState). alState: -1=off, 0=between 1=on
	*/
	static void __stdcall SetEntityConnectionStateChangeCallback(string& asName, string& asCallback);
	static void __stdcall SetEntityInteractionDisabled(string& asName, bool abDisabled);
	/**
	 * This function does NOT support asterix!
	 */
	static bool __stdcall GetEntitiesCollide(string& asEntityA, string& asEntityB);

	static void __stdcall SetPropEffectActive(string& asName, bool abActive, bool abFadeAndPlaySounds);
	static void __stdcall SetPropActiveAndFade(string& asName, bool abActive, float afFadeTime);
	static void __stdcall SetPropStaticPhysics(string& asName, bool abX);
	static bool __stdcall GetPropIsInteractedWith(string& asName);
	/**
	 * Rotates the prop up to a set speed. If OffsetArea = "", then center of body is used.
	 */
	static void __stdcall RotatePropToSpeed(string& asName, float afAcc, float afGoalSpeed, float afAxisX, float afAxisY, float afAxisZ, bool abResetSpeed, string& asOffsetArea);	
	static void __stdcall StopPropMovement(string& asName);	

	static void __stdcall AttachPropToBone(string& asChildEntityName, string& asParentEntityName, string& asParentBoneName, float fPosX, float fPosY, float fPosZ, float fRotX, float fRotY, float fRotZ);
	static void __stdcall DetachPropFromBone(string& asChildEntityName);

	static void __stdcall AddAttachedPropToProp(string& asPropName, string& asAttachName, string& asAttachFile, float afPosX, float afPosY, float afPosZ, float afRotX, float afRotY, float afRotZ);
	static void __stdcall RemoveAttachedPropFromProp(string& asPropName, string& asAttachName);

	static void __stdcall SetLampCanBeLitByPlayer(string& asName, bool abX);
	                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             
	static void __stdcall SetLampLit(string& asName, bool abLit, bool abEffects);
	static void __stdcall SetSwingDoorLocked(string& asName, bool abLocked, bool abEffects);
	static void __stdcall SetSwingDoorClosed(string& asName, bool abClosed, bool abEffects);
	static void __stdcall SetSwingDoorDisableAutoClose(string& asName, bool abDisableAutoClose);
	static void __stdcall SetSwingDoorOpenAmount(string& asName, float afOpenAmount, float afDuration, bool abOpenTowardsMaxAngle);
	static bool __stdcall GetSwingDoorLocked(string &asName);
	static bool __stdcall GetSwingDoorClosed(string &asName);
	/**
	 * -1 = angle is close to 0, 1=angle is 70% or higher of max, 0=inbetween -1 and 1.
	 */
	static int __stdcall GetSwingDoorState(string &asName);
	static void __stdcall SetLevelDoorLocked(string& asName, bool abLocked);
	static void __stdcall SetLevelDoorLockedSound(string& asName, string& asSound);
	static void __stdcall SetLevelDoorLockedText(string& asName, string& asTextCat, string& asTextEntry);
	/**
	* State: 0=not stuck 1 = at max -1= at min
	*/
	static void __stdcall SetPropObjectStuckState(string& asName, int alState);
	
	static void __stdcall SetWheelAngle(string& asName, float afAngle, bool abAutoMove);
	static void __stdcall SetWheelStuckState(string& asName, int alState, bool abEffects);
	static void __stdcall SetLeverStuckState(string& asName, int alState, bool abEffects);
	static void __stdcall SetWheelInteractionDisablesStuck(string& asName, bool abX);
	static void __stdcall SetLeverInteractionDisablesStuck(string& asName, bool abX);
	static int __stdcall GetLeverState(string& asName);
	
	static void __stdcall SetMultiSliderStuckState(string& asName, int alStuckState, bool abEffects);
	/**
	 * Called when state changes Syntax: MyFunc(string &in asEntity, int alState)
	 */
	static void __stdcall SetMultiSliderCallback(string& asName, string& asCallback);

	static void __stdcall SetButtonSwitchedOn(string& asName, bool abSwitchedOn, bool abEffects);
    static void __stdcall SetButtonCanBeSwitchedOn(string& asName, bool abCanBeSwitchedOn);
    static void __stdcall SetButtonCanBeSwitchedOff(string& asName, bool abCanBeSwitchedOff);
	static void __stdcall SetAllowStickyAreaAttachment(bool abX);
	static void __stdcall AttachPropToStickyArea(string& asAreaName, string& asProp);
	static void __stdcall AttachBodyToStickyArea(string& asAreaName, string& asBody);
	static void __stdcall DetachFromStickyArea(string& asAreaName);
	static void __stdcall SetNPCAwake(string& asName, bool abAwake, bool abEffects);
	static void __stdcall SetNPCFollowPlayer(string& asName, bool abX);

	static void __stdcall AttachAreaToProp(string& asAreaName, string& asProp, int alBody);

	static void __stdcall SetEnemyDisabled(string& asName, bool abDisabled);
	static void __stdcall SetEnemyIsHallucination(string& asName, bool abX);
	static void __stdcall FadeEnemyToSmoke(string& asName, bool abPlaySound);
	static void __stdcall ShowEnemyPlayerPosition(string& asName);
	static void __stdcall ForceEnemyWaitState(string& asName);
	static void __stdcall SetEnemyDisableTriggers(string& asName, bool abX);
	static void __stdcall AddEnemyPatrolNode(string& asName, string& asNodeName, float afWaitTime, string& asAnimation, bool abLoopAnimation);
	static void __stdcall ClearEnemyPatrolNodes(string& asEnemyName);
	static void __stdcall PlayEnemyAnimation(string & asEnemyName, string & asAnimName, bool abLoop, float afFadeTime);
	static void __stdcall TeleportEnemyToNode(string & asEnemyName, string & asNodeName, bool abChangeY);
	static float __stdcall GetEnemyPlayerDistance(string & asEnemyName);
	static bool __stdcall GetPlayerCanSeeEnemy(string & asEnemyName); 

	static void __stdcall ChangeEnemyPose(string& asName, string& asPoseType);
	static void __stdcall SetEnemyMoveType(string& asName, string& asMoveType );
	static void __stdcall SetManPigType(string& asName, string& asManPigType);
	static void __stdcall SendEnemyTimeOut(string& asName, float afTimeOut );
	static void __stdcall StopAnimationAndContinue(string& asName, float afTimeOut );
	static void __stdcall PlayScriptedAnimation(string& asName, string& asAnimationName, bool abLoopAnimation);

	static void __stdcall SetEnemyBlind(string& asName, bool abX);
	static void __stdcall SetEnemyDeaf(string& asName, bool abX);
	
	static void __stdcall SetTeslaPigFadeDisabled(string& asName, bool abX);
	static void __stdcall SetTeslaPigSoundDisabled(string& asName, bool abX);
	static void __stdcall SetTeslaPigEasyEscapeDisabled(string& asName, bool abX);
	static void __stdcall ForceTeslaPigSighting(string& asName);

	static int __stdcall GetEnemyState(string& asName);
	static string& __stdcall GetEnemyStateName(string& asName);

    static void __stdcall SetPlayerUsesDragFootsteps(bool abX);

	static int __stdcall StringToInt(string& asString);

	static void __stdcall SetPropHealth(string& asName, float afHealth);
	static void __stdcall AddPropHealth(string& asName, float afHealth);
	static float __stdcall GetPropHealth(string& asName);
	static void __stdcall ResetProp(string& asName);
	/**
	 * Callback syntax: MyFunc(string &in asProp)
	 */
	static void __stdcall PlayPropAnimation(string& asProp, string& asAnimation, float afFadeTime, bool abLoop, string &asCallback);
	static void __stdcall StopPropAnimation(string& asProp);
	static void __stdcall PlayCurrentAnimation(string& asProp, float afFadeTime, bool abLoop);
	static void __stdcall PauseCurrentAnimation(string& asProp, float afFadeTime);
	static void __stdcall SetPropAnimationSpeed(string& asProp, float afSpeed);
	static void __stdcall SetPropAnimationPosition(string& asProp, float afPos);
	
	/**
	 * State is 0 -1 where 0 is close and 1 is open. Any intermediate value is also valid!
	 */
	static void __stdcall SetMoveObjectState(string& asName, float afState);
	static void __stdcall SetMoveObjectStateExt(string& asName, float afState, float afAcc, float afMaxSpeed, float afSlowdownDist, bool abResetSpeed);


	static void __stdcall AddPropForce(string& asName, float afX, float afY, float afZ, string& asCoordSystem);
	static void __stdcall AddPropImpulse(string& asName, float afX, float afY, float afZ, string& asCoordSystem);
	static void __stdcall AddBodyForce(string& asName, float afX, float afY, float afZ, string& asCoordSystem);
	static void __stdcall AddBodyImpulse(string& asName, float afX, float afY, float afZ, string& asCoordSystem);
	/**
	* Do not use this on joints in SwingDoors, Levers, Wheels, etc where the joint is part of an interaction. That will make the game crash.
	*/
	static void __stdcall BreakJoint(string& asName);

	// Parent can have asterix in name but mot child! Entity callbacks alStates = 1=only enter, -1=only leave 0=both. Syntax: void Func(string &in asParent, string &in asChild, int alState). alState: 1=enter, -1=leave.
	static void __stdcall AddEntityCollideCallback(string& asParentName, string& asChildName, string& asFunction, bool abDeleteOnCollide, int alStates);
	// Parent can have asterix in name but mot child!
	static void __stdcall RemoveEntityCollideCallback(string& asParentName, string& asChildName);


    // Entity connections
	static void __stdcall InteractConnectPropWithRope(	string& asName, string& asPropName, string& asRopeName, bool abInteractOnly,
														float afSpeedMul, float afToMinSpeed, float afToMaxSpeed,
														bool abInvert, int alStatesUsed);
	/**
	 * This one should only be used if there must be an exact correspondance to prope "amount" and the moveobject open amount. It is best used for Wheel-door connections!
	 */
	static void __stdcall InteractConnectPropWithMoveObject(string& asName, string& asPropName, string& asMoveObjectName, bool abInteractOnly,
															bool abInvert, int alStatesUsed);
	/**
	 * Callback Syntax: MyFunc(string &in asConnectionName, string &in asMainEntity, string &in asConnectEntity, int alState). State is what is sent to connection entity and will be inverted if abInvertStateSent=true!
	 */
	static void __stdcall ConnectEntities(string& asName, string& asMainEntity, string& asConnectEntity, bool abInvertStateSent, int alStatesUsed, string& asCallbackFunc);


	// TEMP
	/*static void __stdcall CreateRope(	string& asName, 
										string& asStartArea, string& asEndArea, 
										string& asStartBody, string& asEndBody,
										float afMinTotalLength, float afMaxTotalLength,
										float afSegmentLength, float afDamping,
										float afStrength, float afStiffness,
										string& asMaterial, float afRadius,
										float afLengthTileAmount, float afLengthTileSize,
										string& asSound,float afSoundStartSpeed, float afSoundStopSpeed,
										bool abAutoMove, float afAutoMoveAcc, float afAutoMoveMaxSpeed
										);*/
};

//----------------------------------------------

#endif // LUX_SCRIPT_HANDLER_H
