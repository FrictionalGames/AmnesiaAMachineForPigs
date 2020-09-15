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

#ifndef HPLEDITOR_EDITOR_WINDOW_JOINTS_H
#define HPLEDITOR_EDITOR_WINDOW_JOINTS_H

#include "EditorWindow.h"

//----------------------------------------------------------

class cEditorEditModeJoints;

//----------------------------------------------------------

class cEditorWindowJoints : public iEditModeObjectCreatorWindow
{
public:
	cEditorWindowJoints(cEditorEditModeJoints* apEditMode);
	~cEditorWindowJoints();

	void Reset();

protected:
	void OnInit(){}
	void OnInitLayout();

	void OnLoadLevel(){}
	void OnUpdate(float afTimeStep){}
	void OnSetActive(bool abX);

 	void ResetInputs();

	bool WindowSpecificInputCallback(iEditorInput* apInput);


	////////////////////////////////////////////////////
	// Data
	// Layout stuff
	cWidgetComboBox *mpJointSelection;
	cEditorInputEnum* mpInpJointType;
};

//----------------------------------------------------------

#endif // HPLEDITOR_EDITOR_WINDOW_BODIES_H
