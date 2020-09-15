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

#include "EditorEditModeSounds.h"

#include "EditorBaseClasses.h"
#include "EditorActionHandler.h"
#include "EditorWorld.h"

#include "EntityWrapperSound.h"

#include "EditorWindowViewport.h"
#include "EditorWindowSounds.h"

//-----------------------------------------------------------------

///////////////////////////////////////////////////////////////////
// CONSTRUCTORS
///////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------

cEditorEditModeSounds::cEditorEditModeSounds(iEditorBase* apEditor,
										   iEditorWorld* apEditorWorld) : iEditorEditModeObjectCreator(apEditor, "Sounds", apEditorWorld)
{
}

//-----------------------------------------------------------------

//-----------------------------------------------------------------

///////////////////////////////////////////////////////////////////
// PUBLIC METHODS
///////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------

bool cEditorEditModeSounds::SetUpCreationData(iEntityWrapperData* apData)
{
	if(iEditorEditModeObjectCreator::SetUpCreationData(apData)==false)
		return false;

	cEditorWindowSounds* pWin = (cEditorWindowSounds*)GetEditorWindow();
	apData->SetString(eSoundStr_Filename, cString::To8Char(pWin->GetSoundFile()));

	return true;
}

//-----------------------------------------------------------------

///////////////////////////////////////////////////////////////////
// PROTECTED METHODS
///////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------

void cEditorEditModeSounds::CreateTypes()
{
	mvTypes.push_back(hplNew(cEntityWrapperTypeSound,()));
}

//-----------------------------------------------------------------

iEditorWindow* cEditorEditModeSounds::CreateSpecificWindow()
{
	return hplNew(cEditorWindowSounds,(this));
}


