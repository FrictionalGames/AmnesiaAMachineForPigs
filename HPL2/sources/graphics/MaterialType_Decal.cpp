/*
 * Copyright © 2009-2020 Frictional Games
 *
 * This file is part of Amnesia: The Dark Descent.
 *
 * Amnesia: The Dark Descent is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Amnesia: The Dark Descent is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Amnesia: The Dark Descent.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "graphics/MaterialType_Decal.h"

#include "graphics/Image.h"
#include "math/MathTypes.h"
#include "system/LowLevelSystem.h"
#include "system/PreprocessParser.h"

#include "resources/Resources.h"

#include "math/cFrustum.h"
#include "math/Math.h"

#include "graphics/Graphics.h"
#include "graphics/LowLevelGraphics.h"
#include "graphics/Material.h"
#include "graphics/Renderable.h"
#include "graphics/Renderer.h"
#include "system/SystemTypes.h"

namespace hpl {

    cMaterialType_Decal::cMaterialType_Decal(cGraphics* apGraphics, cResources* apResources)
        : iMaterialType(apGraphics, apResources) {
        mbIsTranslucent = true;
        mbIsDecal = true;

        mbHasTypeSpecifics[eMaterialRenderMode_Diffuse] = true;

        AddUsedTexture(eMaterialTexture_Diffuse);
    }

    cMaterialType_Decal::~cMaterialType_Decal() {
    }

    void cMaterialType_Decal::LoadData() {
    }

    void cMaterialType_Decal::DestroyData() {
    }


    iMaterialVars* cMaterialType_Decal::CreateSpecificVariables() {
        return nullptr;
    }

    void cMaterialType_Decal::LoadVariables(cMaterial* apMaterial, cResourceVarsObject* apVars) {
    }

    void cMaterialType_Decal::GetVariableValues(cMaterial* apMaterial, cResourceVarsObject* apVars) {
    }

    void cMaterialType_Decal::CompileMaterialSpecifics(cMaterial* apMaterial) {
        //cMaterialType_Decal_Vars* pVars = static_cast<cMaterialType_Decal_Vars*>(apMaterial->GetVars());

 //       if (apMaterial->HasUvAnimation()) {
////            apMaterial->SetHasSpecificSettings(eMaterialRenderMode_Diffuse, true);
 //       }
    }

} // namespace hpl
