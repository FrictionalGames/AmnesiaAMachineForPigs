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

#ifndef HPLEDITOR_EDITOR_EDIT_MODE_AREAS_H
#define HPLEDITOR_EDITOR_EDIT_MODE_AREAS_H

#include "../common/StdAfx.h"

using namespace hpl;

#include "EditorEditMode.h"

#include "EditorTypes.h"

//------------------------------------------------------------

class iEditorBase;
class iEditorWorld;

class cBoxCreator;

//------------------------------------------------------------


//------------------------------------------------------------

class cEditorEditModeAreas : public iEditorEditModeObjectCreator
{
public:
	cEditorEditModeAreas(iEditorBase* apEditor,
						 iEditorWorld* apEditorWorld);
	~cEditorEditModeAreas();

	void SetAreaType(const tString& asAreaType) { msAreaType = asAreaType; }

	void OnViewportMouseDown(int alButtons);
	void OnViewportMouseUp(int alButtons);

	void OnEditorUpdate(float afTimeStep);

	void DrawPostGrid(cEditorWindowViewport* apViewport, cRendererCallbackFunctions* apFunctions, const cVector3f& avPos);

protected:
	bool SetUpCreationData(iEntityWrapperData* apData);
	void CreateTypes();

	void OnSetCurrent(bool abX);

	iEditorWindow* CreateSpecificWindow();

	tString msAreaType;

	cBoxCreator* mpBoxCreator;
};

//------------------------------------------------------------

#endif // HPLEDITOR_EDITOR_EDIT_MODE_AREAS_H

