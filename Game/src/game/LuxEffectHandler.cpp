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

#include "LuxEffectHandler.h"

#include "LuxMapHandler.h"
#include "LuxMap.h"
#include "LuxPlayer.h"
#include "LuxHelpFuncs.h"
#include "LuxMessageHandler.h"
#include "LuxCommentaryIcon.h"


//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cLuxEffectHandler::cLuxEffectHandler() : iLuxUpdateable("LuxEffectHandler")
{
	mpFade = hplNew( cLuxEffect_Fade, () );
	mvEffects.push_back(mpFade);

	mpFlash = hplNew( cLuxEffect_Flash, () );
	mvEffects.push_back(mpFlash);
	/*
	mpInfectionHealFlash = hplNew( cLuxEffect_InfectionHealFlash, () );
	mvEffects.push_back(mpInfectionHealFlash);
	*/
	mpPlayVoice = hplNew( cLuxEffect_PlayVoice, () );
	mvEffects.push_back(mpPlayVoice);

	mpImageTrail = hplNew( cLuxEffect_ImageTrail, () );
	mvEffects.push_back(mpImageTrail);

	mpScreenShake = hplNew( cLuxEffect_ShakeScreen, () );
	mvEffects.push_back(mpScreenShake);

	mpSepiaColor = hplNew( cLuxEffect_SepiaColor, () );
	mvEffects.push_back(mpSepiaColor);

    mpColorGrading = hplNew( cLuxEffect_ColorGrading, () );
    mpColorGrading->SetActive(true);
	mvEffects.push_back(mpColorGrading);

	mpRadialBlur = hplNew( cLuxEffect_RadialBlur, () );
	mvEffects.push_back(mpRadialBlur);

	mpEmotionFlash = hplNew( cLuxEffect_EmotionFlash, () );
	mvEffects.push_back(mpEmotionFlash);

	mpPlayCommentary = hplNew( cLuxEffect_PlayCommentary, () );
	mvEffects.push_back(mpPlayCommentary);

    mpScreenImage = hplNew( cLuxEffect_ScreenImage, () );
	mvEffects.push_back(mpScreenImage);
}

//-----------------------------------------------------------------------

cLuxEffectHandler::~cLuxEffectHandler()
{
	STLDeleteAll(mvEffects);
}

//-----------------------------------------------------------------------

float iLuxEffect::GetAmountForCurrentInfection()
{
	float infection = gpBase->mpPlayer->GetInfection();
	int numberOfInfectionLevels = gpBase->mpPlayer->GetNumberOfInfectionLevels();
	float infectionStep = 100.0f / numberOfInfectionLevels;
	
	if ( infection == 0.0f )
	{
		return 0.0f;
	}
	else if ( infection <= infectionStep )
	{
		return mfValueAtInfectionLevelOne * infection / infectionStep;
	}
	else if ( infection <= 2*infectionStep )
	{
		float factor = ( 2*infectionStep - infection ) / infectionStep;
		return mfValueAtInfectionLevelOne * factor + mfValueAtInfectionLevelTwo * ( 1.0f - factor );
	}
	else if ( infection <= 3*infectionStep )
	{
		float factor = ( 3*infectionStep - infection ) / infectionStep;
		return mfValueAtInfectionLevelTwo * factor + mfValueAtInfectionLevelThree * ( 1.0f - factor );
	}
	else
	{
		float factor = ( 4*infectionStep - infection ) / infectionStep;
		return mfValueAtInfectionLevelThree * factor + mfValueAtInfectionLevelFour * ( 1.0f - factor );
	}
}

//////////////////////////////////////////////////////////////////////////
// PLAY COMMENTARY
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------


cLuxEffect_PlayCommentary::cLuxEffect_PlayCommentary()
{
	mpSoundHandler = gpBase->mpEngine->GetSound()->GetSoundHandler();
	mpMusicHandler = gpBase->mpEngine->GetSound()->GetMusicHandler();

	mpSoundEntry = NULL;
	mlSoundEntryID = -1;
	Reset();
}

cLuxEffect_PlayCommentary::~cLuxEffect_PlayCommentary()
{
	Reset();
}

//-----------------------------------------------------------------------

void cLuxEffect_PlayCommentary::Start(const tString &asTalker,const tString &asTopic, const tString &asFile, int alIconId)
{
	if(mpSoundEntry)
	{
		Stop();

		//Set the icon as not playing
		if(gpBase->mpMapHandler->GetCurrentMap())
		{
			iLuxEntity *pEntity = gpBase->mpMapHandler->GetCurrentMap()->GetEntityByID(mlIconID, eLuxEntityType_CommentaryIcon);
			if(pEntity)
			{
				cLuxCommentaryIcon *mpIcon = static_cast<cLuxCommentaryIcon*>(pEntity);
				mpIcon->SetPlayingSound(false);
			}
		}
	}
	
	msTalker = asTalker;
	msTopic  = asTopic;
	mlIconID = alIconId;

	mpSoundEntry = mpSoundHandler->PlayGuiStream(asFile,false, 1.0f);
	if(mpSoundEntry)
	{
		mlSoundEntryID = mpSoundEntry->GetId();
		SetActive(true);
		
		mpSoundHandler->FadeGlobalVolume(0.15f,0.5f,  eSoundEntryType_World, eLuxGlobalVolumeType_Commentary, false);
		mpMusicHandler->FadeVolumeMul(0.15f, 0.5f);
		gpBase->mpEffectHandler->GetPlayVoice()->SetVolumeMul(0.1f);
	}
	
}

//-----------------------------------------------------------------------

void cLuxEffect_PlayCommentary::Stop()
{
	if(mbActive==false) return;

	if(mpSoundHandler->IsValid(mpSoundEntry,mlSoundEntryID)) mpSoundEntry->FadeOut(1);
	mpSoundEntry = NULL;

	mpSoundHandler->FadeGlobalVolume(1.0f,0.5f,  eSoundEntryType_World, eLuxGlobalVolumeType_Commentary, false);
	mpMusicHandler->FadeVolumeMul(1.0f, 0.5f);
	gpBase->mpEffectHandler->GetPlayVoice()->SetVolumeMul(1.0f);

	mbActive = false;
}

//-----------------------------------------------------------------------


void cLuxEffect_PlayCommentary::Update(float afTimeStep)
{
	if(mpSoundHandler->IsValid(mpSoundEntry, mlSoundEntryID)) return;

	//Set the icon as not playing
	if(gpBase->mpMapHandler->GetCurrentMap())
	{
		iLuxEntity *pEntity = gpBase->mpMapHandler->GetCurrentMap()->GetEntityByID(mlIconID, eLuxEntityType_CommentaryIcon);
		if(pEntity)
		{
			cLuxCommentaryIcon *mpIcon = static_cast<cLuxCommentaryIcon*>(pEntity);
			mpIcon->SetPlayingSound(false);
		}
	}

	mpSoundEntry = NULL;

	mpSoundHandler->FadeGlobalVolume(1.0f,0.5f,  eSoundEntryType_World, eLuxGlobalVolumeType_Commentary, false);
	mpMusicHandler->FadeVolumeMul(1.0f, 0.5f);
	gpBase->mpEffectHandler->GetPlayVoice()->SetVolumeMul(1.0f);
	
	mbActive = false;
}

//-----------------------------------------------------------------------

void cLuxEffect_PlayCommentary::OnDraw(float afFrameTime)
{
}
//-----------------------------------------------------------------------

void cLuxEffect_PlayCommentary::Reset()
{
	if(mpSoundEntry != NULL && mpSoundHandler->IsValid(mpSoundEntry,mlSoundEntryID)) mpSoundEntry->Stop();
	mpSoundEntry = NULL;

	mpSoundHandler->SetGlobalVolume(1.0f,eSoundEntryType_World, eLuxGlobalVolumeType_Commentary);
	mpMusicHandler->SetVolumeMul(1.0f);
	
	mlIconID = -1;
	msTalker = "";
	msTopic = "";
}


//////////////////////////////////////////////////////////////////////////
// SCREEN IMAGE
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------


cLuxEffect_ScreenImage::cLuxEffect_ScreenImage()
{
	cGui * pGui = gpBase->mpEngine->GetGui();
	mpGuiSet = pGui->CreateSet( "Effect_ScreenImage", NULL );
	mpGuiSet->SetRendersBeforePostEffects( false );
	mpGuiSet->SetDrawMouse( false );
	mpGuiSet->SetDrawPriority(3);
	gpBase->mpMapHandler->GetViewport()->AddGuiSet( mpGuiSet );
    mpTextureGfx = NULL;
	mbActive = false;

    mfCurrentFade = 0.0f;

    mfFadeInDuration = 1.0f;
    mfShowDuration = 1.0f;
    mfFadeOutDuration = 1.0f;
    mfFadeTimer = 1.0f;
}

//-----------------------------------------------------------------------
cLuxEffect_ScreenImage::~cLuxEffect_ScreenImage()
{
    if ( mpTextureGfx != NULL )
    {
        gpBase->mpEngine->GetGui()->DestroyGfx(mpTextureGfx);
        mpTextureGfx = NULL;
    }

    if ( mpGuiSet != NULL )
    {
        gpBase->mpEngine->GetGui()->DestroySet(mpGuiSet);
        mpGuiSet = NULL;
    }
}

//-----------------------------------------------------------------------
void cLuxEffect_ScreenImage::ShowImage(const tString & asImageName, float afX, float afY, float afScale, bool abUseRelativeCoordinates, float afDuration, float afFadeIn, float afFadeOut)
{
    if ( mpTextureGfx != NULL )
    {
        gpBase->mpEngine->GetGui()->DestroyGfx(mpTextureGfx);
        mpTextureGfx = NULL;
    }

    mpTextureGfx = gpBase->mpEngine->GetGui()->CreateGfxTexture(asImageName,eGuiMaterial_Alpha);

    // coordinates are centered around origin, if relative in terms of screen size

    cVector2f screen_size = gpBase->mpEngine->GetGraphics()->GetLowLevel()->GetScreenSizeFloat();
    
    if ( abUseRelativeCoordinates )
    {
        afX = afX * screen_size.x;
        afY = afY * screen_size.y;
    }

    afX += screen_size.x / 2;
    afY += screen_size.y / 2;

    mfFadeInDuration = afFadeIn;
    mfShowDuration = afDuration;
    mfFadeOutDuration = afFadeOut;
    mfFadeTimer = 0.0f;

    if ( mfFadeInDuration > 0 )
    {
        mfCurrentFade = 1.0f;
    }
    else
    {
        mfCurrentFade = 0.0f;
    }

	mbActive = true;
	mvPosition = cVector3f(afX, afY, gpBase->mvHudVirtualStartPos.z + 10.0f);
	mfScale = afScale;

}

//-----------------------------------------------------------------------
void cLuxEffect_ScreenImage::HideImmediately()
{
	mbActive = false;

    if ( mpTextureGfx != NULL )
    {
        gpBase->mpEngine->GetGui()->DestroyGfx(mpTextureGfx);
        mpTextureGfx = NULL;
    }
}

//-----------------------------------------------------------------------
void cLuxEffect_ScreenImage::HideWithFade(float afFadeOut)
{
    if ( mfCurrentFade > 0.0f )
    {
        mfFadeInDuration = 0.0f;
        mfShowDuration = 1.0f;
        mfFadeOutDuration = afFadeOut;
    }
}

//-----------------------------------------------------------------------

void cLuxEffect_ScreenImage::Update(float afTimeStep)
{
    mfFadeTimer += afTimeStep;

    if ( mfFadeTimer > ( mfFadeInDuration + mfShowDuration + mfFadeOutDuration ) )
    {
        mfCurrentFade = 0.0f;
	    mbActive = false;
    }
    else if ( mfFadeOutDuration > 0 && mfFadeTimer > mfFadeInDuration + mfShowDuration )
    {
        // fading out
        float mfFadeFactor = ( mfFadeTimer - (mfFadeInDuration + mfShowDuration) ) / mfFadeOutDuration;    // 0 when starting to fade out, 1 when faded out
        mfCurrentFade = 1.0f - mfFadeFactor;
    }
    else if ( mfFadeTimer > mfFadeInDuration )
    {
        mfCurrentFade = 1.0f;
    }
    else if ( mfFadeInDuration > 0.0f )
    {
        // fading in
        float mfFadeFactor = ( mfFadeTimer ) / mfFadeInDuration;    // 0 when starting to fade in, 1 when faded in
        mfCurrentFade = mfFadeFactor;
    }
}

//-----------------------------------------------------------------------

void cLuxEffect_ScreenImage::OnDraw(float afFrameTime)
{
    if ( gpBase->mpGameHudSet && mpTextureGfx )
    {
        mpGuiSet->DrawGfx(mpTextureGfx, mvPosition, mfScale, cColor(1.0f,mfCurrentFade));
    }
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// EMOTION FLASH
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cLuxEffect_EmotionFlash::cLuxEffect_EmotionFlash()
{
	mpWhiteGfx = gpBase->mpEngine->GetGui()->CreateGfxFilledRect(cColor(1,1), eGuiMaterial_Additive);

	mpFont = NULL;

	mvFontSize = 20;
}
cLuxEffect_EmotionFlash::~cLuxEffect_EmotionFlash()
{

}

//-----------------------------------------------------------------------

void cLuxEffect_EmotionFlash::ClearFonts()
{
	if(mpFont)
		gpBase->mpEngine->GetResources()->GetFontManager()->Destroy(mpFont);

	mpFont = NULL;
}

void cLuxEffect_EmotionFlash::LoadFonts()
{
	tString sFontFile = "game_default.fnt";
	mpFont = gpBase->mpEngine->GetResources()->GetFontManager()->CreateFontData(sFontFile);
}

//-----------------------------------------------------------------------

void cLuxEffect_EmotionFlash::Start(const tString &asTextCat, const tString &asTextEntry, const tString &asSound)
{
	mbActive = true;

	mlStep = 0;

	mfAlpha =0;

	gpBase->mpHelpFuncs->PlayGuiSoundData(asSound, eSoundEntryType_Gui);

	tWString sText = kTranslate(asTextCat, asTextEntry);
	mvTextRows.clear();
	mpFont->GetWordWrapRows(500, mvFontSize.y, mvFontSize, sText, &mvTextRows);

	mfTextTime = 3.0f + 0.15f * (float)sText.length();
	mfTextAlpha =0;

	gpBase->mpEffectHandler->SetPlayerIsPaused(true);
	gpBase->mpPlayer->FadeFOVMulTo(0.5f, 0.5f);
	gpBase->mpEffectHandler->GetRadialBlur()->FadeTo(0.15f, 3);
	gpBase->mpEffectHandler->GetRadialBlur()->SetBlurStartDist(0.6f);

	//Disable enemies
	gpBase->mpMapHandler->GetCurrentMap()->BroadcastEnemyMessage(eLuxEnemyMessage_Reset, false,0,0);

}

void cLuxEffect_EmotionFlash::Reset()
{

}

//-----------------------------------------------------------------------

void cLuxEffect_EmotionFlash::Update(float afTimeStep)
{
	if(mlStep ==0)
	{
		mfAlpha += 0.5f * afTimeStep;
		if(mfAlpha >= 1.0f)
		{
			mfAlpha = 1.0f;
			mlStep=1;
			mfCount = 1;
		}
	}
	else if(mlStep ==1)
	{
		mfTextAlpha += afTimeStep * 3.0f;
		if(mfTextAlpha > 1) mfTextAlpha =1;

		//Check if text has been displayed long enough.
		mfTextTime -= afTimeStep;
		if(mfTextTime < 0)
		{
			gpBase->mpPlayer->FadeFOVMulTo(1.0f, 0.33f);
			gpBase->mpEffectHandler->SetPlayerIsPaused(false);

			gpBase->mpEffectHandler->GetRadialBlur()->FadeTo(0, 1);
			
			mlStep = 2;
		}
	}
	else if(mlStep ==2)
	{
		mfTextAlpha -= afTimeStep * 1.0f;
		if(mfTextAlpha < 0) mfTextAlpha =0;

		mfAlpha -= 0.33f * afTimeStep;
		if(mfAlpha <= 0.0f)
		{
			mbActive = false;
		}
	}
}

//-----------------------------------------------------------------------

void cLuxEffect_EmotionFlash::OnDraw(float afFrameTime)
{
	gpBase->mpGameHudSet->DrawGfx(mpWhiteGfx,gpBase->mvHudVirtualStartPos + cVector3f(0,0,3.2f),gpBase->mvHudVirtualSize,cColor(mfAlpha, 1));

	if(mfTextAlpha > 0)
	{
		float fStartY = 300 - (mvFontSize.y+2.0f) * 0.5f * (float)mvTextRows.size();

		if(mvTextRows.size() == 1)
		{
			gpBase->mpGameHudSet->DrawFont(mvTextRows[0], mpFont, cVector3f(400,fStartY, 4), mvFontSize, cColor(0, mfTextAlpha), eFontAlign_Center);
		}
		else
		{
			float fY = fStartY;
			for(size_t i=0; i<mvTextRows.size(); ++i)
			{
				gpBase->mpGameHudSet->DrawFont(mvTextRows[i], mpFont, cVector3f(150,fY, 4), mvFontSize, cColor(0, mfTextAlpha), eFontAlign_Left);
				fY += mvFontSize.y + 2.0f;
			}
		}
	}
}

//-----------------------------------------------------------------------

void cLuxEffect_EmotionFlash::DoAction(eLuxPlayerAction aAction, bool abPressed)
{
	if(abPressed==false) return;

	if(mlStep==1) mfTextTime = 0;
}

//-----------------------------------------------------------------------


//////////////////////////////////////////////////////////////////////////
// RADIAL BLUR
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cLuxEffect_RadialBlur::cLuxEffect_RadialBlur()
{
	mfSize =0;
	mfSizeGoal =0;
	mfBlurStartDist =0;

	SetActive(true);

	mfValueAtInfectionLevelOne = gpBase->mpGameCfg->GetFloat("Infection_ExtraEffects","RadialBlurLevelOne",0);
	mfValueAtInfectionLevelTwo = gpBase->mpGameCfg->GetFloat("Infection_ExtraEffects","RadialBlurLevelTwo",0);
	mfValueAtInfectionLevelThree = gpBase->mpGameCfg->GetFloat("Infection_ExtraEffects","RadialBlurLevelThree",0);
	mfValueAtInfectionLevelFour = gpBase->mpGameCfg->GetFloat("Infection_ExtraEffects","RadialBlurLevelFour",0);

	mfStartDistAtInfectionLevelOne = gpBase->mpGameCfg->GetFloat("Infection_ExtraEffects","RadialBlurStartDistLevelOne",0);
	mfStartDistAtInfectionLevelTwo = gpBase->mpGameCfg->GetFloat("Infection_ExtraEffects","RadialBlurStartDistLevelTwo",0);
	mfStartDistAtInfectionLevelThree = gpBase->mpGameCfg->GetFloat("Infection_ExtraEffects","RadialBlurStartDistLevelThree",0);
	mfStartDistAtInfectionLevelFour = gpBase->mpGameCfg->GetFloat("Infection_ExtraEffects","RadialBlurStartDistLevelFour",0);
}

//-----------------------------------------------------------------------

void cLuxEffect_RadialBlur::SetBlurStartDist(float afDist)
{
	mfBlurStartDist = afDist;

	cPostEffectParams_RadialBlur radialBlurParams;
	radialBlurParams.mfSize = mfSize;
	radialBlurParams.mfBlurStartDist = mfBlurStartDist;
	gpBase->mpMapHandler->GetPostEffect_RadialBlur()->SetParams(&radialBlurParams);

}

//-----------------------------------------------------------------------

float cLuxEffect_RadialBlur::GetStartDistForCurrentInfection()
{
	float infection = gpBase->mpPlayer->GetInfection();
	int numberOfInfectionLevels = gpBase->mpPlayer->GetNumberOfInfectionLevels();
	float infectionStep = 100.0f / numberOfInfectionLevels;
	
	if ( infection == 0.0f )
	{
		return 0.0f;
	}
	else if ( infection <= infectionStep )
	{
		return mfStartDistAtInfectionLevelOne * infection / infectionStep;
	}
	else if ( infection <= 2*infectionStep )
	{
		float factor = ( 2*infectionStep - infection ) / infectionStep;
		return mfStartDistAtInfectionLevelOne * factor + mfStartDistAtInfectionLevelTwo * ( 1.0f - factor );
	}
	else if ( infection <= 3*infectionStep )
	{
		float factor = ( 3*infectionStep - infection ) / infectionStep;
		return mfStartDistAtInfectionLevelTwo * factor + mfStartDistAtInfectionLevelThree * ( 1.0f - factor );
	}
	else
	{
		float factor = ( 4*infectionStep - infection ) / infectionStep;
		return mfStartDistAtInfectionLevelThree * factor + mfStartDistAtInfectionLevelFour * ( 1.0f - factor );
	}
}

//-----------------------------------------------------------------------

void cLuxEffect_RadialBlur::FadeTo(float afSize, float afSpeed)
{
	mfSizeGoal = afSize;
	mfFadeSpeed = afSpeed;
	gpBase->mpMapHandler->GetPostEffect_RadialBlur()->SetActive(true);
}

//-----------------------------------------------------------------------

void cLuxEffect_RadialBlur::Update(float afTimeStep)
{
	if(mfSizeGoal < mfSize)
	{
		mfSize -= mfFadeSpeed * afTimeStep;
		if(mfSize <= mfSizeGoal)
		{
			mfSize = 	mfSizeGoal;
			SetActive(false);
		}
	}
	else
	{
		mfSize += mfFadeSpeed * afTimeStep;
		if(mfSize >= mfSizeGoal)
		{
			mfSize = mfSizeGoal;
			SetActive(false);
		}
	}

	float startDistForCurrentInfection = GetStartDistForCurrentInfection();

	if ( startDistForCurrentInfection > mfBlurStartDist )
	{
		SetBlurStartDist(startDistForCurrentInfection);
	}

	float finalSize = mfSize;
	float amountForCurrentInfection = GetAmountForCurrentInfection();

	if ( finalSize < amountForCurrentInfection )
	{
		finalSize = amountForCurrentInfection;
	}
	
	if ( finalSize > 0 && !gpBase->mpMapHandler->GetPostEffect_RadialBlur()->IsActive() )
	{
		gpBase->mpMapHandler->GetPostEffect_RadialBlur()->SetActive(true);
	}

	cPostEffectParams_RadialBlur radialBlurParams;
	radialBlurParams.mfSize = finalSize;
	radialBlurParams.mfBlurStartDist = mfBlurStartDist;
	gpBase->mpMapHandler->GetPostEffect_RadialBlur()->SetParams(&radialBlurParams);

	if(finalSize <=0)
	{
		gpBase->mpMapHandler->GetPostEffect_RadialBlur()->SetActive(false);
	}
}

//-----------------------------------------------------------------------

void cLuxEffect_RadialBlur::Reset()
{
	mfSize =0;
	mfSizeGoal =0;
	mfBlurStartDist =0;

	gpBase->mpMapHandler->GetPostEffect_RadialBlur()->Reset();
	gpBase->mpMapHandler->GetPostEffect_RadialBlur()->SetActive(false);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// SEPIA COLOR
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cLuxEffect_SepiaColor::cLuxEffect_SepiaColor()
{
	mfAmount =0;
	mfAmountGoal =0;
	SetActive(true);

	mfValueAtInfectionLevelOne = gpBase->mpGameCfg->GetFloat("Infection_ExtraEffects","SepiaLevelOne",0);
	mfValueAtInfectionLevelTwo = gpBase->mpGameCfg->GetFloat("Infection_ExtraEffects","SepiaLevelTwo",0);
	mfValueAtInfectionLevelThree = gpBase->mpGameCfg->GetFloat("Infection_ExtraEffects","SepiaLevelThree",0);
	mfValueAtInfectionLevelFour = gpBase->mpGameCfg->GetFloat("Infection_ExtraEffects","SepiaLevelFour",0);
}

void cLuxEffect_SepiaColor::FadeTo(float afAmount, float afSpeed)
{
	mfAmountGoal = afAmount;
	mfFadeSpeed = afSpeed;
	gpBase->mpMapHandler->GetPostEffect_Sepia()->SetActive(true);
}

void cLuxEffect_SepiaColor::Update(float afTimeStep)
{
	if(mfAmountGoal < mfAmount)
	{
		mfAmount -= mfFadeSpeed * afTimeStep;
		if(mfAmount <= mfAmountGoal)
		{
			mfAmount = 	mfAmountGoal;
			//SetActive(false);
		}
	}
	else
	{
		mfAmount += mfFadeSpeed * afTimeStep;
		if(mfAmount >= mfAmountGoal)
		{
			mfAmount = mfAmountGoal;
			//SetActive(false);
		}
	}

	float finalAmount = mfAmount;
	float amountForCurrentInfection = GetAmountForCurrentInfection();

	if ( finalAmount < amountForCurrentInfection )
	{
		finalAmount = amountForCurrentInfection;
	}
	
	if ( finalAmount > 0 && !gpBase->mpMapHandler->GetPostEffect_Sepia()->IsActive() )
	{
		gpBase->mpMapHandler->GetPostEffect_Sepia()->SetActive(true);
	}

	cPostEffectParams_ColorConvTex sepiaParams;
	sepiaParams.mfFadeAlpha = finalAmount;
	gpBase->mpMapHandler->GetPostEffect_Sepia()->SetParams(&sepiaParams);
	
	if( finalAmount <=0 )
	{
		gpBase->mpMapHandler->GetPostEffect_Sepia()->SetActive(false);
	}
}

void cLuxEffect_SepiaColor::Reset()
{
	mfAmount =0;
	mfAmountGoal =0;
	gpBase->mpMapHandler->GetPostEffect_Sepia()->Reset();
	gpBase->mpMapHandler->GetPostEffect_Sepia()->SetActive(false);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// COLOR GRADING
//////////////////////////////////////////////////////////////////////////

cLuxEffect_ColorGrading::cLuxEffect_ColorGrading()
{
    mbIsCrossFading = false;
    mfCrossFadeAlpha = 0.0f;
    mbIsFadingUp = false;
    mfFadeSpeed = 1.0f;
    mfGameplayFadeTime = 1.0f;
    msFadeTargetLUT = "";
    msGameplayLUT = "";
}

//-----------------------------------------------------------------------

void cLuxEffect_ColorGrading::InitializeLUT(tString asBaseEnvironmentLUT)
{
    mbIsCrossFading = false;
    mfCrossFadeAlpha = 0.0f;
    mbIsFadingUp = false;
    mfFadeSpeed = 1.0f;
    msFadeTargetLUT = asBaseEnvironmentLUT;
    msGameplayLUT = "";

    msEnvironmentLUTs.clear();
    msEnvironmentLUTs.push_front( asBaseEnvironmentLUT );
    msEnvironmentLUTFadeTimes.clear();
    msEnvironmentLUTFadeTimes.push_front( 5.0f );

    cPostEffectParams_ColorGrading colorGradingParams;
	colorGradingParams.msTextureFile1 = asBaseEnvironmentLUT;
	colorGradingParams.msTextureFile2 = "";
    colorGradingParams.mfCrossFadeAlpha = 0.0f;
    colorGradingParams.mbIsReinitialisation = true;


    gpBase->mpMapHandler->GetPostEffect_ColorGrading()->SetParams( &colorGradingParams );
}

//-----------------------------------------------------------------------

void cLuxEffect_ColorGrading::EnterLUTEnvironment(tString asEnvironmentLUT, float afFadeTime)
{
    if ( afFadeTime < 0.5f ) afFadeTime = 0.5f;

    if ( !mbIsCrossFading )
    {
        // not fading

        if ( msGameplayLUT != "" )
        {
            // we have a gameplay LUT fully active. That means we just push the environment lut, and ignore the fade time. This environment will become active when the gameplay lut fades out
            msEnvironmentLUTs.push_front( asEnvironmentLUT );
            msEnvironmentLUTFadeTimes.push_front( afFadeTime );
        }
        else
        {
            if ( asEnvironmentLUT != *msEnvironmentLUTs.begin() )
            {
                // not fading, new map & no current gameplay LUT -> fade to the new map with the new fadetime

                FadeFromTo( *msEnvironmentLUTs.begin(), asEnvironmentLUT, afFadeTime );
            }

            msEnvironmentLUTs.push_front( asEnvironmentLUT );
            msEnvironmentLUTFadeTimes.push_front( afFadeTime );
        }
    }
    else
    {
        // a crossfade is in progress. Queue ours, when fade is finished it will re-fade

        msEnvironmentLUTs.push_front( asEnvironmentLUT );
        msEnvironmentLUTFadeTimes.push_front( afFadeTime );
    }
}

void cLuxEffect_ColorGrading::FadeFromTo( tString asFromTexture, tString asToTexture, float afFadeTime )
{
    if ( !mbIsFadingUp )
    {
        cPostEffectParams_ColorGrading colorGradingParams;
	    colorGradingParams.msTextureFile1 = asFromTexture;
	    colorGradingParams.msTextureFile2 = asToTexture;
        colorGradingParams.mfCrossFadeAlpha = 0.0f;
        colorGradingParams.mbIsReinitialisation = false;

        gpBase->mpMapHandler->GetPostEffect_ColorGrading()->SetParams( &colorGradingParams );
    }
    else
    {
        cPostEffectParams_ColorGrading colorGradingParams;
	    colorGradingParams.msTextureFile1 = asToTexture;
	    colorGradingParams.msTextureFile2 = asFromTexture;
        colorGradingParams.mfCrossFadeAlpha = 1.0f;
        colorGradingParams.mbIsReinitialisation = false;

        gpBase->mpMapHandler->GetPostEffect_ColorGrading()->SetParams( &colorGradingParams );
    }

    msFadeTargetLUT = asToTexture;
    mbIsFadingUp = !mbIsFadingUp;
    mbIsCrossFading = true;
    mfFadeSpeed = 1.0f / afFadeTime;
}

//-----------------------------------------------------------------------

void cLuxEffect_ColorGrading::LeaveLUTEnvironment(tString asEnvironmentLUT)
{
    tStringListIt second = msEnvironmentLUTs.begin();
    if ( msEnvironmentLUTs.size() > 1 ) second++;

    if ( mbIsCrossFading 
       || msGameplayLUT != ""
       || asEnvironmentLUT != *msEnvironmentLUTs.begin()
       || ( msEnvironmentLUTs.size() > 1 && *second == asEnvironmentLUT )
       )
    {
        tFloatListIt fadeTimeIt = msEnvironmentLUTFadeTimes.begin();

        // just remove the first occurrence of this map
        for(tStringListIt it = msEnvironmentLUTs.begin(); it != msEnvironmentLUTs.end(); ++it)
        {
            if ( *it == asEnvironmentLUT )
            {
                msEnvironmentLUTs.erase( it );
                msEnvironmentLUTFadeTimes.erase( fadeTimeIt );
                break;
            }
            fadeTimeIt++;
        }
    }
    else
    {
        // this means we're the first map, the second is different and we don't have a gameplay map. Start a fade to the second environment map before removing
    
        FadeFromTo( asEnvironmentLUT, *second, *msEnvironmentLUTFadeTimes.begin() );

        msEnvironmentLUTs.erase( msEnvironmentLUTs.begin() );
        msEnvironmentLUTFadeTimes.erase( msEnvironmentLUTFadeTimes.begin() );
    }
}

//-----------------------------------------------------------------------

void cLuxEffect_ColorGrading::FadeGameplayLUTTo(tString asEnvironmentLUT, float afFadeTime)
{
}

//-----------------------------------------------------------------------

void cLuxEffect_ColorGrading::FadeOutGameplayLUT(float afFadeTime)
{
}

//-----------------------------------------------------------------------

void cLuxEffect_ColorGrading::Update(float afTimeStep)
{
    bool bJustFinishedCrossFading = false;

    if ( mbIsCrossFading )
    {
        if ( mbIsFadingUp )
        {
            mfCrossFadeAlpha += afTimeStep * mfFadeSpeed;
            if ( mfCrossFadeAlpha >= 1.0f )
            {
                mfCrossFadeAlpha = 1.0f;
                mbIsCrossFading = false;
                bJustFinishedCrossFading = true;
            }
        }
        else
        {
            mfCrossFadeAlpha -= afTimeStep * mfFadeSpeed;
            if ( mfCrossFadeAlpha <= 0.0f )
            {
                mfCrossFadeAlpha = 0.0f;
                mbIsCrossFading = false;
                bJustFinishedCrossFading = true;
            }
        }

        ((cPostEffect_ColorGrading*)gpBase->mpMapHandler->GetPostEffect_ColorGrading())->SetCrossFadeAlpha( mfCrossFadeAlpha );
    }

    if ( bJustFinishedCrossFading )
    {
        // check if the final state of the crossfade is the current state we want, if not, crossfade.
        tString sDesiredLUT = "";
        float fDesiredFadeTime = 1.0f;

        if ( msGameplayLUT != "" )
        {
            sDesiredLUT = msGameplayLUT;
            fDesiredFadeTime = mfGameplayFadeTime;
        }
        else
        {
            sDesiredLUT = *msEnvironmentLUTs.begin();
            fDesiredFadeTime = *msEnvironmentLUTFadeTimes.begin();
        }

        if ( sDesiredLUT != msFadeTargetLUT )
        {
            FadeFromTo(msFadeTargetLUT,sDesiredLUT,fDesiredFadeTime);
        }
    }
}

//-----------------------------------------------------------------------

void cLuxEffect_ColorGrading::Reset()
{
}

//////////////////////////////////////////////////////////////////////////
// SCREEN SHAKE
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cLuxEffect_ShakeScreen::cLuxEffect_ShakeScreen()
{
}

cLuxEffect_ShakeScreen::~cLuxEffect_ShakeScreen()
{
}

//-----------------------------------------------------------------------

void cLuxEffect_ShakeScreen::Start(float afAmount, float afTime,float afFadeInTime,float afFadeOutTime)
{
	cLuxEffect_ShakeScreen_Shake shake;
	shake.mfSize = afAmount;
	shake.mfMaxSize = afAmount;
	shake.mfTime = afTime;
	shake.mfFadeInTime = afFadeInTime;
	shake.mfMaxFadeInTime = afFadeInTime;
	shake.mfFadeOutTime = afFadeOutTime;
	shake.mfMaxFadeOutTime = afFadeOutTime;

	mlstShakes.push_back(shake);

	SetActive(true);
}

//-----------------------------------------------------------------------

void cLuxEffect_ShakeScreen::Update(float afTimeStep)
{
	float fLargest = 0;

	std::list<cLuxEffect_ShakeScreen_Shake>::iterator it = mlstShakes.begin();
	for(; it != mlstShakes.end(); )
	{
		cLuxEffect_ShakeScreen_Shake &shake = *it;

		if(shake.mfFadeInTime >0)
		{
			shake.mfFadeInTime -= afTimeStep; if(shake.mfFadeInTime<0) shake.mfFadeInTime=0;
			float fT = shake.mfFadeInTime / shake.mfMaxFadeInTime;
			shake.mfSize = (1-fT) * shake.mfMaxSize;
		}
		else if(shake.mfTime >0)
		{
			shake.mfTime -= afTimeStep; if(shake.mfTime<0)shake.mfTime=0;
			shake.mfSize = shake.mfMaxSize;
		}
		else
		{
			shake.mfFadeOutTime -= afTimeStep; if(shake.mfFadeOutTime<0) shake.mfFadeOutTime=0;
			float fT = shake.mfFadeOutTime / shake.mfMaxFadeOutTime;
			shake.mfSize =  fT * shake.mfMaxSize;
		}

		//Log("%f, %f, %f size: %f\n",shake.mfFadeInTime,shake.mfTime,shake.mfFadeOutTime,shake.mfSize);

		if(fLargest < shake.mfSize) fLargest = shake.mfSize;

		if(shake.mfTime <= 0 && shake.mfFadeOutTime <= 0 && shake.mfFadeInTime <= 0)
		{
			it = mlstShakes.erase(it);

			//If all shaking is over, set pos add to 0 and return.
			if(mlstShakes.empty())
			{
				SetActive(false);
				gpBase->mpPlayer->SetHeadPosAdd(eLuxHeadPosAdd_ScreenShake,0);
				return;
			}
		}
		else
		{
			++it;
		}
	}

	cVector3f vAdd(0);
	vAdd.x = cMath::RandRectf(-fLargest,fLargest);
	vAdd.y = cMath::RandRectf(-fLargest,fLargest);
	vAdd.z = cMath::RandRectf(-fLargest,fLargest);

	gpBase->mpPlayer->SetHeadPosAdd(eLuxHeadPosAdd_ScreenShake, vAdd);
}

//-----------------------------------------------------------------------

void cLuxEffect_ShakeScreen::Reset()
{
	mlstShakes.clear();
}


//-----------------------------------------------------------------------


//////////////////////////////////////////////////////////////////////////
// IMAGE TRAIL
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cLuxEffect_ImageTrail::cLuxEffect_ImageTrail()
{
	mfAmount =0;
	mfAmountGoal =0;

	SetActive(true);

	mfValueAtInfectionLevelOne = gpBase->mpGameCfg->GetFloat("Infection_ExtraEffects","ImageTrailLevelOne",0);
	mfValueAtInfectionLevelTwo = gpBase->mpGameCfg->GetFloat("Infection_ExtraEffects","ImageTrailLevelTwo",0);
	mfValueAtInfectionLevelThree = gpBase->mpGameCfg->GetFloat("Infection_ExtraEffects","ImageTrailLevelThree",0);
	mfValueAtInfectionLevelFour = gpBase->mpGameCfg->GetFloat("Infection_ExtraEffects","ImageTrailLevelFour",0);
}

void cLuxEffect_ImageTrail::FadeTo(float afAmount, float afSpeed)
{
	mfAmountGoal = afAmount;
	mfFadeSpeed = afSpeed;
	gpBase->mpMapHandler->GetPostEffect_ImageTrail()->SetActive(true);
}

void cLuxEffect_ImageTrail::Update(float afTimeStep)
{
	if(mfAmountGoal < mfAmount)
	{
		mfAmount -= mfFadeSpeed * afTimeStep;
		if(mfAmount <= mfAmountGoal)
		{
			mfAmount = 	mfAmountGoal;
			//SetActive(false);
		}
	}
	else
	{
		mfAmount += mfFadeSpeed * afTimeStep;
		if(mfAmount >= mfAmountGoal)
		{
			mfAmount = mfAmountGoal;
			//SetActive(false);
		}
	}

	float finalAmount = mfAmount;
	float amountForCurrentInfection = GetAmountForCurrentInfection();

	if ( finalAmount < amountForCurrentInfection )
	{
		finalAmount = amountForCurrentInfection;
	}
	
	if ( finalAmount > 0 && !gpBase->mpMapHandler->GetPostEffect_ImageTrail()->IsActive() )
	{
		gpBase->mpMapHandler->GetPostEffect_ImageTrail()->SetActive(true);
	}
	
	cPostEffectParams_ImageTrail imageTrailParams;
	imageTrailParams.mfAmount = finalAmount;
	gpBase->mpMapHandler->GetPostEffect_ImageTrail()->SetParams(&imageTrailParams);
	
	if(finalAmount <=0)
	{
		gpBase->mpMapHandler->GetPostEffect_ImageTrail()->SetActive(false);
	}
}

void cLuxEffect_ImageTrail::Reset()
{
	mfAmount =0;
	mfAmountGoal =0;
	gpBase->mpMapHandler->GetPostEffect_ImageTrail()->Reset();
	gpBase->mpMapHandler->GetPostEffect_ImageTrail()->SetActive(false);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// FADE
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cLuxEffect_Fade::cLuxEffect_Fade()
{
	mpWhiteGfx = gpBase->mpEngine->GetGui()->CreateGfxFilledRect(cColor(1,1), eGuiMaterial_Modulative);

	Reset();
}

cLuxEffect_Fade::~cLuxEffect_Fade()
{

}

//-----------------------------------------------------------------------

void cLuxEffect_Fade::FadeIn(float afTime, float afGoalAlpha)
{
	mfGoalAlpha = afGoalAlpha;
	if(afTime <= 0) mfAlpha = mfGoalAlpha;
	else			mfFadeSpeed = 1 / afTime;

	SetActive(true);
}

void cLuxEffect_Fade::FadeOut(float afTime, float afGoalAlpha)
{
	mfGoalAlpha = afGoalAlpha;
	if(afTime <= 0) mfAlpha = afGoalAlpha;
	else			mfFadeSpeed = 1 / afTime;

	SetActive(true);
}

//-----------------------------------------------------------------------

void cLuxEffect_Fade::SetDirectAlpha(float afX)
{
	if(afX<=0)	SetActive(false);
	else		SetActive(true);

	mfGoalAlpha = afX;
	mfAlpha = afX;
}

//-----------------------------------------------------------------------

bool cLuxEffect_Fade::IsFading()
{
	return mfGoalAlpha != mfAlpha;
}

//-----------------------------------------------------------------------

void cLuxEffect_Fade::Update(float afTimeStep)
{
	if ( mfAlpha > mfGoalAlpha )
	{
		mfAlpha -= afTimeStep * mfFadeSpeed;
		if(mfAlpha < mfGoalAlpha)
		{
			mfAlpha = mfGoalAlpha;

			if ( mfGoalAlpha == 0 )
			{
				SetActive(false);
			}
		}
	}
	else if( mfAlpha < mfGoalAlpha )
	{
		mfAlpha += afTimeStep * mfFadeSpeed;
		if(mfAlpha > mfGoalAlpha)
		{
			mfAlpha = mfGoalAlpha;
		}
	}
}

//-----------------------------------------------------------------------

void cLuxEffect_Fade::OnDraw(float afFrameTime)
{
	if(mfAlpha <=0) return;

	gpBase->mpGameHudSet->DrawGfx(mpWhiteGfx,gpBase->mvHudVirtualStartPos+cVector3f(0,0,3.2f),gpBase->mvHudVirtualSize,cColor(1-mfAlpha, 1));
}

//-----------------------------------------------------------------------

void cLuxEffect_Fade::Reset()
{
	mfGoalAlpha =0;
	mfAlpha =0;
	mfFadeSpeed = 1;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// SANITY GAIN FLASH
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------
/*
cLuxEffect_InfectionHealFlash::cLuxEffect_InfectionHealFlash()
{
	mpWhiteGfx = gpBase->mpEngine->GetGui()->CreateGfxFilledRect(cColor(1,1), eGuiMaterial_Additive);

	mColor = gpBase->mpGameCfg->GetColor("Player_General","SanityGain_Color", 0);
	msSound = gpBase->mpGameCfg->GetString("Player_General","SanityGain_Sound", "");
	mfFadeInTime = gpBase->mpGameCfg->GetFloat("Player_General","SanityGain_FadeInTime", 0);
	mfFadeOutTime = gpBase->mpGameCfg->GetFloat("Player_General","SanityGain_FadeOutTime", 0);

	Reset();
}
cLuxEffect_InfectionHealFlash::~cLuxEffect_InfectionHealFlash()
{

}

//-----------------------------------------------------------------------

void cLuxEffect_InfectionHealFlash::Reset()
{
	mfAlpha =0;
}

//-----------------------------------------------------------------------

void cLuxEffect_InfectionHealFlash::Start()
{
	if(msSound != "")
		gpBase->mpHelpFuncs->PlayGuiSoundData(msSound, eSoundEntryType_Gui);
	
	mbActive = true;

	mlStep = 0;

	mfAlpha =0;

	mfFadeInSpeed = 1 / mfFadeInTime;
	mfWhiteSpeed = 1 / 0.05f;
	mfFadeOutSpeed = 1 / mfFadeOutTime;
}

//-----------------------------------------------------------------------

void cLuxEffect_InfectionHealFlash::Update(float afTimeStep)
{
	if(mlStep ==0)
	{
		mfAlpha += mfFadeInSpeed * afTimeStep;
		if(mfAlpha >= 1.0f)
		{
			mfAlpha = 1.0f;
			mlStep=1;
			mfCount = 1;
		}
	}
	else if(mlStep ==1)
	{
		mfCount -= mfWhiteSpeed * afTimeStep;
		if(mfCount <= 0)
		{
			mlStep = 2;
		}
	}
	else if(mlStep ==2)
	{
		mfAlpha -= mfFadeOutSpeed * afTimeStep;
		if(mfAlpha <= 0.0f)
		{
			mbActive = false;
		}
	}

}

//-----------------------------------------------------------------------

void cLuxEffect_InfectionHealFlash::OnDraw(float afFrameTime)
{
	DrawFlash(gpBase->mpGameHudSet, afFrameTime);
}	

//-----------------------------------------------------------------------

void cLuxEffect_InfectionHealFlash::DrawFlash(cGuiSet *apSet ,float afTimeStep)
{
	apSet->DrawGfx(mpWhiteGfx,gpBase->mvHudVirtualStartPos+cVector3f(0,0,3.2f),gpBase->mvHudVirtualSize,mColor*mfAlpha);
}
*/
//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// FLASH
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cLuxEffect_Flash::cLuxEffect_Flash()
{
	mpWhiteGfx = gpBase->mpEngine->GetGui()->CreateGfxFilledRect(cColor(1,1), eGuiMaterial_Additive);
}
cLuxEffect_Flash::~cLuxEffect_Flash()
{
	
}

//-----------------------------------------------------------------------

void cLuxEffect_Flash::Start(float afFadeIn, float afWhite, float afFadeOut)
{
	mbActive = true;

	mlStep = 0;

	mfAlpha =0;

	if(afFadeIn==0) afFadeIn = 0.000001f;
	if(afWhite==0) afWhite = 0.000001f;
	if(afFadeOut==0) afFadeOut = 0.000001f;


	mfFadeInSpeed = 1 / afFadeIn;
	mfWhiteSpeed = 1 / afWhite;
	mfFadeOutSpeed = 1 / afFadeOut;
}

//-----------------------------------------------------------------------

void cLuxEffect_Flash::Reset()
{
	mlStep = 0;

	mfAlpha =0;
}

//-----------------------------------------------------------------------

void cLuxEffect_Flash::Update(float afTimeStep)
{
	if(mlStep ==0)
	{
		mfAlpha += mfFadeInSpeed * afTimeStep;
		if(mfAlpha >= 1.0f)
		{
			mfAlpha = 1.0f;
			mlStep=1;
			mfCount = 1;
		}
	}
	else if(mlStep ==1)
	{
		mfCount -= mfWhiteSpeed * afTimeStep;
		if(mfCount <= 0)
		{
			mlStep = 2;
		}
	}
	else if(mlStep ==2)
	{
		mfAlpha -= mfFadeOutSpeed * afTimeStep;
		if(mfAlpha <= 0.0f)
		{
			mbActive = false;
		}
	}

}

//-----------------------------------------------------------------------

void cLuxEffect_Flash::OnDraw(float afFrameTime)
{
	gpBase->mpGameHudSet->DrawGfx(mpWhiteGfx,gpBase->mvHudVirtualStartPos+cVector3f(0,0,3.2f),gpBase->mvHudVirtualSize,cColor(mfAlpha, 1));
}	


//////////////////////////////////////////////////////////////////////////
// PLAY VOICE
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cLuxEffect_PlayVoice::cLuxEffect_PlayVoice()
{
	mpSoundHandler = gpBase->mpEngine->GetSound()->GetSoundHandler();

	mvFontSize = gpBase->mpGameCfg->GetVector2f("Effects","VoiceTextFontSize",1);
	mfRowWidth = gpBase->mpGameCfg->GetFloat("Effects","VoiceTextRowWidth",1);

	mpVoiceEntry = NULL;
	mlVoiceEntryID = -1;
	mpEffectEntry = NULL;
	mlEffectEntryID = -1;

	Reset();
}

cLuxEffect_PlayVoice::~cLuxEffect_PlayVoice()
{
	Reset();
}

//-----------------------------------------------------------------------

void cLuxEffect_PlayVoice::StopVoices(float afFadeOutSpeed)
{
	if(mpSoundHandler->IsValid(mpVoiceEntry,mlVoiceEntryID)) mpVoiceEntry->FadeOut(afFadeOutSpeed);
	mpVoiceEntry = NULL;

	if(mpSoundHandler->IsValid(mpEffectEntry,mlEffectEntryID)) mpEffectEntry->FadeOut(afFadeOutSpeed);
	mpEffectEntry = NULL;
	
	mlstVoices.clear();

	mbActive = false;
}
//-----------------------------------------------------------------------

void cLuxEffect_PlayVoice::AddVoice(const tString& asVoiceFile, const tString& asEffectFile,
									const tString& asTextCat, const tString& asTextEntry, bool abUsePostion, 
									const cVector3f& avPosition, float afMinDistance, float afMaxDistance, int alPriority, bool abRemoveInterrupted)
{
	AddMultiSubbedVoice(
		asVoiceFile, asEffectFile, asTextCat,
		asTextEntry, 0.0f,
		"", 0.0f,
		"", 0.0f,
		"", 0.0f,
		"", 0.0f,
		"", 0.0f,
		"", 0.0f,
		abUsePostion, avPosition, afMinDistance, afMaxDistance, alPriority, abRemoveInterrupted );
}

//-----------------------------------------------------------------------

void cLuxEffect_PlayVoice::AddMultiSubbedVoice(
	const tString& asVoiceFile, const tString& asEffectFile, const tString& asTextCat,
	const tString& asTextEntry, float afTextDelay,
	const tString& asText2Entry, float afText2Delay,
	const tString& asText3Entry, float afText3Delay,
	const tString& asText4Entry, float afText4Delay,
	const tString& asText5Entry, float afText5Delay,
	const tString& asText6Entry, float afText6Delay,
	const tString& asText7Entry, float afText7Delay,
	bool abUsePostion, 
	const cVector3f& avPosition, float afMinDistance, float afMaxDistance, int alPriority, bool abRemoveInterrupted
	)
{
	cLuxVoiceData voiceData;

	//Log("Adding sounds: '%s' and '%s'\n", asVoiceFile.c_str(), asEffectFile.c_str());
	
    voiceData.msVoiceFile = asVoiceFile;
	voiceData.msEffectFile = asEffectFile;
	
	if(asTextCat != "" && asTextEntry != "")
	{
		voiceData.msText = kTranslate(asTextCat, asTextEntry);
		voiceData.mfTextDelay = afTextDelay;
	}
	else
	{
		voiceData.msText = _W("");
		voiceData.mfTextDelay = 0.0f;
	}

	if(asTextCat != "" && asText2Entry != "")
	{
		voiceData.msText2 = kTranslate(asTextCat, asText2Entry);
		voiceData.mfText2Delay = afText2Delay;
	}
	else
	{
		voiceData.msText2 = _W("");
		voiceData.mfText2Delay = 0.0f;
	}

	if(asTextCat != "" && asText3Entry != "")
	{
		voiceData.msText3 = kTranslate(asTextCat, asText3Entry);
		voiceData.mfText3Delay = afText3Delay;
	}
	else
	{
		voiceData.msText3 = _W("");
		voiceData.mfText3Delay = 0.0f;
	}

	if(asTextCat != "" && asText4Entry != "")
	{
		voiceData.msText4 = kTranslate(asTextCat, asText4Entry);
		voiceData.mfText4Delay = afText4Delay;
	}
	else
	{
		voiceData.msText4 = _W("");
		voiceData.mfText4Delay = 0.0f;
	}

	if(asTextCat != "" && asText5Entry != "")
	{
		voiceData.msText5 = kTranslate(asTextCat, asText5Entry);
		voiceData.mfText5Delay = afText5Delay;
	}
	else
	{
		voiceData.msText5 = _W("");
		voiceData.mfText5Delay = 0.0f;
	}

	if(asTextCat != "" && asText6Entry != "")
	{
		voiceData.msText6 = kTranslate(asTextCat, asText6Entry);
		voiceData.mfText6Delay = afText6Delay;
	}
	else
	{
		voiceData.msText6 = _W("");
		voiceData.mfText6Delay = 0.0f;
	}

	if(asTextCat != "" && asText7Entry != "")
	{
		voiceData.msText7 = kTranslate(asTextCat, asText7Entry);
		voiceData.mfText7Delay = afText7Delay;
	}
	else
	{
		voiceData.msText7 = _W("");
		voiceData.mfText7Delay = 0.0f;
	}

	voiceData.mbUsePosition = abUsePostion;
	voiceData.mvPosition = avPosition;
	voiceData.mfMinDistance = afMinDistance;
	voiceData.mfMaxDistance = afMaxDistance;
	voiceData.mlPriority = alPriority;
	voiceData.mfCurrentTime = 0;
	voiceData.mfInterruptedAt = -1;

	if(mbActive && mlstVoices.empty() == false)
	{
		cLuxVoiceData& currentVoiceData = mlstVoices.front();

		if(currentVoiceData.mlPriority <= alPriority)
		{
			/////////////////
			// Replace the current voice
			if(mpSoundHandler->IsValid(mpVoiceEntry,mlVoiceEntryID))
			{
				mpVoiceEntry->FadeOut(0.5f);
				currentVoiceData.mfCurrentTime = mpVoiceEntry->GetChannel()->GetElapsedTime();
				voiceData.mfInterruptedAt = gpBase->mpEngine->GetGameTime();
			}
			
			mpVoiceEntry = NULL;

			if(mpSoundHandler->IsValid(mpEffectEntry,mlEffectEntryID)) mpEffectEntry->FadeOut(0.5f);
			mpEffectEntry = NULL;
			
			/////////////
			// Remove the old one and place this first
			if(abRemoveInterrupted)
			{
				mlstVoices.pop_front();
			}

			mlstVoices.push_front(voiceData);
		}
		else
		{
			/////////////
		// Add to the back
			mlstVoices.push_back(voiceData);
		}
	}
	else
	{
		/////////////
		// Add to the back
		mlstVoices.push_back(voiceData);
	}

	mbActive = true;
}

//-----------------------------------------------------------------------

void cLuxEffect_PlayVoice::PauseCurrentVoices()
{
	if(mbActive==false || mbPaused) return;

	mbPaused = true;
	
	//Voice
	if(mpVoiceEntry && mpSoundHandler->IsValid(mpVoiceEntry, mlVoiceEntryID))
	{
		mpVoiceEntry->SetPaused(true);
	}

	//Effect
	if(mpEffectEntry && mpSoundHandler->IsValid(mpEffectEntry, mlEffectEntryID))
	{
		mpEffectEntry->SetPaused(true);
	}
}

//-----------------------------------------------------------------------

void cLuxEffect_PlayVoice::UnpauseCurrentVoices()
{
	if(mbActive==false || mbPaused==false) return;
	
	mbPaused = false;

	//Voice
	if(mpVoiceEntry && mpSoundHandler->IsValid(mpVoiceEntry, mlVoiceEntryID))
	{
		mpVoiceEntry->SetPaused(false);
	}

	//Effect
	if(mpEffectEntry && mpSoundHandler->IsValid(mpEffectEntry, mlEffectEntryID))
	{
		mpEffectEntry->SetPaused(false);
	}
}

//-----------------------------------------------------------------------

static bool SortVoiceData(const cLuxVoiceData& aLhs, const cLuxVoiceData& aRhs)
{
	return aLhs.mlPriority > aRhs.mlPriority;
}

void cLuxEffect_PlayVoice::Update(float afTimeStep)
{
	//do not want to have like this, because then loading save when playing last voice + callback will not work and callback will not be called.
	//if(mpVoiceEntry==NULL && mpEffectEntry==NULL && mlstVoices.empty()) return; 

	for(size_t i=0; i<mvTextEntryQueue.size(); ++i)
	{
		mvTextEntryQueue[i].mfDelay -= afTimeStep;
	}

	while ( !mvTextEntryQueue.empty() && mvTextEntryQueue.front().mfDelay <= 0.0f )
	{
		mvCurrentTextRows.clear();
		gpBase->mpDefaultFont->GetWordWrapRows(mfRowWidth,mvFontSize.y+2,mvFontSize, mvTextEntryQueue.front().msTextEntry, &mvCurrentTextRows);
		mvTextEntryQueue.pop_front();
	}

	if(mfVolumeMul <1.0f)
	{
		if(mpVoiceEntry && mpSoundHandler->IsValid(mpVoiceEntry, mlVoiceEntryID))
			mpVoiceEntry->SetVolumeMul(mfVolumeMul);

		if(mpEffectEntry && mpSoundHandler->IsValid(mpEffectEntry, mlEffectEntryID))
			mpEffectEntry->SetVolumeMul(mfVolumeMul);
	}
	
	if(mpSoundHandler->IsValid(mpVoiceEntry, mlVoiceEntryID)) return;
	if(mpVoiceEntry==NULL && mpSoundHandler->IsValid(mpEffectEntry, mlEffectEntryID)) return;

	if(mlstVoices.empty())
	{
		//Need to save as it will be reseted otherwise!
		tString sCallback = msOverCallback; 

		//Reset before calling so it is possible to start voices from callback!
		float fPreVolMul = mfVolumeMul;
		Reset();
		mfVolumeMul = fPreVolMul;
		SetActive(false);

		if(sCallback!="")
			gpBase->mpMapHandler->GetCurrentMap()->RunScript(sCallback+"()");
		
		return;
	}

	////////////
	// Sort the priorities
	mlstVoices.sort(SortVoiceData);

    cLuxVoiceData& voiceData = mlstVoices.front();
	
	double fStartTime = voiceData.mfCurrentTime;

	if(voiceData.mfInterruptedAt != -1.0)
	{
		////////////////
		// Add the elapsed time since interrupting it
		fStartTime += gpBase->mpEngine->GetGameTime() - voiceData.mfInterruptedAt;
	}

	//////////////////////
	//GUI sound
	if(voiceData.mbUsePosition==false)	
	{
		mpVoiceEntry = mpSoundHandler->PlayGuiStream(voiceData.msVoiceFile,false, 1.0f);
		if(mpVoiceEntry)
		{
			mlVoiceEntryID = mpVoiceEntry->GetId();
			mpVoiceEntry->GetSoundChannel()->SetElapsedTime(fStartTime);
		}

		if(voiceData.msEffectFile!="")
		{
			mpEffectEntry = mpSoundHandler->PlayGuiStream(voiceData.msEffectFile,false, 1.0f);
			if(mpEffectEntry) 
			{
				mlEffectEntryID = mpEffectEntry->GetId();
				mpEffectEntry->GetSoundChannel()->SetElapsedTime(fStartTime);
			}
		}
	}
	//////////////////////
	//3D sound with position
	else
	{
		mpVoiceEntry = mpSoundHandler->Play(voiceData.msVoiceFile,false, 1.0f, voiceData.mvPosition,voiceData.mfMinDistance, voiceData.mfMaxDistance,
											eSoundEntryType_Gui,false,true,0, true);
		if(mpVoiceEntry)
		{
			mlVoiceEntryID = mpVoiceEntry->GetId();
			mpVoiceEntry->GetSoundChannel()->SetElapsedTime(fStartTime);
		}
		
		if(voiceData.msEffectFile!="")
		{
			mpEffectEntry = mpSoundHandler->Play(	voiceData.msEffectFile,false, 1.0f, voiceData.mvPosition,voiceData.mfMinDistance, voiceData.mfMaxDistance,
													eSoundEntryType_Gui,false,true,0, true);
			if(mpEffectEntry) 
			{
				mlEffectEntryID = mpEffectEntry->GetId();
				mpEffectEntry->GetSoundChannel()->SetElapsedTime(fStartTime);
			}
		}
	}

	//////////////////////
	//Text
	mvCurrentTextRows.clear();
	mvTextEntryQueue.clear();

	if(voiceData.msText != _W(""))
	{
		if ( voiceData.mfTextDelay <= 0.0f )
		{
			gpBase->mpDefaultFont->GetWordWrapRows(mfRowWidth,mvFontSize.y+2,mvFontSize, voiceData.msText, &mvCurrentTextRows);
		}
		else
		{
			cTextQueueEntry entry;
			entry.mfDelay = voiceData.mfTextDelay;
			entry.msTextEntry = voiceData.msText;
			mvTextEntryQueue.push_back( entry );
		}
	}

	if(voiceData.msText2 != _W(""))
	{
		cTextQueueEntry entry;
		entry.mfDelay = voiceData.mfText2Delay;
		entry.msTextEntry = voiceData.msText2;
		mvTextEntryQueue.push_back( entry );
	}

	if(voiceData.msText3 != _W(""))
	{
		cTextQueueEntry entry;
		entry.mfDelay = voiceData.mfText3Delay;
		entry.msTextEntry = voiceData.msText3;
		mvTextEntryQueue.push_back( entry );
	}

	if(voiceData.msText4 != _W(""))
	{
		cTextQueueEntry entry;
		entry.mfDelay = voiceData.mfText4Delay;
		entry.msTextEntry = voiceData.msText4;
		mvTextEntryQueue.push_back( entry );
	}

	if(voiceData.msText5 != _W(""))
	{
		cTextQueueEntry entry;
		entry.mfDelay = voiceData.mfText5Delay;
		entry.msTextEntry = voiceData.msText5;
		mvTextEntryQueue.push_back( entry );
	}

	if(voiceData.msText6 != _W(""))
	{
		cTextQueueEntry entry;
		entry.mfDelay = voiceData.mfText6Delay;
		entry.msTextEntry = voiceData.msText6;
		mvTextEntryQueue.push_back( entry );
	}

	if(voiceData.msText7 != _W(""))
	{
		cTextQueueEntry entry;
		entry.mfDelay = voiceData.mfText7Delay;
		entry.msTextEntry = voiceData.msText7;
		mvTextEntryQueue.push_back( entry );
	}

	//////////////////////
	//Pop!
	mlstVoices.pop_front();

	//////////////////////
	//Extra check in case the voices does not load.
	if(mlstVoices.empty() && mpVoiceEntry==NULL && mpEffectEntry==NULL)
	{
		//Reset before calling so it is possible to start voices from callback!
		Reset();
		SetActive(false);
		
		if(msOverCallback!="")
			gpBase->mpMapHandler->GetCurrentMap()->RunScript(msOverCallback+"()");
	}
}

//-----------------------------------------------------------------------

void cLuxEffect_PlayVoice::OnDraw(float afFrameTime)
{
	if(gpBase->mpMessageHandler->ShowSubtitles()==false) return;
	if(mvCurrentTextRows.empty()) return;
    
	cVector3f vStartPos(400-mfRowWidth/2, 580 - (mvCurrentTextRows.size()*(mvFontSize.y+2)), 4);
	
    for(size_t i=0; i<mvCurrentTextRows.size(); ++i)
	{
		gpBase->mpGameHudSet->DrawFont(mvCurrentTextRows[i],gpBase->mpDefaultFont, vStartPos, mvFontSize,cColor(1,1));
		vStartPos.y+= mvFontSize.y+2;
	}
}
//-----------------------------------------------------------------------

void cLuxEffect_PlayVoice::Reset()
{
	if(mpVoiceEntry != NULL && mpSoundHandler->IsValid(mpVoiceEntry,mlVoiceEntryID)) mpVoiceEntry->Stop();
	mpVoiceEntry = NULL;

	if(mpEffectEntry != NULL && mpSoundHandler->IsValid(mpEffectEntry,mlEffectEntryID)) mpEffectEntry->Stop();
	mpEffectEntry = NULL;
	
	mvCurrentTextRows.clear();

	mlstVoices.clear();
	
	msOverCallback = "";

	mbPaused = false;

	mfVolumeMul = 1.0f;
}

void cLuxEffect_PlayVoice::SetVolumeMul(float afMul)
{
	mfVolumeMul = afMul;

	if(mpVoiceEntry && mpSoundHandler->IsValid(mpVoiceEntry, mlVoiceEntryID))
		mpVoiceEntry->SetVolumeMul(mfVolumeMul);

	if(mpEffectEntry && mpSoundHandler->IsValid(mpEffectEntry, mlEffectEntryID))
		mpEffectEntry->SetVolumeMul(mfVolumeMul);    
}

//-----------------------------------------------------------------------

bool cLuxEffect_PlayVoice::VoiceDonePlaying()
{
	if(mlstVoices.empty() && mpVoiceEntry==NULL)
	{
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------


//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cLuxEffectHandler::OnClearFonts()
{
	mpEmotionFlash->ClearFonts();
}

void cLuxEffectHandler::LoadFonts()
{
	mpEmotionFlash->LoadFonts();
}

void cLuxEffectHandler::OnStart()
{
}

//-----------------------------------------------------------------------


void cLuxEffectHandler::Reset()
{
	/////////////////////////
	// Effects
	for(size_t i=0; i<mvEffects.size(); ++i)
	{
		iLuxEffect *pEffect = mvEffects[i];
		pEffect->Reset();
		if ( ! pEffect->IsAlwaysOn() )
		{
			pEffect->SetActive(false);
		}
	}

	/////////////////////////
	// World sound mul
    for(size_t i=0; i<eLuxGlobalVolumeType_LastEnum; ++i)
	{
		gpBase->mpEngine->GetSound()->GetSoundHandler()->SetGlobalSpeed(1, eSoundEntryType_World, i);
		gpBase->mpEngine->GetSound()->GetSoundHandler()->SetGlobalVolume(1, eSoundEntryType_World, i);
	}

	mbPlayerIsPaused = false;
}

//-----------------------------------------------------------------------

void cLuxEffectHandler::Update(float afTimeStep)
{
	for(size_t i=0; i<mvEffects.size(); ++i)
	{
		iLuxEffect *pEffect = mvEffects[i];
		if(pEffect->IsActive()) pEffect->Update(afTimeStep);
	}
}

//-----------------------------------------------------------------------

void cLuxEffectHandler::OnMapEnter(cLuxMap *apMap)
{
	
}

//-----------------------------------------------------------------------

void cLuxEffectHandler::OnMapLeave(cLuxMap *apMap)
{
	//////////////////
	// Reset some effects on map leave
	mpSepiaColor->FadeTo(0, 1);
	mpRadialBlur->FadeTo(0, 1);
    if(mpPlayCommentary->IsActive()) mpPlayCommentary->Stop();
}

//-----------------------------------------------------------------------



void cLuxEffectHandler::OnDraw(float afFrameTime)
{
	for(size_t i=0; i<mvEffects.size(); ++i)
	{
		iLuxEffect *pEffect = mvEffects[i];
		if(pEffect->IsActive()) pEffect->OnDraw(afFrameTime);
	}
}

//-----------------------------------------------------------------------

void cLuxEffectHandler::DoAction(eLuxPlayerAction aAction, bool abPressed)
{
	for(size_t i=0; i<mvEffects.size(); ++i)
	{
		iLuxEffect *pEffect = mvEffects[i];
		if(pEffect->IsActive()) pEffect->DoAction(aAction, abPressed);
	}
}

//-----------------------------------------------------------------------

void cLuxEffectHandler::SetPlayerIsPaused(bool abX)
{
	mbPlayerIsPaused = abX;
	gpBase->mpPlayer->SetActive(!abX);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------


//-----------------------------------------------------------------------



