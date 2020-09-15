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

#ifndef HPLEDITOR_EDITOR_WINDOW_ENTITY_EDIT_BOX_STATIC_OBJECT_H
#define HPLEDITOR_EDITOR_WINDOW_ENTITY_EDIT_BOX_STATIC_OBJECT_H

#include "../common/StdAfx.h"
using namespace hpl;

#include "EditorWindowEntityEditBox.h"

//----------------------------------------------------------------------

class iEntityWrapper;

class cEditorEditModeSelect;

//----------------------------------------------------------------------

class cEditorWindowEntityEditBoxStaticObject : public cEditorWindowEntityEditBox
{
public:
	cEditorWindowEntityEditBoxStaticObject(cEditorEditModeSelect* apEditMode,cEntityWrapperStaticObject* apObject);
	virtual ~cEditorWindowEntityEditBoxStaticObject();

	void Create();
protected:
	void AddPropertySetStaticObject(cWidgetTab* apParentTab);

	bool WindowSpecificInputCallback(iEditorInput* apInput);

	void OnUpdate(float afTimeStep);

	cWidgetTab* mpTabGeneral;
	cWidgetTab* mpTabStaticObject;

	cEntityWrapperStaticObject* mpEntity;

	cEditorInputFile* mpInpMeshFile;
	cEditorInputBool* mpInpCastShadows;
	cEditorInputBool* mpInpCollides;
	cEditorInputBool* mpInpIsOccluder;
};

//----------------------------------------------------------------------


#endif // HPLEDITOR_EDITOR_WINDOW_ENTITY_EDIT_BOX_STATIC_OBJECT_H
