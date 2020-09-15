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

#include "EditorWindowEntityEditBoxBone.h"

#include "EntityWrapperBone.h"

#include "EditorEditModeSelect.h"

#include "EditorActionEntity.h"
#include "EditorHelper.h"

#include "EditorInput.h"

#include <algorithm>

//----------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------

cEditorWindowEntityEditBoxBone::cEditorWindowEntityEditBoxBone(cEditorEditModeSelect* apEditMode, cEntityWrapperBone* apObject) : cEditorWindowEntityEditBox(apEditMode,apObject)
{
	mpEntity = apObject;
}

//----------------------------------------------------------------------------

cEditorWindowEntityEditBoxBone::~cEditorWindowEntityEditBoxBone()
{
}

//----------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------

void cEditorWindowEntityEditBoxBone::Create()
{
	mpTabGeneral = mpTabs->AddTab(_W("General"));
	mpTabAttachments = mpTabs->AddTab(_W("Attachments"));

	////////////////////////////////////////
	// Properties in Tab 'General'
	AddPropertyName(mpTabGeneral);
	mpInpName->SetPosition(cVector3f(5,5,0.1f));

    ////////////////////////////////////////
	// Attachment stuff
	AddPropertySetAttachments(mpTabAttachments);
}

//----------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////
// PROTECTED METHODS
//////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

void cEditorWindowEntityEditBoxBone::OnUpdate(float afTimeStep)
{
	cEditorWindowEntityEditBox::OnUpdate(afTimeStep);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
