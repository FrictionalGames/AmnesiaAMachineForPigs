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

#ifndef LUX_HELP_FUNCS_H
#define LUX_HELP_FUNCS_H

//----------------------------------------------

#include "LuxBase.h"

//----------------------------------------------

class cLuxSoundExtraData
{
public:
	float mfMinDistance;
	float mfMaxDistance;
	float mfVolume;
	float mfAIVolume;
	cSoundEntry *mpSoundEntry;
};

//-------------------------------------

class cLuxModelCache
{
public:
	cLuxModelCache();
	~cLuxModelCache();

	void Create();
	void Destroy();

private:
	std::list<cMesh*> mlstMeshCache;
	std::list<cAnimation*> mlstAnimationCache;
	std::list<iTexture*> mlstTextureCache;
};

//-------------------------------------

class cLuxHelpFuncs : public iLuxUpdateable
{
public:	
	cLuxHelpFuncs();
	~cLuxHelpFuncs();

	void LoadFonts();

		
	bool PlayGuiSoundData(	const tString& asName, eSoundEntryType aDestType, float afVolMul=1.0f,
							eSoundEntityType aSoundType= eSoundEntityType_Main, bool abSkipPreviousRandom=true,
							cLuxSoundExtraData *apOutputData= NULL);

	void DrawSetToScreen(bool abClearScreen=true, const cColor& aCol = cColor(0,0), cGuiSet* apSet=NULL);
	
	void CleanupData();

	tWString ParseString(const tWString& asInput);

	float GetStringDuration(const tWString& asStr);

	/**
	 * To be used in journal and inventory, renders the screen without icons and such.
	 */
	void RenderBackgroundScreen(bool abDrawFullHUD);


	cGuiSet* GetSet(){ return mpSet;}
private:
	tWString ParseStringCommand(const tWString& asCommand);

	cGuiSet *mpSet;
	iLowLevelGraphics *mpLowLevelGfx;

	iFontData *mpFontDefault;

	float mfTextDuration_StartTime;
	float mfTextDuration_MinTime;
	float mfTextDuration_CharTime;
};

//----------------------------------------------


#endif // LUX_HELP_FUNCS_H
