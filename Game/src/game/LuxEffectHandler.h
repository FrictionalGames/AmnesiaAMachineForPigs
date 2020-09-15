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

#ifndef LUX_EFFECT_HANDLER_H
#define LUX_EFFECT_HANDLER_H

//----------------------------------------------

#include "LuxBase.h"
#include <deque>

//----------------------------------------------


class iLuxEffect
{
public:
	iLuxEffect() : mbActive(false) {}
	~iLuxEffect(){}

	virtual void Update(float afTimeStep)=0;
	virtual void OnDraw(float afFrameTime)=0;
	virtual void Reset()=0;

	virtual void DoAction(eLuxPlayerAction aAction, bool abPressed){}

	float GetAmountForCurrentInfection();

	virtual bool IsAlwaysOn() { return false; }
	bool IsActive(){ return mbActive;}
	void SetActive(bool abX){ mbActive = abX;}

protected:
	bool mbActive;

	float
		mfValueAtInfectionLevelOne,
		mfValueAtInfectionLevelTwo,
		mfValueAtInfectionLevelThree,
		mfValueAtInfectionLevelFour;
};


//----------------------------------------------

class cLuxEffect_PlayCommentary : public iLuxEffect
{
public:
	cLuxEffect_PlayCommentary();
	~cLuxEffect_PlayCommentary();

	void Start(const tString &asTalker,const tString &asTopic, const tString &asFile, int alIconId);
	void Stop();
	
	void Update(float afTimeStep);
	void OnDraw(float afFrameTime);
	void Reset();

private:
	//////////////////////
	//Data
	cSoundHandler *mpSoundHandler;
	cMusicHandler *mpMusicHandler;
	
	//////////////////////
	//Variables
	cSoundEntry *mpSoundEntry;
	int mlSoundEntryID;

	tString msTalker;
	tString msTopic;
	int mlIconID;
};

//----------------------------------------------

class cLuxEffect_ScreenImage : public iLuxEffect
{
public:
	cLuxEffect_ScreenImage();
	~cLuxEffect_ScreenImage();

    void ShowImage(const tString & asImageName, float afX, float afY, float afScale, bool abUseRelativeCoordinates, float afDuration, float afFadeIn, float afFadeOut);
	void HideImmediately();
	void HideWithFade(float afFadeOut);

	void Update(float afTimeStep);
	void OnDraw(float afFrameTime);
    void Reset() {};

private:
	
    //////////////////////
	//Data
	cGuiSet * mpGuiSet;
	
	tString msTextureName;

    cGuiGfxElement * mpTextureGfx;

    cVector3f
        mvPosition;

    float
        mfFadeInDuration,
        mfShowDuration,
        mfFadeOutDuration,
        mfFadeTimer,
		mfScale,
        mfCurrentFade;
};


//----------------------------------------------

class cLuxEffect_EmotionFlash : public iLuxEffect
{
public:
	cLuxEffect_EmotionFlash();
	~cLuxEffect_EmotionFlash();

	void ClearFonts();
	void LoadFonts();

	void Start(const tString &asTextCat, const tString &asTextEntry, const tString &asSound);
	void Reset();
	
	void Update(float afTimeStep);
	void OnDraw(float afFrameTime);
	
	void DoAction(eLuxPlayerAction aAction, bool abPressed);

private:
	cGuiGfxElement *mpWhiteGfx;
	iFontData *mpFont;
	cVector2f mvFontSize;

	float mfAlpha;

	int mlStep;
	float mfCount;
	
	float mfTextAlpha;
	float mfTextTime;
	tWStringVec mvTextRows;
};

//----------------------------------------------

class cLuxEffect_RadialBlur : public iLuxEffect
{
	friend class cLuxEffectHandler_SaveData;
public:
	cLuxEffect_RadialBlur();

	void SetBlurStartDist(float afDist);
	void FadeTo(float afSize, float afSpeed);

	float GetCurrentSize(){ return mfSize;}
	virtual bool IsAlwaysOn() { return true; }

	float GetStartDistForCurrentInfection();

	void Update(float afTimeStep);
	void OnDraw(float afFrameTime){}
	void Reset();

private:
	float mfSize;
	float mfSizeGoal;
	float mfFadeSpeed;
	float mfBlurStartDist;

	float mfStartDistAtInfectionLevelOne;
	float mfStartDistAtInfectionLevelTwo;
	float mfStartDistAtInfectionLevelThree;
	float mfStartDistAtInfectionLevelFour;
};

//----------------------------------------------

//class cLuxEffect_ViewportImage : public iLuxEffect
//{
//	friend class cLuxEffectHandler_SaveData;
//public:
//	cLuxEffect_ViewportImage();
//
//	void ShowImage(string & imageFileName, float xPos, float yPos, float xSize, float ySize, bool applyPostProcessEffects); 
//
//	virtual bool IsAlwaysOn() { return false; }
//
//	void Update(float afTimeStep);
//	void OnDraw(float afFrameTime){}
//	void Reset();
//
//private:
//	
//};

//----------------------------------------------

class cLuxEffect_SepiaColor : public iLuxEffect
{
	friend class cLuxEffectHandler_SaveData;
public:
	cLuxEffect_SepiaColor();

	void FadeTo(float afAmount, float afSpeed);

	virtual bool IsAlwaysOn() { return true; }

	void Update(float afTimeStep);
	void OnDraw(float afFrameTime){}
	void Reset();

private:
	float mfAmount;
	float mfAmountGoal;
	float mfFadeSpeed;
};

class cLuxEffect_ColorGrading : public iLuxEffect
{
	friend class cLuxEffectHandler_SaveData;
public:
	cLuxEffect_ColorGrading();

	void InitializeLUT(tString asBaseEnvironmentLUT);
	void EnterLUTEnvironment(tString asEnvironmentLUT, float afFadeTime);
	void LeaveLUTEnvironment(tString asEnvironmentLUT);
	void FadeGameplayLUTTo(tString asEnvironmentLUT, float afFadeTime);
	void FadeOutGameplayLUT(float afFadeTime);

	virtual bool IsAlwaysOn() { return true; }

	void Update(float afTimeStep);
	void OnDraw(float afFrameTime){}
	void Reset();

private:

    void FadeFromTo( tString asFromTexture, tString asToTexture, float afFadeTime );

    bool
        mbIsCrossFading;

	tStringList msEnvironmentLUTs;
    tFloatList msEnvironmentLUTFadeTimes;

	tString msGameplayLUT;
    float mfGameplayFadeTime;

    tString msFadeTargetLUT;

    float mfCrossFadeAlpha;
    bool mbIsFadingUp;
    float mfFadeSpeed;
};

//-----------------------------------------

class cLuxEffect_ShakeScreen_Shake
{
public:
	float mfMaxSize;
	float mfSize;
	float mfTime;
	float mfFadeInTime;
	float mfMaxFadeInTime;
	float mfFadeOutTime;
	float mfMaxFadeOutTime;
};

class cLuxEffect_ShakeScreen : public iLuxEffect
{
public:
	cLuxEffect_ShakeScreen();
	~cLuxEffect_ShakeScreen();

	void Start(float afAmount, float afTime, float afFadeInTime,float afFadeOutTime);

	void Update(float afTimeStep);
	void OnDraw(float afFrameTime){}
	void Reset();

private:

	std::list<cLuxEffect_ShakeScreen_Shake> mlstShakes;
};

//----------------------------------------------

class cLuxEffect_ImageTrail : public iLuxEffect
{
friend class cLuxEffectHandler_SaveData;
public:
	cLuxEffect_ImageTrail();

	void FadeTo(float afAmount, float afSpeed);
	virtual bool IsAlwaysOn() { return true; }

	void Update(float afTimeStep);
	void OnDraw(float afFrameTime){}
	void Reset();

private:
	float mfAmount;
	float mfAmountGoal;
	float mfFadeSpeed;
};

//----------------------------------------------

class cLuxEffect_Fade : public iLuxEffect
{
friend class cLuxEffectHandler_SaveData;
public:
	cLuxEffect_Fade();
	~cLuxEffect_Fade();

	void FadeIn(float afTime, float afGoalAlpha = 0.0f);
	void FadeOut(float afTime, float afGoalAlpha = 1.0f);

	bool IsFading();

	void Update(float afTimeStep);
	void OnDraw(float afFrameTime);
	void Reset();

	void SetDirectAlpha(float afX);

	float GetCurrentAlpha() { return mfAlpha; }

private:
	cGuiGfxElement *mpWhiteGfx;
	
	float mfGoalAlpha;
	float mfAlpha;
	float mfFadeSpeed;
};
/*

//----------------------------------------------

class cLuxEffect_InfectionHealFlash : public iLuxEffect
{
public:
	cLuxEffect_InfectionHealFlash();
	~cLuxEffect_InfectionHealFlash();

	void Start();

	void Update(float afTimeStep);
	void OnDraw(float afFrameTime);
	void Reset();

	void DrawFlash(cGuiSet *apSet ,float afTimeStep);

private:
	cGuiGfxElement *mpWhiteGfx;

	float mfAlpha;

	int mlStep;
	float mfCount;

	float mfFadeInSpeed;
	float mfWhiteSpeed;
	float mfFadeOutSpeed;

	cColor mColor;
	tString msSound;
	float mfFadeInTime;
	float mfFadeOutTime;
};
*/
//----------------------------------------------

class cLuxEffect_Flash : public iLuxEffect
{
public:
	cLuxEffect_Flash();
	~cLuxEffect_Flash();
	
	void Start(float afFadeIn, float afWhite, float afFadeOut);
	void Reset();

	void Update(float afTimeStep);
	void OnDraw(float afFrameTime);

private:
	cGuiGfxElement *mpWhiteGfx;

	float mfAlpha;

	int mlStep;
	float mfCount;

	float mfFadeInSpeed;
	float mfWhiteSpeed;
	float mfFadeOutSpeed;
};

//----------------------------------------------

typedef std::list<cLuxVoiceData> tLuxVoiceDataList;
typedef tLuxVoiceDataList::iterator tLuxVoiceDataListIt;

class cLuxEffect_PlayVoice : public iLuxEffect
{
friend class cLuxEffectHandler_SaveData;
public:
	cLuxEffect_PlayVoice();
	~cLuxEffect_PlayVoice();

	void StopVoices(float afFadeOutSpeed);
	void AddVoice(	const tString& asVoiceFile, const tString& asEffectFile,
					const tString& asTextCat, const tString& asTextEntry, bool abUsePostion, 
					const cVector3f& avPosition, float afMinDistance, float afMaxDistance, int alPriority = 0, bool abRemoveInterrupted = true);

	// yes, there are much more elegant ways, but I have a 2 hour deadline and a save system that doesn't like stringlists.
	void AddMultiSubbedVoice(
		const tString& asVoiceFile, const tString& asEffectFile, const tString& asTextCat,
		const tString& asTextEntry, float afTextDelay,
		const tString& asText2Entry, float afText2Delay,
		const tString& asText3Entry, float afText3Delay,
		const tString& asText4Entry, float afText4Delay,
		const tString& asText5Entry, float afText5Delay,
		const tString& asText6Entry, float afText6Delay,
		const tString& asText7Entry, float afText7Delay,
		bool abUsePostion, 
		const cVector3f& avPosition, float afMinDistance, float afMaxDistance, int alPriority = 0, bool abRemoveInterrupted = true
		);

	void PauseCurrentVoices();
	void UnpauseCurrentVoices();

	void SetOverCallback(const tString& asFunc){ msOverCallback = asFunc;}

	void Update(float afTimeStep);
	void OnDraw(float afFrameTime);
	void Reset();

	void SetVolumeMul(float afMul);
	float GetVolumeMul(float afMul){ return mfVolumeMul;}

	/**
	 * Returns true if all voices (not effect files) are done playing
	 */
	bool VoiceDonePlaying();

private:
	//////////////////////
	//Data
	cSoundHandler *mpSoundHandler;
	cVector2f mvFontSize;
	float mfRowWidth;

	//////////////////////
	//Variables
	bool mbPaused;
	tString msOverCallback;
	tLuxVoiceDataList mlstVoices;
	cSoundEntry *mpVoiceEntry;
	int mlVoiceEntryID;
	cSoundEntry *mpEffectEntry;
	int mlEffectEntryID;
	std::vector<tWString> mvCurrentTextRows;
	std::deque<cTextQueueEntry> mvTextEntryQueue;

	float mfVolumeMul;
};

//----------------------------------------------

class cLuxEffectHandler : public iLuxUpdateable
{
public:	
	cLuxEffectHandler();
	~cLuxEffectHandler();

	///////////////////////////
	// General
	void OnClearFonts();
	void LoadFonts();
	void OnStart();
	void Update(float afTimeStep);
	void Reset();

	void OnMapEnter(cLuxMap *apMap);
	void OnMapLeave(cLuxMap *apMap);

	void OnDraw(float afFrameTime);

	void DoAction(eLuxPlayerAction aAction, bool abPressed);
	
	///////////////////////////
	// Properties
	bool GetPlayerIsPaused(){ return mbPlayerIsPaused;}
	void SetPlayerIsPaused(bool abX);

	///////////////////////////
	// Effects
	cLuxEffect_Fade *GetFade(){ return mpFade;}
	cLuxEffect_Flash *GetFlash(){ return mpFlash;}
	//cLuxEffect_InfectionHealFlash *GetInfectionHealFlash(){ return mpInfectionHealFlash;}
	cLuxEffect_PlayVoice *GetPlayVoice(){ return mpPlayVoice;}
	cLuxEffect_ImageTrail *GetImageTrail(){ return mpImageTrail;}
	cLuxEffect_ShakeScreen *GetScreenShake(){ return mpScreenShake;}
	cLuxEffect_SepiaColor *GetSepiaColor(){ return mpSepiaColor;}
	cLuxEffect_ColorGrading *GetColorGrading(){ return mpColorGrading;}
	cLuxEffect_RadialBlur *GetRadialBlur(){ return mpRadialBlur;}
	cLuxEffect_EmotionFlash *GetEmotionFlash(){ return mpEmotionFlash;}
	cLuxEffect_PlayCommentary *GetPlayCommentary(){ return  mpPlayCommentary;}
	cLuxEffect_ScreenImage *GetScreenImage(){ return mpScreenImage;}

private:
	cLuxEffect_Fade *mpFade;
	cLuxEffect_Flash *mpFlash;
	//cLuxEffect_InfectionHealFlash *mpInfectionHealFlash;
	cLuxEffect_PlayVoice *mpPlayVoice;
	cLuxEffect_ImageTrail *mpImageTrail;
	cLuxEffect_ShakeScreen *mpScreenShake;
	cLuxEffect_SepiaColor *mpSepiaColor;
	cLuxEffect_ColorGrading *mpColorGrading;
	cLuxEffect_RadialBlur *mpRadialBlur;
	cLuxEffect_EmotionFlash *mpEmotionFlash;
	cLuxEffect_PlayCommentary *mpPlayCommentary;
	cLuxEffect_ScreenImage *mpScreenImage;

	std::vector<iLuxEffect*> mvEffects;	

	bool mbPlayerIsPaused;
};

//----------------------------------------------


#endif // LUX_EFFECT_HANDLER_H
