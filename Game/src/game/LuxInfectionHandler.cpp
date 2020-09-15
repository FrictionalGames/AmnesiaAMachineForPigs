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

#include "LuxInfectionHandler.h"
#include "LuxMapHandler.h"

//////////////////////////////////////////////////////////////////////////
// INFECTION HANDLER
//////////////////////////////////////////////////////////////////////////

cLuxInfectionHandler::cLuxInfectionHandler() : iLuxUpdateable("LuxInfectionHandler")
{
	//CreateCellGuiSet();
	//LoadCellTextures();
}

//-----------------------------------------------------------------------

cLuxInfectionHandler::~cLuxInfectionHandler()
{
}

//-----------------------------------------------------------------------

void cLuxInfectionHandler::CreateCellGuiSet()
{
	cGui * pGui = gpBase->mpEngine->GetGui();
	mpCellGuiSet = pGui->CreateSet( "Cells", NULL );
	mpCellGuiSet->SetRendersBeforePostEffects( true );
	mpCellGuiSet->SetDrawMouse( false );
	gpBase->mpMapHandler->GetViewport()->AddGuiSet( mpCellGuiSet );
}

//-----------------------------------------------------------------------

void cLuxInfectionHandler::LoadCellTextures()
{
	mvCellGfx.push_back(gpBase->mpEngine->GetGui()->CreateGfxTexture("cell1.tga",eGuiMaterial_Modulative));
	mvCellGfx.push_back(gpBase->mpEngine->GetGui()->CreateGfxTexture("cell2.tga",eGuiMaterial_Modulative));	
}

//-----------------------------------------------------------------------

void cLuxInfectionHandler::OnDraw(float afFrameTime)
{
	//mpCellGuiSet->DrawGfx(mvCellGfx[0], cVector3f(205, 150,0));
	//mpCellGuiSet->DrawGfx(mvCellGfx[1], cVector3f(205, 200,0));
}