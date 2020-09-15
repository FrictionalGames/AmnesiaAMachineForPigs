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

#ifndef HPLEDITOR_EDITOR_EDIT_MODE_BODIES_H
#define HPLEDITOR_EDITOR_EDIT_MODE_BODIES_H

#include "../common/StdAfx.h"

using namespace hpl;

#include "EditorEditMode.h"

//------------------------------------------------------------

class iEditorBase;

class cEditorWindowViewport;

class cEditorWindowBodies;
class iEditorWorld;

//------------------------------------------------------------

class cEditorEditModeBodies : public iEditorEditModeObjectCreator
{
public:
	cEditorEditModeBodies(iEditorBase* apEditor,
						  iEditorWorld* apEditorWorld);

	void DrawObjectPreview(cEditorWindowViewport* apViewport, cRendererCallbackFunctions *apFunctions, const cVector3f& avPos, bool abPreCreationActive);
protected:
	iEditorWindow* CreateSpecificWindow();

	bool SetUpCreationData(iEntityWrapperData* apData);
	void CreateTypes();
};

//------------------------------------------------------------

#endif // HPLEDITOR_EDITOR_EDIT_MODE_BODIES_H

