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

#ifndef HPLEDITOR_EDITOR_LEVEL_SETTINGS_H
#define HPLEDITOR_EDITOR_LEVEL_SETTINGS_H

#include "../common/EditorWindow.h"

class cLevelEditorWindowLevelSettings : public iEditorWindowPopUp
{
public:
	cLevelEditorWindowLevelSettings(iEditorBase* apEditor);
	~cLevelEditorWindowLevelSettings();

protected:
	void OnUpdate(float afTimeStep);
	void OnInitLayout();
	void OnSetActive(bool abX);

	bool WindowSpecificInputCallback(iEditorInput* apInput);

	bool ResetDecals(iWidget* apWidget, const cGuiMessageData& aData);
	kGuiCallbackDeclarationEnd(ResetDecals);

	////////////////////////////////////
	// Data

	////////////////////////
	// Layout stuff


	cEditorInputBool* mpInpSkyboxActive;
	cEditorInputFile* mpInpSkyboxTexture;
	cEditorInputColorFrame* mpInpSkyboxColor;

	cEditorInputBool* mpInpFogActive;
	cEditorInputBool* mpInpFogCulling;
	cEditorInputNumber* mpInpFogStart;
	cEditorInputNumber* mpInpFogEnd;
	cEditorInputNumber* mpInpFogFallOffExp;
	cEditorInputColorFrame* mpInpFogColor;

	cEditorInputNumber* mpInpGlobalMaxDecalTris;
	cWidgetButton*	mpBResetDecals;
};

#endif //HPLEDITOR_EDITOR_WORLD_SETTINGS_H 
