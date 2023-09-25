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

#include "graphics/MaterialType_BasicTranslucent.h"

#include "graphics/GraphicsTypes.h"
#include "graphics/Image.h"
#include "graphics/RendererDeferred.h"
#include "math/MathTypes.h"
#include "system/LowLevelSystem.h"
#include "system/PreprocessParser.h"
#include "system/String.h"

#include "resources/Resources.h"

#include "scene/Light.h"
#include "scene/World.h"

#include "math/cFrustum.h"
#include "math/Math.h"

#include "graphics/Graphics.h"
#include "graphics/LowLevelGraphics.h"
#include "graphics/Material.h"
#include "graphics/RenderList.h"
#include "graphics/Renderable.h"
#include "graphics/Renderer.h"
#include "system/SystemTypes.h"
#include <cstdint>
#include <iterator>

namespace hpl
{
    static inline float GetMaxColorValue(const cColor& aCol)
    {
        return cMath::Max(cMath::Max(aCol.r, aCol.g), aCol.b);
    }

    cMaterialType_Translucent::cMaterialType_Translucent(cGraphics* apGraphics, cResources* apResources)
        : iMaterialType(apGraphics, apResources)
    {
        mbIsTranslucent = true;

        AddUsedTexture(eMaterialTexture_Diffuse);
        AddUsedTexture(eMaterialTexture_NMap);
        AddUsedTexture(eMaterialTexture_CubeMap);
        AddUsedTexture(eMaterialTexture_CubeMapAlpha);

        AddVarBool("Refraction", false, "If the material has refraction (distortion of bg). Uses NMap and/or normals of mesh");
        AddVarBool("RefractionEdgeCheck", true, "If true, there is no bleeding with foreground objects, but takes some extra power.");
        AddVarBool("RefractionNormals", false, "If normals should be used when refracting. If no NMap is set this is forced true!");
        AddVarBool("RefractionNormals", false, "If normals should be used when refracting. If no NMap is set this is forced true!");
        AddVarFloat("RefractionScale", 0.1f, "The amount refraction offsets the background");
        AddVarFloat(
            "FrenselBias",
            0.2f,
            "Bias for Fresnel term. values: 0-1. Higher means that more of reflection is seen when looking straight at the surface.");
        AddVarFloat(
            "FrenselPow", 8.0f, "The higher the 'sharper' the reflection is, meaning that it is only clearly seen at sharp angles.");
        AddVarFloat(
            "RimLightMul",
            0.0f,
            "The amount of rim light based on the reflection. This gives an edge to the object. Values: 0 - inf (although 1.0f should be "
            "used for max)");
        AddVarFloat("RimLightPow", 8.0f, "The sharpness of the rim lighting.");
        AddVarBool("AffectedByLightLevel", false, "The the material alpha is affected by the light level.");

        mbHasTypeSpecifics[eMaterialRenderMode_Diffuse] = true;
        mbHasTypeSpecifics[eMaterialRenderMode_DiffuseFog] = true;
        mbHasTypeSpecifics[eMaterialRenderMode_Illumination] = true;
        mbHasTypeSpecifics[eMaterialRenderMode_IlluminationFog] = true;

    }

    cMaterialType_Translucent::~cMaterialType_Translucent()
    {
    }

    void cMaterialType_Translucent::LoadData()
    {
    }

    void cMaterialType_Translucent::DestroyData()
    {
    }

    iMaterialVars* cMaterialType_Translucent::CreateSpecificVariables()
    {
        return nullptr;
    }

    //--------------------------------------------------------------------------

    void cMaterialType_Translucent::LoadVariables(cMaterial* apMaterial, cResourceVarsObject* apVars)
    {
       // cMaterialType_Translucent_Vars* pVars = (cMaterialType_Translucent_Vars*)apMaterial->GetVars();
       // if (pVars == NULL)
       // {
       //     pVars = static_cast<cMaterialType_Translucent_Vars*>(CreateSpecificVariables());
       //     apMaterial->SetVars(pVars);
       // }

       // pVars->mbRefraction = apVars->GetVarBool("Refraction", false);
       // pVars->mbRefractionEdgeCheck = apVars->GetVarBool("RefractionEdgeCheck", true);
       // pVars->mbRefractionNormals = apVars->GetVarBool("RefractionNormals", true);
       // pVars->mfRefractionScale = apVars->GetVarFloat("RefractionScale", 1.0f);
       // pVars->mfFrenselBias = apVars->GetVarFloat("FrenselBias", 0.2f);
       // pVars->mfFrenselPow = apVars->GetVarFloat("FrenselPow", 8.0);
       // pVars->mfRimLightMul = apVars->GetVarFloat("RimLightMul", 0.0f);
       // pVars->mfRimLightPow = apVars->GetVarFloat("RimLightPow", 8.0f);
       // pVars->mbAffectedByLightLevel = apVars->GetVarBool("AffectedByLightLevel", false);
    }

    //--------------------------------------------------------------------------

    void cMaterialType_Translucent::GetVariableValues(cMaterial* apMaterial, cResourceVarsObject* apVars)
    {
      //  cMaterialType_Translucent_Vars* pVars = (cMaterialType_Translucent_Vars*)apMaterial->GetVars();

      //  apVars->AddVarBool("Refraction", pVars->mbRefraction);
      //  apVars->AddVarBool("RefractionEdgeCheck", pVars->mbRefractionEdgeCheck);
      //  apVars->AddVarBool("RefractionNormals", pVars->mbRefractionNormals);
      //  apVars->AddVarFloat("RefractionScale", pVars->mfRefractionScale);
      //  apVars->AddVarFloat("FrenselBias", pVars->mfFrenselBias);
      //  apVars->AddVarFloat("FrenselPow", pVars->mfFrenselPow);
      //  apVars->AddVarFloat("RimLightMul", pVars->mfRimLightMul);
      //  apVars->AddVarFloat("RimLightPow", pVars->mfRimLightPow);
      //  apVars->AddVarBool("AffectedByLightLevel", pVars->mbAffectedByLightLevel);
    }

    //--------------------------------------------------------------------------

    void cMaterialType_Translucent::CompileMaterialSpecifics(cMaterial* apMaterial)
    {
      //  cMaterialType_Translucent_Vars* pVars = static_cast<cMaterialType_Translucent_Vars*>(apMaterial->GetVars());

        /////////////////////////////////////
        // Set up specifics
       // apMaterial->SetHasSpecificSettings(eMaterialRenderMode_Diffuse, true);
       // apMaterial->SetHasObjectSpecificsSettings(eMaterialRenderMode_Diffuse, true);

       // apMaterial->SetHasSpecificSettings(eMaterialRenderMode_DiffuseFog, true);
       // apMaterial->SetHasObjectSpecificsSettings(eMaterialRenderMode_DiffuseFog, true);

       // apMaterial->SetHasSpecificSettings(eMaterialRenderMode_Illumination, true);
       // apMaterial->SetHasObjectSpecificsSettings(eMaterialRenderMode_Illumination, true);

       // apMaterial->SetHasSpecificSettings(eMaterialRenderMode_IlluminationFog, true);
       // apMaterial->SetHasObjectSpecificsSettings(eMaterialRenderMode_IlluminationFog, true);

        /////////////////////////////////////
        // Set up the refraction
        // apMaterial->SetHasRefraction(pVars->mbRefraction);
        // apMaterial->SetUseRefractionEdgeCheck(pVars->mbRefractionEdgeCheck);

		 //bool bRefractionEnabled = pVars->mbRefraction && iRenderer::GetRefractionEnabled();
         //if (bRefractionEnabled)
         //{
         //
         //    // Note: No need to set blend mode to None since rendered sets that when refraction is true!
         //    //		Also, this gives problems when recompiling, since the material data would not be accurate!
         //}
         //
         ///////////////////////////////////////
         //// Set up the reflections
         //if (apMaterial->GetImage(eMaterialTexture_CubeMap))
         //{
         //   if (bRefractionEnabled == false) {
         //       apMaterial->SetHasTranslucentIllumination(true);
         //   }
         //}
    }

    //--------------------------------------------------------------------------

} // namespace hpl
