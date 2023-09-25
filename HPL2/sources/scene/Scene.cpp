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

#include "scene/Scene.h"

#include "engine/IUpdateEventLoop.h"
#include "engine/Interface.h"
#include "graphics/RenderTarget.h"
#include "math/MathTypes.h"
#include "scene/Camera.h"
#include "scene/Viewport.h"
#include "scene/World.h"

#include "system/LowLevelSystem.h"
#include "system/Platform.h"
#include "system/Script.h"
#include "system/String.h"

#include "resources/FileSearcher.h"
#include "resources/Resources.h"
#include "resources/ScriptManager.h"
#include "resources/WorldLoaderHandler.h"

#include "graphics/Graphics.h"
#include "graphics/LowLevelGraphics.h"
#include "graphics/PostEffectComposite.h"
#include "graphics/Renderer.h"
#include <algorithm>

#include "sound/LowLevelSound.h"
#include "sound/Sound.h"
#include "sound/SoundHandler.h"

#include "gui/Gui.h"
#include "gui/GuiSet.h"

#include "physics/Physics.h"

namespace hpl {

    cScene::cScene(
        cGraphics* apGraphics,
        cResources* apResources,
        cSound* apSound,
        cPhysics* apPhysics,
        cSystem* apSystem,
        cAI* apAI,
        cGui* apGui,
        cHaptic* apHaptic)
        : mpGraphics(apGraphics)
        , mpResources(apResources)
        , mpSound(apSound)
        , mpPhysics(apPhysics)
        , mpSystem(apSystem)
        , mpAI(apAI)
        , mpGui(apGui)
        , mpHaptic(apHaptic)
        , mpCurrentListener(nullptr) {
    }

    cScene::~cScene() {
        Log("Exiting Scene Module\n");
        Log("--------------------------------------------------------\n");

        STLDeleteAll(m_viewports);
        STLDeleteAll(m_worlds);
        STLDeleteAll(mlstCameras);

        Log("--------------------------------------------------------\n\n");
    }

    cViewport* cScene::PrimaryViewport() {
        for (auto& viewport : m_viewports) {
            if (!viewport->IsVisible()) {
                continue;
            }
            return viewport;
        }
        return nullptr;
    }

    cViewport* cScene::CreateViewport(cCamera* apCamera, cWorld* apWorld, bool abPushFront) {
        cViewport* pViewport = hplNew(cViewport, (this));

        pViewport->SetCamera(apCamera);
        pViewport->SetWorld(apWorld);
        // pViewport->SetSize(-1);
        pViewport->SetRenderer(mpGraphics->GetRenderer(eRenderer_Main));

        if (abPushFront) {
            m_viewports.insert(m_viewports.begin(), 1, pViewport);
        } else {
            m_viewports.push_back(pViewport);
        }

        return pViewport;
    }

    void cScene::DestroyViewport(cViewport* apViewPort) {
        auto it = std::find(m_viewports.begin(), m_viewports.end(), apViewPort);
        if (it != m_viewports.end()) {
            delete *it;
            m_viewports.erase(it);
        }
    }
    bool cScene::ViewportExists(cViewport* apViewPort) {
        auto it = std::find(m_viewports.begin(), m_viewports.end(), apViewPort);
        return it != m_viewports.end();
    }

    void cScene::SetCurrentListener(cViewport* apViewPort) {
        // If there was a previous listener make sure that world is not a listener.
        if (mpCurrentListener != NULL && ViewportExists(mpCurrentListener)) {
            mpCurrentListener->SetIsListener(false);
            cWorld* pWorld = mpCurrentListener->GetWorld();
            if (pWorld && WorldExists(pWorld))
                pWorld->SetIsSoundEmitter(false);
        }

        mpCurrentListener = apViewPort;
        if (mpCurrentListener) {
            mpCurrentListener->SetIsListener(true);
            cWorld* pWorld = mpCurrentListener->GetWorld();
            if (pWorld)
                pWorld->SetIsSoundEmitter(true);
        }
    }

    void cScene::Update(float timeStep) {
        for (auto& world : m_worlds) {
            if (world->IsActive()) {
                world->Update(timeStep);
            }
        }

        if (mpCurrentListener && mpCurrentListener->GetCamera()) {
            cCamera* pCamera3D = mpCurrentListener->GetCamera();
            mpSound->GetLowLevel()->SetListenerAttributes(
                pCamera3D->GetPosition(), cVector3f(0, 0, 0), pCamera3D->GetForward() * -1.0f, pCamera3D->GetUp());
        }
    }

    cCamera* cScene::CreateCamera(eCameraMoveMode aMoveMode) {
        cCamera* pCamera = hplNew(cCamera, ());
        pCamera->SetAspect(mpGraphics->GetLowLevel()->GetScreenSizeFloat().x / mpGraphics->GetLowLevel()->GetScreenSizeFloat().y);
        mlstCameras.push_back(pCamera);

        return pCamera;
    }

    void cScene::DestroyCamera(cCamera* apCam) {
        auto it = std::find(mlstCameras.begin(), mlstCameras.end(), apCam);
        if (it != mlstCameras.end()) {
            delete *it;
            mlstCameras.erase(it);
        }
    }

    void cScene::Render(const ForgeRenderer::Frame& frame, float afFrameTime, tFlag alFlags) {
        // Increase the frame count (do this at top, so render count is valid until this Render is called again!)
        iRenderer::IncRenderFrameCount();

        ///////////////////////////////////////////
        // Iterate all viewports and render
        for (auto& pViewPort : m_viewports) {
            if (!pViewPort->IsVisible()) {
                continue;
            }

            if(!pViewPort->IsValid()) {
                continue;
            }

            //////////////////////////////////////////////
            // Init vars
            cPostEffectComposite* pPostEffectComposite = pViewPort->GetPostEffectComposite();
            bool bPostEffects = false;
            iRenderer* pRenderer = pViewPort->GetRenderer();
            cCamera* pCamera = pViewPort->GetCamera();
            cFrustum* pFrustum = pCamera ? pCamera->GetFrustum() : NULL;
            //////////////////////////////////////////////
            // Render world and call callbacks
            if (alFlags & tSceneRenderFlag_World) {
                pViewPort->RunViewportCallbackMessage(eViewportMessage_OnPreWorldDraw);

                if (pPostEffectComposite && (alFlags & tSceneRenderFlag_PostEffects)) {
                    bPostEffects = pPostEffectComposite->HasActiveEffects();
                }

                if (pRenderer && pViewPort->GetWorld() && pFrustum) {
                    START_TIMING(RenderWorld)
                    pRenderer->Draw(
                        frame,
                        *pViewPort,
                        afFrameTime,
                        pFrustum,
                        pViewPort->GetWorld(),
                        pViewPort->GetRenderSettings(),
                        bPostEffects);
                    STOP_TIMING(RenderWorld)
                } else {
                    // If no renderer sets up viewport do that by our selves.
                    //  cRenderTarget* pRenderTarget = pViewPort->GetRenderTarget();
                    //  mpGraphics->GetLowLevel()->SetCurrentFrameBuffer(	pRenderTarget->mpFrameBuffer,
                    //  													pRenderTarget->mvPos,
                    //  													pRenderTarget->mvSize);
                    //  umm need to workout how this framebuffer is used ...
                }
                pViewPort->RunViewportCallbackMessage(eViewportMessage_OnPostWorldDraw);

                //////////////////////////////////////////////
                // Render 3D GuiSets
                //  Should this really be here? Or perhaps send in a frame buffer depending on the renderer.
                START_TIMING(Render3DGui)
                Render3DGui(frame, pViewPort, pFrustum, afFrameTime);
                STOP_TIMING(Render3DGui)
            }

            auto forgeRenderer = Interface<ForgeRenderer>::Get();

            //////////////////////////////////////////////
            // Render Post effects
            auto outputImage = pRenderer->GetOutputImage(frame.m_frameIndex, *pViewPort);
            if(outputImage.IsValid()) {
                if (bPostEffects) {
                    START_TIMING(RenderPostEffects)
                    const bool isViewportTarget = pViewPort->Target().IsValid();
                    auto& target = isViewportTarget ? pViewPort->Target().m_handle : frame.m_finalRenderTarget;
                    if (isViewportTarget) {
                        cmdBindRenderTargets(frame.m_cmd, 0, NULL, NULL, NULL, NULL, NULL, -1, -1);
                        std::array rtBarriers = {
                            RenderTargetBarrier{ target, RESOURCE_STATE_SHADER_RESOURCE, RESOURCE_STATE_RENDER_TARGET },
                        };
                        cmdResourceBarrier(frame.m_cmd, 0, NULL, 0, NULL, rtBarriers.size(), rtBarriers.data());
                    }
                    pPostEffectComposite->Draw(
                        frame,
                        *pViewPort,
                         afFrameTime,
                         outputImage.m_handle->pTexture,
                        target);
                    if (isViewportTarget) {
                        cmdBindRenderTargets(frame.m_cmd, 0, NULL, NULL, NULL, NULL, NULL, -1, -1);
                        std::array rtBarriers = {
                            RenderTargetBarrier{ target, RESOURCE_STATE_RENDER_TARGET ,RESOURCE_STATE_SHADER_RESOURCE},
                        };
                        cmdResourceBarrier(frame.m_cmd, 0, NULL, 0, NULL, rtBarriers.size(), rtBarriers.data());
                    }
                     STOP_TIMING(RenderPostEffects)
                 } else {
                    auto size = pViewPort->GetSize();
                    cRect2l rect = cRect2l(0, 0, size.x, size.y);
                    forgeRenderer->cmdCopyTexture(frame.m_cmd, outputImage.m_handle->pTexture,
                        frame.m_finalRenderTarget);
                    // context.CopyTextureToFrameBuffer(
                    //     *outputImage, rect, pViewPort->GetRenderTarget());
                 }
            }

            //////////////////////////////////////////////
            // Render Screen GUI
            if (alFlags & tSceneRenderFlag_Gui) {
                START_TIMING(RenderGUI)
                RenderScreenGui(frame, pViewPort, afFrameTime);
                STOP_TIMING(RenderGUI)
            }
        }
    }


    cWorld* cScene::LoadWorld(const tString& asFile, tWorldLoadFlag aFlags) {
        ///////////////////////////////////
        // Load the map file
        tWString asPath = mpResources->GetFileSearcher()->GetFilePath(asFile);
        if (asPath == _W("")) {
            if (cResources::GetCreateAndLoadCompressedMaps())
                asPath = mpResources->GetFileSearcher()->GetFilePath(cString::SetFileExt(asFile, "cmap"));

            if (asPath == _W("")) {
                Error("World '%s' doesn't exist\n", asFile.c_str());
                return NULL;
            }
        }

        cWorld* pWorld = mpResources->GetWorldLoaderHandler()->LoadWorld(asPath, aFlags);
        if (pWorld == NULL) {
            Error("Couldn't load world from '%s'\n", cString::To8Char(asPath).c_str());
            return NULL;
        }

        return pWorld;
    }

    //-----------------------------------------------------------------------

    cWorld* cScene::CreateWorld(const tString& asName) {
        cWorld* pWorld = hplNew(cWorld, (asName, mpGraphics, mpResources, mpSound, mpPhysics, this, mpSystem, mpAI, mpHaptic));

        m_worlds.push_back(pWorld);

        return pWorld;
    }

    //-----------------------------------------------------------------------

    void cScene::DestroyWorld(cWorld* apWorld) {
        auto it = std::find(m_worlds.begin(), m_worlds.end(), apWorld);
        if (it != m_worlds.end()) {
            delete *it;
            m_worlds.erase(it);
        }
    }

    //-----------------------------------------------------------------------

    bool cScene::WorldExists(cWorld* apWorld) {
        auto it = std::find(m_worlds.begin(), m_worlds.end(), apWorld);
        return it != m_worlds.end();
    }

    void cScene::Render3DGui(const ForgeRenderer::Frame& frame, cViewport* apViewPort, cFrustum* apFrustum, float afTimeStep) {
        if (apViewPort->GetCamera() == NULL)
            return;

        cGuiSetListIterator it = apViewPort->GetGuiSetIterator();
        while (it.HasNext()) {
            cGuiSet* pSet = it.Next();
            if (pSet->Is3D()) {
                pSet->Draw(frame, apFrustum);
            }
        }
    }

    void cScene::RenderScreenGui(const ForgeRenderer::Frame& frame, cViewport* apViewPort, float afTimeStep) {
        ///////////////////////////////////////
        // Put all of the non 3D sets in to a sorted map
        typedef std::multimap<int, cGuiSet*> tPrioMap;
        tPrioMap mapSortedSets;

        cGuiSetListIterator it = apViewPort->GetGuiSetIterator();
        while (it.HasNext()) {
            cGuiSet* pSet = it.Next();

            if (pSet->Is3D() == false)
                mapSortedSets.insert(tPrioMap::value_type(pSet->GetDrawPriority(), pSet));
        }

        ///////////////////////////////////////
        // Iterate and render all sets
        if (mapSortedSets.empty()) {
            return;
        }

        tPrioMap::iterator SortIt = mapSortedSets.begin();
        for (; SortIt != mapSortedSets.end(); ++SortIt) {
            cGuiSet* pSet = SortIt->second;
            pSet->Draw(frame, NULL);
        }
    }

} // namespace hpl
