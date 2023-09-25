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

#include "graphics/Renderer.h"

#include "graphics/Enum.h"
#include "graphics/RenderTarget.h"
#include "math/BoundingVolume.h"
#include "math/Math.h"

#include "math/MathTypes.h"
#include "scene/SceneTypes.h"
#include "system/LowLevelSystem.h"
#include "system/PreprocessParser.h"
#include "system/String.h"

#include "graphics/FrameBuffer.h"
#include "graphics/Graphics.h"
#include "graphics/LowLevelGraphics.h"
#include "graphics/Material.h"
#include "graphics/MaterialType.h"
#include "graphics/Mesh.h"
#include "graphics/MeshCreator.h"
#include "graphics/RenderList.h"
#include "graphics/Renderable.h"
#include "graphics/SubMesh.h"
#include "graphics/Texture.h"
#include "graphics/VertexBuffer.h"

#include "resources/LowLevelResources.h"
#include "resources/MeshManager.h"
#include "resources/Resources.h"
#include "resources/TextureManager.h"

#include "scene/Camera.h"
#include "scene/FogArea.h"
#include "scene/LightBox.h"
#include "scene/LightPoint.h"
#include "scene/LightSpot.h"
#include "scene/RenderableContainer.h"
#include "scene/SubMeshEntity.h"
#include "scene/World.h"

#include <algorithm>
#include <memory>

namespace hpl
{

    eShadowMapQuality iRenderer::mShadowMapQuality = eShadowMapQuality_Medium;
    eShadowMapResolution iRenderer::mShadowMapResolution = eShadowMapResolution_High;
    eParallaxQuality iRenderer::mParallaxQuality = eParallaxQuality_Low;
    bool iRenderer::mbParallaxEnabled = true;
    bool iRenderer::mbRefractionEnabled = true;

    //-----------------------------------------------------------------------

    int iRenderer::mlRenderFrameCount = 0;

    namespace rendering::detail
    {

        cRect2l GetClipRectFromObject(iRenderable* apObject, float afPaddingPercent, cFrustum* apFrustum, const cVector2l& avScreenSize, float afHalfFovTan) {
            cBoundingVolume* pBV = apObject->GetBoundingVolume();

            cRect2l clipRect;
            if (afHalfFovTan == 0)
                afHalfFovTan = tan(apFrustum->GetFOV() * 0.5f);
            cMath::GetClipRectFromBV(clipRect, *pBV, apFrustum, avScreenSize, afHalfFovTan);

            // Add 20% padding on clip rect
            int lXInc = (int)((float)clipRect.w * afHalfFovTan);
            int lYInc = (int)((float)clipRect.h * afHalfFovTan);

            clipRect.x = cMath::Max(clipRect.x - lXInc, 0);
            clipRect.y = cMath::Max(clipRect.y - lYInc, 0);
            clipRect.w = cMath::Min(clipRect.w + lXInc * 2, avScreenSize.x - clipRect.x);
            clipRect.h = cMath::Min(clipRect.h + lYInc * 2, avScreenSize.y - clipRect.y);

            return clipRect;
        }

        void UpdateRenderListWalkAllNodesTestFrustumAndVisibility(
            cRenderList* apRenderList,
            cFrustum* frustum,
            iRenderableContainerNode* apNode,
            std::span<cPlanef> clipPlanes,
            tRenderableFlag neededFlags) {
            apNode->UpdateBeforeUse();

            ///////////////////////////////////////
            // Get frustum collision, if previous was inside, then this is too!
            eCollision frustumCollision = frustum->CollideNode(apNode);

            ////////////////////////////////
            // Do a visible check but always iterate the root node!
            if (apNode->GetParent()) {
                if (frustumCollision == eCollision_Outside) {
                    return;
                }
                if (detail::IsRenderableNodeIsVisible(apNode, clipPlanes) == false) {
                    return;
                }
            }

            ////////////////////////
            // Iterate children
            if (apNode->HasChildNodes()) {
                for(auto& node: apNode->GetChildNodes()) {
                    UpdateRenderListWalkAllNodesTestFrustumAndVisibility(apRenderList, frustum, node, clipPlanes, neededFlags);
                }
            }

            /////////////////////////////
            // Iterate objects
            if (apNode->HasObjects()) {
                for(auto& object: apNode->GetObjects()) {
                    if (detail::IsObjectIsVisible(object, neededFlags) == false) {
                        continue;
                    }

                    if (frustumCollision == eCollision_Inside || object->CollidesWithFrustum(frustum)) {
                        apRenderList->AddObject(object);
                    }
                }
            }
        }

        eShadowMapResolution GetShadowMapResolution(eShadowMapResolution aWanted, eShadowMapResolution aMax)
        {
            if (aMax == eShadowMapResolution_High)
            {
                return aWanted;
            }
            else if (aMax == eShadowMapResolution_Medium)
            {
                return aWanted == eShadowMapResolution_High ? eShadowMapResolution_Medium : aWanted;
            }
            return eShadowMapResolution_Low;
        }


        bool IsRenderableNodeIsVisible(iRenderableContainerNode* apNode, std::span<cPlanef> clipPlanes) {
            for(auto& plane: clipPlanes)
            {
                if (cMath::CheckPlaneAABBCollision(plane, apNode->GetMin(), apNode->GetMax(), apNode->GetCenter(), apNode->GetRadius()) ==
                    eCollision_Outside)
                {
                    return false;
                }
            }
            return true;
        }

        bool IsObjectVisible(iRenderable* apObject, tRenderableFlag alNeededFlags, std::span<cPlanef> occlusionPlanes)
        {
            if (!apObject->IsVisible())
            {
                return false;
            }

            if ((apObject->GetRenderFlags() & alNeededFlags) != alNeededFlags)
            {
                return false;
            }

            for (auto& plane : occlusionPlanes)
            {
                cBoundingVolume* pBV = apObject->GetBoundingVolume();
                if (cMath::CheckPlaneBVCollision(plane, *pBV) == eCollision_Outside)
                {
                    return false;
                }
            }
            return true;
        }


        void UpdateRenderListWalkAllNodesTestFrustumAndVisibility(
            cRenderList* apRenderList,
            cFrustum* frustum,
            iRenderableContainer* apContainer,
            std::span<cPlanef> clipPlanes,
            tRenderableFlag neededFlags) {
            apContainer->UpdateBeforeRendering();

            rendering::detail::UpdateRenderListWalkAllNodesTestFrustumAndVisibility(
                apRenderList,
                frustum,
                apContainer->GetRoot(),
                clipPlanes,
                neededFlags);
        }

        bool IsObjectIsVisible(
            iRenderable* object,
            tRenderableFlag neededFlags,
            std::span<cPlanef> clipPlanes) {

            if (object->IsVisible() == false)
                return false;

            /////////////////////////////
            // Check flags
            if ((object->GetRenderFlags() & neededFlags) != neededFlags)
                return false;

            if (!clipPlanes.empty())
            {
                cBoundingVolume* pBV = object->GetBoundingVolume();
                for(auto& planes: clipPlanes)
                {
                    if (cMath::CheckPlaneBVCollision(planes, *pBV) == eCollision_Outside) {
                        return false;
                    }
                }
            }
            return true;
        }
    } // namespace rendering::detail

    bool cRendererNodeSortFunc::operator()(const iRenderableContainerNode* apNodeA, const iRenderableContainerNode* apNodeB) const
    {
        if (apNodeA->IsInsideView() != apNodeB->IsInsideView())
        {
            return apNodeA->IsInsideView() > apNodeB->IsInsideView();
        }

        return apNodeA->GetViewDistance() > apNodeB->GetViewDistance();
    }

    cRenderSettings::cRenderSettings(bool abIsReflection)
    {
        ////////////////////////
        // Create data
        mpRenderList = hplNew(cRenderList, ());

        ////////////////////////
        // Set up General Variables
        mbIsReflection = abIsReflection;
        mbLog = false;

        mClearColor = cColor(0, 0);

        ////////////////////////
        // Set up Render Variables
        // Change this later I assume:
        mlMinimumObjectsBeforeOcclusionTesting = 0; // 8;//8 should be good default, giving a good amount of colliders, or? Clarifiction:
                                                    // Minium num of object rendered until node visibility tests start!
        mlSampleVisiblilityLimit = 3;

        mbUseCallbacks = true;

        mbUseEdgeSmooth = false;

        mbUseOcclusionCulling = true;

        mMaxShadowMapResolution = eShadowMapResolution_High;
        if (mbIsReflection)
            mMaxShadowMapResolution = eShadowMapResolution_Medium;

        mbClipReflectionScreenRect = true;

        mbUseScissorRect = false;

        mbRenderWorldReflection = true;

        ////////////////////////
        // Set up Shadow Variables
        mbRenderShadows = true;
        mfShadowMapBias = 4; // The constant bias
        mfShadowMapSlopeScaleBias = 2; // The bias based on sloping of depth.

        ////////////////////////////
        // Light settings
        mbSSAOActive = mbIsReflection ? false : true;

        ////////////////////////
        // Set up Output Variables
        mlNumberOfLightsRendered = 0;
        mlNumberOfOcclusionQueries = 0;

        ////////////////////////
        // Set up Private Variables

        ////////////////////////
        // Create Reflection settings
        mpReflectionSettings = NULL;
        if (mbIsReflection == false)
        {
            mpReflectionSettings = hplNew(cRenderSettings, (true));
        }
    }

    cRenderSettings::~cRenderSettings()
    {
        hplDelete(mpRenderList);

        if (mpReflectionSettings)
            hplDelete(mpReflectionSettings);
    }

    //-----------------------------------------------------------------------

    void cRenderSettings::ResetVariables()
    {

        if (mpReflectionSettings)
            mpReflectionSettings->ResetVariables();
    }

//-----------------------------------------------------------------------

//////////////////////////////////////////////////
// The render settings will use the default setup, except for the variables below
//  This means SSAO, edgesmooth, etc are always off for reflections.
#define RenderSettingsCopy(aVar) mpReflectionSettings->aVar = aVar
    void cRenderSettings::SetupReflectionSettings()
    {
        if (mpReflectionSettings == NULL)
            return;
        RenderSettingsCopy(mbLog);

        RenderSettingsCopy(mClearColor);

        ////////////////////////////
        // Render settings
        RenderSettingsCopy(mlMinimumObjectsBeforeOcclusionTesting);
        RenderSettingsCopy(mlSampleVisiblilityLimit);

        mpReflectionSettings->mbUseScissorRect = false;

        ////////////////////////////
        // Shadow settings
        RenderSettingsCopy(mbRenderShadows);
        RenderSettingsCopy(mfShadowMapBias);
        RenderSettingsCopy(mfShadowMapSlopeScaleBias);

        ////////////////////////////
        // Light settings
        // RenderSettingsCopy(mbSSAOActive);

        ////////////////////////////
        // Output
        RenderSettingsCopy(mlNumberOfLightsRendered);
        RenderSettingsCopy(mlNumberOfOcclusionQueries);
    }

    iRenderer::iRenderer(const tString& asName, cGraphics* apGraphics, cResources* apResources)
    {
        mpGraphics = apGraphics;
        mpResources = apResources;

        /////////////////////////////////
        // Set up the render functions
        SetupRenderFunctions(mpGraphics->GetLowLevel());

        mfTimeCount = 0;
    }

    //-----------------------------------------------------------------------

    iRenderer::~iRenderer()
    {

    }


    void iRenderer::Update(float afTimeStep)
    {
        mfTimeCount += afTimeStep;
    }

    void iRenderer::BeginRendering(
        float afFrameTime,
        cFrustum* apFrustum,
        cWorld* apWorld,
        cRenderSettings* apSettings,
        bool abSendFrameBufferToPostEffects,
        bool abAtStartOfRendering)
    {
        //////////////////////////////////////////
        // Set up variables
        mfCurrentFrameTime = afFrameTime;
        mpCurrentSettings = apSettings;
    }

} // namespace hpl
