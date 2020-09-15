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

#ifndef LUX_SAVED_MAP_H
#define LUX_SAVED_MAP_H

//----------------------------------------------

#include "LuxBase.h"
#include "LuxSavedEngineTypes.h"

//----------------------------------------------

class iLuxEntity_SaveData;
class cLuxMap;

//----------------------------------------------

class cLuxSavedMap : public iSerializable
{
	kSerializableClassInit(cLuxSavedMap)
public:	
	cLuxSavedMap();
	~cLuxSavedMap();

	void FromMap(cLuxMap *apMap);
	void ToMap(cLuxMap *apMap);

	tString msFileName;

	tString msDisplayNameEntry;

	tString msLanternLitCallback;

	int mlNumberOfQuests;
	int mlTotalCompletionAmount;
	int mlCurrentCompletionAmount;

	bool mbSkyBoxActive;
	tString msSkyboxTexture;
	cColor mSkyBoxColor;

	bool mbFogActive;
	bool mbFogCulling;
	float mfFogStart;
	float mfFogEnd;
	float mfFogFalloffExp;
	cColor mFogColor;

	tString msCheckPointName;
	tString msCheckPointStartPos;
	tString msCheckPointCallback;
	int mlCheckPointCount;
	tString msCheckPointMusic;
	int mlCheckPointMusicPrio;
	bool mbCheckPointMusicResume;
	float mfCheckPointMusicVolume;
	
	cContainerList<cEngineRope_SaveData> mlstRopes;
	cContainerList<cEngineLight_SaveData> mlstLights;
	cContainerList<cEngineSound_SaveData> mlstSounds;
	cContainerList<cEnginePS_SaveData> mlstPS;

	cContainerList<iLuxEntity_SaveData*> mlstEntities;

	cContainerList<cLuxEventTimer> mlstTimers;

	cContainerList<cLuxScriptVar> mlstVars;

	cContainerVec<cLuxUseItemCallback> mvUseItemCallbacks;

private:
};

//----------------------------------------------


#endif // LUX_SAVED_MAP_H
