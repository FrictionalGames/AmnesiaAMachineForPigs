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

#ifndef MODEL_EDITOR_EDIT_MODE_TOOLBOX_H
#define MODEL_EDITOR_EDIT_MODE_TOOLBOX_H

#include "../common/StdAfx.h"
using namespace hpl;

#include "../common/EditorWindow.h"

//----------------------------------------------------------------

class cModelEditor;

//----------------------------------------------------------------

class cModelEditorEditModeToolbox : public iEditorWindow
{
public:
	cModelEditorEditModeToolbox(cModelEditor* apEditor);
	virtual ~cModelEditorEditModeToolbox();

	void OnInitLayout();

	

    
protected:
	///////////////////////////
	// Own functions
	bool Button_Pressed(iWidget* apWidget, const cGuiMessageData& aData);
	kGuiCallbackDeclarationEnd(Button_Pressed);

	///////////////////////////
	// Data
	cModelEditor* mpEditor;

	cWidgetButton* mvButtons[8];
	cGuiGfxElement* mvGfxButtons[8];
};

//----------------------------------------------------------------

#endif // MODEL_EDITOR_EDIT_MODE_TOOLBOX_H

