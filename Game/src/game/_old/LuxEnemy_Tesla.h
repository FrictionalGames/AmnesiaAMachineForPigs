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

#ifndef LUX_ENEMY_TESLA_H
#define LUX_ENEMY_TESLA_H

//----------------------------------------------

#include "LuxEnemy_ManPig.h"
#include "LuxProp_Lamp.h"

//----------------------------------------------

class cLuxEnemy_Tesla_SaveData : public cLuxEnemy_ManPig_SaveData
{
	kSerializableClassInit(cLuxEnemy_Tesla_SaveData)
public:
	////////////////
	//Properties
		
};

//----------------------------------------------

class cLuxEnemy_Tesla : public cLuxEnemy_ManPig
{
friend class cLuxEnemyLoader_Tesla;
typedef cLuxEnemy_ManPig super_class;
public:	
	cLuxEnemy_Tesla(const tString &asName, int alID, cLuxMap *apMap);
	virtual ~cLuxEnemy_Tesla();

    bool CanSeeMovingPlayer();

	void UpdateEnemySpecific(float afTimeStep);
    void CreateSpotLight(const cVector3f& avPos=0, const cVector3f& avAngles=0,float afRadius =20, bool abShadows=true, float afFOV=kPi2f , const cColor &aColor=cColor(1,1,1,1));
    void DebugRenderMouthLight();
	void OnRenderSolidImplemented(cRendererCallbackFunctions* apFunctions);

    bool StateEventImplement(int alState, eLuxEnemyStateEvent aEvent, cLuxStateMessage *apMessage);

protected:

    virtual bool SeesPlayer() { return CanSeeMovingPlayer(); }    // manpig sees when it sees. used for tesla, which sees when it sees movement.
 
private:

	float mfPulseTimer;
    cLightSpot * mpMouthLight;
    cBoneState * mpMouthBone;

    cVector3f mTranslationOffset;
    cVector3f mAngleOffset;
    cColor mLightColor;

    // darkening

    bool ShouldUpdateLightList(float afTimeStep);
	void UpdateLightList(tLightList & alstAddedLights, tLightList & alstRemovedLights);
    void UpdateDarkening(float afTimeStep);
    void UpdateStrobeState(float afTimeStep);

    float mfLightUpdateTimer;
    float mfLightUpdateInterval;
    tLightList mlstInfluencedLights;
    tLightList mlstStrobeLights;
    std::list<cLuxProp_Lamp*> mlstStrobeLamps;

    bool mbInfluencedLightsOn;
    bool mbUseDarkeningEffect;

    float mfMinTimeBetweenStrobeFlashes;
    float mfMaxTimeBetweenStrobeFlashes;
    float mfChanceOfDoubleStrobeFlash;
    float mfChanceOfTripleStrobeFlash;
    float mfMinTimeBetweenRepeatedFlashes;
    float mfMaxTimeBetweenRepeatedFlashes;
    float mfMinStrobeFlashDuration;
    float mfMaxStrobeFlashDuration;
    int mnRepeatedFlashCount;
    float mfTimeToNextFlashEvent;
    float mfDarkeningRadius;
    float mfStrobeRadius;
};

//----------------------------------------------

class cLuxEnemyLoader_Tesla : public cLuxEnemyLoader_ManPig
{
public:
	cLuxEnemyLoader_Tesla(const tString& asName);
	virtual ~cLuxEnemyLoader_Tesla(){}

	iLuxEnemy *CreateEnemy(const tString& asName, int alID, cLuxMap *apMap);
	void LoadVariables(iLuxEnemy *apEnemy, cXmlElement *apRootElem);
	void LoadInstanceVariables(iLuxEnemy *apEnemy, cResourceVarsObject *apInstanceVars);
};

//----------------------------------------------


#endif // LUX_ENEMY_TESLA_H
