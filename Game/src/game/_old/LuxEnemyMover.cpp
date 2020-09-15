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

#include "LuxEnemyMover.h"

#include "LuxEnemy.h"
#include "LuxMap.h"
#include "LuxMapHelper.h"

//-----------------------------------------------------------------------

std::vector<cVector3f> cLuxEnemyMover::mvPrecalcSampleDirs;

#define kNumOfDirectionPartitions (4)

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cLuxEnemyMover::cLuxEnemyMover(iLuxEnemy *apEnemy, iCharacterBody *apCharBody)
{
	mpEnemy = apEnemy;
	mpCharBody = apCharBody;

	mbTurning = false;
	mfTurnGoalAngle =0;
	mfTurnSpeed =0;
	mfTurnBreakAcc =0;

	mfStuckLimit = 0.3f;
	mfMaxStuckCounter = 2.0f;

	mfStuckCounter =0;

	mMoveState = eLuxEnemyMoveState_LastEnum;
	mbOverideMoveState = false;

	mbWallAvoidActive = false;
	mfWallAvoidRadius = 0;
	mfWallAvoidSteerAmount = 0;
	mlSampleMaxPartCount = 0;

	mvSteeringVec=0;

	mvCurrentGoal=0;

	//////////////////////////
	// Generate sample directions
	if(mvPrecalcSampleDirs.empty())
	{
		cMath::Randomize(1013); //Want to make sure the same directions are always generated.

		///////////////////////////////
		// Setup data
		mvPrecalcSampleDirs.resize(200);
		std::vector<int> vPartionCount; 
		vPartionCount.resize(kNumOfDirectionPartitions, 0);
		int lMaxPartCount = mvPrecalcSampleDirs.size() / kNumOfDirectionPartitions;
		int lPartionsFull=0;

		float TopZ = sin(cMath::ToRad(60));

		///////////////////////////////
		// Generate directions and filter the samples into partitions
		while(1)
		{
			cVector3f vDir = cMath::RandomSphereSurfacePoint(1);
			if(vDir.z<0) vDir.z = -vDir.z;

			/////////////////////////////
			// Calculate the partition
			int lPart = 0;
			cVector3f vAngles = cMath::Vector3ToDeg(cMath::GetAngleFromPoints3D(0, vDir));
			vAngles.x = cMath::GetAngleDistanceDeg(0, vAngles.x);
			vAngles.y = cMath::GetAngleDistanceDeg(0, vAngles.y);

			if(vAngles.x < -35)
			{
				if(vAngles.y > 0)	lPart =0;
				else				lPart =1;
			}
			else if(vAngles.x > 35)
			{
				if(vAngles.y > 0)	lPart =2;
				else				lPart =3;
			}
			else
			{
				//Part of upper circle, just skip
				if(vDir.z > TopZ)
				{
					continue;
				}
				//part of the other pieces
				else
				{
					if(vAngles.x < 0)
					{
						if(vAngles.y > 0)	lPart =0;
						else				lPart =1;
					}
					else
					{
						if(vAngles.y > 0)	lPart =2;
						else				lPart =3;
					}
				}
			}

			//If partition is already full, just skip it.
			if(vPartionCount[lPart]==lMaxPartCount) continue;

			//Add to partition and see if full. 
			mvPrecalcSampleDirs[lPart*lMaxPartCount + vPartionCount[lPart]] = vDir;
			vPartionCount[lPart]++;

			if(vPartionCount[lPart]==lMaxPartCount) lPartionsFull++;

			//Check if all partitions are full
			if(lPartionsFull == (int)vPartionCount.size()) break;
		}

		cMath::Randomize(); //Go pure random again

	}
}

//-----------------------------------------------------------------------

cLuxEnemyMover::~cLuxEnemyMover()
{
	
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cLuxEnemyMover::SetupAfterLoad(cWorld *apWorld)
{
}
//-----------------------------------------------------------------------

void cLuxEnemyMover::OnUpdate(float afTimeStep)
{
	mvSteeringVec =0;
	UpdateWallAvoidance(afTimeStep);

	UpdateStuckCounter(afTimeStep);
	UpdateTurning(afTimeStep);	
	UpdateMoveAnimation(afTimeStep);
	UpdateStepEffects(afTimeStep);
}

//-----------------------------------------------------------------------

void cLuxEnemyMover::MoveToPos(const cVector3f& avFeetPos)
{
	mvCurrentGoal = avFeetPos;

	/////////////////////////////////
	// Turn
	float fDist = cMath::Vector3Dist(mpCharBody->GetPosition(), avFeetPos);
	float fMaxDist = 2.0f + mpCharBody->GetSize().x*2;

	float fSteerAmount = cMath::Min(fDist / fMaxDist, 1.0f);
	cVector3f vSteerVec = mvSteeringVec*fSteerAmount;
	vSteerVec.y =0;

	TurnToPos(avFeetPos + vSteerVec);

	/////////////////////////////////
	// Move
	mpCharBody->Move(eCharDir_Forward,1.0f);
}

//-----------------------------------------------------------------------

void cLuxEnemyMover::MoveBackwardsToPos(const cVector3f& avFeetPos)
{
	TurnAwayFromPos(avFeetPos);
	mpCharBody->Move(eCharDir_Forward,-1.0f);
}

//-----------------------------------------------------------------------

void cLuxEnemyMover::TurnAwayFromPos(const cVector3f& avFeetPos)
{
	cVector3f vStartPos = mpCharBody->GetPosition(); 

	float fGoalAngle = -cMath::GetAngleFromPoints2D(cVector2f(vStartPos.x, vStartPos.z), 
													cVector2f(avFeetPos.x, avFeetPos.z));

	if ( fGoalAngle >= 0 )
	{
		TurnToAngle(fGoalAngle - kPif);
	}
	else
	{
		TurnToAngle(fGoalAngle + kPif);
	}
}

//-----------------------------------------------------------------------

void cLuxEnemyMover::TurnToPos(const cVector3f& avFeetPos)
{
	cVector3f vStartPos = mpCharBody->GetPosition(); 

	float fGoalAngle = -cMath::GetAngleFromPoints2D(cVector2f(vStartPos.x, vStartPos.z), 
													cVector2f(avFeetPos.x, avFeetPos.z));
	TurnToAngle(fGoalAngle);
}

void cLuxEnemyMover::TurnToAngle(float afAngle)
{
	mbTurning = true;

	mfTurnGoalAngle = afAngle;
}

//-----------------------------------------------------------------------

void cLuxEnemyMover::UseMoveStateAnimations()
{
	if(mbOverideMoveState==false) return;
	
	mbOverideMoveState = false;
	mMoveState = eLuxEnemyMoveState_LastEnum;
}

//-----------------------------------------------------------------------

float cLuxEnemyMover::CalculateSpeedMul(float afTimeStep)
{
	float fMul = 1.0f;

	//////////////////////
	//Break when making sharp turns
	if(mbTurning)
	{
		float fAngleDist = std::fabs(cMath::GetAngleDistanceRad(mpCharBody->GetYaw(), mfTurnGoalAngle));

		if(fAngleDist >= mpEnemy->mfTurnMinBreakAngle && mpCharBody->GetMoveSpeed(eCharDir_Forward)>0.15f)
		{
			fMul -= mpEnemy->mfTurnBreakMul * fAngleDist;
			if(fMul<0) fMul =0;
		}

		//Log("%p: %f, %f, %f\n", this, cMath::ToDeg(fAngleDist), fMul, mpCharBody->GetMoveSpeed(eCharDir_Forward));
	}

	return fMul;
}

//-----------------------------------------------------------------------

float cLuxEnemyMover::GetMoveSpeed()
{
	cVector3f vVel = mpCharBody->GetVelocity(gpBase->mpEngine->GetStepSize());
	vVel.y =0;
	return vVel.Length();
}
//-----------------------------------------------------------------------

float cLuxEnemyMover::GetWantedSpeedAmount()
{
	float fWantedSpeed = mpCharBody->GetMoveSpeed(eCharDir_Forward);
	float fRealSpeed = GetMoveSpeed();
	
	if(fabs(fWantedSpeed) < 0.001f) return 1;

	return fRealSpeed / fWantedSpeed;
}

//-----------------------------------------------------------------------

void cLuxEnemyMover::SetupWallAvoidance(float afRadius, float afSteerAmount, int alSamples)
{
	mfWallAvoidRadius = afRadius;
	mfWallAvoidSteerAmount = afSteerAmount;

	mvSampleRays.resize(alSamples);
	mvSampleRayBaseDir.resize(alSamples);
	mvSampleRaysCollide.resize(alSamples, false);
	mvSampleRaysAmount.resize(alSamples, 0);
	mvSamplePartitionUsed.resize(alSamples, -1);

	mlSampleMaxPartCount = (alSamples-1)/kNumOfDirectionPartitions + 1;
}

//-----------------------------------------------------------------------

void cLuxEnemyMover::OnRenderSolid(cRendererCallbackFunctions* apFunctions)
{
	bool bMoving =	fabs(mpCharBody->GetMoveSpeed(eCharDir_Forward))>0.001f ||
					fabs(mpCharBody->GetMoveSpeed(eCharDir_Right))>0.001f;

	/////////////////////////////////////////
	// Steer amount
	if(bMoving && mbWallAvoidActive)
	{
		float fDist = cMath::Vector3Dist(mpCharBody->GetPosition(), mvCurrentGoal);

		float fSteerAmount = cMath::Min(fDist / (mfWallAvoidRadius*2), 1.0f);

		cVector3f vGoal = mvCurrentGoal;
		vGoal.y = mpCharBody->GetPosition().y;

		apFunctions->GetLowLevelGfx()->DrawLine(mpCharBody->GetPosition(), vGoal+ mvSteeringVec*fSteerAmount, cColor(1,0,0));
		apFunctions->GetLowLevelGfx()->DrawSphere(vGoal+ mvSteeringVec*fSteerAmount, 0.3f, cColor(1,0,0));
	}
	/////////////////////////////////////////
	// Wall avoid
	if(mbWallAvoidActive && bMoving)
	{
		
		for(size_t i=0; i<mvSampleRays.size(); ++i)
		{
			cColor col = cColor(0,1,0);
			if(mvSampleRaysCollide[i])
			{
				col = cColor(0,mvSampleRaysAmount[i],1);
			}

			apFunctions->GetLowLevelGfx()->DrawLine(mpCharBody->GetPosition(), mpCharBody->GetPosition()+mvSampleRays[i]*mfWallAvoidRadius,
				col);
		}

		/*int lPartCount = mvPrecalcSampleDirs.size() / kNumOfDirectionPartitions;

		cVector3f vMoveAngles(mpCharBody->GetPitch(), mpCharBody->GetYaw(), 0);
		cMatrixf mtxMoveRot = cMath::MatrixRotate(vMoveAngles, eEulerRotationOrder_XYZ);

		cColor vDebugColors[] = {cColor(1,0,0), cColor(0,1,0), cColor(0,0,1), cColor(1,0,1), cColor(1,1,0), cColor(1,1,1)};
		for(size_t i=0; i<mvPrecalcSampleDirs.size(); ++i)
		{
			cVector3f vLocalDir = mvPrecalcSampleDirs[i];
		
			if(mbUse3DMovement==false) ConvertLocalDirTo2D(vLocalDir);
	
			cVector3f vDir = cMath::MatrixMul(mtxMoveRot, vLocalDir)*-1;
		
			//cVector3f &vDir = mvPrecalcSampleDirs[i];

			//apFunctions->GetLowLevelGfx()->DrawLine(mpCharBody->GetPosition(), mpCharBody->GetPosition()+vDir*2, vDebugColors[i / lPartCount]);
			apFunctions->GetLowLevelGfx()->DrawSphere(mpCharBody->GetPosition()+vDir*2, 0.05f, vDebugColors[i /lPartCount]);
		}*/
	}
}

//-----------------------------------------------------------------------


//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cLuxEnemyMover::ConvertLocalDirTo2D(cVector3f& avLocalDir)
{
	if(fabs(avLocalDir.x) < fabs(avLocalDir.y))
	{
		float fTemp = avLocalDir.x;
		avLocalDir.x = fabs(avLocalDir.y) * cMath::Sign(avLocalDir.x);
		avLocalDir.y = fabs(fTemp) * cMath::Sign(avLocalDir.y) ;
	}

	avLocalDir.y *=0.15f;
	avLocalDir.Normalize();
}

//-----------------------------------------------------------------------

cMatrixf cLuxEnemyMover::GetMovementDirectionMatrix()
{
	cVector3f vMoveAngles(mpCharBody->GetPitch(), mpCharBody->GetYaw(), 0);

	return cMath::MatrixRotate(vMoveAngles, eEulerRotationOrder_XYZ);
}
//-----------------------------------------------------------------------

void cLuxEnemyMover::UpdateStuckCounter(float afTimeStep)
{
	///////////////////////
	// Get the wanted speed and the real speed
	float fWantedSpeed = mpCharBody->GetMoveSpeed(eCharDir_Forward);
	float fRealSpeed = cMath::Vector3Dist(mpCharBody->GetPosition(), mpCharBody->GetLastPosition());
	fRealSpeed = fRealSpeed / afTimeStep;

	///////////////////////
	// Get the wanted direction and real direction
	cVector3f vWantedDir = mpCharBody->GetForward();
	cVector3f vRealDir = mpCharBody->GetPosition() - mpCharBody->GetLastPosition();
	vRealDir.Normalize();

	float fCos = cMath::Vector3Dot(vWantedDir,vRealDir);

	///////////////////////
	// Calculate 
	if( fRealSpeed/fWantedSpeed < mfStuckLimit || (std::fabs(fCos) < 0.3f && fWantedSpeed > 0.001f) )
	{
		mfStuckCounter += afTimeStep ;
		if(mfStuckCounter > mfMaxStuckCounter) mfStuckCounter = mfMaxStuckCounter;
	}
	else
	{
		mfStuckCounter -= afTimeStep*0.8f;
		if(mfStuckCounter<0)mfStuckCounter =0;
	}
}

//-----------------------------------------------------------------------

void cLuxEnemyMover::UpdateTurning(float afTimeStep)
{
	if(mbTurning==false) return;
	
	//////////////////////////////
	//Get distance to goal angle
	float fAngleDist = cMath::GetAngleDistanceRad(mpCharBody->GetYaw(), mfTurnGoalAngle);

	/////////////////
	//Check if close enough to goal
	if(std::fabs(fAngleDist) < 0.001f)
	{
		mbTurning = false;
		mfTurnSpeed =0;
		mfTurnBreakAcc =0;
		return;
	}

	//////////////////////
	//Calculate the turn speed
	mfTurnSpeed = cMath::Min(mpEnemy->mfTurnSpeedMul * std::fabs(fAngleDist), mpEnemy->mfTurnMaxSpeed);

	if(fAngleDist < 0)	mpCharBody->AddYaw(-mfTurnSpeed * afTimeStep);
	else				mpCharBody->AddYaw(mfTurnSpeed * afTimeStep);
}

//-----------------------------------------------------------------------

void cLuxEnemyMover::ForceMoveState(eLuxEnemyMoveState aMoveState)
{
	switch(aMoveState)
	{
		case eLuxEnemyMoveState_Running:
		case eLuxEnemyMoveState_Walking:
			{
				mpEnemy->GetMeshEntity()->Stop();
				mpCharBody->StopMovement();
				mpCharBody->SetMoveSpeed(eCharDir_Forward, mpEnemy->mfForwardSpeed);
				mpCharBody->SetMoveAcc(eCharDir_Forward,0.0f);
				mpCharBody->Move(eCharDir_Forward,1.0f);
				mpCharBody->SetLastPosition( mpCharBody->GetPosition() - mpCharBody->GetForward() * mpEnemy->mfForwardSpeed * gpBase->mpEngine->GetStepSize() );
				mpEnemy->PlayAnim(
					aMoveState == eLuxEnemyMoveState_Running ? mpEnemy->GetRunAnimationName() : mpEnemy->GetWalkAnimationName(),
					true,
					0.0f,
					true,
					mpEnemy->mfMoveSpeedAnimMul,
					false,
					false
					);
				break;
			}
	}

	mMoveState = aMoveState;
}

//-----------------------------------------------------------------------

void cLuxEnemyMover::SetOverideMoveState(bool abX)
{
	if(mbOverideMoveState == abX) return;

	mbOverideMoveState = abX;
	if(mbOverideMoveState==false) mMoveState = eLuxEnemyMoveState_LastEnum;
}

//-----------------------------------------------------------------------

void cLuxEnemyMover::UpdateMoveAnimation(float afTimeStep)
{
	if(mbOverideMoveState) return;

	float fSpeed = GetMoveSpeed();
	if(mpCharBody->GetMoveSpeed(eCharDir_Forward) <0) fSpeed = -fSpeed;

	//Log("Update move anim. Speed: %f Current: %d\n", fSpeed, mMoveState);

	bool bCanJog = mpEnemy->mfWalkToJogSpeed>0 && mpEnemy->mfRunToJogSpeed>0 && mpEnemy->GetJogAnimationName()!="";

	eLuxEnemyPoseType pose = mpEnemy->mCurrentPose;

	////////////////////////////////
	// Move state animation
	eLuxEnemyMoveState prevMoveState = mMoveState;

	switch(mMoveState)
	{
	/////////////////
	// Backward
	case eLuxEnemyMoveState_Backward:
		if(fSpeed >= 0)
			mMoveState = eLuxEnemyMoveState_Stopped;

		break;

	/////////////////
	// Stopped State
	case eLuxEnemyMoveState_Stopped:
		if(fSpeed < -0.05f)
		{
			mMoveState = eLuxEnemyMoveState_Backward;
		}
		else if(fSpeed >= mpEnemy->mfStoppedToWalkSpeed[pose])
		{
			mMoveState = eLuxEnemyMoveState_Walking;
		}
		else if(std::fabs(mfTurnSpeed) > 0.07f && mpCharBody->GetMoveDelay()<=0)
		{
			mMoveState = eLuxEnemyMoveState_Walking;
		}
		else if(fSpeed >= mpEnemy->mfWalkToRunSpeed[pose])
		{
			mMoveState = eLuxEnemyMoveState_Running;
		}
		else if(fSpeed >= mpEnemy->mfWalkToJogSpeed[pose] && bCanJog)
		{
			mMoveState = eLuxEnemyMoveState_Jogging;
		}

		break;

	/////////////////
	// Walking State
	case eLuxEnemyMoveState_Walking:
		if(fSpeed >= mpEnemy->mfWalkToRunSpeed[pose])
		{
			mMoveState = eLuxEnemyMoveState_Running;
		}
		else if(fSpeed >= mpEnemy->mfWalkToJogSpeed[pose] && bCanJog)
		{
			mMoveState = eLuxEnemyMoveState_Jogging;
		}
		else if(fSpeed <= mpEnemy->mfWalkToStoppedSpeed[pose])
		{
			if(std::fabs(mfTurnSpeed) < 0.03f) mMoveState = eLuxEnemyMoveState_Stopped;
		}

		break;

	/////////////////
	// Jogging State
	case eLuxEnemyMoveState_Jogging:
		//To Stop
		if(fSpeed <= mpEnemy->mfWalkToStoppedSpeed[pose])
		{
			mMoveState = eLuxEnemyMoveState_Stopped;
		}
		//To Walk
		else if(fSpeed <= mpEnemy->mfRunToWalkSpeed[pose])
		{
			mMoveState = eLuxEnemyMoveState_Walking;
		}
		//To Run
		else if(fSpeed >= mpEnemy->mfWalkToRunSpeed[pose])
		{
			mMoveState = eLuxEnemyMoveState_Running;
		}


		break;

	/////////////////
	// Running State
	case eLuxEnemyMoveState_Running:
		if(fSpeed <= mpEnemy->mfWalkToStoppedSpeed[pose])
		{
			mMoveState = eLuxEnemyMoveState_Stopped;
		}
		else if(fSpeed <= mpEnemy->mfRunToWalkSpeed[pose])
		{
			mMoveState = eLuxEnemyMoveState_Walking;
		}
		else if(fSpeed <= mpEnemy->mfWalkToJogSpeed[pose] && bCanJog)
		{
			mMoveState = eLuxEnemyMoveState_Jogging;
		}


		break;

	/////////////////
	// NULL
	case eLuxEnemyMoveState_LastEnum:
		if(fSpeed < -0.05f)
		{
			mMoveState = eLuxEnemyMoveState_Backward;
		}
		else if(fSpeed >= mpEnemy->mfStoppedToWalkSpeed[pose])
		{
			mMoveState = eLuxEnemyMoveState_Walking;
		}
		else if(std::fabs(mfTurnSpeed) > 0.07f && mpCharBody->GetMoveDelay()<=0)
		{
			mMoveState = eLuxEnemyMoveState_Walking;
		}
		else if(fSpeed >= mpEnemy->mfWalkToRunSpeed[pose])
		{
			mMoveState = eLuxEnemyMoveState_Running;
		}
		else if(fSpeed >= mpEnemy->mfWalkToJogSpeed[pose] && bCanJog)
		{
			mMoveState = eLuxEnemyMoveState_Jogging;
		}
		else
		{
			mMoveState = eLuxEnemyMoveState_Stopped;
		}
		break;
	}
	
	//////////////////////////////////////////////
	//If move state has changed, change animation
	if(prevMoveState != mMoveState)
	{
		float fFadeSpeed = prevMoveState==eLuxEnemyMoveState_Stopped ? 0.3f : 0.5f;

		//Backward
		if(mMoveState == eLuxEnemyMoveState_Backward)
		{
			//Log(" To Backward\n");
			mpEnemy->PlayAnim(mpEnemy->GetBackwardAnimationName(), true, fFadeSpeed, true, mpEnemy->mfMoveSpeedAnimMul, false, false);
		}
		//Stopped
		else if(mMoveState == eLuxEnemyMoveState_Stopped)
		{
			//Log(" To Stop\n");
			mpEnemy->PlayAnim(mpEnemy->GetIdleAnimationName(),true,fFadeSpeed,false,1.0f,false,false);
		}
		//Walking
		else if(mMoveState == eLuxEnemyMoveState_Walking)
		{
			bool bSync = prevMoveState == eLuxEnemyMoveState_Running || eLuxEnemyMoveState_Jogging ? true : false;
			//Log(" To Walk. Synch: %d\n", bSync);

			mpEnemy->PlayAnim(mpEnemy->GetWalkAnimationName(),true,fFadeSpeed, true, mpEnemy->mfMoveSpeedAnimMul, bSync, false);
		}
		//Jogging
		else if(mMoveState == eLuxEnemyMoveState_Jogging)
		{
			bool bSync = prevMoveState == eLuxEnemyMoveState_Running || eLuxEnemyMoveState_Walking ? true : false;
			//Log(" To Walk. Synch: %d\n", bSync);

			mpEnemy->PlayAnim(mpEnemy->GetJogAnimationName(),true,fFadeSpeed, true, mpEnemy->mfMoveSpeedAnimMul, bSync, false);
		}
		//Running
		else if(mMoveState == eLuxEnemyMoveState_Running)
		{
			bool bSync = prevMoveState == eLuxEnemyMoveState_Walking || eLuxEnemyMoveState_Jogging ? true : false;
			//Log(" To Run. Synch: %d\n", bSync);
			
			mpEnemy->PlayAnim(mpEnemy->GetRunAnimationName(),true, fFadeSpeed, true, mpEnemy->mfMoveSpeedAnimMul, bSync, false);
		}
	}

	/////////////////////////////////
	//Update animation speed
	if(mbOverideMoveState==false && mpEnemy->mpCurrentAnimation && mMoveState != eLuxEnemyMoveState_Stopped)
	{
		if(std::fabs(fSpeed) > 0.05f)
		{
			//Log(" Anim speed: %f\n", std::fabs(fSpeed) * mpEnemy->mfMoveSpeedAnimMul);
			mpEnemy->mpCurrentAnimation->SetSpeed(std::fabs(fSpeed) * mpEnemy->mfMoveSpeedAnimMul);
		}
		else
		{
			//Log(" Anim speed: %f\n", std::fabs(mfTurnSpeed) * mpEnemy->mfMoveSpeedAnimMul * 2);
			mpEnemy->mpCurrentAnimation->SetSpeed(std::fabs(mfTurnSpeed) * mpEnemy->mfMoveSpeedAnimMul * 2);
		}
	}
}

//-----------------------------------------------------------------------

void cLuxEnemyMover::UpdateStepEffects(float afTimeStep)
{
	if(mpEnemy->IsInWater()==false || mpEnemy->GetWaterSurfaceData()==NULL) return;

	cAnimationState *pAnim = mpEnemy->GetCurrentAnimation();
	if(pAnim==NULL) return;

	///////////////////////////////////////
	// Check if a step is occuring!
	bool bStep = false;
	for(int i=0; i<pAnim->GetEventNum(); ++i)
	{
		cAnimationEvent *pEvent = pAnim->GetEvent(i);
        if(pEvent->mType != eAnimationEventType_Step) continue;

		if(	pEvent->mfTime >= pAnim->GetPreviousTimePosition() &&
			pEvent->mfTime < pAnim->GetTimePosition())
		{
			bStep = true;
			break;
		}
	}
	if(bStep==false) return;
	
	///////////////////////////////////////
	// Do the liquid effect
	cSurfaceData *pSurface = mpEnemy->GetWaterSurfaceData();
	
	//Get the speed
	float fSpeed = mpEnemy->mfWaterStepSpeedMisc;
	if(mMoveState == eLuxEnemyMoveState_Walking || mMoveState == eLuxEnemyMoveState_Backward)
	{
		fSpeed = mpEnemy->mfWaterStepSpeedWalk;
	}
	else if(mMoveState == eLuxEnemyMoveState_Running)
	{
		fSpeed = mpEnemy->mfWaterStepSpeedWalk;
	}

	//Get impact effect and coordinate
	cSurfaceImpactData *pImpact = pSurface->GetImpactDataFromSpeed(3.0f);
	cVector3f vEffectPos = mpCharBody->GetPosition();
	vEffectPos.y =mpEnemy->GetWaterSurfaceY()+0.01f;

	//Create sounda and ps for effect	
	cWorld *pWorld = mpEnemy->GetMap()->GetWorld();

	if(pImpact->GetPSName() != "")
	{
		cParticleSystem *pPS = pWorld->CreateParticleSystem("Splash", pImpact->GetPSName(),1);
		if(pPS) pPS->SetPosition(vEffectPos);
	}

	if(pImpact->GetSoundName() != "")
	{
		cSoundEntity *pSound = pWorld->CreateSoundEntity("Splash",pImpact->GetSoundName(),true);
		if(pSound) pSound->SetPosition(vEffectPos);
	}
}

//-----------------------------------------------------------------------


void cLuxEnemyMover::UpdateWallAvoidance(float afTimeStep)
{
	const float fUpdateTime=0.1f;

	if(mbWallAvoidActive==false) return;

	///////////////////////////////////////
	// Check if character is moving
	if(	fabs(mpCharBody->GetMoveSpeed(eCharDir_Forward))<0.001f &&
		fabs(mpCharBody->GetMoveSpeed(eCharDir_Right))<0.001f)
	{
		return;
	}

	///////////////////////////////////////
	// Check if time for update
	mfWallAvoidCount+=afTimeStep;
	if(mfWallAvoidCount >= fUpdateTime)
	{
		mfWallAvoidCount =0;

		///////////////////////////////////
		// Set up variables
		iPhysicsWorld *pPhysicsWorld = mpEnemy->GetMap()->GetPhysicsWorld();

		bool bIntersect = false;
		float fSamples  = (float)mvSampleRays.size();
		int lNumOfPartAngles = mvPrecalcSampleDirs.size() / kNumOfDirectionPartitions;

		cMatrixf mtxMoveRot = GetMovementDirectionMatrix();


		///////////////////////////////////
		// Calculate the usage count for each partition
		std::vector<int> vParitionUsageCount;
		vParitionUsageCount.resize(kNumOfDirectionPartitions, 0);

		for(size_t i=0; i<mvSamplePartitionUsed.size(); ++i)
		{
			if(mvSamplePartitionUsed[i]<0) continue;

			vParitionUsageCount[mvSamplePartitionUsed[i]]++;
		}

		///////////////////////////////////
		// Cast rays
		for(size_t i=0; i<mvSampleRays.size(); ++i)
		{
			//////////////////////////////////////
			// Get the ray direction
			cVector3f vDir;
			if(mvSampleRaysCollide[i]==false)
			{
				/////////////////////////////////////////////
				// Get the angle ID and make sure it is not used
				int lAngleIdx = cMath::RandRectl(0, mvPrecalcSampleDirs.size()-1);
				int lPart = lAngleIdx  / lNumOfPartAngles;
				while(vParitionUsageCount[lPart]==mlSampleMaxPartCount)
				{
					lAngleIdx += lNumOfPartAngles;
					if(lAngleIdx >= (int)mvPrecalcSampleDirs.size()) lAngleIdx -= (int)mvPrecalcSampleDirs.size();

					lPart = lAngleIdx  / lNumOfPartAngles;
				}

				/////////////////////////////////////////////
				// Get the direction of the ray.
				cVector3f vLocalDir = mvPrecalcSampleDirs[lAngleIdx];
				ConvertLocalDirTo2D(vLocalDir);

				vDir = cMath::MatrixMul(mtxMoveRot, vLocalDir)*-1;

				mvSampleRays[i] = vDir;
				mvSampleRaysAmount[i] = fUpdateTime;
				mvSamplePartitionUsed[i] = lPart;
			}
			else
			{
				vDir = mvSampleRays[i];
			}

			//////////////////////////////////////
			// Check for intersection
			float fDistance=0;
			bool bCollide = gpBase->mpMapHelper->GetClosestCharCollider(mpCharBody->GetPosition(), vDir, mfWallAvoidRadius,false, &fDistance, NULL, NULL);

			if(bCollide==false)	
				mvSamplePartitionUsed[i] = -1;
			else if(mvSampleRaysCollide[i]==false)
				vParitionUsageCount[mvSamplePartitionUsed[i]]++;

			mvSampleRaysCollide[i] = bCollide;


			//////////////////////////////////////
			// If intersection, increase the steering.
			if(bCollide)
			{
				mvSampleRayBaseDir[i] =vDir* -(1 - fDistance / mfWallAvoidRadius);
			}
		}
	}


	///////////////////////////////////////
	// Calculate steering amount (and fade each one in)
	float fTotalHits = 0;
	float fTotalAmount = 0;
	cVector3f vTotalSteer=0;
	for(size_t i=0; i<mvSampleRays.size(); ++i)
	{
		if(mvSampleRaysCollide[i])
		{
			mvSampleRaysAmount[i] += afTimeStep*5;
			if(mvSampleRaysAmount[i]>1.0f) mvSampleRaysAmount[i] = 1.0f;

			vTotalSteer += mvSampleRayBaseDir[i]*mvSampleRaysAmount[i];
			fTotalHits += 1;
		}
	}

	if(fTotalHits>0)
	{
		if(vTotalSteer.SqrLength()>1) vTotalSteer.Normalize();

		mvSteeringVec += vTotalSteer * mfWallAvoidSteerAmount;
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// SAVE DATA STUFF
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

kBeginSerializeBase(cLuxEnemyMover_SaveData)

kSerializeVar(mbTurning, eSerializeType_Bool)
kSerializeVar(mfTurnGoalAngle, eSerializeType_Float32)
kSerializeVar(mfTurnSpeed, eSerializeType_Float32)
kSerializeVar(mfTurnBreakAcc, eSerializeType_Float32)

kSerializeVar(mfStuckCounter, eSerializeType_Float32)

kSerializeVar(mvCurrentGoal, eSerializeType_Vector3f)

kSerializeVar(mlMoveState, eSerializeType_Int32)
kSerializeVar(mbOverideMoveState, eSerializeType_Bool)

kEndSerialize()

//-----------------------------------------------------------------------

void cLuxEnemyMover_SaveData::FromMover(cLuxEnemyMover *apMover)
{	
	mbTurning = apMover->mbTurning;
	mfTurnGoalAngle = apMover->mfTurnGoalAngle;
	mfTurnSpeed = apMover->mfTurnSpeed;
	mfTurnBreakAcc = apMover->mfTurnBreakAcc;

	mfStuckCounter = apMover->mfStuckCounter;

	mvCurrentGoal = apMover->mvCurrentGoal;

	mlMoveState = apMover->mMoveState;
	mbOverideMoveState = apMover->mbOverideMoveState;
}

//-----------------------------------------------------------------------

void cLuxEnemyMover_SaveData::ToMover(cLuxEnemyMover *apMover)
{
	apMover->mbTurning = mbTurning;
	apMover->mfTurnGoalAngle = mfTurnGoalAngle;
	apMover->mfTurnSpeed = mfTurnSpeed;
	apMover->mfTurnBreakAcc = mfTurnBreakAcc;

	apMover->mfStuckCounter = mfStuckCounter;

	apMover->mvCurrentGoal= mvCurrentGoal;

	apMover->mMoveState = (eLuxEnemyMoveState)mlMoveState;
	apMover->mbOverideMoveState = mbOverideMoveState;
}

//-----------------------------------------------------------------------
