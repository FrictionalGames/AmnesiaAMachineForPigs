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
#pragma once

#include "engine/RTTI.h"
#include "graphics/Enum.h"
#include "graphics/ForgeHandles.h"
#include "graphics/ForgeRenderer.h"
#include "graphics/GraphicsTypes.h"
#include "graphics/Image.h"
#include "graphics/RenderTarget.h"
#include "math/cFrustum.h"
#include "math/MathTypes.h"
#include "scene/SceneTypes.h"

#include "graphics/RenderFunctions.h"
#include "scene/Viewport.h"
#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

#include "Common_3/Graphics/Interfaces/IGraphics.h"
#include <FixPreprocessor.h>


#include <span>

namespace hpl {


    //---------------------------------------------

    class cGraphics;
    class cResources;
    class cEngine;
    class iLowLevelResources;
    class cMeshCreator;
    class iRenderable;
    class cWorld;
    class cRenderSettings;
    class cRenderList;
    class iLight;
    class cBoundingVolume;
    class iRenderableContainer;
    class iRenderableContainerNode;
    class cVisibleRCNodeTracker;
    class RenderCallbackMessage;
    class iRenderer;

    namespace rendering::detail {

        eShadowMapResolution GetShadowMapResolution(eShadowMapResolution aWanted, eShadowMapResolution aMax);
        /**
        * @brief Checks if the given object is occluded by the given planes.
        * @param apObject The object to check.
        * @param alNeededFlags The flags that the object must have to be considered.
        * @param occlusionPlanes The planes to check against.
        */
        bool IsObjectVisible(iRenderable *apObject, tRenderableFlag alNeededFlags, std::span<cPlanef> occlusionPlanes);

        bool IsRenderableNodeIsVisible(iRenderableContainerNode* apNode, std::span<cPlanef> clipPlanes);
        bool IsObjectIsVisible(iRenderable* object, tRenderableFlag neededFlags, std::span<cPlanef> clipPlanes = {});

        void UpdateRenderListWalkAllNodesTestFrustumAndVisibility(
            cRenderList* apRenderList,
            cFrustum* frustum,
            iRenderableContainerNode* apNode,
            std::span<cPlanef> clipPlanes,
            tRenderableFlag neededFlags);

        void UpdateRenderListWalkAllNodesTestFrustumAndVisibility(
            cRenderList* apRenderList,
            cFrustum* frustum,
            iRenderableContainer* apContainer,
            std::span<cPlanef> clipPlanes,
            tRenderableFlag neededFlags);

        cRect2l GetClipRectFromObject(iRenderable* apObject, float afPaddingPercent, cFrustum* apFrustum, const cVector2l& avScreenSize, float afHalfFovTan);
    }

    class cRenderSettings
    {
    public:
        cRenderSettings(bool abIsReflection = false);
        ~cRenderSettings();

        ////////////////////////////
        //Helper methods
        void ResetVariables();

        void SetupReflectionSettings();

        cRenderList *mpRenderList;

        cRenderSettings *mpReflectionSettings;

        ////////////////////////////
        //General settings
        bool mbLog;

        cColor mClearColor;
        ////////////////////////////
        //Render settings
        int mlMinimumObjectsBeforeOcclusionTesting;
        int mlSampleVisiblilityLimit;
        bool mbIsReflection;
        bool mbClipReflectionScreenRect;

        bool mbUseOcclusionCulling;

        bool mbUseEdgeSmooth;

        bool mbUseCallbacks;

        eShadowMapResolution mMaxShadowMapResolution;

        bool mbUseScissorRect;
        cVector2l mvScissorRectPos;
        cVector2l mvScissorRectSize;

        bool mbRenderWorldReflection;

        ////////////////////////////
        //Shadow settings
        bool mbRenderShadows;
        float mfShadowMapBias;
        float mfShadowMapSlopeScaleBias;

        ////////////////////////////
        //Light settings
        bool mbSSAOActive;

        ////////////////////////////
        //Output
        int mlNumberOfLightsRendered;
        int mlNumberOfOcclusionQueries;
    };

    //---------------------------------------------

    class cRendererNodeSortFunc
    {
    public:
        bool operator()(const iRenderableContainerNode* apNodeA, const iRenderableContainerNode* apNodeB) const;
    };


    class iRenderer : public iRenderFunctions
    {
        HPL_RTTI_CLASS(iRenderer, "{A3E0F5A0-0F9B-4F5C-9B9E-0F9B4F5C9B9E}")

        friend class cRendererCallbackFunctions;
        friend class cRenderSettings;
    public:

        iRenderer(const tString& asName, cGraphics *apGraphics,cResources* apResources);
        virtual ~iRenderer();

        // plan to just use the single draw call need to call BeginRendering to setup state
        // ensure the contents is copied to the RenderViewport
        virtual void Draw(
            const ForgeRenderer::Frame& context,
            cViewport& viewport,
            float afFrameTime,
            cFrustum* apFrustum,
            cWorld* apWorld,
            cRenderSettings* apSettings,
            bool abSendFrameBufferToPostEffects){};

        void Update(float afTimeStep);

        inline static int GetRenderFrameCount()  { return mlRenderFrameCount;}
        inline static void IncRenderFrameCount() { ++mlRenderFrameCount;}

        float GetTimeCount(){ return mfTimeCount;}

        virtual bool LoadData()=0;
        virtual void DestroyData()=0;

        virtual SharedRenderTarget GetOutputImage(uint32_t frameIndex, cViewport& viewport) { return SharedRenderTarget();}

        //Static settings. Must be set before renderer data load.
        static void SetShadowMapQuality(eShadowMapQuality aQuality) { mShadowMapQuality = aQuality;}
        static eShadowMapQuality GetShadowMapQuality(){ return mShadowMapQuality;}

        static void SetShadowMapResolution(eShadowMapResolution aResolution) { mShadowMapResolution = aResolution;}
        static eShadowMapResolution GetShadowMapResolution(){ return mShadowMapResolution;}

        static void SetParallaxQuality(eParallaxQuality aQuality) { mParallaxQuality = aQuality;}
        static eParallaxQuality GetParallaxQuality(){ return mParallaxQuality;}

        static void SetParallaxEnabled(bool abX) { mbParallaxEnabled = abX;}
        static bool GetParallaxEnabled(){ return mbParallaxEnabled;}

        static void SetRefractionEnabled(bool abX) { mbRefractionEnabled = abX;}
        static bool GetRefractionEnabled(){ return mbRefractionEnabled;}
    protected:
        void BeginRendering(float afFrameTime,cFrustum *apFrustum, cWorld *apWorld, cRenderSettings *apSettings,
                            bool abSendFrameBufferToPostEffects, bool abAtStartOfRendering=true);
        cResources* mpResources;

        float mfCurrentFrameTime;
        cRenderSettings *mpCurrentSettings;

        static int mlRenderFrameCount;
        float mfTimeCount;

        static eShadowMapQuality mShadowMapQuality;
        static eShadowMapResolution mShadowMapResolution;
        static eParallaxQuality mParallaxQuality;
        static bool mbParallaxEnabled;
        static bool mbRefractionEnabled;
    };

    class cRendererCallbackFunctions
    {
    public:
        cRendererCallbackFunctions(cViewport& viewport,iRenderer *apRenderer) :
            m_viewport(viewport),
            mpRenderer(apRenderer) {}

        iLowLevelGraphics *GetLowLevelGfx(){ return mpRenderer->mpLowLevelGraphics;}

    private:
        iRenderer *mpRenderer;
        cViewport& m_viewport;
    };

};
