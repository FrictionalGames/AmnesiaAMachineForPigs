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

#ifndef LUX_PLAYER_HELPERS_H
#define LUX_PLAYER_HELPERS_H

//----------------------------------------------

#include "LuxBase.h"

//----------------------------------------------

class cLuxPlayer;

//----------------------------------------------

class cLuxPlayerStamina : public iLuxPlayerHelper
{
public:
	cLuxPlayerStamina(cLuxPlayer *apPlayer);
	~cLuxPlayerStamina();

	void Update(float afTimeStep);

	float GetExhaustionFactor();

private:

	float
        mfTimeSinceLastSound,
		mfStaminaTimer,
        mfSprintSlowdownStartTime,
		mfSprintExhaustionTime,
		mfExhaustionSpeedMultiplier,
		mfSprintRecoverySpeed;
};

//----------------------------------------------

class cLuxPlayerVoiceFlashback : public iLuxPlayerHelper
{
friend class cLuxPlayer_SaveData;
public:
	cLuxPlayerVoiceFlashback(cLuxPlayer *apPlayer);
	~cLuxPlayerVoiceFlashback();

	void Update(float afTimeStep);
	
    bool AllowsJump() { return !( mbIsPlaying && ( mbObstructMovement ) ); }

    void StopPlaying();
    void Start(
		const tString & asVoiceFile,
		const tString & asTextEntry1, float afDelay1,
		const tString & asTextEntry2, float afDelay2,
		const tString & asTextEntry3, float afDelay3,
		const tString & asTextEntry4, float afDelay4,
		const tString & asTextEntry5, float afDelay5,
		const tString & asTextEntry6, float afDelay6,
		const tString & asTextEntry7, float afDelay7,
		bool abUseEffects, bool abObstructMovement );
    void StartEffects();
    void StopEffects();
    void StopMovementObstruction();
    void StartMovementObstruction();
    void SetStopSound( const tString& asStopSound ) { m_sStopSound = asStopSound; }
    bool GetIsPlaying() { return mbIsPlaying; }

    void SetMoveSpeedMultiplier( float afMoveSpeedMultiplier ) { mfMoveSpeedMultiplier = afMoveSpeedMultiplier; }
    void SetMouseSensitivityModifier( float afMouseSensitivityModifier ) { mfMouseSensitivityModifier = afMouseSensitivityModifier; }
	
private:

    bool
    	mbIsDelaying,
    	mbIsPlaying,
        mbUseEffects,
        mbObstructMovement;
    float
		mfEffectFadeInTime,
		mfEffectFadeOutTime,
		mfSepiaAmount,
		mfLightFadeAmount,
		mfImageTrailAmount,
		mfBlurStartDistance,
		mfBlurAmount,
		mfFovMultiplier,
		mfMoveSpeedMultiplier,
		mfMouseSensitivityModifier;
    tString
        m_sStopSound;
};

class cLuxPlayerInfectionCollapse : public iLuxPlayerHelper
{
friend class cLuxPlayer_SaveData;
public:
	cLuxPlayerInfectionCollapse(cLuxPlayer *apPlayer);
	~cLuxPlayerInfectionCollapse();

	void Reset();

	void Start();
	void Stop();
	bool IsActive(){ return mbActive; }

	void Update(float afTimeStep);

private:
	bool mbActive;

	int mlState;

	float mfHeightAddCollapseSpeed;
	float mfHeightAddAwakeSpeed;
	float mfRollCollapseSpeed;
	float mfRollAwakeSpeed;

	float mfHeightAddGoal;
	
	float mfAwakenInfection;
    
	float mfSleepTime;

	float mfSleepSpeedMul;
	float mfWakeUpSpeedMul;
	
	tString msStartSound;
	tString msAwakenSound;
	tString msSleepLoopSound;
	float mfSleepLoopSoundVolume;

	tString msSleepRandomSound;
	float mfSleepRandomMinTime;
	float mfSleepRandomMaxTime;
	
	float mfHeightAdd;
	float mfRoll;

	float mfTimer;
	float mfRandomCount;

	cSoundEntry *mpLoopSound;
	int mlLoopSoundID;
};

//----------------------------------------------

class cLuxPlayerCamDirEffects : public iLuxPlayerHelper
{
public:
	cLuxPlayerCamDirEffects(cLuxPlayer *apPlayer);
	~cLuxPlayerCamDirEffects();

	void Reset();

	void Update(float afTimeStep);

	float AddAndGetYawAdd(float afX);
	float AddAndGetPitchAdd(float afX);

private:
	void SetSwayActive(bool abX);
	void UpdateSway(float afTimeStep);

	float mfStartSwayMinInfection;

	bool mbSwayActive;

	tVector2fList mlstPrevAdd;
	cVector2f mvSwayAdd;
	int mlMaxPositions;

	float mfSwayAlpha;

	cVector2f mvNextAdd;
};

//----------------------------------------------

class cLuxPlayerSpawnPS_SpawnPos
{
public:
	cParticleSystem *mpPS;
	cVector3f mvPos;
	cVector3f mvLastLocalPos;
};

class cLuxPlayerSpawnPS : public iLuxPlayerHelper
{
public:
	cLuxPlayerSpawnPS (cLuxPlayer *apPlayer);
	~cLuxPlayerSpawnPS();

	void Reset();

	void Start(const tString& asFileName);
	void Stop();

	void RespawnAll();

	bool IsActive(){ return mbActive;}
	const tString& GetFileName(){ return msFileName;}

	void Update(float afTimeStep);
private:
	void GenerateAllSpawnPos();
	cParticleSystem* CreatePS(cLuxPlayerSpawnPS_SpawnPos *apSpawnPos);

	void DestroyAllSpawnPoints();

	bool LoadSpawnPSFile(const tString& asFileName);

	bool mbActive;
	tString msFileName;
	
	tString msParticleSystem;
	float mfHeightFromFeet;
	float mfHeightAddMin;
	float mfHeightAddMax;
	float mfDensity;
	float mfRadius;
	cColor mPSColor;
	bool mbFadePS;
	float mfPSMinFadeStart;
	float mfPSMinFadeEnd;
	float mfPSMaxFadeStart;
	float mfPSMaxFadeEnd;

	std::vector<cLuxPlayerSpawnPS_SpawnPos> mvSpawnPos;
};

//----------------------------------------------

class cLuxPlayerHurt : public iLuxPlayerHelper
{
public:
	cLuxPlayerHurt (cLuxPlayer *apPlayer);
	~cLuxPlayerHurt();

	void Reset();

	void Update(float afTimeStep);

	void OnDraw(float afFrameTime);

private:
	std::vector<cGuiGfxElement*> mvNoiseGfx;
	float mfEffectStartHealth;	
	float mfMinMoveMul;
	float mfMaxPantCount;
	float mfPantSpeed;
	float mfPantSize;

	int mlCurrentNoise;
	float mfNoiseUpdateCount;

	float mfNoiseAlpha;
	float mfNoiseFreq;
	cColor mNoiseColor;
	
	float mfPantCount;
	float mfPantPosAdd;
	float mfPantPosAddVel;
	float mfPantPosAddDir;
	float mfAlpha;

	float mfHealthRegainSpeed;
	float mfHealthRegainLimit;
};

//----------------------------------------------

class cLuxFlashbackData
{
public:
	cLuxFlashbackData(){}
	cLuxFlashbackData(const tString &asFile, const tString &asCallback) : msFile(asFile), msCallback(asCallback) {}

	tString msFile;
	tString msCallback;
};

typedef std::list<cLuxFlashbackData> tLuxFlashbackDataList;
typedef tLuxFlashbackDataList::iterator tLuxFlashbackDataListIt;

//----------------------------------------------

class cLuxPlayerFlashback : public iLuxPlayerHelper
{
friend class cLuxPlayer_SaveData;
public:
	cLuxPlayerFlashback (cLuxPlayer *apPlayer);
	~cLuxPlayerFlashback ();

	void Reset();
	
	void Start(const tString &asFlashbackFile, const tString &asCallback);

	void Update(float afTimeStep);

	void OnDraw(float afFrameTime);

	bool IsActive(){ return mbActive; }

private:
	void LoadAndPlayFlashbackFile(const tString& asFlashbackFile);

	//Data
	float mfRadialBlurSize;
	float mfRadialBlurStartDist;
	float mfWorldSoundVolume;
	float mfMoveSpeedMul;
	float mfRunSpeedMul;

	//Variables
	float mfFlashDelay;
	bool mbActive;
	float mfFlashbackStartCount;
	tString msFlashbackFile;
	tString msCallback;
	tLuxFlashbackDataList mlstFlashbackQueue;
	
};

//---------------------------------------------

class cLuxPlayerLookAt : public iLuxPlayerHelper
{
friend class cLuxPlayer_SaveData;
public:
	cLuxPlayerLookAt(cLuxPlayer *apPlayer);
	~cLuxPlayerLookAt();

	void Update(float afTimeStep);

	void Reset();

	void SetTarget(const cVector3f &avTargetPos, float afSpeedMul, float afMaxSpeed, const tString& asAtTargetCallback);
	
	void SetActive(bool abX);
	bool IsActive(){ return mbActive;}

private:
	bool mbActive;
	cVector3f mvTargetPos;

	cVector3f mvCurrentSpeed;
	float mfSpeedMul;
	float mfMaxSpeed;

	tString msAtTargetCallback;

	float mfDestFovMul;
	float mfFov;
	float mfFovSpeed;
	float mfFovMaxSpeed;
};

//----------------------------------------------

class cLuxPlayerInfection : public iLuxPlayerHelper
{
friend class cLuxPlayer_SaveData;
public:
	cLuxPlayerInfection(cLuxPlayer *apPlayer);
	~cLuxPlayerInfection();

	void Reset();

	void StartHit();
	void StartInfectionIncreaseEffects();

	void Update(float afTimeStep);

	void OnDraw(float afFrameTime);

	float GetTimeAtHighInfection(){ return mfTimeAtHighInfection;}

    void SetFauxMode( bool abIsFauxMode ) { mbIsFauxMode = abIsFauxMode; }

private:
	float GetCurrentSizeMul();

	void UpdateInfectionEffects(float afTimeStep);
	void UpdateCheckEnemySeen(float afTimeStep);
	void UpdateHit(float afTimeStep);
	
	void UpdateInfectionVisuals(float afTimeStep);
	void UpdateEnemySeenEffect(float afTimeStep);
	void UpdateInfectionIncreaseSounds(float afTimeStep);
	void UpdateHighInfection(float afTimeStep);
	
	void UpdateStingersAndLoops(float afTimeStep);
	
	int mnPreviousSoundInfectionLevel;
	cSoundEntry *mpCurrentInfectionLoopSound;
	int mlCurrentInfectionLoopSoundID;

	float mfHitAlpha;
	bool mbHitActive;
    bool mbIsFauxMode;
	float mfInfectionIncreaseSoundEffectsTimer;

	float mfTimeAtHighInfection;

	bool mbHitIsUpdated;
	bool mbInfectionIncreaseSoundEffectsAreUpdated;

	float mfPantCount;

	float mfTimeUntilNextEnemySeenCheck;

	bool mbInfectionVisualEffectUpdated;

	float mfTimer;
	float mfInfectionWaveAlpha;

	float mfInfectionIncreaseSoundTimer;
	float mfInfectionIncreaseVolume;
	float mfInfectionIncreaseHeartbeatTimer;

	float mfEnemySeenTimer;
	bool mbEnemyIsSeen;

	float mfShowHintTimer;

	//////////////
	// Data
	float mfHitZoomInSpeed;
	float mfHitZoomOutSpeed;
	float mfHitZoomInFOVMul;
	float mfHitZoomInAspectMul;

	float mfInfectionDecreaseSpeed;
	int miInfectionDecreaseLimitLevel;
	float mfInfectionVeryHighLimit;
	float mfInfectionEffectsStart;

	float mfInfectionWaveAlphaMul;
	float mfInfectionWaveSpeedMul;

	float mfHighInfectionLimitForDeathTimer;
	float mfTimeUntilDeathAtHighInfection;

	float mfCheckEnemyNearOrSeenInterval;
	float mfNearEnemyIncrease;
	float mfNearCritterIncrease;
};

//----------------------------------------------

class cLuxPlayerLantern : public iLuxPlayerHelper
{
public:	
	cLuxPlayerLantern(cLuxPlayer *apPlayer);
	~cLuxPlayerLantern();

	void OnStart();
	void Update(float afTimeStep);
	void Reset();

	void OnMapEnter(cLuxMap *apMap);
	void OnMapLeave(cLuxMap *apMap);

	void CreateWorldEntities(cLuxMap *apMap);
	void DestroyWorldEntities(cLuxMap *apMap);

	void SetActive(bool abX, bool abUseEffects, bool abCheckForOilAndItems=true, bool abCheckIfAllowed=true);
	bool IsActive(){ return mbActive;}

	void SetDisabled(bool abX);
	bool GetDisabled(){ return mbDisabled;}

	iLight* GetLight(){ return mpLight;}

private:
	cColor mDefaultColor;
	float mfRadius;
	tString msGobo;
	bool mbCastShadows;
	cVector3f mvLocalOffset;
	tString msTurnOnSound;
	tString msTurnOffSound;
	tString msDisabledSound;
	
	bool mbDisabled;
	bool mbActive;
	float mfAlpha;
	cLightPoint *mpLight;
};

//----------------------------------------------

class cLuxPlayerDeath : public iLuxPlayerHelper
{
public:
	cLuxPlayerDeath(cLuxPlayer *apPlayer);
	~cLuxPlayerDeath();

	void LoadFonts();
	void LoadUserConfig();
	void SaveUserConfig();

	void Reset();

	void Start();

	void Update(float afTimeStep);

	void OnDraw(float afFrameTime);

	void OnPressButton();

	float GetFadeAlpha(){ return mfFadeAlpha;}

	void DisableStartSound(){ mbSkipStartSound = true; }

	bool ShowHint(){ return mbShowHint;}
	void SetShowHint(bool abX){ mbShowHint=abX;}

	void ReleasePlayerFromLimbo() { mbHoldsPlayerInLimbo = false; }

	void SetHint(const tString& asCat, const tString& asEntry);
	const tString& GetHintCat(){ return msHintCat; }
	const tString& GetHintEntry(){ return msHintEntry; }

	void SetKeepPlayerInLimbo( bool abKeepPlayerInLimbo ) { mbKeepPlayerInLimbo = abKeepPlayerInLimbo; }


private:
	void ResetGame();

	bool mbActive;

	bool mbKeepPlayerInLimbo;
	bool mbHoldsPlayerInLimbo;
	bool mbHasRunCheckPointCallbackScript;

	bool mbShowHint;

	int mlState;

	tString msStartSound;
	tString msAwakenSound;

	bool mbSkipStartSound;

	float mfHeightAddSpeed;
	float mfRollSpeed;

	float mfHeightAddGoal;
	float mfHeightAddGoalCrouch;
	float mfFadeOutTime;

	float mfMaxSanityGain;
	float mfMaxHealthGain;
	float mfMaxOilGain;
	float mfMinSanityGain;
	float mfMinHealthGain;
	float mfMinOilGain;

	float mfHeightAdd;
	float mfRoll;

	float mfTimer;

	float mfMinHeightAdd;

	tString msHintCat;
	tString msHintEntry;

	tWString msCurrentHintText;

	cGuiGfxElement *mpWhiteModGfx;

	iFontData *mpFont;
    	
	float mfFadeAlpha;
	float mfTextAlpha1;
	float mfTextAlpha2;
	float mfTextOnScreenCount;
	float mfWhiteCount;
	
	cLinearOscillation mFlashOscill;

	cSoundEntry *mpVoiceEntry;
	int mlVoiceEntryId;
};


//----------------------------------------------

class cLuxPlayerLean : public iLuxPlayerHelper
{
public:	
	cLuxPlayerLean(cLuxPlayer *apPlayer);
	~cLuxPlayerLean();
	
	void CreateWorldEntities(cLuxMap *apMap);
	void DestroyWorldEntities(cLuxMap *apMap);

	void Update(float afTimeStep);

	void Lean(float afMul);

	void Reset();
private:
	iCollideShape *mpHeadShape;

	float mfDir;
	float mfMovement;
	float mfRotation;

	float mfMaxMovement;
	float mfMaxRotation;
	float mfMaxTime;

	float mfMoveSpeed;

	bool mbIntersect;

	bool mbPressed;
};

//----------------------------------------------

class cLuxPlayerDamageData
{
public:
	std::vector<cGuiGfxElement*> mvImages;
};

class cLuxPlayerHudEffect_Splash
{
public:
	cGuiGfxElement *mpImage;	
	cVector3f mvPos;
	cVector2f mvSize;
	float mfAlpha;
	float mfAlphaMul;

	cVector3f mvPosVel;
	cVector2f mvSizeVel;
	float mfAlphaVel;
	float mfAlphaMoveStart;
};

typedef std::list<cLuxPlayerHudEffect_Splash> cLuxPlayerHudEffect_SplashList;
typedef cLuxPlayerHudEffect_SplashList::iterator cLuxPlayerHudEffect_SplashListIt;

class cLuxPlayerHudEffect : public iLuxPlayerHelper
{
public:
	cLuxPlayerHudEffect(cLuxPlayer *apPlayer);
	~cLuxPlayerHudEffect();

	void AddDamageSplash(eLuxDamageType aType);
	void Flash(const cColor& aColor, eGuiMaterial aFlashMaterial, float afInTime, float afOutTime);

	void OnDraw(float afFrameTime);
	void Update(float afTimeStep);
	void Reset();
	
private:
	void DrawSplashes(float afFrameTime);
	void UpdateSplashes(float afTimeStep);

	void DrawFlash(float afFrameTime);
	void UpdateFlash(float afTimeStep);

	void LoadDamageData(cLuxPlayerDamageData *apData, const tString& asName);

	std::vector<cLuxPlayerDamageData> mvDamageTypes;
	cLuxPlayerHudEffect_SplashList mlstSplashes;

	cGuiGfxElement *mpFlashGfx;
	cColor mFlashColor;
	eGuiMaterial mFlashMaterial;
	float mfFlashAlphaSpeed;
	float mfFlashAlphaOutSpeed;
	float mfFlashAlpha;
	bool mbFlashActive;
};

//----------------------------------------------

class cLuxPlayerLightLevel : public iLuxPlayerHelper
{
public:	
	cLuxPlayerLightLevel(cLuxPlayer *apPlayer);
	~cLuxPlayerLightLevel();
	
	void OnStart();
	void Update(float afTimeStep);
	void Reset();

	void OnMapEnter(cLuxMap *apMap);

	float GetExtendedLightLevel(){ return mfExtendedLightLevel;}
	float GetNormalLightLevel(){ return mfNormalLightLevel;}

private:
	float mfExtendedLightLevel;	//Uses longer range on point lights
	float mfNormalLightLevel;	//Uses normal radius
	float mfUpdateCount;

	float mfRadiusAdd;
};

//----------------------------------------------

class cLuxPlayerIsMoving : public iLuxPlayerHelper
{
public:	
	cLuxPlayerIsMoving(cLuxPlayer *apPlayer);
	~cLuxPlayerIsMoving();

	void OnStart();
	void Update(float afTimeStep);
	void Reset();

    bool PlayerIsMovingFast();
    bool PlayerIsMovingSlowly();

    //bool PlayerIsMovingMouseFast();

private:

    float
        mfFastMovementThreshold,
        mfSlowMovementThreshold,
        mfImmediatePlayerSpeed,
        mfAveragePlayerSpeed;
};

//----------------------------------------------

class cLuxPlayerStrobeHelper : public iLuxPlayerHelper
{

};

//----------------------------------------------
/*
class cLuxPlayerInDarkness : public iLuxPlayerHelper
{
public:	
	cLuxPlayerInDarkness(cLuxPlayer *apPlayer);
	~cLuxPlayerInDarkness();

	void OnStart();
	void Update(float afTimeStep);
	void Reset();

	void OnMapEnter(cLuxMap *apMap);
	void OnMapLeave(cLuxMap *apMap);

	void CreateWorldEntities(cLuxMap *apMap);
	void DestroyWorldEntities(cLuxMap *apMap);

	cLightPoint* GetAmbientLight(){ return mpAmbientLight;}

	bool InDarkness();

private:
	cSoundHandler *mpSoundHandler;

	float mfMinDarknessLightLevel;
	
	float mfAmbientLightMinLightLevel;
	float mfAmbientLightRadius;
	float mfAmbientLightIntensity;
	float mfAmbientLightFadeInTime;
	float mfAmbientLightFadeOutTime;
	cColor mAmbientLightColor;

	tString msLoopSoundFile;
	float mfLoopSoundVolume;
	float mfLoopSoundStartupTime;
	float mfLoopSoundFadeInSpeed;
	float mfLoopSoundFadeOutSpeed;

	bool mbInDarkness;
	float mfShowHintTimer;

	cSoundEntry *mpLoopSound;
	float mfLoopSoundCount;

	cLightPoint *mpAmbientLight;
	bool mbAmbientLightIsOn;
};
*/
//----------------------------------------------


#endif // LUX_MAP_HANDLER_H
