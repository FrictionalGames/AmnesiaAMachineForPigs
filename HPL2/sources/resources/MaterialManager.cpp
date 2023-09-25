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

#include "resources/MaterialManager.h"

#include "graphics/Image.h"
#include "graphics/IndexPool.h"
#include "system/LowLevelSystem.h"
#include "system/Platform.h"
#include "system/String.h"
#include "system/System.h"

#include "graphics/Graphics.h"
#include "graphics/LowLevelGraphics.h"
#include "graphics/Material.h"
#include "graphics/MaterialType.h"

#include "resources/LowLevelResources.h"
#include "resources/Resources.h"
#include "resources/TextureManager.h"
#include "resources/XmlDocument.h"

#include "impl/tinyXML/tinyxml.h"

#include "Common_3/Utilities/Interfaces/ILog.h"
#include <FixPreprocessor.h>

namespace hpl {

    namespace internal {
        static IndexPool m_MaterialIndexPool(cMaterial::MaxMaterialID);
    }

    cMaterialManager::cMaterialManager(cGraphics* apGraphics, cResources* apResources)
        : iResourceManager(apResources->GetFileSearcher(), apResources->GetLowLevel(), apResources->GetLowLevelSystem()) {
        mpGraphics = apGraphics;
        mpResources = apResources;

        mlTextureSizeDownScaleLevel = 0;
        mTextureFilter = eTextureFilter_Bilinear;
        mfTextureAnisotropy = 1.0f;

        mbDisableRenderDataLoading = false;

        mlIdCounter = 0;
    }

    cMaterialManager::~cMaterialManager() {
        DestroyAll();

        Log(" Done with materials\n");
    }

    cMaterial* cMaterialManager::CreateMaterial(const tString& asName) {
        if (asName == "")
            return NULL;

        tWString sPath;
        cMaterial* pMaterial;
        tString asNewName;

        BeginLoad(asName);

        asNewName = cString::SetFileExt(asName, "mat");

        pMaterial = static_cast<cMaterial*>(this->FindLoadedResource(asNewName, sPath));

        if (pMaterial == NULL && sPath != _W("")) {
            pMaterial = LoadFromFile(asNewName, sPath);

            if (pMaterial == NULL) {
                Error("Couldn't load material '%s'\n", asNewName.c_str());
                EndLoad();
                return NULL;
            }

            AddResource(pMaterial);
        }

        if (pMaterial)
            pMaterial->IncUserCount();
        else
            Error("Couldn't create material '%s'\n", asNewName.c_str());

        EndLoad();
        return pMaterial;
    }

    void cMaterialManager::Update(float afTimeStep) {
    }

    void cMaterialManager::Unload(iResourceBase* apResource) {
    }

    void cMaterialManager::Destroy(iResourceBase* apResource) {
        apResource->DecUserCount();

        if (apResource->HasUsers() == false) {
            RemoveResource(apResource);
            hplDelete(apResource);
        }
    }

    void cMaterialManager::SetTextureFilter(eTextureFilter aFilter) {
        if (aFilter == mTextureFilter)
            return;
        mTextureFilter = aFilter;

        tResourceBaseMapIt it = m_mapResources.begin();
        for (; it != m_mapResources.end(); ++it) {
            cMaterial* pMat = static_cast<cMaterial*>(it->second);
            pMat->setTextureFilter(aFilter);
        }
    }

    void cMaterialManager::SetTextureAnisotropy(float afX) {
        if (mfTextureAnisotropy == afX)
            return;
        mfTextureAnisotropy = afX;

        tResourceBaseMapIt it = m_mapResources.begin();
        for (; it != m_mapResources.end(); ++it) {
            cMaterial* pMat = static_cast<cMaterial*>(it->second);
            pMat->SetTextureAnisotropy(afX);
        }
    }

    tString cMaterialManager::GetPhysicsMaterialName(const tString& asName) {
        tWString sPath;
        cMaterial* pMaterial;
        tString asNewName;

        asNewName = cString::SetFileExt(asName, "mat");

        pMaterial = static_cast<cMaterial*>(this->FindLoadedResource(asNewName, sPath));

        if (pMaterial == NULL && sPath != _W("")) {
            FILE* pFile = cPlatform::OpenFile(sPath, _W("rb"));
            if (pFile == NULL)
                return "";

            TiXmlDocument* pDoc = hplNew(TiXmlDocument, ());
            if (!pDoc->LoadFile(pFile)) {
                fclose(pFile);
                hplDelete(pDoc);
                return "";
            }
            fclose(pFile);

            TiXmlElement* pRoot = pDoc->RootElement();

            TiXmlElement* pMain = pRoot->FirstChildElement("Main");
            if (pMain == NULL) {
                hplDelete(pDoc);
                Error("Main child not found in '%s'\n", sPath.c_str());
                return "";
            }

            tString sPhysicsName = cString::ToString(pMain->Attribute("PhysicsMaterial"), "Default");

            hplDelete(pDoc);

            return sPhysicsName;
        }

        if (pMaterial)
            return pMaterial->GetPhysicsMaterial();
        else
            return "";
    }

    //-----------------------------------------------------------------------

    cMaterial* cMaterialManager::CreateCustomMaterial(const tString& asName, iMaterialType* apMaterialType) {
        cMaterial* pMat = hplNew(cMaterial, (asName, cString::To16Char(asName), mpResources));
        pMat->IncUserCount();
        AddResource(pMat);
        return pMat;
    }

    cMaterial* cMaterialManager::LoadFromFile(const tString& asName, const tWString& asPath) {
        iXmlDocument* pDoc = mpResources->GetLowLevel()->CreateXmlDocument();
        if (pDoc->CreateFromFile(asPath) == false) {
            mpResources->DestroyXmlDocument(pDoc);
            return NULL;
        }

        cXmlElement* pMain = pDoc->GetFirstElement("Main");
        if (pMain == NULL) {
            mpResources->DestroyXmlDocument(pDoc);
            Error("Main child not found.\n");
            return NULL;
        }

        tString sType = pMain->GetAttributeString("Type");
        if (sType == "") {
            mpResources->DestroyXmlDocument(pDoc);
            Error("Type not found.\n");
            return NULL;
        }

        /////////////////////////////
        // Get General Propertries
        bool bDepthTest = pMain->GetAttributeBool("DepthTest", true);
        float fValue = pMain->GetAttributeFloat("Value", 1);
        tString sPhysicsMatName = pMain->GetAttributeString("PhysicsMaterial", "Default");
        tString sBlendMode = pMain->GetAttributeString("BlendMode", "Add");

        /////////////////////////////
        // Make a "fake" material, with a blank type
        if (mbDisableRenderDataLoading) {
            cMaterial* pMat = hplNew(cMaterial, (asName, asPath, mpResources));
            pMat->SetPhysicsMaterial(sPhysicsMatName);

            mpResources->DestroyXmlDocument(pDoc);
            return pMat;
        }

        /////////////////////////////
        // CreateType
        tString normalizedMaterialName = cString::ToLowerCase(sType);
        auto metaInfo = std::find_if(cMaterial::MaterialMetaTable.begin(), cMaterial::MaterialMetaTable.end(), [&](auto& info) {
            return info.m_name == normalizedMaterialName;
        });

        if (metaInfo == cMaterial::MaterialMetaTable.end()) {
            mpResources->DestroyXmlDocument(pDoc);
            LOGF(eERROR, "Invalid material type %s", sType.c_str());
            return NULL;
        }
        cMaterial* pMat = new cMaterial(asName, asPath, mpResources);
        pMat->SetDepthTest(bDepthTest);
        pMat->SetPhysicsMaterial(sPhysicsMatName);

        ///////////////////////////
        // Textures
        cXmlElement* pTexRoot = pDoc->GetFirstElement("TextureUnits");
        if (pTexRoot == NULL) {
            mpResources->DestroyXmlDocument(pDoc);
            Error("TextureUnits child not found.\n");
            return NULL;
        }

        for (eMaterialTexture textureType : metaInfo->m_usedTextures) {
            tString sTextureType = GetTextureString(textureType);

            cXmlElement* pTexChild = pTexRoot->GetFirstElement(sTextureType.c_str());
            if (pTexChild == NULL) {
                // Log(" Texture unit element missing!\n");
                /*hplDelete(pMat);
                return NULL;*/
                continue;
            }

            eTextureType type = GetType(pTexChild->GetAttributeString("Type", ""));
            tString sFile = pTexChild->GetAttributeString("File", "");
            bool bMipMaps = pTexChild->GetAttributeBool("MipMaps", true);
            bool bCompress = pTexChild->GetAttributeBool("Compress", false);
            eTextureWrap wrap = GetWrap(pTexChild->GetAttributeString("Wrap", ""));

            eTextureAnimMode animMode = GetAnimMode(pTexChild->GetAttributeString("AnimMode", "None"));
            float fFrameTime = pTexChild->GetAttributeFloat("AnimFrameTime", 1.0f);

            if (sFile == "")
                continue;

            if (cString::GetFilePath(sFile).length() <= 1) {
                sFile = cString::SetFilePath(sFile, cString::To8Char(cString::GetFilePathW(asPath)));
            }

            cTextureManager::ImageOptions options;
            iResourceBase* pImageResource = nullptr;
            if (animMode != eTextureAnimMode_None) {
                auto animatedImage = mpResources->GetTextureManager()->CreateAnimImage(
                    sFile, bMipMaps, type, eTextureUsage_Normal, mlTextureSizeDownScaleLevel);
                animatedImage->SetFrameTime(fFrameTime);
                animatedImage->SetAnimMode(animMode);
                pMat->SetImage(textureType, animatedImage);
                pImageResource = animatedImage;

            } else {
                Image* pImage = nullptr;
                switch (type) {
                case eTextureType_1D:
                    pImage =
                        mpResources->GetTextureManager()->Create1DImage(sFile, bMipMaps, eTextureUsage_Normal, mlTextureSizeDownScaleLevel);
                    break;
                case eTextureType_2D:
                    pImage = mpResources->GetTextureManager()->Create2DImage(
                        sFile, bMipMaps, eTextureType_2D, eTextureUsage_Normal, mlTextureSizeDownScaleLevel);
                    break;
                case eTextureType_CubeMap:
                    pImage = mpResources->GetTextureManager()->CreateCubeMapImage(
                        sFile, bMipMaps, eTextureUsage_Normal, mlTextureSizeDownScaleLevel);
                    break;
                case eTextureType_3D:
                    pImage =
                        mpResources->GetTextureManager()->Create3DImage(sFile, bMipMaps, eTextureUsage_Normal, mlTextureSizeDownScaleLevel);
                    break;
                default:
                    {
                        ASSERT(false && "Invalid texture type");
                        break;
                    }
                }
                pImageResource = pImage;

                pMat->setTextureWrap(wrap);
                pMat->setTextureFilter(mTextureFilter);
                pMat->SetTextureAnisotropy(mfTextureAnisotropy);
                if (pImage) {
                    pMat->SetImage(textureType, pImage);
                }
            }
            if (!pImageResource) {
                mpResources->DestroyXmlDocument(pDoc);
                hplDelete(pMat);
                return nullptr;
            }
        }

        ///////////////////////////
        // Animations
        cXmlElement* pUvAnimRoot = pDoc->GetFirstElement("UvAnimations");
        if (pUvAnimRoot) {
            cXmlNodeListIterator it = pUvAnimRoot->GetChildIterator();
            while (it.HasNext()) {
                cXmlElement* pAnimElem = it.Next()->ToElement();

                eMaterialUvAnimation animType = GetUvAnimType(pAnimElem->GetAttributeString("Type").c_str());
                eMaterialAnimationAxis animAxis = GetAnimAxis(pAnimElem->GetAttributeString("Axis").c_str());
                float fSpeed = pAnimElem->GetAttributeFloat("Speed", 0);
                float fAmp = pAnimElem->GetAttributeFloat("Amplitude", 0);

                pMat->AddUvAnimation(animType, fSpeed, fAmp, animAxis);
            }
        }

        ///////////////////////////
        // Variables
        cXmlElement* pUserVarsRoot = pDoc->GetFirstElement("SpecificVariables");
        cResourceVarsObject userVars;
        if (pUserVarsRoot)
            userVars.LoadVariables(pUserVarsRoot);

        tString materialID = cString::ToLowerCase(sType);
        for (auto& meta : cMaterial::MaterialMetaTable) {
            if (materialID == meta.m_name) {
                pMat->SetHandle(IndexPoolHandle(&internal::m_MaterialIndexPool));
                MaterialDescriptor materialDescriptor;
                materialDescriptor.m_id = meta.m_id;
                switch (meta.m_id) {
                case MaterialID::SolidDiffuse:
                    {
                        //	    type.m_data.m_solid.m_heightMapScale = userVars.GetVarFloat("HeightMapScale", 0.1f);
                        //		type.m_data.m_solid.m_heightMapBias = userVars.GetVarFloat("HeightMapBias", 0);
                        //		type.m_data.m_solid.m_frenselBias = userVars.GetVarFloat("FrenselBias", 0.2f);
                        //		type.m_data.m_solid.m_frenselPow = userVars.GetVarFloat("FrenselPow", 8.0f);
                        //		type.m_alphaDissolveFilter = userVars.GetVarBool("AlphaDissolveFilter", false);

                        materialDescriptor.m_solid.m_heightMapScale = userVars.GetVarFloat("HeightMapScale", 0.1f);
                        materialDescriptor.m_solid.m_heightMapBias = userVars.GetVarFloat("HeightMapBias", 0);
                        materialDescriptor.m_solid.m_frenselBias = userVars.GetVarFloat("FrenselBias", 0.2f);
                        materialDescriptor.m_solid.m_frenselPow = userVars.GetVarFloat("FrenselPow", 8.0f);
                        materialDescriptor.m_solid.m_alphaDissolveFilter = userVars.GetVarBool("AlphaDissolveFilter", false);
                        break;
                    }
                case MaterialID::Translucent:
                    {
                       // pMat->SetHasRefraction(userVars.GetVarBool("Refraction", false));
                       // pMat->SetIsAffectedByLightLevel(userVars.GetVarBool("AffectedByLightLevel", false));
                       // pMat->SetHasRefractionNormals(userVars.GetVarBool("RefractionNormals", true));
                       // pMat->SetUseRefractionEdgeCheck(userVars.GetVarBool("RefractionEdgeCheck", true));
                        //		type.m_data.m_translucentUniformBlock.mfRefractionScale =
                        //userVars.GetVarFloat("RefractionScale", 1.0f); 		type.m_data.m_translucentUniformBlock.mfFrenselBias =
                        //userVars.GetVarFloat("FrenselBias", 0.2f); 		type.m_data.m_translucentUniformBlock.mfFrenselPow =
                        //userVars.GetVarFloat("FrenselPow", 8.0); 		type.m_data.m_translucentUniformBlock.mfRimLightMul =
                        //userVars.GetVarFloat("RimLightMul", 0.0f); 		type.m_data.m_translucentUniformBlock.mfRimLightPow =
                        //userVars.GetVarFloat("RimLightPow", 8.0f);

                        materialDescriptor.m_translucent.m_hasRefraction = userVars.GetVarBool("Refraction", false);
                        materialDescriptor.m_translucent.m_refractionNormals = userVars.GetVarBool("RefractionNormals", true);
                        materialDescriptor.m_translucent.m_refractionEdgeCheck = userVars.GetVarBool("RefractionEdgeCheck", true);
                        materialDescriptor.m_translucent.m_isAffectedByLightLevel = userVars.GetVarBool("AffectedByLightLevel", false);

                        materialDescriptor.m_translucent.m_refractionScale = userVars.GetVarFloat("RefractionScale", 1.0f);
                        materialDescriptor.m_translucent.m_frenselBias = userVars.GetVarFloat("FrenselBias", 0.2f);
                        materialDescriptor.m_translucent.m_frenselPow = userVars.GetVarFloat("FrenselPow", 8.0);
                        materialDescriptor.m_translucent.m_rimLightMul = userVars.GetVarFloat("RimLightMul", 0.0f);
                        materialDescriptor.m_translucent.m_rimLightPow = userVars.GetVarFloat("RimLightPow", 8.0f);
                        materialDescriptor.m_translucent.m_blend = GetBlendMode(sBlendMode);
                        break;
                    }
                case MaterialID::Water:
                    {
                        // type.m_data.m_waterUniformBlock.mbHasReflection = userVars.GetVarBool("HasReflection", true);
                        //	type.m_data.m_waterUniformBlock.mfRefractionScale =
                        // userVars.GetVarFloat("RefractionScale", 1.0f); 		type.m_data.m_waterUniformBlock.mfFrenselBias =
                        // userVars.GetVarFloat("FrenselBias", 0.2f); 		type.m_data.m_waterUniformBlock.mfFrenselPow =
                        // userVars.GetVarFloat("FrenselPow", 8.0f);
                        //		type.m_data.m_waterUniformBlock.mfReflectionFadeStart =
                        // userVars.GetVarFloat("ReflectionFadeStart", 0);
                        //		type.m_data.m_waterUniformBlock.mfReflectionFadeEnd =
                        // userVars.GetVarFloat("ReflectionFadeEnd", 0); 		type.m_data.m_waterUniformBlock.mfWaveSpeed =
                        // userVars.GetVarFloat("WaveSpeed", 1.0f); 		type.m_data.m_waterUniformBlock.mfWaveAmplitude =
                        // userVars.GetVarFloat("WaveAmplitude", 1.0f); 		type.m_data.m_waterUniformBlock.mfWaveFreq =
                        // userVars.GetVarFloat("WaveFreq", 1.0f);

                        //pMat->SetWorldReflectionOcclusionTest(userVars.GetVarBool("OcclusionCullWorldReflection", true));
                        //pMat->SetMaxReflectionDistance(userVars.GetVarFloat("ReflectionFadeEnd", 0.0f));
                        //pMat->SetLargeTransperantSurface(userVars.GetVarBool("LargeSurface", false));

                        materialDescriptor.m_water.m_hasReflection = userVars.GetVarBool("HasReflection", true);
                        materialDescriptor.m_water.m_refractionScale = userVars.GetVarFloat("RefractionScale", 1.0f);
                        materialDescriptor.m_water.m_frenselBias = userVars.GetVarFloat("FrenselBias", 0.2f);
                        materialDescriptor.m_water.m_frenselPow = userVars.GetVarFloat("FrenselPow", 8.0f);
                        materialDescriptor.m_water.m_reflectionFadeStart = userVars.GetVarFloat("ReflectionFadeStart", 0);
                        materialDescriptor.m_water.m_reflectionFadeEnd = userVars.GetVarFloat("ReflectionFadeEnd", 0);
                        materialDescriptor.m_water.m_waveSpeed = userVars.GetVarFloat("WaveSpeed", 1.0f);
                        materialDescriptor.m_water.m_waveAmplitude = userVars.GetVarFloat("WaveAmplitude", 1.0f);
                        materialDescriptor.m_water.m_waveFreq = userVars.GetVarFloat("WaveFreq", 1.0f);

                        materialDescriptor.m_water.m_isLargeSurface = userVars.GetVarBool("LargeSurface", false);
                        materialDescriptor.m_water.m_worldReflectionOcclusionTest = userVars.GetVarBool("OcclusionCullWorldReflection", true);
                        break;
                    }
                case MaterialID::Decal:
                    {
                        materialDescriptor.m_translucent.m_blend = GetBlendMode(sBlendMode);
                        break;
                    }
                default:
                    ASSERT(false && "Invalid material type");
                    break;
                }
                pMat->SetDescriptor(materialDescriptor);
                break;
            }
        }

        mpResources->DestroyXmlDocument(pDoc);


        return pMat;
    }

    //-----------------------------------------------------------------------

    eTextureType cMaterialManager::GetType(const tString& asType) {
        if (cString::ToLowerCase(asType) == "cube")
            return eTextureType_CubeMap;
        else if (cString::ToLowerCase(asType) == "1d")
            return eTextureType_1D;
        else if (cString::ToLowerCase(asType) == "2d")
            return eTextureType_2D;
        else if (cString::ToLowerCase(asType) == "3d")
            return eTextureType_3D;

        return eTextureType_2D;
    }
    //-----------------------------------------------------------------------

    tString cMaterialManager::GetTextureString(eMaterialTexture aType) {
        switch (aType) {
        case eMaterialTexture_Diffuse:
            return "Diffuse";
        case eMaterialTexture_Alpha:
            return "Alpha";
        case eMaterialTexture_NMap:
            return "NMap";
        case eMaterialTexture_Height:
            return "Height";
        case eMaterialTexture_Illumination:
            return "Illumination";
        case eMaterialTexture_Specular:
            return "Specular";
        case eMaterialTexture_CubeMap:
            return "CubeMap";
        case eMaterialTexture_DissolveAlpha:
            return "DissolveAlpha";
        case eMaterialTexture_CubeMapAlpha:
            return "CubeMapAlpha";
        }

        return "";
    }

    //-----------------------------------------------------------------------

    eTextureWrap cMaterialManager::GetWrap(const tString& asType) {
        if (cString::ToLowerCase(asType) == "repeat")
            return eTextureWrap_Repeat;
        else if (cString::ToLowerCase(asType) == "clamp")
            return eTextureWrap_Clamp;
        else if (cString::ToLowerCase(asType) == "clamptoedge")
            return eTextureWrap_ClampToEdge;
        return eTextureWrap_Repeat;
    }

    eTextureAnimMode cMaterialManager::GetAnimMode(const tString& asType) {
        if (cString::ToLowerCase(asType) == "none")
            return eTextureAnimMode_None;
        else if (cString::ToLowerCase(asType) == "loop")
            return eTextureAnimMode_Loop;
        else if (cString::ToLowerCase(asType) == "oscillate")
            return eTextureAnimMode_Oscillate;
        return eTextureAnimMode_None;
    }

    //-----------------------------------------------------------------------

    eMaterialBlendMode cMaterialManager::GetBlendMode(const tString& asType) {
        tString sLow = cString::ToLowerCase(asType);
        if (sLow == "add")
            return eMaterialBlendMode_Add;
        if (sLow == "mul")
            return eMaterialBlendMode_Mul;
        if (sLow == "mulx2")
            return eMaterialBlendMode_MulX2;
        if (sLow == "alpha")
            return eMaterialBlendMode_Alpha;
        if (sLow == "premulalpha")
            return eMaterialBlendMode_PremulAlpha;

        Warning("Material BlendMode '%s' does not exist!\n", asType.c_str());

        return eMaterialBlendMode_Add;
    }

    //-----------------------------------------------------------------------

    eMaterialUvAnimation cMaterialManager::GetUvAnimType(const char* apString) {
        if (apString == NULL) {
            Error("Uv animation attribute Type does not exist!\n");
            return eMaterialUvAnimation_LastEnum;
        }

        tString sLow = cString::ToLowerCase(apString);

        if (sLow == "translate")
            return eMaterialUvAnimation_Translate;
        if (sLow == "sin")
            return eMaterialUvAnimation_Sin;
        if (sLow == "rotate")
            return eMaterialUvAnimation_Rotate;

        Error("Invalid uv animation type %s\n", apString);
        return eMaterialUvAnimation_LastEnum;
    }

    eMaterialAnimationAxis cMaterialManager::GetAnimAxis(const char* apString) {
        if (apString == NULL) {
            Error("Uv animation attribute Axis does not exist!\n");
            return eMaterialAnimationAxis_LastEnum;
        }

        tString sLow = cString::ToLowerCase(apString);

        if (sLow == "x")
            return eMaterialAnimationAxis_X;
        if (sLow == "y")
            return eMaterialAnimationAxis_Y;
        if (sLow == "z")
            return eMaterialAnimationAxis_Z;

        Error("Invalid animation axis %s\n", apString);
        return eMaterialAnimationAxis_LastEnum;
    }

    //-----------------------------------------------------------------------
} // namespace hpl
