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

#include "LuxEnemy_Child.h"

#include "LuxEnemyMover.h"
#include "LuxEnemyPathfinder.h"

#include "LuxMap.h"
#include "LuxMapHelper.h"
#include "LuxMusicHandler.h"
#include "LuxDebugHandler.h"
#include "LuxGlobalDataHandler.h"

#include "LuxPlayer.h"
#include "LuxPlayerHelpers.h"

//////////////////////////////////////////////////////////////////////////
// LOADER
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cLuxEnemyLoader_Child::cLuxEnemyLoader_Child(const tString& asName) : iLuxEnemyLoader(asName)
{
}

//-----------------------------------------------------------------------

iLuxEnemy *cLuxEnemyLoader_Child::CreateEnemy(const tString& asName, int alID, cLuxMap *apMap)
{
	return hplNew(cLuxEnemy_Child, (asName, alID,apMap) );
}

//-----------------------------------------------------------------------

void cLuxEnemyLoader_Child::LoadVariables(iLuxEnemy *apEnemy, cXmlElement *apRootElem)
{
	cLuxEnemy_Child *pChild = static_cast<cLuxEnemy_Child*>(apEnemy);
}

//-----------------------------------------------------------------------

static eLuxEnemyMoveSpeed ToMoveSpeed(const tString& asStr)
{
	if(asStr == "Run") return eLuxEnemyMoveSpeed_Run;
	if(asStr == "Walk") return eLuxEnemyMoveSpeed_Walk;

	Error("eLuxEnemyMoveSpeed '%s' does not exist, falling back to walk!\n", asStr.c_str());
	return eLuxEnemyMoveSpeed_Walk;
}

void cLuxEnemyLoader_Child::LoadInstanceVariables(iLuxEnemy *apEnemy, cResourceVarsObject *apInstanceVars)
{
	cLuxEnemy_Child *pChild = static_cast<cLuxEnemy_Child*>(apEnemy);

	pChild->mPatrolMoveSpeed = ToMoveSpeed(apInstanceVars->GetVarString("PatrolMoveSpeed", "Walk"));
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cLuxEnemy_Child::cLuxEnemy_Child(const tString &asName, int alID, cLuxMap *apMap) : iLuxEnemy(asName,alID,apMap, eLuxEnemyType_Child)
{
	for(int i=0; i<eLuxEnemyMoveType_LastEnum; ++i)
	{
		eLuxEnemyPoseType pose = eLuxEnemyPoseType_Biped;
		msIdleAnimationName[i][pose] = "IdleBiped";
		msWalkAnimationName[i][pose] = "WalkBiped";
		msJogAnimationName[i][pose] = "";
		msRunAnimationName[i][pose] =  "RunBiped";
	}
}

//-----------------------------------------------------------------------

cLuxEnemy_Child::~cLuxEnemy_Child()
{
	
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cLuxEnemy_Child::OnSetupAfterLoad(cWorld *apWorld)
{
	mpMover->SetWallAvoidanceActive(false);
}


//-----------------------------------------------------------------------

void cLuxEnemy_Child::OnAfterWorldLoad()
{

}

//-----------------------------------------------------------------------

void cLuxEnemy_Child::UpdateEnemySpecific(float afTimeStep)
{
}

//-----------------------------------------------------------------------

bool cLuxEnemy_Child::StateEventImplement(int alState, eLuxEnemyStateEvent aEvent, cLuxStateMessage *apMessage)
{
	kLuxBeginStateMachine
	
	////////////////////////////////
	// Default
	////////////////////////////////

	//------------------------------

	kLuxOnMessage(eLuxEnemyMessage_Reset)
		ChangeState(eLuxEnemyState_Wait);

	//------------------------------

	kLuxOnMessage(eLuxEnemyMessage_PlayerDetected)
		
	
	//------------------------------

	kLuxOnMessage(eLuxEnemyMessage_TakeHit)
		
	//------------------------------

	//If enemy is out of range (having been in, then turn him off)
	kLuxOnMessage(eLuxEnemyMessage_PlayerOutOfRange)
	
	//------------------------------

	kLuxOnMessage(eLuxEnemyMessage_PlayerDead)
	
	//------------------------------

	kLuxOnMessage(eLuxEnemyMessage_HelpMe)
	
	//------------------------------

	kLuxOnMessage(eLuxEnemyMessage_ChangePose)
		
		

	//------------------------------

	////////////////////////////////
	// Idle
	////////////////////////////////

	//------------------------------

	kLuxState(eLuxEnemyState_Idle)
		kLuxOnEnter
			SetMoveSpeed(eLuxEnemyMoveSpeed_Walk);

		kLuxOnMessage(eLuxEnemyMessage_PlayerInRange)
		
		kLuxOnMessage(eLuxEnemyMessage_PlayerDetected)
			//Nothing

	//------------------------------
	
	////////////////////////////////
	// Go Home
	////////////////////////////////

	//------------------------------

	kLuxState(eLuxEnemyState_GoHome)
		kLuxOnEnter
			mpPathfinder->MoveTo(mvStartPosition);
			SetMoveSpeed(eLuxEnemyMoveSpeed_Walk);
		
		kLuxOnMessage(eLuxEnemyMessage_EndOfPath)
			ChangeState(eLuxEnemyState_Idle);

	//------------------------------

	////////////////////////////////
	// Wait
	////////////////////////////////

	//------------------------------

	kLuxState(eLuxEnemyState_Wait)
		kLuxOnEnter
			SendMessage(eLuxEnemyMessage_TimeOut, mfWaitTime, true);
			mfWaitTime =0;

			SetMoveSpeed(eLuxEnemyMoveSpeed_Walk);
			mpPathfinder->Stop();

		kLuxOnLeave
			mpMover->SetOverideMoveState(false);

		//------------------------------
		
		kLuxOnMessage(eLuxEnemyMessage_TimeOut)
			
			if(GetPatrolNodeNum()>0)
			{
				FadeOutCurrentAnim(0.2f);
				ChangeState(eLuxEnemyState_Patrol);	
			}
			else
			{
				SendMessage(eLuxEnemyMessage_TimeOut, cMath::RandRectf(0.1f, 0.3f), true);
			}

		//------------------------------

		kLuxOnMessage(eLuxEnemyMessage_AnimationOver)
			SendMessage(eLuxEnemyMessage_TimeOut_2, cMath::RandRectf(4, 13), true);
	
		
	//------------------------------

	////////////////////////////////
	// Patrol
	////////////////////////////////

	//------------------------------
	kLuxState(eLuxEnemyState_Patrol)
		kLuxOnEnter
			ChangeSoundState(eLuxEnemySoundState_Idle);
			SetMoveSpeed(mPatrolMoveSpeed);
			
			cLuxEnemyPatrolNode* pPatrolNode = GetCurrentPatrolNode();
			mpPathfinder->MoveTo(pPatrolNode->mpNode->GetPosition());


		kLuxOnMessage(eLuxEnemyMessage_EndOfPath)
			
			cLuxEnemyPatrolNode *pNode = GetCurrentPatrolNode();
			if(pNode)	mfWaitTime = pNode->mfWaitTime;
			else		mfWaitTime = 0.1f;
			
			if(pNode->msAnimation == "")
			{
				ChangeState(eLuxEnemyState_Wait);
			}
			else
			{
				PlayAnim(pNode->msAnimation, false, 0.2f);
			}

			bool bAtLast = IsAtLastPatrolNode();
			IncCurrentPatrolNode(false);
			if(bAtLast) ClearPatrolNodes();


		kLuxOnMessage(eLuxEnemyMessage_AnimationOver)
			ChangeState(eLuxEnemyState_Wait);
			
	//------------------------------
	
	

	
	kLuxEndStateMachine
}


//-----------------------------------------------------------------------

void cLuxEnemy_Child::OnRenderSolidImplemented(cRendererCallbackFunctions* apFunctions)
{
	iPhysicsWorld *pPhysicsWorld = mpMap->GetPhysicsWorld();

	if(mCurrentState == eLuxEnemyState_AttackMeleeShort)
	{
		pPhysicsWorld->RenderShapeDebugGeometry(GetAttackShape(0), GetDamageShapeMatrix(cVector3f(0,0,1)), apFunctions->GetLowLevelGfx(),
												cColor(1,0,0,1));
	}
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

bool cLuxEnemy_Child::PlayerIsDetected()
{
	return false;
}

//-----------------------------------------------------------------------

void cLuxEnemy_Child::OnDisableTriggers()
{
}

//-----------------------------------------------------------------------

float cLuxEnemy_Child::GetDamageMul(float afAmount, int alStrength)
{
	return 1.0f;
}

//-----------------------------------------------------------------------


//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// SAVE DATA STUFF
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

kBeginSerialize(cLuxEnemy_Child_SaveData, iLuxEnemy_SaveData)

kSerializeVar(mPatrolMoveSpeed, eSerializeType_Int32)
kSerializeVar(mfWaitTime, eSerializeType_Float32)

kEndSerialize()

//-----------------------------------------------------------------------

iLuxEntity_SaveData* cLuxEnemy_Child::CreateSaveData()
{
	return hplNew(cLuxEnemy_Child_SaveData, ());
}

//-----------------------------------------------------------------------

void cLuxEnemy_Child::SaveToSaveData(iLuxEntity_SaveData* apSaveData)
{
	//////////////////
	//Init
	super_class::SaveToSaveData(apSaveData);
	cLuxEnemy_Child_SaveData *pData = static_cast<cLuxEnemy_Child_SaveData*>(apSaveData);

	//////////////////
	//Set variables
	kCopyToVar(pData,mPatrolMoveSpeed);
	kCopyToVar(pData,mfWaitTime);
}

//-----------------------------------------------------------------------

void cLuxEnemy_Child::LoadFromSaveData(iLuxEntity_SaveData* apSaveData)
{
	//////////////////
	//Init
	super_class::LoadFromSaveData(apSaveData);
	cLuxEnemy_Child_SaveData *pData = static_cast<cLuxEnemy_Child_SaveData*>(apSaveData);

	//////////////////
	//Set variables
	mPatrolMoveSpeed  = (eLuxEnemyMoveSpeed)pData->mPatrolMoveSpeed;
	kCopyFromVar(pData, mfWaitTime);
}

//-----------------------------------------------------------------------

void cLuxEnemy_Child::SetupSaveData(iLuxEntity_SaveData *apSaveData)
{
	super_class::SetupSaveData(apSaveData);
}


//-----------------------------------------------------------------------

