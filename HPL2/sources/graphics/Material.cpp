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

#include "graphics/Material.h"

#include "graphics/GraphicsTypes.h"
#include "graphics/Image.h"
#include "graphics/MaterialResource.h"
#include "system/LowLevelSystem.h"
#include "system/String.h"

#include "resources/ImageManager.h"
#include "resources/Resources.h"
#include "resources/TextureManager.h"

#include "graphics/MaterialType.h"
#include "graphics/Renderable.h"

#include "math/Math.h"
#include <utility>

#include "tinyimageformat_query.h"

namespace hpl {

    cMaterial::cMaterial(
        const tString& asName, const tWString& asFullPath, cResources* apResources)
        : iResourceBase(asName, asFullPath, 0),
          mpResources(apResources) {
        m_generation = rand();
        m_descriptor.m_id = MaterialID::Unknown;
    }

    cMaterial::~cMaterial() {
    }

    void cMaterial::SetTextureAnisotropy(float afx) {
        if (afx >= 16.0f) {
            m_antistropy = Antistropy_16;
        } else if (afx >= 8.0f) {
            m_antistropy = Antistropy_8;
        } else {
            m_antistropy = Antistropy_None;
        }
        IncreaseGeneration();
    }

    eMaterialBlendMode cMaterial::GetBlendMode() const {
        // for water we enforce a blend mode
        switch (m_descriptor.m_id) {
        case MaterialID::Translucent:
            return m_descriptor.m_translucent.m_blend;
        case MaterialID::Decal:
            return m_descriptor.m_decal.m_blend;
        default:
            break;
        }
        ASSERT(false && "material type does not have a blend mode");
        return eMaterialBlendMode_LastEnum;
    }
    eMaterialAlphaMode cMaterial::GetAlphaMode() const {
        switch (m_descriptor.m_id) {
        case MaterialID::SolidDiffuse:
            if (GetImage(eMaterialTexture_Alpha)) {
                return eMaterialAlphaMode_Trans;
            }
            break;
        default:
            break;
        }
        return eMaterialAlphaMode_Solid;
    }
    bool cMaterial::IsAffectedByLightLevel() const {
        switch (m_descriptor.m_id) {
        case MaterialID::Translucent:
            return m_descriptor.m_translucent.m_isAffectedByLightLevel;
        default:
            break;
        }

        return false;
    }
    bool cMaterial::HasRefraction() const {
        switch (m_descriptor.m_id) {
        case MaterialID::Translucent:
            return m_descriptor.m_translucent.m_hasRefraction;
        case MaterialID::Water:
            return true;
        default:
            break;
        }
        return false;
    }
    bool cMaterial::HasReflection() const {
        switch (m_descriptor.m_id) {
        case MaterialID::Water:
            return m_descriptor.m_water.m_hasReflection;
        default:
            break;
        }
        return false;
    }
    bool cMaterial::HasWorldReflection() const {
        return m_descriptor.m_id == MaterialID::Water &&
            HasReflection() &&
            !GetImage(eMaterialTexture_CubeMap);
    }
    bool cMaterial::GetLargeTransperantSurface() const {
        switch (m_descriptor.m_id) {
        case MaterialID::Water:
            return m_descriptor.m_water.m_isLargeSurface;
        default:
            break;
        }
        return false;
    }
    float cMaterial::GetMaxReflectionDistance() const {
        switch (m_descriptor.m_id) {
        case MaterialID::Water:
            return m_descriptor.m_water.m_reflectionFadeEnd;
        default:
            break;
        }
        return 0.0f;
    }

    void cMaterial::SetImage(eMaterialTexture aType, iResourceBase* apTexture) {
        // increase version number to dirty material
        m_image[aType].SetAutoDestroyResource(false);
        if (apTexture) {
            ASSERT(
                TypeInfo<Image>().IsType(*apTexture) ||
                TypeInfo<AnimatedImage>().IsType(*apTexture) && "cMaterial::SetImage: apTexture is not an Image");
            m_image[aType] = std::move(ImageResourceWrapper(mpResources->GetTextureManager(), apTexture, mbAutoDestroyTextures));
        } else {
            m_image[aType] = ImageResourceWrapper();
        }
        IncreaseGeneration();
    }

    Image* cMaterial::GetImage(eMaterialTexture aType) {
        return m_image[aType].GetImage();
    }

    const Image* cMaterial::GetImage(eMaterialTexture aType) const {
        return m_image[aType].GetImage();
    }

    void cMaterial::SetAutoDestroyTextures(bool abX) {
        mbAutoDestroyTextures = abX;
        for (auto& image : m_image) {
            image.SetAutoDestroyResource(abX);
        }
    }

   // cResourceVarsObject* cMaterial::GetVarsObject() {
   //     cResourceVarsObject* pVarsObject = hplNew(cResourceVarsObject, ());
   //     mpType->GetVariableValues(this, pVarsObject);

   //     return pVarsObject;
   // }

   // void cMaterial::LoadVariablesFromVarsObject(cResourceVarsObject* apVarsObject) {
   //     mpType->LoadVariables(this, apVarsObject);
   // }

    void cMaterial::setTextureFilter(eTextureFilter filter) {
        m_textureFilter = filter;
        IncreaseGeneration();
    }

    void cMaterial::setTextureWrap(eTextureWrap wrap) {
        m_textureWrap = wrap;
        IncreaseGeneration();
    }

    void cMaterial::SetDepthTest(bool abDepthTest) {
        mbDepthTest = abDepthTest;
        IncreaseGeneration();
    }

    static cVector3f GetAxisVector(eMaterialAnimationAxis aAxis) {
        switch (aAxis) {
        case eMaterialAnimationAxis_X:
            return cVector3f(1, 0, 0);
        case eMaterialAnimationAxis_Y:
            return cVector3f(0, 1, 0);
        case eMaterialAnimationAxis_Z:
            return cVector3f(0, 0, 1);
        default:
            break;
        }
        return 0;
    }

    void cMaterial::UpdateBeforeRendering(float afTimeStep) {
        if(!mvUvAnimations.empty()) {
            m_mtxUV = cMatrixf::Identity;
            for (size_t i = 0; i < mvUvAnimations.size(); ++i) {
                cMaterialUvAnimation* pAnim = &mvUvAnimations[i];
                switch(pAnim->mType) {
                    case eMaterialUvAnimation_Translate: {
                        cVector3f vDir = GetAxisVector(pAnim->mAxis);
                        cMatrixf mtxAdd = cMath::MatrixTranslate(vDir * pAnim->mfSpeed * mfAnimTime);
                        m_mtxUV = cMath::MatrixMul(m_mtxUV, mtxAdd);
                        break;
                    }
                    case eMaterialUvAnimation_Sin: {
                        cVector3f vDir = GetAxisVector(pAnim->mAxis);
                        cMatrixf mtxAdd = cMath::MatrixTranslate(vDir * sin(mfAnimTime * pAnim->mfSpeed) * pAnim->mfAmp);
                        m_mtxUV = cMath::MatrixMul(m_mtxUV, mtxAdd);
                        break;
                    }
                    case eMaterialUvAnimation_Rotate: {
                        cVector3f vDir = GetAxisVector(pAnim->mAxis);

                        cMatrixf mtxRot = cMath::MatrixRotate(vDir * pAnim->mfSpeed * mfAnimTime, eEulerRotationOrder_XYZ);
                        m_mtxUV = cMath::MatrixMul(m_mtxUV, mtxRot);
                        break;
                    }
                    default: {
                        ASSERT(false && "Unknown Animation Type");
                        break;
                    }
                }
            }
            IncreaseGeneration();
            mfAnimTime += afTimeStep;
        }
    }

    void cMaterial::AddUvAnimation(eMaterialUvAnimation aType, float afSpeed, float afAmp, eMaterialAnimationAxis aAxis) {
        mvUvAnimations.push_back(cMaterialUvAnimation(aType, afSpeed, afAmp, aAxis));
    }

    //-----------------------------------------------------------------------

    void cMaterial::ClearUvAnimations() {
        mvUvAnimations.clear();
        m_mtxUV = cMatrixf::Identity;
    }

} // namespace hpl
