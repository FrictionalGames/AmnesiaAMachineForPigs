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

#include "LuxHandObject_LightSource.h"

#include "LuxMap.h"
#include "LuxPlayer.h"
#include "LuxPlayerHands.h"
#include "LuxEnemy.h"
#include "LuxMapHelper.h"
#include "LuxMapHandler.h"
#include "LuxHelpFuncs.h"
#include "LuxDebugHandler.h"
#include <limits>

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cLuxHandObject_LightSource::cLuxHandObject_LightSource(const tString& asName, cLuxPlayerHands *apHands) : iLuxHandObject(asName, apHands)
{
	mfSwayAngle =0;
	mfSwayVel =0;

    mbFlickering = false;
	mfFlickeringSpeed =1.0f;
	mlFlickeringState =1;

	mfFlickerAmount=1;
	mfFlickerTime=0;
	mfFlickerPauseTime =0;

	mfStrobeDroneTime=0;
	mpDroneSound=NULL;
	mlDroneSoundId=-1;
}

cLuxHandObject_LightSource::~cLuxHandObject_LightSource()
{
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cLuxHandObject_LightSource::LoadImplementedVars(cXmlElement *apVarsElem)
{
	float fFadeInTime = apVarsElem->GetAttributeFloat("FadeInTime", 0);
	float fFadeOutTime = apVarsElem->GetAttributeFloat("FadeOutTime", 0);
	if(fFadeInTime == 0) fFadeInTime = 0.001f;
	if(fFadeOutTime == 0) fFadeOutTime = 0.001f;
	mfFadeInSpeed = 1.0f / fFadeInTime;
	mfFadeOutSpeed = 1.0f / fFadeOutTime;

	mbHasSwayPhysics = apVarsElem->GetAttributeBool("HasSwayPhysics", false);
	mfMaxSwayVel = apVarsElem->GetAttributeFloat("MaxSwayVel", 0);
	mvSwayAngleLimits = cMath::Vector2ToRad(apVarsElem->GetAttributeVector2f("SwayAngleLimits", 0));
	mvSwayDownAngleLimits= cMath::Vector2ToRad(apVarsElem->GetAttributeVector2f("SwayDownAngleLimits", 0));
	
	mfSwayGravity = apVarsElem->GetAttributeFloat("SwayGravity", 0);
	mfSwayFriction = apVarsElem->GetAttributeFloat("SwayFriction", 0);
	mvSwayPinDir = apVarsElem->GetAttributeVector3f("SwayPinDir", 1);
	mvSwayPinDir.Normalize();
	mfSwayPlayerSpeedMul = apVarsElem->GetAttributeFloat("SwayPlayerSpeedMul", 0);
	mfSwayCameraRollMul = apVarsElem->GetAttributeFloat("SwayCameraRollMul", 0);

    
	msSkipSwaySubMesh = apVarsElem->GetAttributeString("SkipSwaySubMesh", "");
}

//-----------------------------------------------------------------------

void cLuxHandObject_LightSource::ImplementedCreateEntity(cLuxMap *apMap)
{
	mvDefaultLightColors.resize(mvLights.size());
	mvDefaultLightFlicker.resize(mvLights.size());
	mvLightFadeOutColor.resize(mvLights.size());

	for(size_t i=0; i<mvLights.size(); ++i)
	{
		mvDefaultLightColors[i] = mvLights[i]->GetDiffuseColor();
		mvDefaultLightFlicker[i] =mvLights[i]->GetFlickerActive();
		mvLights[i]->SetFlickerActive(false);
	}

	if ( mpMeshEntity )
	{
		mvDefaultSubMeshMatrix.resize(mpMeshEntity->GetSubMeshEntityNum());
		for(size_t i=0; i<mpMeshEntity->GetSubMeshEntityNum(); ++i)
		{
			cSubMeshEntity *pSubEnt = mpMeshEntity->GetSubMeshEntity(i);
			mvDefaultSubMeshMatrix[i] = pSubEnt->GetLocalMatrix();
		}
	}

	mvDefaultLightMatrix.resize(mvLights.size());
	for(size_t i=0; i<mvLights.size(); ++i)
	{
		mvDefaultLightMatrix[i] = mvLights[i]->GetLocalMatrix();
	}
		
	mvDefaultBillboardMatrix.resize(mvBillboards.size());
	for(size_t i=0; i<mvBillboards.size(); ++i)
	{
		mvDefaultBillboardMatrix[i] = mvBillboards[i]->GetLocalMatrix();
	}
}

//-----------------------------------------------------------------------

void cLuxHandObject_LightSource::ImplementedDestroyEntity(cLuxMap *apMap)
{
}

//-----------------------------------------------------------------------

void cLuxHandObject_LightSource::ImplementedReset()
{
	mfSwayAngle =0;
	mfSwayVel =0;
}

//-----------------------------------------------------------------------

void cLuxHandObject_LightSource::Update(float afTimeStep)
{
	bool bUpdate = true;
	float fAlpha = 1;
	
    ///////////////////
	// Sway Physics
    if(mbHasSwayPhysics)	
	{
		UpdateSwayPhysics(afTimeStep);	
	}

	///////////////////
	// Fade out
	if(mpHands->GetState() == eLuxHandsState_Holster)
	{
		mpHands->mfHandObjectAlpha -= mfFadeOutSpeed * afTimeStep;
		if(mpHands->mfHandObjectAlpha < 0.0f) mpHands->mfHandObjectAlpha = 0.0f;

		if(mpDroneSound)
		{
			cSoundHandler *pSoundHandler = gpBase->mpEngine->GetSound()->GetSoundHandler();

			if(pSoundHandler->IsValid(mpDroneSound, mlDroneSoundId))
				mpDroneSound->FadeOut(0.5f);
			mpDroneSound = NULL;
		}
	}
	///////////////////
	// Fade in
	else if(mpHands->mfHandObjectAlpha < 1.0f)
	{
		mpHands->mfHandObjectAlpha += mfFadeInSpeed * afTimeStep;
		if(mpHands->mfHandObjectAlpha > 1.0f)
		{
			mpHands->mfHandObjectAlpha = 1.0f;
		}
	}

	///////////////////
	// Flickering
	float fFlicker = UpdateFlickering(afTimeStep);
	
	///////////////////
	// Set alpha
	if(bUpdate) fAlpha = mpHands->mfHandObjectAlpha * fFlicker;

	///////////////////
	// Calculate fade out color
	if(mpHands->GetState() != eLuxHandsState_Holster)
	{
		for(size_t i=0; i<mvLights.size(); ++i)
		{
			mvLightFadeOutColor[i] = mvLights[i]->GetDiffuseColor();
		}
	}
	

	///////////////////
	// Set alpha
	if(bUpdate)
	{
		if ( mpMeshEntity ) mpMeshEntity->SetIlluminationAmount(fAlpha);
       
		for(size_t i=0; i<mvBillboards.size(); ++i)
		{
			cColor col = mvBillboards[i]->GetColor();
			col.a = fAlpha;
			mvBillboards[i]->SetColor(col);
		}
		
		for(size_t i=0; i<mvParticleSystems.size(); ++i)
		{
			cColor col = mvParticleSystems[i]->GetColor();
			col.a = fAlpha;
			mvParticleSystems[i]->SetColor(col);
		}
		
		for(size_t i=0; i<mvLights.size(); ++i)
		{
			if(mpHands->GetState() == eLuxHandsState_Holster)
			{
				mvLights[i]->SetDiffuseColor(mvLightFadeOutColor[i] * fAlpha);
			}
			else
			{
				mvLights[i]->SetDiffuseColor(mvDefaultLightColors[i] * fAlpha);
			}
		}
	}
}

//-----------------------------------------------------------------------

bool cLuxHandObject_LightSource::DoAction(eLuxPlayerAction aAction, bool abPressed)
{
	return false;
}

//-----------------------------------------------------------------------

bool cLuxHandObject_LightSource::AnimationIsOver()
{
	return true;
}

//-----------------------------------------------------------------------

void cLuxHandObject_LightSource::SetFlickering(bool abX)
{
	mbFlickering = abX;

	if(mbFlickering)
	{
		mfFlickerAmount = 1;
		mfFlickerTime = 0.05f;
		mlFlickeringState = 1;
		mfFlickerPauseTime=0;
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cLuxHandObject_LightSource::UpdateSwayPhysics(float afTimeStep)
{
	/////////////////////////////
	// Update vel
	{
		/////////////////////////////
		// Gravity
		cCamera *pCam = gpBase->mpPlayer->GetCamera();
		float fDownAngle = pCam->GetPitch() * -1.0f;
		fDownAngle += pCam->GetRoll() * mfSwayCameraRollMul;
		fDownAngle = cMath::Clamp(fDownAngle, mvSwayDownAngleLimits.x, mvSwayDownAngleLimits.y);

		float fAngle = mfSwayAngle - fDownAngle;

		float fForce = -sin(fAngle) * mfSwayGravity;
		mfSwayVel += fForce * afTimeStep;

		/////////////////////////////
		// Player velocity
        iCharacterBody *pCharBody = gpBase->mpPlayer->GetCharacterBody();
        float fPlayerSpeed = pCharBody->GetVelocity(gpBase->mpEngine->GetStepSize()).Length();	
		if(pCharBody->GetMoveSpeed(eCharDir_Forward)<0) fPlayerSpeed = -fPlayerSpeed;

		mfSwayVel += -fPlayerSpeed * mfSwayPlayerSpeedMul;

		/////////////////////////////
		// Cap and Friction
		if(mfSwayVel> mfMaxSwayVel)		mfSwayVel = mfMaxSwayVel;
		if(mfSwayVel< -mfMaxSwayVel)	mfSwayVel = -mfMaxSwayVel;

		mfSwayVel -= mfSwayVel*mfSwayFriction*afTimeStep;
	}

	/////////////////////////////
	// Update Angle
    mfSwayAngle += mfSwayVel * afTimeStep;

	//Min
	if(mfSwayAngle < mvSwayAngleLimits.x)
	{
		mfSwayVel =0;
		mfSwayAngle = mvSwayAngleLimits.x;
	}

	//Max
	if(mfSwayAngle > mvSwayAngleLimits.y)
	{
		mfSwayVel =0;
		mfSwayAngle = mvSwayAngleLimits.y;
	}
	
	/////////////////////////////
	// Update Model matrix
	cMatrixf mtxSway = cMath::MatrixRotate(mvSwayPinDir * mfSwayAngle, eEulerRotationOrder_XYZ);
	cMatrixf mtxSwayLight = cMath::MatrixRotate(mvSwayPinDir * mfSwayAngle * 0.125f, eEulerRotationOrder_XYZ);
	//cMatrixf mtxTrans = cMath::MatrixMul(m_mtxOffset, mtxSway);
	//mpMeshEntity->SetMatrix(mtxTrans);

	if ( mpMeshEntity )
	{
		for(size_t i=0; i<mpMeshEntity->GetSubMeshEntityNum(); ++i)
		{
			cSubMeshEntity *pSubEnt = mpMeshEntity->GetSubMeshEntity(i);
			if(pSubEnt->GetSubMesh()->GetName() == msSkipSwaySubMesh) continue;
			//Log("'%s'\n",pSubEnt->GetSubMesh()->GetName().c_str());
		
			pSubEnt->SetMatrix(cMath::MatrixMul(mtxSway, mvDefaultSubMeshMatrix[i]) );
		}
		mpMeshEntity->SetMatrix(m_mtxOffset);
	}

	for(size_t i=0; i<mvLights.size(); ++i)
	{
		mvLights[i]->SetMatrix(cMath::MatrixMul(mtxSwayLight, mvDefaultLightMatrix[i]) );
	}
	
	for(size_t i=0; i<mvBillboards.size(); ++i)
	{
		mvBillboards[i]->SetMatrix(cMath::MatrixMul(mtxSway, mvDefaultBillboardMatrix[i]) );
	}
	// Update 
}

//----------------------------------------------------------------------

float cLuxHandObject_LightSource::UpdateFlickering(float afTimeStep)
{
	if(mpHands->GetState() == eLuxHandsState_Holster) return 1.0f;

	////////////////////////////
	// See if an enemy is in the spotlight.
	bool bEnemyInSpotLight = false;
	cLuxMap* pMap = gpBase->mpMapHandler->GetCurrentMap();
	cLuxEnemyIterator it = pMap->GetEnemyIterator();
	while(it.HasNext())
	{
		iLuxEnemy* pEnemy = it.Next();
		if(pEnemy->IsActive()==false) continue;
		if(pEnemy->GetInLanternLightCount()>0)
		{
			bEnemyInSpotLight=true;
			break;
		}
	}

	////////////////////////////
	// Strobe drone
	cSoundHandler *pSoundHandler = gpBase->mpEngine->GetSound()->GetSoundHandler();

	if(bEnemyInSpotLight) mfStrobeDroneTime = 1.0f;
	if(mfStrobeDroneTime>0)
	{
		mfStrobeDroneTime -= afTimeStep;

		if(mpDroneSound==NULL || pSoundHandler->IsValid(mpDroneSound, mlDroneSoundId)==false)
		{
			mpDroneSound = pSoundHandler->PlayGui("ui_lanterndrone.ogg", true, 1);
			if(mpDroneSound) {
				mlDroneSoundId = mpDroneSound->GetId();
				mpDroneSound->FadeIn(1.0f, 0.5f);
			}
		}
	}
	else
	{
		if(mpDroneSound)
		{
			if(pSoundHandler->IsValid(mpDroneSound, mlDroneSoundId))
				mpDroneSound->FadeOut(0.5f);
			mpDroneSound = NULL;
		}
	}

	////////////////////////////
	// If In spotlight, do a strobing effect
	if(bEnemyInSpotLight)
	{
		//Make sure within bounds
		if(mfFlickerTime<0.01) mfFlickerTime = 0.025f; 
		if(mfFlickerTime>0.1) mfFlickerTime = 0.1f; 

		//Puase
		if(mfFlickerPauseTime>0)
		{
			mfFlickerPauseTime-=afTimeStep;
			return mfFlickerAmount;
		}
		
		////////////////////////////
		// Fade in
		if(mlFlickeringState==1)
		{
			mfFlickerAmount += afTimeStep*(1.0f/mfFlickerTime);
			if(mfFlickerAmount>=1) 
			{
				mfFlickerAmount=1;
				mfFlickerTime = cMath::RandRectf(0.1f, 1.0f);
				mlFlickeringState=0;
				gpBase->mpHelpFuncs->PlayGuiSoundData("lantern_flicker_strobe", eSoundEntryType_World);
			}
		}
		////////////////////////////
		// Fade out
		else if(mlFlickeringState==0)
		{
			mfFlickerAmount -= afTimeStep*(1.0f/mfFlickerTime);
			if(mfFlickerAmount<=0)
			{
				if(cMath::RandRectl(0, 5)==0)
					mfFlickerPauseTime = cMath::RandRectf(0, 1);

				mfFlickerAmount=0;
				mfFlickerTime = cMath::RandRectf(0.025f, 0.2f);
				mlFlickeringState=1;
				gpBase->mpHelpFuncs->PlayGuiSoundData("lantern_flicker_strobe", eSoundEntryType_World);
			}
		}

		return mfFlickerAmount;
	}

	////////////////////////////
	// Check if active
    if(mbFlickering==false)
	{
		mfFlickerAmount += afTimeStep*12;
		if(mfFlickerAmount>=1) mfFlickerAmount=1;
		return mfFlickerAmount;
	}

	////////////////////////////
	// Fade in
	if(mlFlickeringState==1 && mfFlickerAmount<1)
	{
		mfFlickerAmount += afTimeStep*35;
		if(mfFlickerAmount>=1) mfFlickerAmount=1;
	}
	////////////////////////////
	// Fade out
	else if(mlFlickeringState==0 && mfFlickerAmount>0)
	{
		mfFlickerAmount -= afTimeStep*35;
		if(mfFlickerAmount<=0) mfFlickerAmount=0;
	}
	////////////////////////////
	// Check of time for state switch
	else
	{
		mfFlickerTime -= afTimeStep*mfFlickeringSpeed;
		if(mfFlickerTime<=0)
		{
			///////////////////////////
			// Off -> On
			if(mlFlickeringState == 1)
			{
				mfFlickerTime = cMath::RandRectf(0.0f, 0.15f);
				mlFlickeringState =0;
				gpBase->mpHelpFuncs->PlayGuiSoundData("lantern_flicker", eSoundEntryType_World);
			}
			///////////////////////////
			// On -> Off
			else
			{
				mfFlickerTime = cMath::RandRectf(0.1f, 0.8f);
				mlFlickeringState =1;
				gpBase->mpHelpFuncs->PlayGuiSoundData("lantern_flicker", eSoundEntryType_World);
			}
		}
	}

	return mfFlickerAmount;
}

//----------------------------------------------------------------------