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

#ifndef LUX_ENEMY_MOVER_H
#define LUX_ENEMY_MOVER_H

//----------------------------------------------

#include "LuxBase.h"

//----------------------------------------------

class iLuxEnemy;

//----------------------------------------------

enum eLuxEnemyMoveState
{
	eLuxEnemyMoveState_Backward,
	eLuxEnemyMoveState_Stopped,
	eLuxEnemyMoveState_Walking,
	eLuxEnemyMoveState_Jogging,
	eLuxEnemyMoveState_Running,

	eLuxEnemyMoveState_LastEnum
};

//----------------------------------------------

class cLuxEnemyMover
{
friend class cLuxEnemyMover_SaveData;
friend class iLuxEnemy;
public:	
	cLuxEnemyMover(iLuxEnemy *apEnemy, iCharacterBody *apCharBody);
	virtual ~cLuxEnemyMover();

	//////////////////////
	//General
	void SetupAfterLoad(cWorld *apWorld);

	void OnUpdate(float afTimeStep);

	//////////////////////
	//Actions
	void MoveToPos(const cVector3f& avFeetPos);

	void TurnToPos(const cVector3f& avFeetPos);
	void TurnToAngle(float afAngle);

	void MoveBackwardsToPos(const cVector3f& avFeetPos);
	void TurnAwayFromPos(const cVector3f& avFeetPos);

	void UseMoveStateAnimations();
	
	void ForceMoveState(eLuxEnemyMoveState aMoveState);

	//////////////////////
	//Properties

	//This is used by enemy when caclulting final max speed.
	float CalculateSpeedMul(float afTimeStep);

	//This get the speed along the movement direction (so it does not accound for falling speeds, climbing, etc
	float GetMoveSpeed();

	//This gets speed / wanted_speed
	float GetWantedSpeedAmount();

	void SetOverideMoveState(bool abX);
	bool GetOverideMoveState(){ return mbOverideMoveState;}

	float GetStuckCounter(){ return mfStuckCounter; }
	float GetMaxStuckCounter(){ return mfMaxStuckCounter; }
	bool GetStuckCounterIsAtMax(){ return mfStuckCounter >= mfMaxStuckCounter;}
	void ResetStuckCounter(){ mfStuckCounter =0; }

	void StopTurning() { mbTurning = false; }

	void SetWallAvoidanceActive(bool abX){ mbWallAvoidActive = abX;}

	void SetupWallAvoidance(float afRadius, float afSteerAmount, int alSamples);

	//////////////////////
	//UPdate
	void UpdateMoveAnimation(float afTimeStep);
	void UpdateStuckCounter(float afTimeStep);	
	void UpdateTurning(float afTimeStep);
	void UpdateStepEffects(float afTimeStep);
	void UpdateWallAvoidance(float afTimeStep);

	//////////////////////
	//Debug
	void OnRenderSolid(cRendererCallbackFunctions* apFunctions);
	
	//////////////////////
	//Save data stuff
	
private:
	void ConvertLocalDirTo2D(cVector3f& avLocalDir);
	cMatrixf GetMovementDirectionMatrix();

	///////////////////
	// Data
	iLuxEnemy *mpEnemy;
	iCharacterBody *mpCharBody;

	float mfStuckLimit;
	float mfMaxStuckCounter;

	float mfWallAvoidRadius;
	float mfWallAvoidSteerAmount;

	static std::vector<cVector3f> mvPrecalcSampleDirs;

	///////////////////
	// Variables
	bool mbTurning;
	float mfTurnGoalAngle;
	float mfTurnSpeed;
	float mfTurnBreakAcc;

	cVector3f mvCurrentGoal;

	float mfStuckCounter;

	cVector3f mvSteeringVec;

	eLuxEnemyMoveState mMoveState;
	bool mbOverideMoveState;

	bool mbWallAvoidActive;
	float mfWallAvoidCount;
	std::vector<cVector3f> mvSampleRays;
	std::vector<cVector3f> mvSampleRayBaseDir;
	std::vector<bool> mvSampleRaysCollide;
	std::vector<float> mvSampleRaysAmount;
	std::vector<int> mvSamplePartitionUsed;
	int mlSampleMaxPartCount;
};

//----------------------------------------------


#endif // LUX_ENEMY_MOVER_H
