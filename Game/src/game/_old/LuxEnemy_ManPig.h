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

#ifndef LUX_ENEMY_MAN_PIG_H
#define LUX_ENEMY_MAN_PIG_H

//----------------------------------------------

#include "LuxEnemy.h"
#include "LuxIdleBehavior_ManPig.h"

//----------------------------------------------

class cLuxEnemy_ManPig_SaveData : public iLuxEnemy_SaveData
{
	kSerializableClassInit(cLuxEnemy_ManPig_SaveData)
public:
	////////////////
	//Properties
	bool mbThreatenOnAlert;
	bool mbFleeFromPlayer;
	
	int mIdleBehavior;
};

//----------------------------------------------

class cLuxEnemy_ManPig : public iLuxEnemy
{
friend class cLuxEnemyLoader_ManPig;
typedef iLuxEnemy super_class;
public:	
	cLuxEnemy_ManPig(const tString &asName, int alID, cLuxMap *apMap);
	virtual ~cLuxEnemy_ManPig();

	//////////////////////
	//General
	void OnSetupAfterLoad(cWorld *apWorld);
	void OnAfterWorldLoad();
	void UpdateEnemySpecific(float afTimeStep);

	//////////////////////
	//Actions
	bool StateEventImplement(int alState, eLuxEnemyStateEvent aEvent, cLuxStateMessage *apMessage);


	//////////////////////
	//Debug
	void OnRenderSolidImplemented(cRendererCallbackFunctions* apFunctions);
		
	
	//////////////////////
	//Save data stuff
	iLuxEntity_SaveData* CreateSaveData();
	void SaveToSaveData(iLuxEntity_SaveData* apSaveData);
	void LoadFromSaveData(iLuxEntity_SaveData* apSaveData);
	void SetupSaveData(iLuxEntity_SaveData *apSaveData);

protected:

    virtual bool SeesPlayer() { return CanSeePlayer(); }    // manpig sees when it sees. used for tesla, which sees when it sees movement.
    virtual bool PlayerIsDetected();

private:
	//////////////////////
	// Callbacks

	void OnDisableTriggers();

	float GetDamageMul(float afAmount, int alStrength);

	void AILog( const tString & logMessage );

	//////////////////////
	// State stuff
	bool CheckEnemyAutoRemoval(float afDistance);
	void PatrolUpdateGoal();
	void PatrolEndOfPath();
	void FinishPatrolEndOfPath(bool callPatrolUpdateNow);
	bool FleeTryToFindSafeNode();
	bool StalkFindNode();
	bool TrackFindNode();
	bool TrackTeleportBehindPlayer();

	
	//////////////
	//Data
	tString msNoticeSound;
	tString msGiveUpNoticeSound;
	tString msEnabledSound; 

	float mfGroggyDamageCount;
	float mfAlertToHuntDistance;
	float mfAlertToInstantHuntDistance;
	float mfHuntPauseMinTime;
	float mfHuntPauseMaxTime;
	float mfIncreaseAlertSpeedDistance;
	float mfIncreasedAlertSpeedMul;
	float mfAlertRunTowardsToHuntLimit;
	float mfAlertRunTowardsCheckDistance;
	
	//////////////
	//Variables
	float mfWaitTime;
	float mfAlertRunTowardsCount;

	bool mbThreatenOnAlert;
	bool mbFleeFromPlayer;
	
	eLuxIdleBehavior mIdleBehavior;

	eLuxEnemyState mfPostAnimState;

	bool mbPathReversed;
	bool mbAllowZeroNodeWaitTimes;
    eLuxEnemyMoveSpeed mPatrolMoveSpeed;
};

//----------------------------------------------

class cLuxEnemyLoader_ManPig : public iLuxEnemyLoader
{
public:
	cLuxEnemyLoader_ManPig(const tString& asName);
	virtual ~cLuxEnemyLoader_ManPig(){}

	iLuxEnemy *CreateEnemy(const tString& asName, int alID, cLuxMap *apMap);
	void LoadVariables(iLuxEnemy *apEnemy, cXmlElement *apRootElem);
	void LoadInstanceVariables(iLuxEnemy *apEnemy, cResourceVarsObject *apInstanceVars);
};

//----------------------------------------------


#endif // LUX_ENEMY_MAN_PIG_H
