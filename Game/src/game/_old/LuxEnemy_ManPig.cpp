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

#include "LuxEnemy_ManPig.h"

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

extern tWString gsLuxEnemyStates[];

#define LOG_AI    1
#if LOG_AI
    #define AILOG(x) AILog(x)
#else
    #define AILOG(x)
#endif

//-----------------------------------------------------------------------

cLuxEnemyLoader_ManPig::cLuxEnemyLoader_ManPig(const tString& asName) : iLuxEnemyLoader(asName)
{
}

//-----------------------------------------------------------------------

iLuxEnemy *cLuxEnemyLoader_ManPig::CreateEnemy(const tString& asName, int alID, cLuxMap *apMap)
{
	return hplNew(cLuxEnemy_ManPig, (asName, alID,apMap) );
}

//-----------------------------------------------------------------------

void cLuxEnemyLoader_ManPig::LoadVariables(iLuxEnemy *apEnemy, cXmlElement *apRootElem)
{
	cLuxEnemy_ManPig *pManPig = static_cast<cLuxEnemy_ManPig*>(apEnemy);

	pManPig->msNoticeSound = GetVarString("NoticeSound");
	pManPig->msGiveUpNoticeSound = GetVarString("GiveUpNoticeSound");
	pManPig->msEnabledSound = GetVarString("EnabledSound");
	gpBase->PreloadSound(pManPig->msNoticeSound);
	gpBase->PreloadSound(pManPig->msEnabledSound);

	pManPig->mfGroggyDamageCount = GetVarFloat("GroggyDamageCount", 0);
	pManPig->mfAlertToHuntDistance = GetVarFloat("AlertToHuntDistance", 0);
	pManPig->mfAlertToInstantHuntDistance = GetVarFloat("AlertToInstantHuntDistance", 0);
	pManPig->mfHuntPauseMinTime = GetVarFloat("HuntPauseMinTime", 0);
	pManPig->mfHuntPauseMaxTime = GetVarFloat("HuntPauseMaxTime", 0);
	pManPig->mfIncreaseAlertSpeedDistance = GetVarFloat("IncreaseAlertSpeedDistance", 0);
	pManPig->mfIncreasedAlertSpeedMul = GetVarFloat("IncreasedAlertSpeedMul", 0);
	
	pManPig->mfAlertRunTowardsToHuntLimit = GetVarFloat("AlertRunTowardsToHuntLimit", 0);
	pManPig->mfAlertRunTowardsCheckDistance = GetVarFloat("AlertRunTowardsCheckDistance", 0);
}

//-----------------------------------------------------------------------

static eLuxIdleBehavior ToIdleBehavior(const tString& asStr)
{
	if(asStr == "None") return eLuxIdleBehavior_None;
	if(asStr == "Stalk") return eLuxIdleBehavior_Stalk;
	if(asStr == "Track") return eLuxIdleBehavior_Track;

	Error("eLuxIdleBehavior behavior '%s' does not exist!\n", asStr.c_str());
	return eLuxIdleBehavior_None;
}

static eLuxEnemyMoveSpeed ToMoveSpeed(const tString& asStr)
{
	if(asStr == "Run") return eLuxEnemyMoveSpeed_Run;
	if(asStr == "Walk") return eLuxEnemyMoveSpeed_Walk;

	Error("eLuxEnemyMoveSpeed '%s' does not exist, falling back to walk!\n", asStr.c_str());
	return eLuxEnemyMoveSpeed_Walk;
}

void cLuxEnemyLoader_ManPig::LoadInstanceVariables(iLuxEnemy *apEnemy, cResourceVarsObject *apInstanceVars)
{
	cLuxEnemy_ManPig *pManPig = static_cast<cLuxEnemy_ManPig*>(apEnemy);

	pManPig->mbThreatenOnAlert = apInstanceVars->GetVarBool("ThreatenOnAlert", false);
	pManPig->mbFleeFromPlayer = apInstanceVars->GetVarBool("FleeFromPlayer", false);
	pManPig->mIdleBehavior = ToIdleBehavior(apInstanceVars->GetVarString("IdleBehavior", "None"));
	pManPig->mbAllowZeroNodeWaitTimes = apInstanceVars->GetVarBool("AllowZeroNodeWaitTimes", false);
    pManPig->mPatrolMoveSpeed = ToMoveSpeed(apInstanceVars->GetVarString("PatrolMoveSpeed", "Walk"));

    if ( pManPig->mbInvisible && pManPig->GetMeshEntity() != NULL )
    {
        pManPig->GetMeshEntity()->SetVisible( false );
    }
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cLuxEnemy_ManPig::cLuxEnemy_ManPig(const tString &asName, int alID, cLuxMap *apMap) : iLuxEnemy(asName,alID,apMap, eLuxEnemyType_ManPig)
{
	mfWaitTime =0;
	mfAlertRunTowardsCount = 0;

	mbAlignEntityWithGroundRay = true;

	mbThreatenOnAlert = false;
	mbFleeFromPlayer = false;
	mIdleBehavior = eLuxIdleBehavior_None;

	mbPathReversed = false;

	for(int i=0; i<eLuxEnemyMoveType_LastEnum; ++i)
	{
		eLuxEnemyPoseType pose = eLuxEnemyPoseType_Biped;
		msIdleAnimationName[i][pose] = "IdleBiped";
		msWalkAnimationName[i][pose] = "WalkBiped";
		msRunAnimationName[i][pose] =  i==eLuxEnemyMoveType_Normal ? "RunBiped" : "FleeBiped";;

		pose = eLuxEnemyPoseType_Quadruped;
		msIdleAnimationName[i][pose] = "IdleQuadruped";
		msWalkAnimationName[i][pose] = "WalkQuadruped";
		msJogAnimationName[i][pose] = "JogQuadruped";
		msRunAnimationName[i][pose] = i==eLuxEnemyMoveType_Normal ? "RunQuadruped" : "FleeQuadruped";
	}

	mfPostAnimState = eLuxEnemyState_Idle;
}

//-----------------------------------------------------------------------

cLuxEnemy_ManPig::~cLuxEnemy_ManPig()
{
	
}

//-----------------------------------------------------------------------

void cLuxEnemy_ManPig::AILog( const tString & logMessage )
{
    time_t tim=time(NULL);
    tm *now=localtime(&tim);

    char buff[512];

	sprintf(buff,"%2d:%2d.%2d %s.%c (%s): %s\n", now->tm_hour, now->tm_min, now->tm_sec, GetName().c_str(), this->mCurrentPose == eLuxEnemyPoseType_Quadruped ? 'q' : 'b', cString::To8Char(gsLuxEnemyStates[mCurrentState]).c_str(), logMessage.c_str());
    Log( "*AI* %s", buff );
	DebugMessage(tString(buff));
    gpBase->mpDebugHandler->AddAILogEntry(buff);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cLuxEnemy_ManPig::OnSetupAfterLoad(cWorld *apWorld)
{
	////////////////////////////////////////
	// Wall avoidance
	mpMover->SetupWallAvoidance(0.9f, 8, 4);
	mpMover->SetWallAvoidanceActive(true);

	////////////////////////////////////////
	// Walk start transition
	AddTransitionAnimation("WalkBiped", "WalkStartBiped", "IdleBiped");
	AddTransitionAnimation("WalkBiped", "WalkStartBiped", "IdleBipedExtra1");
	AddTransitionAnimation("WalkBiped", "WalkStartBiped", "IdleBipedExtra2");
	AddTransitionAnimation("WalkBiped", "WalkStartBiped", "NoticeBiped");

	AddTransitionAnimation("IdleBiped", "WalkStopBiped", "WalkBiped");
	AddTransitionAnimation("IdleBiped", "WalkStopBiped", "RunBiped");
	AddTransitionAnimation("IdleBiped", "WalkStopBiped", "FleeBiped");
	AddTransitionAnimation("IdleBiped", "WalkStopBiped", "ChargeBiped");

	AddTransitionAnimation("WalkQuadruped", "WalkStartQuadruped", "IdleQuadruped");
	AddTransitionAnimation("WalkQuadruped", "WalkStartQuadruped", "IdleQuadrupedExtra1");
	AddTransitionAnimation("WalkQuadruped", "WalkStartQuadruped", "IdleQuadrupedExtra2");
	AddTransitionAnimation("WalkQuadruped", "WalkStartQuadruped", "NoticeQuadruped");

	AddTransitionAnimation("IdleQuadruped", "WalkStopQuadruped", "WalkQuadruped");
	AddTransitionAnimation("IdleQuadruped", "WalkStopQuadruped", "JogQuadruped");
	AddTransitionAnimation("IdleQuadruped", "WalkStopQuadruped", "RunQuadruped");

	//////////////////////////////
	//Threat transition stuff
	AddTransitionAnimation("ThreatLoop", "ThreatStartBiped", "IdleBiped");
	AddTransitionAnimation("ThreatLoop", "ThreatStartBiped", "IdleBipedExtra1");
	AddTransitionAnimation("ThreatLoop", "ThreatStartBiped", "IdleBipedExtra2");
	//AddTransitionAnimation("ThreatLoop", "ThreatStartBiped", "NoticeBiped"); //bad blend
	AddTransitionAnimation("ThreatLoop", "ThreatStartBiped", "WalkBiped");

	AddTransitionAnimation("ThreatLoop", "ThreatStartQuadruped", "IdleQuadruped");
	AddTransitionAnimation("ThreatLoop", "ThreatStartQuadruped", "IdleQuadrupedExtra1");
	AddTransitionAnimation("ThreatLoop", "ThreatStartQuadruped", "IdleQuadrupedExtra2");
	//AddTransitionAnimation("ThreatLoop", "ThreatStartQuadruped", "NoticeQuadruped");//bad blend
	AddTransitionAnimation("ThreatLoop", "ThreatStartQuadruped", "WalkQuadruped");
	AddTransitionAnimation("ThreatLoop", "ThreatStartQuadruped", "JogQuadruped");

	AddTransitionAnimation("WalkBiped", "ThreatEndBiped", "ThreatLoop");
	AddTransitionAnimation("IdleBiped", "ThreatEndBiped", "ThreatLoop");
	AddTransitionAnimation("RunBiped", "ThreatEndBiped", "ThreatLoop");

	AddTransitionAnimation("IdleQuadruped", "ThreatEndQuadruped", "ThreatLoop");
	AddTransitionAnimation("WalkQuadruped", "ThreatEndQuadruped", "ThreatLoop");
	AddTransitionAnimation("JogQuadruped", "ThreatEndQuadruped", "ThreatLoop");
	AddTransitionAnimation("RunQuadruped", "ThreatEndQuadruped", "ThreatLoop");

	//////////////////////////////
	//Pose change transitions
	AddTransitionAnimation("WalkBiped", "IdleQuadrupedToBiped", "WalkQuadruped");
	AddTransitionAnimation("RunBiped", "IdleQuadrupedToBiped", "RunQuadruped");
	//AddTransitionAnimation("JogBiped", "IdleQuadrupedToBiped", "WalkQuadruped");
	AddTransitionAnimation("IdleBiped", "IdleQuadrupedToBiped", "IdleQuadruped");
	AddTransitionAnimation("IdleBiped", "IdleQuadrupedToBiped", "WalkQuadruped");
	AddTransitionAnimation("IdleBiped", "IdleQuadrupedToBiped", "RunQuadruped");
	AddTransitionAnimation("IdleBipedExtra1", "IdleQuadrupedToBiped", "IdleQuadruped");
	AddTransitionAnimation("IdleBipedExtra2", "IdleQuadrupedToBiped", "IdleQuadruped");
	AddTransitionAnimation("NoticeBiped", "IdleQuadrupedToBiped", "IdleQuadruped");
	
	AddTransitionAnimation("WalkQuadruped", "IdleBipedToQuadruped", "WalkBiped");
	AddTransitionAnimation("JogQuadruped", "IdleBipedToQuadruped", "RunBiped");//"JogBiped");
	AddTransitionAnimation("RunQuadruped", "IdleBipedToQuadruped", "RunBiped");
	AddTransitionAnimation("IdleQuadruped", "IdleBipedToQuadruped", "IdleBiped");
	AddTransitionAnimation("IdleQuadruped", "IdleBipedToQuadruped", "WalkQuadruped");
	AddTransitionAnimation("IdleQuadruped", "IdleBipedToQuadruped", "JogQuadruped");
	AddTransitionAnimation("IdleQuadruped", "IdleBipedToQuadruped", "RunQuadruped");
	AddTransitionAnimation("IdleQuadrupedExtra1", "IdleBipedToQuadruped", "IdleBiped");
	AddTransitionAnimation("IdleQuadrupedExtra2", "IdleBipedToQuadruped", "IdleBiped");
	AddTransitionAnimation("NoticeQuadruped", "IdleBipedToQuadruped", "IdleBiped");
}


//-----------------------------------------------------------------------

void cLuxEnemy_ManPig::OnAfterWorldLoad()
{

}

//-----------------------------------------------------------------------

void cLuxEnemy_ManPig::UpdateEnemySpecific(float afTimeStep)
{
	
}

//-----------------------------------------------------------------------

bool cLuxEnemy_ManPig::StateEventImplement(int alState, eLuxEnemyStateEvent aEvent, cLuxStateMessage *apMessage)
{
	kLuxBeginStateMachine
	
	////////////////////////////////
	// Default
	////////////////////////////////

	//------------------------------

	kLuxOnMessage(eLuxEnemyMessage_Reset)
    {
		gpBase->mpMusicHandler->RemoveEnemy(eLuxEnemyMusic_Attack,this);
		gpBase->mpPlayer->RemoveTerrorEnemy(this);

        AILOG( "received eLuxEnemyMessage_Reset at root, changing to WAIT." );
		ChangeState(eLuxEnemyState_Wait);
    }

	//------------------------------

	kLuxOnMessage(eLuxEnemyMessage_PlayerDetected)
    {
        AILOG( "received eLuxEnemyMessage_PlayerDetected at root." );
		ChangeState(eLuxEnemyState_Alert);
    }
	//------------------------------

	kLuxOnMessage(eLuxEnemyMessage_TakeHit)
	{
        AILOG( "received eLuxEnemyMessage_TakeHit at root." );

        mfDamageCount += apMessage->mfCustomValue;
		
        if(mfDamageCount > mfGroggyDamageCount)
		{
            AILOG( "damage was big enough, switching to HURT." );

			ChangeState(eLuxEnemyState_Hurt);
			mfDamageCount =0;
		}
    }
	
	//------------------------------

	//If enemy is out of range (having been in, then turn him off)
	kLuxOnMessage(eLuxEnemyMessage_PlayerOutOfRange)
    {
        AILOG( "received eLuxEnemyMessage_PlayerOutOfRange at root, deactivating." );
		SetActive(false);
    }

	//------------------------------

	kLuxOnMessage(eLuxEnemyMessage_PlayerDead)
    {
        AILOG( "ROOT: received eLuxEnemyMessage_PlayerDead, switching to IDLE." );
		ChangeState(eLuxEnemyState_Idle);
    }

	//------------------------------

	kLuxOnMessage(eLuxEnemyMessage_HelpMe)
    {
        AILOG( "ROOT: received eLuxEnemyMessage_HelpMe, switching to HUNT" );
        ShowPlayerPosition();
		ChangeState(eLuxEnemyState_Hunt);
    }
	//------------------------------

	kLuxOnMessage(eLuxEnemyMessage_ChangePose)
		
		

	//------------------------------

	////////////////////////////////
	// Idle
	////////////////////////////////

	//------------------------------

	kLuxState(eLuxEnemyState_Idle)

		kLuxOnEnter
        {
			mbPlayerInRange = false; //Reset so new checks are made in case player is near.
			SetMoveSpeed(eLuxEnemyMoveSpeed_Walk);
        }

		kLuxOnMessage(eLuxEnemyMessage_PlayerInRange)
        {
			gpBase->mpDebugHandler->AddMessage(_W("Enemy ") + cString::To16Char(msName)+_W(" enabled!"), false );
			
			if(gpBase->mpGlobalDataHandler->GetEnemyActivateSoundAllowed())
			{
				gpBase->mpGlobalDataHandler->SetEnemyActivateSoundMade();
				PlaySound(msEnabledSound);
			}
			

			if(mvPatrolNodes.empty())
			{
                AILOG( "received eLuxEnemyMessage_PlayerInRange, enabling and switching to WAIT (no patrol nodes)." );
				ChangeState(eLuxEnemyState_Wait);
			}
			else
			{
                AILOG( "received eLuxEnemyMessage_PlayerInRange, enabling and switching to PATROL." );
				ChangeState(eLuxEnemyState_Patrol);
			}
        }

		kLuxOnMessage(eLuxEnemyMessage_PlayerDetected)
			//Nothing

	//------------------------------
	
	////////////////////////////////
	// Go Home
	////////////////////////////////

	//------------------------------

	kLuxState(eLuxEnemyState_GoHome)
		kLuxOnEnter
        {
            AILOG( "moving to start position." );
			mpPathfinder->MoveTo(mvStartPosition);
			SetMoveSpeed(eLuxEnemyMoveSpeed_Walk);
        }
		
		kLuxOnMessage(eLuxEnemyMessage_EndOfPath)
        {
            AILOG( "received eLuxEnemyMessage_EndOfPath, switching to IDLE." );
			ChangeState(eLuxEnemyState_Idle);
        }

		kLuxOnMessage(eLuxEnemyMessage_SoundHeard)
        {
            AILOG( "received eLuxEnemyMessage_SoundHeard, sound:" + msHeardSoundName );
			if(apMessage->mfCustomValue > mfHearVolume)
			{
                AILOG( "sound was loud enough, switching to INVESTIGATE" );
				ChangeState(eLuxEnemyState_Investigate);
				mvTempPos = apMessage->mvCustomValue;
				mfTempVal = apMessage->mfCustomValue;
			}
        }
	
	//------------------------------

	////////////////////////////////
	// Wait
	////////////////////////////////

	//------------------------------

	kLuxState(eLuxEnemyState_Wait)
	
        kLuxOnEnter
        {
			if(mfWaitTime <= 0)
            {
				SendMessage(eLuxEnemyMessage_TimeOut, cMath::RandRectf(1, 3), true);
            }
			else
            {
				SendMessage(eLuxEnemyMessage_TimeOut, mfWaitTime, true);
            }

			mfWaitTime =0;

			SendMessage(eLuxEnemyMessage_TimeOut_2, cMath::RandRectf(2, 5), true);

			SetMoveSpeed(eLuxEnemyMoveSpeed_Walk);
			mpPathfinder->Stop();
        }

		kLuxOnLeave
        {
			mpMover->SetOverideMoveState(false);
        }

		//------------------------------
		
		kLuxOnMessage(eLuxEnemyMessage_TimeOut)
			
			if(mPreviousState == eLuxEnemyState_Search ||
				mPreviousState == eLuxEnemyState_Stalk ||
				mPreviousState == eLuxEnemyState_Track)
			{
				FadeOutCurrentAnim(0.2f);

                #if LOG_AI
                    tString string = "received eLuxEnemyMessage_TimeOut, switching back to ";
                    string = string + cString::To8Char(gsLuxEnemyStates[mPreviousState]) + ".";
                    AILOG( string );
                #endif

				ChangeState(mPreviousState);	
			}
			else
			{
				//if(GetPatrolNodeNum()>0)
				//{
					FadeOutCurrentAnim(0.2f);
                    AILOG( "received eLuxEnemyMessage_TimeOut, switching to PATROL." );
					ChangeState(eLuxEnemyState_Patrol);	
				//}
				//else
				//{
                //    AILOG( "received eLuxEnemyMessage_TimeOut, but no patrol nodes, so staying in WAIT." );
				//	SendMessage(eLuxEnemyMessage_TimeOut, cMath::RandRectf(3, 5), true);
				//}
			}

		kLuxOnMessage(eLuxEnemyMessage_TimeOut_2)
        {
            AILOG( "received eLuxEnemyMessage_TimeOut2, playing idle extra anim." );
			int lRand = cMath::RandRectl(1,2);
			PlayAnim("Idle"+GetCurrentPoseSuffix()+"Extra"+cString::ToString(lRand),false, 0.3f);
        }

		//------------------------------

		kLuxOnMessage(eLuxEnemyMessage_AnimationOver)
        {
			SendMessage(eLuxEnemyMessage_TimeOut_2, cMath::RandRectf(4, 13), true);
        }
	
		kLuxOnMessage(eLuxEnemyMessage_SoundHeard)
        {
            AILOG( "received eLuxEnemyMessage_SoundHeard, sound:" + msHeardSoundName );
			if(apMessage->mfCustomValue > mfHearVolume)
			{
                AILOG( "sound was loud enough, switching to INVESTIGATE" );
				ChangeState(eLuxEnemyState_Investigate);
				mvTempPos = apMessage->mvCustomValue;
				mfTempVal = apMessage->mfCustomValue;
			}
        }
	//------------------------------

	////////////////////////////////
	// Patrol
	////////////////////////////////

	//------------------------------
	kLuxState(eLuxEnemyState_Patrol)
		kLuxOnEnter
			gpBase->mpMusicHandler->RemoveEnemy(eLuxEnemyMusic_Search,this);
			gpBase->mpMusicHandler->RemoveEnemy(eLuxEnemyMusic_Attack,this);
			gpBase->mpPlayer->RemoveTerrorEnemy(this);

			ChangeSoundState(eLuxEnemySoundState_Idle);
			SetMoveSpeed(mPatrolMoveSpeed);
			PatrolUpdateGoal();


		kLuxOnUpdate
			if(mbStuckAtDoor)// && mpMap->DoorIsClosed(mlStuckDoorID))
			{
				iLuxEntity *pDoorEnt = mpMap->GetEntityByID(mlStuckDoorID);
				mvTempPos = pDoorEnt->GetAttachEntity()->GetWorldPosition();
				ChangeState(eLuxEnemyState_BreakDoor);
			}
		
		kLuxOnMessage(eLuxEnemyMessage_EndOfPath)
			PatrolEndOfPath();

		kLuxOnMessage(eLuxEnemyMessage_SoundHeard)
			if(apMessage->mfCustomValue > mfHearVolume)
			{
				ChangeState(eLuxEnemyState_Investigate);
				mvTempPos = apMessage->mvCustomValue;
				mfTempVal = apMessage->mfCustomValue;
			}
	
	//------------------------------
	
	////////////////////////////////
	// Investigate
	////////////////////////////////

	//------------------------------	
	kLuxState(eLuxEnemyState_Investigate)

		kLuxOnEnter	
        {
			ChangeSoundState(eLuxEnemySoundState_Alert);
			SetMoveSpeed(eLuxEnemyMoveSpeed_Walk);
			
			mpPathfinder->Stop();
            AILOG( "stopping pathfinder, playing notice anim." );
			PlayAnim("Notice"+GetCurrentPoseSuffix(), false, 0.3f);

			//mfForwardSpeed *= 1.5f;
			PlaySound(msNoticeSound);

			mfFOVMul = 4.0f; //When hearing a sound, enemy gets extra alert.

			gpBase->mpDebugHandler->AddMessage(_W("Sound Heard! Vol: ")+cString::ToStringW(mfTempVal),false);
        }
		
        kLuxOnLeave
        {
            mfFOVMul = 1.0f;
			FadeOutCurrentAnim(0.2f);
			SetMoveSpeed(eLuxEnemyMoveSpeed_Walk);
        }


		//------------------------------

		kLuxOnMessage(eLuxEnemyMessage_AnimationOver)
        {
			SetMoveSpeed(eLuxEnemyMoveSpeed_Walk);

			cAINode *pNode = mpPathfinder->GetNodeAtPos(mvTempPos, 0, 10, true, true, true,NULL,1);
			if(pNode)
			{
                AILOG( "moving to node near sound." );
				mpPathfinder->MoveTo(pNode->GetPosition());
			}
			else
			{
                AILOG( "coudln't find node near sound, trying to move directly to sound pos via pathfinder." );
				gpBase->mpDebugHandler->AddMessage(_W("Could not find node near sound!"),false);
				mpPathfinder->MoveTo(mvTempPos);
			}
        }

		//------------------------------

		kLuxOnMessage(eLuxEnemyMessage_EndOfPath)
			
			PlaySound(msGiveUpNoticeSound);

			if(mPreviousState == eLuxEnemyState_Search)
            {
                AILOG( "received eLuxEnemyMessage_EndOfPath, switching back to SEARCH." );
				ChangeState(eLuxEnemyState_Search);	
            }
			else
            {
                AILOG( "received eLuxEnemyMessage_EndOfPath, switching to PATROL." );
				ChangeState(eLuxEnemyState_Patrol);	
            }

		//------------------------------
			
		kLuxOnUpdate
        {
			if(mbStuckAtDoor)// && mpMap->DoorIsClosed(mlStuckDoorID))
			{
				iLuxEntity *pDoorEnt = mpMap->GetEntityByID(mlStuckDoorID);
				mvTempPos = pDoorEnt->GetAttachEntity()->GetWorldPosition();
				ChangeState(eLuxEnemyState_BreakDoor);
			}
        }

		//------------------------------

		
		kLuxOnMessage(eLuxEnemyMessage_SoundHeard)
        {
			//If a new sound is loader than the previous go for that instead!
			if(mpPathfinder->IsMoving() && apMessage->mfCustomValue > mfTempVal)
			{
                AILOG( "received eLuxEnemyMessage_SoundHeard, sound:" + msHeardSoundName + ", sound is louder than previous, updating goal." );
				cAINode *pNode = mpPathfinder->GetNodeAtPos(mvTempPos, 0, 10, true, true, true,NULL,1);
				if(pNode)
                {
                    AILOG( "moving to node near pos." );
					mpPathfinder->MoveTo(pNode->GetPosition());
                }
				
				mfTempVal = apMessage->mfCustomValue;
			}
        }

	//------------------------------

	////////////////////////////////
	// Alert
	////////////////////////////////

	//------------------------------

	kLuxState(eLuxEnemyState_Alert)
		//////////////
		//Enter
		kLuxOnEnter
        {
			/////////////////////////
			// Flee
			if(mbFleeFromPlayer && mbThreatenOnAlert==false)
			{
                AILOG( "FleeFromPlayer set, and ThreatenOnAlert not, so changing to FLEE" );
				ChangeState(eLuxEnemyState_Flee);
			}
			/////////////////////////
			// Go into alert!
			else
			{
				/////////////////////////
				// Set up sound
				ChangeSoundState(eLuxEnemySoundState_Alert);

				gpBase->mpPlayer->AddTerrorEnemy(this);
				//gpBase->mpMusicHandler->AddEnemy(eLuxEnemyMusic_Search,this);

				/////////////////////////
				// Stay and threaten
				if(mbThreatenOnAlert)
				{
                    AILOG( "ThreatenOnAlert set, so playing threatloop" );

					float fThreatLength = mbFleeFromPlayer ? 3.0f : 6.0f;
					float fThreatFirstStopCheck = mbFleeFromPlayer ? 1.0f : 3.5f;

					mpPathfinder->Stop();
					PlayAnim("ThreatLoop", true, 0.3f);		
					SendMessage(eLuxEnemyMessage_TimeOut_2, fThreatLength, true); //Attack!!
					SendMessage(eLuxEnemyMessage_TimeOut_3, fThreatFirstStopCheck, true);//Check if stop the threat
					mbSkipVisibilityRangeHandicaps = true;
				}
				/////////////////////////
				// Move towards player
				else
				{
                    AILOG( "moving to last known player pos" );

					mpPathfinder->MoveTo(mvLastKnownPlayerPos);
					SetMoveSpeed(eLuxEnemyMoveSpeed_Walk);
					mfForwardSpeed *= 1.2f;
					SendMessage(eLuxEnemyMessage_TimeOut, 0.3f, true); //To update path
				}

				/////////////////////////
				// Set up properties
				mfFOVMul = 4.0f;

				mfAlertRunTowardsCount =0;
			}
        }

		//////////////
		//Leave
		kLuxOnLeave	
        {
			mbSkipVisibilityRangeHandicaps = false;

			if(mbThreatenOnAlert)
			{
				mpCharBody->SetMoveDelay(0.6f);
				mpMover->SetOverideMoveState(false);
			}

			mfFOVMul = 1.0f;
			if(mNextState != eLuxEnemyState_BreakDoor)
			{
				SetMoveSpeed(eLuxEnemyMoveSpeed_Walk);
				gpBase->mpPlayer->RemoveTerrorEnemy(this);
			}
        }

		//------------------------------
		
		//////////////
		//Enter
		kLuxOnUpdate
			float fDistToPlayer = DistToPlayer();

			//////////////////////////////
			//Turn towards player
			if(mbThreatenOnAlert && PlayerIsDetected())
			{
				float fAngleDist = cMath::GetAngleDistanceRad(	mpCharBody->GetYaw()+kPif, 
																gpBase->mpPlayer->GetCharacterBody()->GetYaw());
				if(fabs(fAngleDist)>cMath::ToRad(30))
					mpMover->TurnToPos(GetPlayerFeetPos());
			}

			//////////////////////////////
			//Check if player is moving towards enemy
			if(fDistToPlayer < mfAlertRunTowardsCheckDistance)
			{
				float fPlayerDirAmount = GetPlayerMovementTowardEnemyAmount();
				mfAlertRunTowardsCount += fPlayerDirAmount;
			}
			else
			{
				mfAlertRunTowardsCount-=1;
			}
			if(mfAlertRunTowardsCount<0) mfAlertRunTowardsCount=0;

			//////////////////////////////
			//Update the speed of movement.
			if(fDistToPlayer > mfIncreaseAlertSpeedDistance)
			{
				SetMoveSpeed(eLuxEnemyMoveSpeed_Walk);
				mfForwardSpeed *= 1.2f * mfIncreasedAlertSpeedMul;
			}
			else
			{
				SetMoveSpeed(eLuxEnemyMoveSpeed_Walk);
				mfForwardSpeed *= 1.2f;
			}
						
			//////////////////////////////
			//Stuck at door, break it
			if(gpBase->mpPlayer->GetTerror() >= 1 && mbStuckAtDoor)// && mpMap->DoorIsClosed(mlStuckDoorID))
			{
				iLuxEntity *pDoorEnt = mpMap->GetEntityByID(mlStuckDoorID);
				mvTempPos = pDoorEnt->GetAttachEntity()->GetWorldPosition();
                AILOG( "detected stuck at door, switching to BREAKDOOR." );
				ChangeState(eLuxEnemyState_BreakDoor);
			}
			//////////////////////////////
			//Player is no longer seen, see if time to search or wait
			else if(PlayerIsDetected()==false)
			{
				if(mbThreatenOnAlert==false)
				{
					if(gpBase->mpPlayer->GetTerror() < 1)
                    {
                        AILOG( "player no longer seen & terror < 1, switching to SEARCH." );
						ChangeState(eLuxEnemyState_Search);
                    }
				}
			}
			//////////////////////////////
			//Player is seen, see if close enough for hunt
			else
			{
				if(mbThreatenOnAlert)
				{
					//////////////
					//Flee
					if(mbFleeFromPlayer)
					{
						//If close enough or running quicly towards enemy
						if(mfAlertRunTowardsCount>mfAlertRunTowardsToHuntLimit || fDistToPlayer < mfAlertToInstantHuntDistance*2)
						{
							if(cMath::RandRectl(0,3)>0)
                            {
                                AILOG( "FleeFromPlayer set, player close enough or running quickly towards enemy, rolled a dice came up < 25%, switching to FLEE." );
								ChangeState(eLuxEnemyState_Flee);
                            }
							else
                            {
                                AILOG( "FleeFromPlayer set, player close enough or running quickly towards enemy, rolled a dice came up < 25%, switching to HUNT." );
								ChangeState(eLuxEnemyState_Hunt);
                            }
						}
					}
					//////////////
					//Attack
					else
					{
						//If terror is topped and distance to player is under a value or player is running towards piggie
						//Or if distance to player is less than a value
						if( (gpBase->mpPlayer->GetTerror() >= 1 && ( mfAlertRunTowardsCount>mfAlertRunTowardsToHuntLimit || fDistToPlayer < mfAlertToInstantHuntDistance*2)) ||
							fDistToPlayer < mfAlertToInstantHuntDistance)
						{
							gpBase->mpPlayer->SetTerror(1.0f);
                            AILOG( "player seen & close enough, switching to HUNT." );
							ChangeState(eLuxEnemyState_Hunt);
						}
					}
				}
				else
				{
					//If terror is topped and distance to player is over a value or player is running towards piggie
					//Or if distance to player is less than a value
					if(	(gpBase->mpPlayer->GetTerror() >= 1 && (fDistToPlayer > mfAlertToHuntDistance || mfAlertRunTowardsCount>mfAlertRunTowardsToHuntLimit) ) || 
						fDistToPlayer < mfAlertToInstantHuntDistance)
					{
						gpBase->mpPlayer->SetTerror(1.0f);
                        AILOG( "player seen & close enough, switching to HUNT." );
						ChangeState(eLuxEnemyState_Hunt);
					}
				}
			}

		//------------------------------

		//////////////
		//Reach end of path
		kLuxOnMessage(eLuxEnemyMessage_EndOfPath)
        {
			if(mbThreatenOnAlert==false)
			{
				float fDistToPlayer = DistToPlayer();

				//Path ended and player is not seen or enemy is stuck (this should only happen when at a distance!
				if(PlayerIsDetected()==false || (apMessage->mlCustomValue == 1 && fDistToPlayer>5))
				{
                    AILOG( "path ended & player not seen or enemy stuck, switching to SEARCH." );
					ChangeState(eLuxEnemyState_Search);
				}
				else if(apMessage->mlCustomValue==1 && PlayerIsDetected())
				{
					//This is when the enemy should just stand still but I think nothing is really needed
				}
			}
        }
		
		//------------------------------		

		//////////////
		//Update move to
		kLuxOnMessage(eLuxEnemyMessage_TimeOut)
        {
			if(PlayerIsDetected())
            {
                AILOG( "received eLuxEnemyMessage_TimeOut & player detected, updating pathfinder goal." );
				mpPathfinder->MoveTo(mvLastKnownPlayerPos);
            }
			
			SendMessage(eLuxEnemyMessage_TimeOut, 0.3f, true);
        }

		//------------------------------

		//////////////
		//Start Hunt
		kLuxOnMessage(eLuxEnemyMessage_TimeOut_2)
        {
			if(mbFleeFromPlayer)
			{
                AILOG( "was threatening, received eLuxEnemyMessage_TimeOut_2 & FleeFromPlayer set, switching to FLEE." );
				ChangeState(eLuxEnemyState_Flee);
			}
			else
			{
				gpBase->mpPlayer->SetTerror(1.0f);
                AILOG( "was threatening, received eLuxEnemyMessage_TimeOut_2 & FleeFromPlayer not set, switching to HUNT." );
				ChangeState(eLuxEnemyState_Hunt);
			}
        }

		/////////////
		//Check if threat should end
		kLuxOnMessage(eLuxEnemyMessage_TimeOut_3)
        {
			if(PlayerIsDetected()==false)
            {
                AILOG( "was threatening, received eLuxEnemyMessage_TimeOut_3 & player not detected, switching to PATROL." );
				ChangeState(eLuxEnemyState_Patrol);
            }
			else
            {
                AILOG( "was threatening, received eLuxEnemyMessage_TimeOut_3 & player detected, keep threatening." );
				SendMessage(eLuxEnemyMessage_TimeOut_3, 0.75f, true);
            }
        }


		//------------------------------

		//////////////
		//Takes a hit
		kLuxOnMessage(eLuxEnemyMessage_TakeHit)
        {
            AILOG( "received eLuxEnemyMessage_TakeHit, switching to HURT." );
			ChangeState(eLuxEnemyState_Hurt);
			gpBase->mpMusicHandler->RemoveEnemy(eLuxEnemyMusic_Search,this);
        }

		//------------------------------

		///////////
		//Overload
		kLuxOnMessage(eLuxEnemyMessage_PlayerDetected)

	//------------------------------

	////////////////////////////////
	// Search
	////////////////////////////////

	//------------------------------	
	
	kLuxState(eLuxEnemyState_Search)

		kLuxOnEnter
        {
			ChangeSoundState(eLuxEnemySoundState_Alert);

			SendMessage(eLuxEnemyMessage_TimeOut, mfPlayerSearchTime, true);
			
			SendMessage(eLuxEnemyMessage_TimeOut_2,cMath::RandRectf(0,1), true);
		
			gpBase->mpMusicHandler->RemoveEnemy(eLuxEnemyMusic_Attack,this);
			gpBase->mpMusicHandler->AddEnemy(eLuxEnemyMusic_Search,this);
			
			SetMoveSpeed(eLuxEnemyMoveSpeed_Walk);
			mfForwardSpeed *= 1.0f;
        }

		kLuxOnLeave
        {
			SetMoveSpeed(eLuxEnemyMoveSpeed_Walk);
        }

		//------------------------------
		
		kLuxOnUpdate
        {
			if(mbStuckAtDoor)// && mpMap->DoorIsClosed(mlStuckDoorID))
			{
				iLuxEntity *pDoorEnt = mpMap->GetEntityByID(mlStuckDoorID);
				mvTempPos = pDoorEnt->GetAttachEntity()->GetWorldPosition();
                AILOG( "detected stuck at door, moving to BREAKDOOR." );
				ChangeState(eLuxEnemyState_BreakDoor);
			}
        }
		
		//------------------------------

		//At node
		kLuxOnMessage(eLuxEnemyMessage_EndOfPath)
        {
            AILOG( "received eLuxEnemyMessage_EndOfPath, stopping pathfinder and waiting between 1 and 3 seconds." );
			mpPathfinder->Stop();
			SendMessage(eLuxEnemyMessage_TimeOut_2,cMath::RandRectf(1,3), true);
        }

		//Wait a few secs
		kLuxOnMessage(eLuxEnemyMessage_TimeOut_2)
        {
			//cAINode * pNode = GetSearchForPlayerNode();
			cAINode * pNode = mpPathfinder->GetNodeAtPos(gpBase->mpPlayer->GetCharacterBody()->GetFeetPosition(), 4, 12,false, false, true, NULL);
			if(pNode)
            {
                AILOG( "received eLuxEnemyMessage_TimeOut_2, found path node near player feet position, moving to that node." );
				mpPathfinder->MoveTo(pNode->GetPosition());
            }
			else
            {
                AILOG( "received eLuxEnemyMessage_TimeOut_2, did not find path node near player feet position, switching to PATROL." );
				ChangeState(eLuxEnemyState_Patrol);
            }
        }
		
		//End of searching
		kLuxOnMessage(eLuxEnemyMessage_TimeOut)
        {
            AILOG( "received eLuxEnemyMessage_TimeOut, end of searching, switching to PATROL." );
			ChangeState(eLuxEnemyState_Patrol);
			gpBase->mpMusicHandler->RemoveEnemy(eLuxEnemyMusic_Search,this);
        }

		//Hear sound
		kLuxOnMessage(eLuxEnemyMessage_SoundHeard)
        {
            AILOG( "received eLuxEnemyMessage_SoundHeard, sound:" + msHeardSoundName );
			if(apMessage->mfCustomValue > mfHearVolume)
			{
                AILOG( "sound was loud enough, switching to INVESTIGATE." );
				ChangeState(eLuxEnemyState_Investigate);
				mvTempPos = apMessage->mvCustomValue;
				mfTempVal = apMessage->mfCustomValue;
			}
        }
	
	//------------------------------

	////////////////////////////////
	// Flee
	////////////////////////////////

	//------------------------------

	kLuxState(eLuxEnemyState_Flee)
		///////////////////////
		// Enter
		kLuxOnEnter
        {
			if(FleeTryToFindSafeNode())
			{
                AILOG( "found safe node, fleeing" );

				SetMoveSpeed(eLuxEnemyMoveSpeed_Run);
				gpBase->mpMusicHandler->RemoveEnemy(eLuxEnemyMusic_Search,this);
				gpBase->mpMusicHandler->RemoveEnemy(eLuxEnemyMusic_Attack,this);
				gpBase->mpPlayer->RemoveTerrorEnemy(this);
				
				SendMessage(eLuxEnemyMessage_TimeOut, 2.0f, true); //Check if player is too close!

				ChangeMoveType(eLuxEnemyMoveType_Flee);
			}
			else
			{
                AILOG( "didn't find safe node, switching to HUNT" );
				ChangeState(eLuxEnemyState_Hunt);
			}
        }
		
		///////////////////////
		// Leave
		kLuxOnLeave
        {
			ChangeMoveType(eLuxEnemyMoveType_Normal);
        }

		//------------------------------

		//////////////////
		// Check if player is too close!
		kLuxOnMessage(eLuxEnemyMessage_TimeOut)
        {
			if(DistToPlayer2D()<2.5f)
			{
                AILOG( "player is too close, switching to HUNT" );
				ChangeState(eLuxEnemyState_Hunt);
			}
			
			SendMessage(eLuxEnemyMessage_TimeOut, 1.0f, true);
        }

		//------------------------------
		
		////////////////////////
		// End of current path
		kLuxOnMessage(eLuxEnemyMessage_EndOfPath)
        {
			//Check if pig can be auto removed, else continue fleeing or, go on patrol
            AILOG( "received eLuxEnemyMessage_EndOfPath, see if pig can be removed." );

			if(CheckEnemyAutoRemoval(15)==false)
			{
				if(PlayerIsDetected())
				{
                    AILOG( "no, and player is detected, see if we can find a new safe node." );

					if(FleeTryToFindSafeNode()==false)
                    {
                        AILOG( "didn't find one, switching to ALERT" );
						ChangeState(eLuxEnemyState_Alert);
                    }
                    else
                    {
                        AILOG( "found one." );
                    }
				}
				else
				{
                    AILOG( "no, and player is not detected, switching to PATROL." );
					ChangeState(eLuxEnemyState_Patrol);
				}
			}
        }

		////////////////////////
		// Overload global
		kLuxOnMessage(eLuxEnemyMessage_PlayerDetected)
		kLuxOnMessage(eLuxEnemyMessage_HelpMe)
	
	//------------------------------

	////////////////////////////////
	// Stalk
	// - The enemy walks close to the player but tries to avoid being seen or forced to go into alert mode.
	////////////////////////////////

	//------------------------------
	
	kLuxState(eLuxEnemyState_Stalk)	
		///////////////////////
		// Enter
		kLuxOnEnter
        {
			SetMoveSpeed(eLuxEnemyMoveSpeed_Walk);
			SendMessage(eLuxEnemyMessage_TimeOut, 2.5f, true);
			mfFOVMul =0.5f; //We want small FOV so that we do not go into alert unless needed.

			if(StalkFindNode()==false)
			{
                AILOG( "didn't find stalk node, waiting between 2 and 4 seconds, switching to WAIT" );

				mfWaitTime = cMath::RandRectf(2, 4);
				ChangeState(eLuxEnemyState_Wait);
			}
            else
            {
                AILOG( "found stalk node." );
            }
        }

		///////////////////////
		// Leave
		kLuxOnLeave
			mfFOVMul = 1.0f;
		//------------------------------

		////////////////////////////////
		//See if player see the enemy, might
		kLuxOnMessage(eLuxEnemyMessage_TimeOut)
        {
			bool bSeenByPlayer = IsSeenByPlayer();
			float fPlayerDistance = DistToPlayer();

			///////////////////////////
			// Select the move speed
			if(fPlayerDistance < 7.5f || bSeenByPlayer)
				SetMoveSpeed(eLuxEnemyMoveSpeed_Run);
			else
				SetMoveSpeed(eLuxEnemyMoveSpeed_Walk);

			///////////////////////////
			// See if a new node needs to be found.
			if(	fPlayerDistance < 4.5f || 
				bSeenByPlayer ||
				IsVisibleToPlayerAtFeetPos(mpPathfinder->GetNextGoalPos()) ||
				IsVisibleToPlayerAtFeetPos(mpPathfinder->GetFinalGoalPos())
				)
			{
                AILOG( "received eLuxEnemyMessage_TimeOut, current position not favorable, looking for a new stalk node" );

				if(StalkFindNode()==false)
				{
                    AILOG( "didn't find one, waiting 1-3 secs, switching to WAIT" );
					mfWaitTime = cMath::RandRectf(1, 3);
					ChangeState(eLuxEnemyState_Wait);
				}
                else
                {
                    AILOG( "found one" );
                }
			}
			///////////////////////////
			// If player is too close, we need to go into alert
			if(fPlayerDistance < 6.0f && bSeenByPlayer)
            {
                AILOG( "received eLuxEnemyMessage_TimeOut, player is too close and sees us, switching to ALERT" );
				ChangeState(eLuxEnemyState_Alert);
            }

			///////////////////////////
			// Repeat check after 2.5 seconds.
			SendMessage(eLuxEnemyMessage_TimeOut, 2.5f, true);
        }
		
		//------------------------------
		
		////////////////////////
		// End of current path
		kLuxOnMessage(eLuxEnemyMessage_EndOfPath)
        {
            AILOG( "received eLuxEnemyMessage_EndOfPath, waiting 2-4 secs switching to WAIT" );
			mfWaitTime = cMath::RandRectf(2, 4);
			ChangeState(eLuxEnemyState_Wait);
        }
		//------------------------------
		
		////////////////////////
		// Player detected
		kLuxOnMessage(eLuxEnemyMessage_PlayerDetected)
        {
            AILOG( "received eLuxEnemyMessage_PlayerDetected, triggerign re-evaluation in 0.2 seconds" );
			//Quickly check if we need new position
			SendMessage(eLuxEnemyMessage_TimeOut, 0.2f, true); 
        }
	

		//------------------------------
	
	//------------------------------

	////////////////////////////////
	// Track
	// - The enemy is out to get the player, but do so undiscovered and coming up from behind.
	////////////////////////////////

	//------------------------------
	kLuxState(eLuxEnemyState_Track)	
		///////////////////////
		// Enter
		kLuxOnEnter
        {
			SetMoveSpeed(eLuxEnemyMoveSpeed_Walk);
			SendMessage(eLuxEnemyMessage_TimeOut, 2.5f, true);
			
			if(TrackFindNode()==false)
			{
                AILOG( "didn't find track node, waiting between 1 and 2 seconds, switching to WAIT" );

				mfWaitTime = cMath::RandRectf(1, 2);
				ChangeState(eLuxEnemyState_Wait);
			}
            else
            {
                AILOG( "found track node" );
            }
        }

		///////////////////////
		// Leave
		kLuxOnLeave
			mfFOVMul = 1.0f;

		//------------------------------

		//////////////////////////
		//Check if enemy might be seen
		kLuxOnMessage(eLuxEnemyMessage_TimeOut)
        {
			if(	IsInPlayerFovAtFeetPos(mpCharBody->GetFeetPosition()) &&    // :MAARTEN: does this not risk the player seeing a teleporting pig?
				IsVisibleToPlayerAtFeetPos(mpCharBody->GetFeetPosition()) == false &&
				DistToPlayer2D() > 6.0f)
			{
                AILOG( "received eLuxEnemyMessage_TimeOut, in player FOV at feet pos but not visible, and distance larger than 6m, teleporting behind player" );

				TrackTeleportBehindPlayer();
			}
			SendMessage(eLuxEnemyMessage_TimeOut, 2.5f, true);
        }

		
		//------------------------------
		
		////////////////////////
		// End of current path
		kLuxOnMessage(eLuxEnemyMessage_EndOfPath)
        {
            AILOG( "received eLuxEnemyMessage_EndOfPath, looking for new track node" );

			if(TrackFindNode()==false)
			{
				if(DistToPlayer2D() < 4.0f)
				{
                    AILOG( "didn't find one, dist to player < 4m, waiting 2-6 seconds, switching to WAIT" );
					mfWaitTime = cMath::RandRectf(2, 6);
					ChangeState(eLuxEnemyState_Wait);
				}
				else if(DistToPlayer2D() < 6.0f)
				{
                    AILOG( "didn't find one, dist to player > 4 m and < 6m, waiting 1-3 seconds, switching to WAIT" );
					mfWaitTime = cMath::RandRectf(1, 3);
					ChangeState(eLuxEnemyState_Wait);
				}
				else
				{
                    AILOG( "didn't find one, dist to player > 6m, trying again" );

					if(TrackFindNode()==false)
					{
                        AILOG( "still no luck, waiting 1-2 seconds, switching to WAIT" );
						mfWaitTime = cMath::RandRectf(1, 2);
						ChangeState(eLuxEnemyState_Wait);
					}
                    else
                    {
                        AILOG( "found one" );
                    }
				}
				
			}
            else
            {
                AILOG( "found one" );
            }

        }
		
		//------------------------------

		////////////////////////
		// Sound Heard
		kLuxOnMessage(eLuxEnemyMessage_SoundHeard)
        {
            AILOG( "received eLuxEnemyMessage_SoundHeard, sound:" + msHeardSoundName );

			if(apMessage->mfCustomValue > mfHearVolume)
			{
                AILOG( "sound was loud enough, switching to INVESTIGATE." );
				ChangeState(eLuxEnemyState_Investigate);
				mvTempPos = apMessage->mvCustomValue;
				mfTempVal = apMessage->mfCustomValue;
			}
        }

		//------------------------------

	//------------------------------


	////////////////////////////////
	// Hunt
	////////////////////////////////

	//------------------------------

	kLuxState(eLuxEnemyState_Hunt)	
		///////////////////////
		// Enter
		kLuxOnEnter
        {
			gpBase->mpMusicHandler->RemoveEnemy(eLuxEnemyMusic_Search,this);

			if(mlAttackHitCounter >= 1)
			{
				mlAttackHitCounter =0;
								
                AILOG( "attackhitcounter > 1, switching to HUNTPAUSE." );
				ChangeState(eLuxEnemyState_HuntPause);
			}
			else
			{
				ChangeSoundState(eLuxEnemySoundState_Hunt);
				SetMoveSpeed(eLuxEnemyMoveSpeed_Run);
				SendMessage(eLuxEnemyMessage_TimeOut, 0.1f, true);
				mfFOVMul = 4.0f;
				
				gpBase->mpMusicHandler->AddEnemy(eLuxEnemyMusic_Attack,this);
				gpBase->mpPlayer->AddTerrorEnemy(this);

                AILOG( "moving to last known player pos." );
				mpPathfinder->MoveTo(mvLastKnownPlayerPos);

				SendMessage(eLuxEnemyMessage_TimeOut_2, 0.6f, true);
			}
        }
			

		///////////////////////
		// Leave
		kLuxOnLeave
			SetMoveSpeed(eLuxEnemyMoveSpeed_Walk);
			mfFOVMul = 1.0f;

		//------------------------------
		
		///////////////////////
		// Update
		kLuxOnUpdate
			//////////////////////////
			//Short attack
			float fDistToPlayer = DistToPlayer();
			if(CanSeePlayer() && fDistToPlayer < mfNormalAttackDistance)
			{
				//ChangeState(eLuxEnemyState_AttackMeleeShort);
                AILOG( "seeing player and dist to player smaller than NormalAttackDistance, switching to ATTACKMELEELONG." );
				ChangeState(eLuxEnemyState_AttackMeleeLong);
			}
			else if(mbStuckAtDoor)// && mpMap->DoorIsClosed(mlStuckDoorID))
			{
				iLuxEntity *pDoorEnt = mpMap->GetEntityByID(mlStuckDoorID);
				mvTempPos = pDoorEnt->GetAttachEntity()->GetWorldPosition();
                AILOG( "detected stuck at door, switching to BREAKDOOR." );
				ChangeState(eLuxEnemyState_BreakDoor);
			}
		
		//------------------------------
		
		/////////////////////////////
		// Check if close enough for launch attack
		kLuxOnMessage(eLuxEnemyMessage_TimeOut_2)
        {
			//////////////////////////
			//Launch attack
			float fDist = DistToPlayer();
			if(CanSeePlayer() && fDist > mfNormalAttackDistance && fDist < mfNormalAttackDistance*4.0f && mpMover->GetStuckCounter()<0.5f)
			{
                AILOG( "received eLuxEnemyMessage_TimeOut_2, player in sight & distance conditions right, switching to ATTACKMELEELONG." );
				ChangeState(eLuxEnemyState_AttackMeleeLong);
			}
			SendMessage(eLuxEnemyMessage_TimeOut_2, 0.6f, true);
        }

		//------------------------------

		////////////////////////
		// End of current path
		kLuxOnMessage(eLuxEnemyMessage_EndOfPath)
        {
			float fDistToPlayer = DistToPlayer();
			
			//Check if was end of path because of stuck.,
			if(apMessage->mlCustomValue==1 && fDistToPlayer >= mfNormalAttackDistance)
			{
				if(PlayerIsDetected() == false)
				{
					//Give some extra time to find player!
                    AILOG( "received eLuxEnemyMessage_EndOfPath because stuck and player not detected, waiting a little." );
					SendMessage(eLuxEnemyMessage_TimeOut_3, 1.5f, true);
				}
				else 
				{
                    AILOG( "received eLuxEnemyMessage_EndOfPath because stuck and player detected, switching to HUNTWANDER." );
					ChangeState(eLuxEnemyState_HuntWander);
				}
			}
			//Get new path
			else
			{
                AILOG( "received eLuxEnemyMessage_EndOfPath, moving to new LastKnownPlayerPos." );
				mpPathfinder->MoveTo(mvLastKnownPlayerPos);

				if(PlayerIsDetected() == false)
				{
                    AILOG( "but since player isn't detected, we wait a little longer (FG: 'When lost player give some extra time to catch up')." );
					//When lost player give some extra time to catch up
					SendMessage(eLuxEnemyMessage_TimeOut_3, 1.5f, true);
				}
			}
        }
			
		//------------------------------

		//////////////////
		// Update path and call for help!
		kLuxOnMessage(eLuxEnemyMessage_TimeOut)
        {
            AILOG( "received eLuxEnemyMessage_TimeOut, updating path and calling for help." );
			mpMap->BroadcastEnemyMessage(eLuxEnemyMessage_HelpMe, true, mpCharBody->GetPosition(), mfActivationDistance*0.5f,
											0,false, mpCharBody->GetFeetPosition());
		
			mpPathfinder->MoveTo(mvLastKnownPlayerPos);
			
			SendMessage(eLuxEnemyMessage_TimeOut, 0.2f, true);
        }

		//------------------------------
		
		//////////////////
		//When lost player give some extra time to catch up
		kLuxOnMessage(eLuxEnemyMessage_TimeOut_3)
        {
			if(PlayerIsDetected() == false)
			{
				gpBase->mpPlayer->RemoveTerrorEnemy(this);
				gpBase->mpMusicHandler->RemoveEnemy(eLuxEnemyMusic_Attack,this);

				if(mbThreatenOnAlert)
                {
                    AILOG( "received eLuxEnemyMessage_TimeOut_3 ThreateOnAlert set and player still not detected, switching to PATROL." );
					ChangeState(eLuxEnemyState_Patrol);
                }
				else
                {
                    AILOG( "received eLuxEnemyMessage_TimeOut_3 ThreateOnAlert not set and player still not detected, switching to SEARCH." );
					ChangeState(eLuxEnemyState_Search);
                }
			}
			else
			{
                AILOG( "received eLuxEnemyMessage_TimeOut_3 and player detected, short delay to eLuxEnemyMessage_TimeOut." );
				SendMessage(eLuxEnemyMessage_TimeOut, 0.2f, true);
			}
        }

		////////////////////////
		// Overload global
		kLuxOnMessage(eLuxEnemyMessage_PlayerDetected)
		kLuxOnMessage(eLuxEnemyMessage_HelpMe)
	
	//------------------------------
	
	////////////////////////////////////////////////////////////////////////////////////
	// Play patrol anim first, and if not looped, go back to patrol mode
	////////////////////////////////////////////////////////////////////////////////////
	kLuxState(eLuxEnemyState_PatrolAnimation)
	
		kLuxOnEnter
		{
			GetCharacterBody()->StopMovement();
			PlayAnim( GetCurrentPatrolNode()->msAnimation, GetCurrentPatrolNode()->mbLoopAnimation, 0.3f );			
		}

		kLuxOnMessage(eLuxEnemyMessage_AnimationOver)
		{
            #if LOG_AI
                tString string = "received eLuxEnemyMessage_AnimationOver, switching back to ";
                string = string + cString::To8Char(gsLuxEnemyStates[mfPostAnimState]) + ".";
                AILOG( string );
            #endif
			ChangeState(mfPostAnimState);
		}

		kLuxOnMessage(eLuxEnemyMessage_StopPatrolAnimation)
		{
            //SetMoveType(eLuxEnemyMoveType_FleeBiped);
			mpMover->UseMoveStateAnimations();
            #if LOG_AI
                tString string = "received eLuxEnemyMessage_StopPatrolAnimation, switching back to ";
                string = string + cString::To8Char(gsLuxEnemyStates[mfPostAnimState]) + ".";
                AILOG( string );
            #endif
			ChangeState(mfPostAnimState);
		}

		kLuxOnLeave
		{
			FinishPatrolEndOfPath(false);
		}

	///////////////////////////////
	// Hunt Wander (Remain in hunt mode but go to nearby reachable node)
	////////////////////////////////

	//------------------------------

	kLuxState(eLuxEnemyState_HuntWander)
		///////////////////////
		// Enter
		kLuxOnEnter
        {
			mpMover->ResetStuckCounter();

			SetMoveSpeed(eLuxEnemyMoveSpeed_Walk);
			mfForwardSpeed *= 1.2f;

			mfFOVMul = 4.0f;

			cAINode * pNode = mpPathfinder->GetNodeAtPos(mpCharBody->GetFeetPosition(), 2, 5, false, true, true, NULL);
			if(pNode)
            {
                AILOG( "moving to nearby reachable node." );
				mpPathfinder->MoveTo(pNode->GetPosition());
            }
			else
            {
                AILOG( "no nearby reachable node found, switching to HUNT." );
				ChangeState(eLuxEnemyState_Hunt);
            }
        }

		///////////////////////
		// Leave
		kLuxOnLeave
        {
			mfFOVMul = 1.0f;
        }

		//------------------------------

		kLuxOnMessage(eLuxEnemyMessage_EndOfPath)
        {
            AILOG( "received eLuxEnemyMessage_EndOfPath, switching to HUNT." );
			ChangeState(eLuxEnemyState_Hunt);
        }

		////////////////////////
		// Overload global
		kLuxOnMessage(eLuxEnemyMessage_PlayerDetected)
		kLuxOnMessage(eLuxEnemyMessage_HelpMe)

	//------------------------------

	////////////////////////////////
	// Hunt Pause (Walk slowly towards the player)
	////////////////////////////////

	//------------------------------

	kLuxState(eLuxEnemyState_HuntPause)	
		///////////////////////
		// Enter
		kLuxOnEnter	
		SendMessage(eLuxEnemyMessage_TimeOut_2, cMath::RandRectf(mfHuntPauseMinTime, mfHuntPauseMaxTime), true);
			mpPathfinder->Stop();
			
			SetMoveSpeed(eLuxEnemyMoveSpeed_Walk);
			mfForwardSpeed *= 0.6f;

			mfFOVMul = 4.0f;

			SendMessage(eLuxEnemyMessage_TimeOut, 0.2f, true);

		///////////////////////
		// Leave
		kLuxOnLeave
			mfFOVMul = 1.0f;

		//------------------------------
		
		///////////////////////
		// Update
		kLuxOnMessage(eLuxEnemyMessage_TimeOut)
			
			if(DistToPlayer2D() > 2.0f)
            {
                AILOG( "received eLuxEnemyMessage_TimeOut, > 2m to player, so walking towards player." );
				mpPathfinder->MoveTo(gpBase->mpPlayer->GetCharacterBody()->GetFeetPosition());
            }

			if( SeesPlayer() )
			{
				SendMessage(eLuxEnemyMessage_TimeOut, 0.2f, true);	
			}
		
		///////////////////////
		// Time out
		kLuxOnMessage(eLuxEnemyMessage_TimeOut_2)
        {
            AILOG( "received eLuxEnemyMessage_TimeOut_2, so hunt pause timed out, switching to ALERT." );
			ChangeState(eLuxEnemyState_Alert);
        }

		////////////////////////
		// Overload global
		kLuxOnMessage(eLuxEnemyMessage_PlayerDetected)
		kLuxOnMessage(eLuxEnemyMessage_HelpMe)
		
	//------------------------------

	////////////////////////////////
	// Break door
	////////////////////////////////

	//------------------------------

	kLuxState(eLuxEnemyState_BreakDoor)	
		kLuxOnEnter
            AILOG( "stopping pathfinder, playing breakdoor anim." );
			mpPathfinder->Stop();
			PlayAnim("BreakDoor",false, 0.3f);
			mfFOVMul = 4.0f;

		kLuxOnLeave
			mlAttackHitCounter =0; //When returning from door breakage there should be no pause!
			mfFOVMul = 1.0f;

		//------------------------------

		kLuxOnUpdate
			//Turn towards the door!
			mpMover->TurnToPos(mvTempPos);

		//------------------------------

		kLuxOnMessage(eLuxEnemyMessage_AnimationOver)
			if(PlayerIsDetected())
			{
                AILOG( "received eLuxEnemyMessage_AnimationOver and player detected, switching to ALERT." );
				ChangeState(eLuxEnemyState_Alert);
			}
			else if(mpMap->DoorIsBroken(mlStuckDoorID))
			{
				if(mPreviousState == eLuxEnemyState_Hurt)
                {
                    AILOG( "received eLuxEnemyMessage_AnimationOver & door is broken, previous state was HURT, switching to HUNT (sic)." );
					ChangeState(eLuxEnemyState_Hunt);
                }
				else 
                {
                    #if LOG_AI
                        tString string = "received eLuxEnemyMessage_AnimationOver & door is broken, switching back to ";
                        string = string + cString::To8Char(gsLuxEnemyStates[mPreviousState]) + ".";
                        AILOG( string );
                    #endif
					ChangeState(mPreviousState);
                }
			}
			else
			{
                AILOG( "received eLuxEnemyMessage_AnimationOver & door is not broken yet, playing breakdoor again." );
				PlayAnim("BreakDoor",false, 0.3f);
			}

			
		
		kLuxOnMessage(eLuxEnemyMessage_AnimationSpecialEvent)
        {
            AILOG( "received eLuxEnemyMessage_AnimationSpecialEvent, attacking door." );
			Attack(mNormalAttackSize, mBreakDoorAttackDamage);
        }
			

		////////////////////////
		// Overload global
		kLuxOnMessage(eLuxEnemyMessage_PlayerDetected)
		kLuxOnMessage(eLuxEnemyMessage_HelpMe)
        		
	//------------------------------

	////////////////////////////////
	// Attack Short
	////////////////////////////////

	//------------------------------

	kLuxState(eLuxEnemyState_AttackMeleeShort)
		kLuxOnEnter
			mpPathfinder->Stop();
            AILOG( "stopping pathfinder and playing attack animation." );
			PlayAnim("Attack"+GetCurrentPoseSuffix(),false, 0.3f);
			mfFOVMul = 4.0f;

		kLuxOnLeave
			mfFOVMul = 1.0f;

		//------------------------------

		kLuxOnMessage(eLuxEnemyMessage_AnimationOver)
			if(mPreviousState == eLuxEnemyState_Hurt)
            {
                AILOG( "received eLuxEnemyMessage_AnimationOver and previous state was HURT, switching to HUNT (sic)." );
				ChangeState(eLuxEnemyState_Hunt);
            }
			else 
			{
                #if LOG_AI
                    tString string = "received eLuxEnemyMessage_AnimationOver, switching back to ";
                    string = string + cString::To8Char(gsLuxEnemyStates[mPreviousState]) + ".";
                    AILOG( string );
                #endif
                ChangeState(mPreviousState);
            }

		kLuxOnMessage(eLuxEnemyMessage_AnimationSpecialEvent)
        {
            AILOG( "received eLuxEnemyMessage_AnimationSpecialEvent, doing damage." );
			Attack(mNormalAttackSize, mNormalAttackDamage);
        }

		//------------------------------
		
		////////////////////////
		// Overload global
		kLuxOnMessage(eLuxEnemyMessage_PlayerDetected)
		kLuxOnMessage(eLuxEnemyMessage_HelpMe)
	
	//------------------------------

	////////////////////////////////
	// Attack Launch
	////////////////////////////////

	//------------------------------

	kLuxState(eLuxEnemyState_AttackMeleeLong)
		kLuxOnEnter
			mpPathfinder->Stop();
			SetMoveSpeed(eLuxEnemyMoveSpeed_Run);
			mfForwardSpeed *= 2.0f;
			mlTempVal = 0;
			mfFOVMul = 4.0f;
			SendMessage(eLuxEnemyMessage_TimeOut, 1.0f, true); //Run a bit at full speed before charge.

		kLuxOnLeave
			mfFOVMul = 1.0f;
			SetMoveSpeed(eLuxEnemyMoveSpeed_Run);

		//------------------------------

		kLuxOnUpdate
			//If close enough, charge early.
			if(mlTempVal==0 && DistToPlayer2D() < 1.3f)
			{
				mlTempVal = 1;
                AILOG( "close enough, play charge anim" );

				PlayAnim("Charge"+GetCurrentPoseSuffix(),false, 0.3f, false);
			}

			//Move towards player
			mpMover->MoveToPos(gpBase->mpPlayer->GetCharacterBody()->GetFeetPosition());

		//------------------------------
			
		kLuxOnMessage(eLuxEnemyMessage_TimeOut)
			if(mlTempVal==0)
			{
				mlTempVal = 1;
                AILOG( "received eLuxEnemyMessage_TimeOut, playing charge anim." );
				PlayAnim("Charge"+GetCurrentPoseSuffix(),false, 0.3f, false);
			}

		kLuxOnMessage(eLuxEnemyMessage_AnimationSpecialEvent)
			if(mlTempVal==1)
            {
                AILOG( "received eLuxEnemyMessage_AnimationSpecialEvent and tempval == 1, doing damage." );

				Attack(mNormalAttackSize, mNormalAttackDamage);
            }
            else
            {
                AILOG( "received eLuxEnemyMessage_AnimationSpecialEvent and tempval != 1, ignoring." );
            }
		
		kLuxOnMessage(eLuxEnemyMessage_AnimationOver)
        {
            #if LOG_AI
                tString string = "received eLuxEnemyMessage_AnimationOver, switching back to ";
                string = string + cString::To8Char(gsLuxEnemyStates[mPreviousState]) + ".";
                AILOG( string );
            #endif
			ChangeState(mPreviousState);
        }

		//------------------------------

		////////////////////////
		// Overload global
		kLuxOnMessage(eLuxEnemyMessage_PlayerDetected)
		kLuxOnMessage(eLuxEnemyMessage_HelpMe)
	
	//------------------------------

	////////////////////////////////
	// Hurt
	////////////////////////////////

	//------------------------------

	kLuxState(eLuxEnemyState_Hurt)
		kLuxOnEnter
			mpPathfinder->Stop();	
            AILOG( "stopping pathfinder and playing Flinch animation." );
			PlayAnim("Flinch"+GetCurrentPoseSuffix(),false, 0.5f);
			SendMessage(eLuxEnemyMessage_TimeOut, 0.2f, true);
			mfFOVMul = 4.0f;

		kLuxOnLeave
			mfFOVMul = 1.0f;

		//------------------------------

		kLuxOnMessage(eLuxEnemyMessage_AnimationOver)
			if(PlayerIsDetected())
			{
                AILOG( "received eLuxEnemyMessage_AnimationOver and player detected, switching to HUNT." );
				ChangeState(eLuxEnemyState_Hunt);	
			}
			else
			{
                AILOG( "received eLuxEnemyMessage_AnimationOver and player detected, switching to ALERT." );
				ChangeState(eLuxEnemyState_Alert);	
			}
		
		kLuxOnMessage(eLuxEnemyMessage_TimeOut)
			if(cMath::RandRectl(0,1)==0 && DistToPlayer() < mfNormalAttackDistance*1.3f)
			{
                AILOG( "received eLuxEnemyMessage_TimeOut and distance conditions OK, switching to ATTACKMELEESHORT." );
				ChangeState(eLuxEnemyState_AttackMeleeShort);
			}

		////////////////////////
		// Overload global
		kLuxOnMessage(eLuxEnemyMessage_PlayerDetected)
		kLuxOnMessage(eLuxEnemyMessage_HelpMe)

	//------------------------------

	////////////////////////////////
	// Dead (TEMP! USE RAGDOLL)
	////////////////////////////////

	//------------------------------

	kLuxState(eLuxEnemyState_Dead)
		kLuxOnEnter
			mpPathfinder->Stop();
			//PlayAnim("Dead",false, 0.4f);	
            AILOG( "stopping pathfinder and playing Dead animation." );
			PlayAnim("Dead",false, 0.3f,false,1.0f,false,true,false);	
			gpBase->mpPlayer->RemoveTerrorEnemy(this);
			gpBase->mpMusicHandler->RemoveEnemy(eLuxEnemyMusic_Attack,this);
			mpCharBody->SetActive(false);


		kLuxOnMessage(eLuxEnemyMessage_AnimationOver)
			//PlayAnim("Dead",false, 0.3f,false,1.0f,false,true,false);	

		////////////////////////
		// Overload global
		kLuxOnMessage(eLuxEnemyMessage_PlayerDetected)
		kLuxOnMessage(eLuxEnemyMessage_HelpMe)

	kLuxEndStateMachine
}


//-----------------------------------------------------------------------

void cLuxEnemy_ManPig::OnRenderSolidImplemented(cRendererCallbackFunctions* apFunctions)
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

bool cLuxEnemy_ManPig::PlayerIsDetected()
{
	if(SeesPlayer())
	{
		return true;
	}
	else
	{
		return (DistToPlayer() < mpCharBody->GetSize().x && PlayerInFOV());
	}
}

//-----------------------------------------------------------------------

void cLuxEnemy_ManPig::OnDisableTriggers()
{
	if(mCurrentState == eLuxEnemyState_Idle) return;

	ChangeState(eLuxEnemyState_Patrol);
}

//-----------------------------------------------------------------------

float cLuxEnemy_ManPig::GetDamageMul(float afAmount, int alStrength)
{
	if(	mCurrentState == eLuxEnemyState_Idle ||
		mCurrentState == eLuxEnemyState_Wait ||
		mCurrentState == eLuxEnemyState_Patrol ||
		mCurrentState == eLuxEnemyState_Investigate) 
	{
		return 1.0f;
	}
	if( mCurrentState == eLuxEnemyState_Hurt) return 0.5f;
	
	return 0.2f;
}

//-----------------------------------------------------------------------

bool cLuxEnemy_ManPig::CheckEnemyAutoRemoval(float afDistance)
{
	if(mbIsSeenByPlayer==false && DistToPlayer() > afDistance && mbAutoRemoveAtPathEnd)
	{
		SetActive(false);	

		RunCallbackFunc("OnAutoDisabled");

		return true;
	}
	return false;
}

//-----------------------------------------------------------------------

void cLuxEnemy_ManPig::PatrolUpdateGoal()
{
	if(mvPatrolNodes.empty())
	{
		switch(mIdleBehavior)
		{
		case eLuxIdleBehavior_None:
			ChangeState(eLuxEnemyState_Wait);
			break;

		case eLuxIdleBehavior_Stalk:
			ChangeState(eLuxEnemyState_Stalk);
			break;

		case eLuxIdleBehavior_Track:
			ChangeState(eLuxEnemyState_Track);
			break;

		}
		
		
		return;
	}
	
	cLuxEnemyPatrolNode* pPatrolNode = GetCurrentPatrolNode();
	
	mpPathfinder->MoveTo(pPatrolNode->mpNode->GetPosition());
	
	/*if(OutsideStartRadius() && DistToPlayer() > mfActivationDistance * 0.5f)
	{
		ChangeState(eLuxEnemyState_GoHome);
	}
	else
	{
		cAINode * pNode = GetPatrolAroundPlayerNode();
		if(pNode)
			mpPathfinder->MoveTo(pNode->GetPosition());
		else
			ChangeState(eLuxEnemyState_Wait);
	}*/
}

//-----------------------------------------------------------------------

void cLuxEnemy_ManPig::PatrolEndOfPath()
{
	cLuxEnemyPatrolNode *pNode = GetCurrentPatrolNode();

	if ( pNode && !pNode->msAnimation.empty() )
	{
		mfPostAnimState = GetCurrentEnemyState();
        AILOG( "end of patrol path & have a patrol animation, switching to PATROLANIMATION." );
		ChangeState(eLuxEnemyState_PatrolAnimation);
		return;
	}

	FinishPatrolEndOfPath(true);
}

//-----------------------------------------------------------------------

void cLuxEnemy_ManPig::FinishPatrolEndOfPath(bool callPatrolUpdateNow)
{
	cLuxEnemyPatrolNode *pNode = GetCurrentPatrolNode();

	if( !mbPathReversed && IsAtLastPatrolNode()
        || mbPathReversed && IsAtFirstPatrolNode() )
	{
		if ( CheckEnemyAutoRemoval( mfAutoRemoveMinPlayerDistance ) )
		{
            AILOG( "FinishPatrolEndOfPath, at last patrol node, not seen by player, distance to player > AutoRemoveMinPlayerDistance and AutoRemoveAtPathEnd is set, so deactivating & calling OnAutoDisabled." );
            SetActive(false);	

			RunCallbackFunc("OnAutoDisabled");

			return;
		}

        if ( mbAutoReverseAtPathEnd )
        {
            mbPathReversed = !mbPathReversed;
        }
	}

	if(pNode)
		mfWaitTime = pNode->mfWaitTime;
	else
		mfWaitTime =0;

	if ( mbPathReversed )
	{
		DecCurrentPatrolNode(true);
	}
	else
	{
		IncCurrentPatrolNode(true);
	}

	if ( mfWaitTime == 0 && mbAllowZeroNodeWaitTimes )
	{
        if ( callPatrolUpdateNow )
        {
		    PatrolUpdateGoal();
        }
	}
	else
	{
        AILOG( "FinishPatrolEndOfPath, wait time specified, so switching to WAIT." );
		ChangeState(eLuxEnemyState_Wait);
	}
}

//-----------------------------------------------------------------------

bool cLuxEnemy_ManPig::FleeTryToFindSafeNode()
{
	float fLength=20;
	cVector3f vDirToPlayer = GetDirection2DToPlayer();

	//Try a couple of times to find a good node!
	for(int i=0; i<10; ++i)
	{
		/////////////////////////////////////
		//Find a node to run to
		cAINode * pNode = mpPathfinder->GetNodeAtPos(	mpCharBody->GetPosition() + vDirToPlayer*fLength*0.2f,
														0, fLength, false, false, true, NULL);
		if(pNode == NULL && DistToPlayer2D(pNode->GetPosition())<4.0f)
		{
			continue;
		}

		/////////////////////////////////////
		//See if first node takes you away from player
		mpPathfinder->MoveTo(pNode->GetPosition());
		cVector3f vDirToNode = GetDirection2D(mpPathfinder->GetNextGoalPos());
		if(cMath::Vector3Angle(vDirToPlayer, vDirToNode)<cMath::ToRad(45))
		{
			mpPathfinder->Stop();
			continue;
		}

		//Path was found!
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------

bool cLuxEnemy_ManPig::StalkFindNode()
{

	//Try a couple of times to find a good node!
	int lNumOfTries=10;
	for(int i=0; i<lNumOfTries; ++i)
	{
		/////////////////////////////////////
		//Find a node to run to
		cAINode * pNode = mpPathfinder->GetNodeAtPos(GetPlayerFeetPos(), 7, 30, false, false, true, NULL);
		if(pNode==NULL)
		{
			return false;
		}

		////////////////////////////////////
		//Check if player cannot see enemy at this position or if it is far enough away.
		float fDistance = DistToPlayer2D(pNode->GetPosition());
		
		if(fDistance < 15.5f && IsVisibleToPlayerAtFeetPos(pNode->GetPosition()))
		{
			continue;
		}

		//////////////////////////////
		//Move to pos
		if ( !mpPathfinder->MoveTo(pNode->GetPosition()) )
        {
            continue;    // if we don't have a path towards the new node, discard it. This could be a node on another floor.
        }

		/////////////////////////////////////
		//Check so player does not see the first path node
		if(IsVisibleToPlayerAtFeetPos(mpPathfinder->GetNextGoalPos()))
		{
			mpPathfinder->Stop();
			continue;
		}
		
		return true; //Take this node!
	}

	return false;
}

//-----------------------------------------------------------------------

bool cLuxEnemy_ManPig::TrackFindNode()
{
	float fDistToPlayer = DistToPlayer2D();

	float fMaxDistance = cMath::Max(fDistToPlayer *0.65f, 4.0f);
	float fMinDistance = fDistToPlayer *0.4f;
	
	int lNumOfTries=10;
	for(int i=0; i<lNumOfTries; ++i)
	{
		/////////////////////////////////////
		//Find a node to run to
		cAINode * pNode = mpPathfinder->GetNodeAtPos(GetPlayerFeetPos(), fMinDistance, fMaxDistance, false, false, true, NULL);
		if(pNode==NULL)
		{
			return false;
		}

		if(i==lNumOfTries-1)
		{
			mpPathfinder->MoveTo(pNode->GetPosition());
			return true;
		}

		if(IsVisibleToPlayerAtFeetPos(pNode->GetPosition()))
		{
			continue;
		}

		//////////////////////////////
		//Move to pos
		if (!mpPathfinder->MoveTo(pNode->GetPosition()))
        {
            continue;
        }

		/////////////////////////////////////
		//Check so player does not see the first path node
		if(IsVisibleToPlayerAtFeetPos(mpPathfinder->GetNextGoalPos()))
		{
			mpPathfinder->Stop();
			continue;
		}

		return true; //Take this node!
	}

	return false;
	
}

//-----------------------------------------------------------------------

bool cLuxEnemy_ManPig::TrackTeleportBehindPlayer()
{
	float fDistance = DistToPlayer2D()*0.7;

	cVector3f vPlayerBackward = gpBase->mpPlayer->GetCharacterBody()->GetForward()*-1;

	int lNumOfTries=10;
	for(int i=0; i<lNumOfTries; ++i)
	{
		/////////////////////////////////////
		//Find a node to run to
		cAINode * pNode = mpPathfinder->GetNodeAtPos(GetPlayerFeetPos()+vPlayerBackward*fDistance,
														0, fDistance*0.5f, false, false, true, NULL);
		if(pNode==NULL)
		{
			return false;
		}

		if(i==lNumOfTries-1)
		{
			mpPathfinder->MoveTo(pNode->GetPosition());
			return true;
		}

		if(IsVisibleToPlayerAtFeetPos(pNode->GetPosition()))
		{
			continue;
		}
		
		mpCharBody->SetFeetPosition(pNode->GetPosition());

		return true; //Take this node!
	}

	return false;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// SAVE DATA STUFF
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

kBeginSerialize(cLuxEnemy_ManPig_SaveData, iLuxEnemy_SaveData)

kSerializeVar(mbThreatenOnAlert, eSerializeType_Bool)
kSerializeVar(mbFleeFromPlayer, eSerializeType_Bool)
kSerializeVar(mIdleBehavior, eSerializeType_Int32)

kEndSerialize()

//-----------------------------------------------------------------------

iLuxEntity_SaveData* cLuxEnemy_ManPig::CreateSaveData()
{
	return hplNew(cLuxEnemy_ManPig_SaveData, ());
}

//-----------------------------------------------------------------------

void cLuxEnemy_ManPig::SaveToSaveData(iLuxEntity_SaveData* apSaveData)
{
	//////////////////
	//Init
	super_class::SaveToSaveData(apSaveData);
	cLuxEnemy_ManPig_SaveData *pData = static_cast<cLuxEnemy_ManPig_SaveData*>(apSaveData);

	//////////////////
	//Set variables
	kCopyToVar(pData,mbThreatenOnAlert);
	kCopyToVar(pData,mbFleeFromPlayer);
	kCopyToVar(pData,mIdleBehavior);
}

//-----------------------------------------------------------------------

void cLuxEnemy_ManPig::LoadFromSaveData(iLuxEntity_SaveData* apSaveData)
{
	//////////////////
	//Init
	super_class::LoadFromSaveData(apSaveData);
	cLuxEnemy_ManPig_SaveData *pData = static_cast<cLuxEnemy_ManPig_SaveData*>(apSaveData);

	//////////////////
	//Set variables
	kCopyFromVar(pData,mbThreatenOnAlert);
	kCopyFromVar(pData,mbFleeFromPlayer);
	mIdleBehavior  = (eLuxIdleBehavior)pData->mIdleBehavior;
}

//-----------------------------------------------------------------------

void cLuxEnemy_ManPig::SetupSaveData(iLuxEntity_SaveData *apSaveData)
{
	super_class::SetupSaveData(apSaveData);
}


//-----------------------------------------------------------------------

