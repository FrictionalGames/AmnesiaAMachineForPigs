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

#include "EditorWindowLoaderStatus.h"

//--------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////

//--------------------------------------------------------------------------

cEditorWindowLoaderStatus::cEditorWindowLoaderStatus(iEditorBase* apEditor) : iEditorWindowPopUp(apEditor, "Loader Status Window")
{
}

//--------------------------------------------------------------------------

cEditorWindowLoaderStatus::~cEditorWindowLoaderStatus()
{
}

//--------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////

//--------------------------------------------------------------------------

void cEditorWindowLoaderStatus::SetCaption(const tWString& asCaption)
{
	if(mpWindow) mpWindow->SetText(asCaption);
}

//--------------------------------------------------------------------------

void cEditorWindowLoaderStatus::SetStatusString(const tWString& asStatus)
{
	if(mpLabelCurrentStatus) mpLabelCurrentStatus->SetText(asStatus);
}

void cEditorWindowLoaderStatus::Draw()
{
/*	SetEnabled(true);
	SetVisible(true);

	cGuiClipRegion clipRegion;
	iLowLevelGraphics* pGfx = mpEditor->GetEngine()->GetGraphics()->GetLowLevel();

	iFrameBuffer* pFB = pGfx->GetCurrentFrameBuffer();
	pGfx->SetCurrentFrameBuffer(NULL);
	pGfx->ClearFrameBuffer(eClearFrameBufferFlag_Color);

	mpEditor->GetEngine()->GetScene()->Render(0,0);
	mpSet->DrawAll(0);
	mpSet->Render(NULL);

	pGfx->WaitAndFinishRendering();
	pGfx->SwapBuffers();

	SetEnabled(false);
	SetVisible(false);

	pGfx->SetCurrentFrameBuffer(pFB);*/
}

//--------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////
// PROTECTED METHODS
//////////////////////////////////////////////////////////////////

//--------------------------------------------------------------------------

void cEditorWindowLoaderStatus::OnInitLayout()
{
	iEditorWindowPopUp::OnInitLayout();

	mpWindow->SetSize(cVector2f(320,200));

	cVector3f vPos = (mpSet->GetVirtualSize() - mpWindow->GetSize())*0.5f;
	vPos.z = 100;
	mpWindow->SetPosition(vPos);

	mpLabelStatusHeader = mpSet->CreateWidgetLabel(cVector3f(20,40,0.1f),0, _W("Current Status:"), mpWindow);
	AddWidget(mpLabelStatusHeader);

	mpLabelCurrentStatus = mpSet->CreateWidgetLabel(cVector3f(120,40,0.1f), 0, _W(""), mpWindow);
	AddWidget(mpLabelCurrentStatus);
}

//--------------------------------------------------------------------------
