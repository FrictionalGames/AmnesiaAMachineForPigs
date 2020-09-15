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

#include "EditorWindowAreas.h"
#include "EditorEditModeAreas.h"

#include "EntityWrapperArea.h"


//---------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------

cEditorWindowAreas::cEditorWindowAreas(cEditorEditModeAreas* apEditMode) : iEditModeObjectCreatorWindow(apEditMode)
{
}

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////
// PROTECTED METHODS
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------

bool cEditorWindowAreas::AreaType_OnChange(iWidget* apWidget, const cGuiMessageData& aData)
{
	cEditorEditModeAreas* pEditMode = (cEditorEditModeAreas*)mpEditMode;
	pEditMode->SetSubType(mpComboBoxAreaType->GetSelectedItem());
	return true;
}
kGuiCallbackDeclaredFuncEnd(cEditorWindowAreas, AreaType_OnChange);

//---------------------------------------------------------------------------



//---------------------------------------------------------------------------

void cEditorWindowAreas::OnInitLayout()
{
	mpBGFrame->SetSize(cVector2f(200,70));
	mpBGFrame->SetClipActive(false);

	mpGroupTypeSelector = mpSet->CreateWidgetGroup(cVector3f(5,8,0.1f), cVector2f(190,50), _W("Area Type"), mpBGFrame);
	AddWidget(mpGroupTypeSelector);

    mpComboBoxAreaType = mpSet->CreateWidgetComboBox(cVector3f(10,10,0.2f), cVector2f(170,25),_W(""), mpGroupTypeSelector);
	mpComboBoxAreaType->AddCallback(eGuiMessage_SelectionChange,this,kGuiCallback(AreaType_OnChange));
	AddWidget(mpComboBoxAreaType);

	cEditorEditModeAreas* pEditMode = (cEditorEditModeAreas*)mpEditMode;
	for(int i=0;i<pEditMode->GetTypeNum();++i)
	{
		cEntityWrapperTypeArea* pAreaType = (cEntityWrapperTypeArea*)pEditMode->GetType(i);
		mpComboBoxAreaType->AddItem(pAreaType->GetAreaType());
	}
	if(mpComboBoxAreaType->GetItemNum()>0)
		mpComboBoxAreaType->SetSelectedItem(0);
}

//---------------------------------------------------------------------------
