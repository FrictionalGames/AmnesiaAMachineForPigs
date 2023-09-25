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

#include "graphics/RenderList.h"

#include "graphics/Material.h"
#include "graphics/MaterialType.h"
#include "graphics/Renderable.h"
#include "graphics/Renderer.h"
#include "graphics/SubMesh.h"

#include "scene/FogArea.h"
#include "scene/Light.h"
#include "scene/MeshEntity.h"
#include "scene/SubMeshEntity.h"

#include "math/cFrustum.h"
#include "math/Math.h"

#include <algorithm>
#include <span>

namespace hpl {

    static bool SortFunc_Z(iRenderable* apObjectA, iRenderable* apObjectB) {
        cMaterial* pMatA = apObjectA->GetMaterial();
        cMaterial* pMatB = apObjectB->GetMaterial();

        //////////////////////////
        // Alpha mode
        if (pMatA->GetAlphaMode() != pMatB->GetAlphaMode()) {
            return pMatA->GetAlphaMode() < pMatB->GetAlphaMode();
        }

        //////////////////////////
        // If alpha, sort by texture (we know alpha is same for both materials, so can just test one)
        if (pMatA->GetAlphaMode() == eMaterialAlphaMode_Trans) {
            if (pMatA->GetImage(eMaterialTexture_Diffuse) != pMatB->GetImage(eMaterialTexture_Diffuse)) {
                return pMatA->GetImage(eMaterialTexture_Diffuse) < pMatB->GetImage(eMaterialTexture_Diffuse);
            }
        }

        //////////////////////////
        // View space depth, no need to test further since Z should almost never be the same for two objects.
        //  use ">" since we want to render things closest to the screen first.
        return apObjectA->GetViewSpaceZ() > apObjectB->GetViewSpaceZ();
        // return apObjectA < apObjectB;
    }

    //-----------------------------------------------------------------------

    static bool SortFunc_Diffuse(iRenderable* apObjectA, iRenderable* apObjectB) {
        cMaterial* pMatA = apObjectA->GetMaterial();
        cMaterial* pMatB = apObjectB->GetMaterial();

        //////////////////////////
        // Vertex buffer
        if (apObjectA->GetVertexBuffer() != apObjectB->GetVertexBuffer()) {
            return apObjectA->GetVertexBuffer() < apObjectB->GetVertexBuffer();
        }

        //////////////////////////
        // Matrix
        if (apObjectA->GetModelMatrixPtr() != apObjectB->GetModelMatrixPtr()) {
            return apObjectA->GetModelMatrixPtr() < apObjectB->GetModelMatrixPtr();
        }

        //////////////////////////
        // Object
        return apObjectA < apObjectB;
    }

    static bool SortFunc_Translucent(iRenderable* apObjectA, iRenderable* apObjectB) {
        ////////////////////////
        // If there is a large plane, then need to sort objects according to this first!
        if (apObjectA->GetLargePlaneSurfacePlacement() != apObjectB->GetLargePlaneSurfacePlacement()) {
            return apObjectA->GetLargePlaneSurfacePlacement() < apObjectB->GetLargePlaneSurfacePlacement();
        }

        //////////////////////////
        // View space depth, no need to test further since Z should almost never be the same for two objects.
        return apObjectA->GetViewSpaceZ() < apObjectB->GetViewSpaceZ();
    }

    static bool SortFunc_Decal(iRenderable* apObjectA, iRenderable* apObjectB) {
        cMaterial* pMatA = apObjectA->GetMaterial();
        cMaterial* pMatB = apObjectB->GetMaterial();

        //////////////////////////
        // Texture
        if (pMatA->GetImage(eMaterialTexture_Illumination) != pMatB->GetImage(eMaterialTexture_Illumination)) {
            return pMatA->GetImage(eMaterialTexture_Illumination) < pMatB->GetImage(eMaterialTexture_Illumination);
        }

        //////////////////////////
        // Vertex buffer
        if (apObjectA->GetVertexBuffer() != apObjectB->GetVertexBuffer()) {
            return apObjectA->GetVertexBuffer() < apObjectB->GetVertexBuffer();
        }

        //////////////////////////
        // Matrix
        if (apObjectA->GetModelMatrixPtr() != apObjectB->GetModelMatrixPtr()) {
            return apObjectA->GetModelMatrixPtr() < apObjectB->GetModelMatrixPtr();
        }

        //////////////////////////
        // Vector (just so that order stays the same
        return apObjectA->GetWorldPosition() < apObjectB->GetWorldPosition();
    }

    static bool SortFunc_Illumination(iRenderable* apObjectA, iRenderable* apObjectB) {
        cMaterial* pMatA = apObjectA->GetMaterial();
        cMaterial* pMatB = apObjectB->GetMaterial();

        //////////////////////////
        // Texture
        if (pMatA->GetImage(eMaterialTexture_Illumination) != pMatB->GetImage(eMaterialTexture_Illumination)) {
            return pMatA->GetImage(eMaterialTexture_Illumination) < pMatB->GetImage(eMaterialTexture_Illumination);
        }

        //////////////////////////
        // Vertex buffer
        if (apObjectA->GetVertexBuffer() != apObjectB->GetVertexBuffer()) {
            return apObjectA->GetVertexBuffer() < apObjectB->GetVertexBuffer();
        }

        //////////////////////////
        // Matrix
        if (apObjectA->GetModelMatrixPtr() != apObjectB->GetModelMatrixPtr()) {
            return apObjectA->GetModelMatrixPtr() < apObjectB->GetModelMatrixPtr();
        }

        //////////////////////////
        // Illumination amount
        return apObjectA->GetIlluminationAmount() < apObjectB->GetIlluminationAmount();
    }

    cRenderList::cRenderList() {
        m_frameTime = 0;
        m_frustum = NULL;
    }

    cRenderList::~cRenderList() {
    }

    void cRenderList::BeginAndReset(float frameTime, cFrustum* frustum) {
        m_frameTime = frameTime;
        m_frustum = frustum;

        // Use resize instead of clear, because that way capacity is preserved and allocation is never
        // needed unless there is a need to increase the vector size.
        m_occlusionQueryObjects.resize(0);
        m_transObjects.resize(0);
        m_decalObjects.resize(0);
        m_solidObjects.resize(0);
        m_illumObjects.resize(0);
        m_lights.resize(0);
        m_fogAreas.resize(0);

        for (int i = 0; i < eRenderListType_LastEnum; ++i) {
            m_sortedArrays[i].resize(0);
        }
    }

    void cRenderList::End(tRenderListCompileFlag aFlags) {
        auto sortRenderType = [&](eRenderListType type) {
            switch (type) {
                case eRenderListType_Translucent:
                    m_sortedArrays[type] = m_transObjects;
                    break;
                case eRenderListType_Decal:
                    m_sortedArrays[type] = m_decalObjects;
                    break;
                case eRenderListType_Illumination:
                    m_sortedArrays[type] = m_illumObjects;
                    break;
                default:
                    m_sortedArrays[type] = m_solidObjects;
                    break;
            }
            switch (type) {
                case eRenderListType_Z:
                    std::sort(m_sortedArrays[type].begin(), m_sortedArrays[type].end(), SortFunc_Z);
                    break;
                case eRenderListType_Diffuse:
                    std::sort(m_sortedArrays[type].begin(), m_sortedArrays[type].end(), SortFunc_Diffuse);
                    break;
                case eRenderListType_Translucent:
                    std::sort(m_sortedArrays[type].begin(), m_sortedArrays[type].end(), SortFunc_Translucent);
                    break;
                case eRenderListType_Decal:
                    std::sort(m_sortedArrays[type].begin(), m_sortedArrays[type].end(), SortFunc_Decal);
                    break;
                case eRenderListType_Illumination:
                    std::sort(m_sortedArrays[type].begin(), m_sortedArrays[type].end(), SortFunc_Illumination);
                    break;
                default:
                    break;
            }
        };

        if (aFlags & eRenderListCompileFlag_Z) {
            sortRenderType(eRenderListType_Z);
        }
        if (aFlags & eRenderListCompileFlag_Diffuse) {
            sortRenderType(eRenderListType_Diffuse);
        }
        if (aFlags & eRenderListCompileFlag_Decal) {
            sortRenderType(eRenderListType_Decal);
        }
        if (aFlags & eRenderListCompileFlag_Illumination) {
            sortRenderType(eRenderListType_Illumination);
        }
		if(aFlags & eRenderListCompileFlag_FogArea) {
			std::sort(m_fogAreas.begin(), m_fogAreas.end(), [](cFogArea* a, cFogArea* b) {
				return a->GetViewSpaceZ() < b->GetViewSpaceZ();
			});
		}
        if (aFlags & eRenderListCompileFlag_Translucent) {
            ////////////////////////////////////
            // Setup variables
            cPlanef nearestSurfacePlane;
            bool bHasLargeSurfacePlane = false;
            float fClosestDist = 0;
            iRenderable* pLargeSurfaceObject = NULL;

            ////////////////////////////////////
            // Find the neareest surface plane
            for (size_t i = 0; i < m_transObjects.size(); ++i) {
                /////////////////////////////////
                // Check so object is of right type
                iRenderable* pObject = m_transObjects[i];
                if (pObject->GetRenderType() != eRenderableType_SubMesh) {
                    continue;
				}

                cMaterial* pMat = pObject->GetMaterial();
                if (pMat->GetLargeTransperantSurface() == false) {
                    continue;
				}

                /////////////////////////////////
                // Check so sub mesh is one sided
                cSubMeshEntity* pSubEnt = static_cast<cSubMeshEntity*>(pObject);
                cSubMesh* pSubMesh = pSubEnt->GetSubMesh();
                if (pSubMesh->GetIsOneSided() == false) {
                    continue;
				}

                cVector3f vSurfaceNormal =
                    cMath::Vector3Normalize(cMath::MatrixMul3x3(pSubEnt->GetWorldMatrix(), pSubMesh->GetOneSidedNormal()));
                cVector3f vSurfacePos = cMath::MatrixMul(pSubEnt->GetWorldMatrix(), pSubMesh->GetOneSidedPoint());

                /////////////////////////////////
                // Make sure it does not face away from frustum.
                float fDot = cMath::Vector3Dot(vSurfacePos - m_frustum->GetOrigin(), vSurfaceNormal);
                if (fDot >= 0) {
                    continue;
				}

                /////////////////////////////////
                // Create surface normal and check if nearest.
                cPlanef surfacePlane;
                surfacePlane.FromNormalPoint(vSurfaceNormal, vSurfacePos);

                float fDist = cMath::PlaneToPointDist(surfacePlane, m_frustum->GetOrigin());
                if (fDist < fClosestDist || bHasLargeSurfacePlane == false) {
                    bHasLargeSurfacePlane = true;
                    fClosestDist = fDist;
                    nearestSurfacePlane = surfacePlane;
                    pLargeSurfaceObject = pObject;
                }
            }

            ////////////////////////////////////
            // Check if objects are above or below surface plane
            if (bHasLargeSurfacePlane) {
                for (size_t i = 0; i < m_transObjects.size(); ++i) {
                    iRenderable* pObject = m_transObjects[i];

                    // If this is large plane, then set value to 0
                    if (pObject == pLargeSurfaceObject) {
                        pObject->SetLargePlaneSurfacePlacement(0);
                        continue;
                    }

                    float fDist = cMath::PlaneToPointDist(nearestSurfacePlane, pObject->GetBoundingVolume()->GetWorldCenter());
                    pObject->SetLargePlaneSurfacePlacement(fDist < 0 ? -1 : 1);
                }
            } else {
                for (size_t i = 0; i < m_transObjects.size(); ++i) {
                    iRenderable* pObject = m_transObjects[i];
                    pObject->SetLargePlaneSurfacePlacement(0);
                }
            }
            sortRenderType(eRenderListType_Translucent);
        }
    }

    void cRenderList::Setup(float afFrameTime, cFrustum* apFrustum) {
        m_frameTime = afFrameTime;
        m_frustum = apFrustum;
    }

    void cRenderList::AddObject(iRenderable* apObject) {
        ASSERT(m_frustum && "call begin with frustum");

        eRenderableType renderType = apObject->GetRenderType();

        ////////////////////////////////////////
        // Update material, if not already done this frame
        cMaterial* pMaterial = apObject->GetMaterial();
        const bool isValidMaterial = pMaterial && pMaterial->Descriptor().m_id != MaterialID::Unknown;
        if (pMaterial && pMaterial->GetRenderFrameCount() != iRenderer::GetRenderFrameCount()) {
            pMaterial->SetRenderFrameCount(iRenderer::GetRenderFrameCount());
            pMaterial->UpdateBeforeRendering(m_frameTime);
        }

        ////////////////////////////////////////
        // Update per frame things, if not done yet.
        if (apObject->GetRenderFrameCount() != iRenderer::GetRenderFrameCount()) {
            apObject->SetRenderFrameCount(iRenderer::GetRenderFrameCount());
            apObject->UpdateGraphicsForFrame(m_frameTime);
        }

        ////////////////////////////////////////
        // Update per viewport specific and set amtrix point
        // Skip this for non-decal translucent! This is because the water rendering might mess it up otherwise!
        if (!isValidMaterial || !cMaterial::IsTranslucent(pMaterial->Descriptor().m_id) || pMaterial->Descriptor().m_id == MaterialID::Decal) {
            // skip rendering if the update return false
            if (apObject->UpdateGraphicsForViewport(m_frustum, m_frameTime) == false) {
                return;
            }

            apObject->SetModelMatrixPtr(apObject->GetModelMatrix(m_frustum));
        }
        // Only set a matrix used for sorting. Calculate the proper in the trans rendering!
        else {
            apObject->SetModelMatrixPtr(apObject->GetModelMatrix(NULL));
        }

        ////////////////////////////////////////
        // Calculate the View Z value
        //  For transparent and non decals!
        if (isValidMaterial && cMaterial::IsTranslucent(pMaterial->Descriptor().m_id) && pMaterial->Descriptor().m_id != MaterialID::Decal) {
            cVector3f vIntersectionPos;
            cBoundingVolume* pBV = apObject->GetBoundingVolume();

            // If there is an intersection (which happens unless inside), use that. Else use world center
            if (cMath::CheckAABBLineIntersection(
                    pBV->GetMin(), pBV->GetMax(), m_frustum->GetOrigin(), pBV->GetWorldCenter(), &vIntersectionPos, NULL) == false) {
                vIntersectionPos = pBV->GetWorldCenter();
            }

            vIntersectionPos = cMath::MatrixMul(m_frustum->GetViewMatrix(), vIntersectionPos);
            apObject->SetViewSpaceZ(vIntersectionPos.z);
        } else {
            cVector3f vCameraPos = cMath::MatrixMul(m_frustum->GetViewMatrix(), apObject->GetBoundingVolume()->GetWorldCenter());
            apObject->SetViewSpaceZ(vCameraPos.z);
        }

        //////////////////////////////
        // If objects uses occlusion queries, add it as such
        if (apObject->UsesOcclusionQuery()) {
            m_occlusionQueryObjects.push_back(apObject);
        }

        //////////////////////////////
        // Light, add to special list
        if (renderType == eRenderableType_Light) {
            m_lights.push_back(static_cast<iLight*>(apObject));
        }
        //////////////////////////////
        // Fog area, add to special list
        if (renderType == eRenderableType_FogArea) {
            m_fogAreas.push_back(static_cast<cFogArea*>(apObject));
        }
        //////////////////////////////
        // GuiSet, add to special list
        else if (renderType == eRenderableType_GuiSet) {
            // TODO....
        }
        ///////////////////////////
        // Normal addition
        else {
            if (pMaterial == NULL)
                return; // Skip if it has no material...

            ////////////////////////
            // Transparent
            if (cMaterial::IsTranslucent(pMaterial->Descriptor().m_id)) {
                if (pMaterial->Descriptor().m_id == MaterialID::Decal) {
                    m_decalObjects.push_back(apObject);
                } else {
                    m_transObjects.push_back(apObject);
                }
            }
            ////////////////////////
            // Solid
            else {
                m_solidObjects.push_back(apObject);
                if (pMaterial->GetImage(eMaterialTexture_Illumination) && apObject->GetIlluminationAmount() > 0) {
                    m_illumObjects.push_back(apObject);
                }
            }
        }
    }

    //-----------------------------------------------------------------------

    void cRenderList::Compile(tRenderListCompileFlag aFlags) {
        auto sortRenderType = [&](eRenderListType type) {
            switch (type) {
                case eRenderListType_Translucent:
                    m_sortedArrays[type] = m_transObjects;
                    break;
                case eRenderListType_Decal:
                    m_sortedArrays[type] = m_decalObjects;
                    break;
                case eRenderListType_Illumination:
                    m_sortedArrays[type] = m_illumObjects;
                    break;
                default:
                    m_sortedArrays[type] = m_solidObjects;
                    break;
            }
            switch (type) {
                case eRenderListType_Z:
                    std::sort(m_sortedArrays[type].begin(), m_sortedArrays[type].end(), SortFunc_Z);
                    break;
                case eRenderListType_Diffuse:
                    std::sort(m_sortedArrays[type].begin(), m_sortedArrays[type].end(), SortFunc_Diffuse);
                    break;
                case eRenderListType_Translucent:
                    std::sort(m_sortedArrays[type].begin(), m_sortedArrays[type].end(), SortFunc_Translucent);
                    break;
                case eRenderListType_Decal:
                    std::sort(m_sortedArrays[type].begin(), m_sortedArrays[type].end(), SortFunc_Decal);
                    break;
                case eRenderListType_Illumination:
                    std::sort(m_sortedArrays[type].begin(), m_sortedArrays[type].end(), SortFunc_Illumination);
                    break;
                default:
                    break;
            }
        };

        if (aFlags & eRenderListCompileFlag_Z) {
            sortRenderType(eRenderListType_Z);
        }
        if (aFlags & eRenderListCompileFlag_Diffuse) {
            sortRenderType(eRenderListType_Diffuse);
        }
        if (aFlags & eRenderListCompileFlag_Decal) {
            sortRenderType(eRenderListType_Decal);
        }
        if (aFlags & eRenderListCompileFlag_Illumination) {
            sortRenderType(eRenderListType_Illumination);
        }
		if(aFlags & eRenderListCompileFlag_FogArea) {
			std::sort(m_fogAreas.begin(), m_fogAreas.end(), [](cFogArea* a, cFogArea* b) {
				return a->GetViewSpaceZ() < b->GetViewSpaceZ();
			});
		}
        if (aFlags & eRenderListCompileFlag_Translucent) {
            ////////////////////////////////////
            // Setup variables
            cPlanef nearestSurfacePlane;
            bool bHasLargeSurfacePlane = false;
            float fClosestDist = 0;
            iRenderable* pLargeSurfaceObject = NULL;

            ////////////////////////////////////
            // Find the neareest surface plane
            for (size_t i = 0; i < m_transObjects.size(); ++i) {
                /////////////////////////////////
                // Check so object is of right type
                iRenderable* pObject = m_transObjects[i];
                if (pObject->GetRenderType() != eRenderableType_SubMesh) {
                    continue;
				}

                cMaterial* pMat = pObject->GetMaterial();
                if (pMat->GetLargeTransperantSurface() == false) {
                    continue;
				}

                /////////////////////////////////
                // Check so sub mesh is one sided
                cSubMeshEntity* pSubEnt = static_cast<cSubMeshEntity*>(pObject);
                cSubMesh* pSubMesh = pSubEnt->GetSubMesh();
                if (pSubMesh->GetIsOneSided() == false) {
                    continue;
				}

                cVector3f vSurfaceNormal =
                    cMath::Vector3Normalize(cMath::MatrixMul3x3(pSubEnt->GetWorldMatrix(), pSubMesh->GetOneSidedNormal()));
                cVector3f vSurfacePos = cMath::MatrixMul(pSubEnt->GetWorldMatrix(), pSubMesh->GetOneSidedPoint());

                /////////////////////////////////
                // Make sure it does not face away from frustum.
                float fDot = cMath::Vector3Dot(vSurfacePos - m_frustum->GetOrigin(), vSurfaceNormal);
                if (fDot >= 0) {
                    continue;
				}

                /////////////////////////////////
                // Create surface normal and check if nearest.
                cPlanef surfacePlane;
                surfacePlane.FromNormalPoint(vSurfaceNormal, vSurfacePos);

                float fDist = cMath::PlaneToPointDist(surfacePlane, m_frustum->GetOrigin());
                if (fDist < fClosestDist || bHasLargeSurfacePlane == false) {
                    bHasLargeSurfacePlane = true;
                    fClosestDist = fDist;
                    nearestSurfacePlane = surfacePlane;
                    pLargeSurfaceObject = pObject;
                }
            }

            ////////////////////////////////////
            // Check if objects are above or below surface plane
            if (bHasLargeSurfacePlane) {
                for (size_t i = 0; i < m_transObjects.size(); ++i) {
                    iRenderable* pObject = m_transObjects[i];

                    // If this is large plane, then set value to 0
                    if (pObject == pLargeSurfaceObject) {
                        pObject->SetLargePlaneSurfacePlacement(0);
                        continue;
                    }

                    float fDist = cMath::PlaneToPointDist(nearestSurfacePlane, pObject->GetBoundingVolume()->GetWorldCenter());
                    pObject->SetLargePlaneSurfacePlacement(fDist < 0 ? -1 : 1);
                }
            } else {
                for (size_t i = 0; i < m_transObjects.size(); ++i) {
                    iRenderable* pObject = m_transObjects[i];
                    pObject->SetLargePlaneSurfacePlacement(0);
                }
            }
            sortRenderType(eRenderListType_Translucent);
        }
    }

    void cRenderList::Clear() {
        // Use resize instead of clear, because that way capacity is preserved and allocation is never
        // needed unless there is a need to increase the vector size.

        m_occlusionQueryObjects.resize(0);
        m_transObjects.resize(0);
        m_decalObjects.resize(0);
        m_solidObjects.resize(0);
        m_illumObjects.resize(0);
        m_lights.resize(0);
        m_fogAreas.resize(0);

        for (int i = 0; i < eRenderListType_LastEnum; ++i) {
            m_sortedArrays[i].resize(0);
        }
    }

    void cRenderList::PrintAllObjects() {
        Log("---------------------------------\n");
        Log("------ RENDER LIST CONTENTS -----\n");

        Log("Trans Objects:\n");
        for (size_t i = 0; i < m_transObjects.size(); ++i)
            Log(" '%s' ViewspaceZ: %f LargeSurfacePlacement: %d Mat: '%s' RenderCount: %d\n",
                m_transObjects[i]->GetName().c_str(),
                m_transObjects[i]->GetViewSpaceZ(),
                m_transObjects[i]->GetLargePlaneSurfacePlacement(),
                m_transObjects[i]->GetMaterial()->GetName().c_str(),
                m_transObjects[i]->GetRenderFrameCount());

        Log("Solid Objects:\n");
        for (size_t i = 0; i < m_solidObjects.size(); ++i)
            Log(" '%s' Mat: '%s' RenderCount: %d\n",
                m_solidObjects[i]->GetName().c_str(),
                m_solidObjects[i]->GetMaterial()->GetName().c_str(),
                m_solidObjects[i]->GetRenderFrameCount());

        Log("Decal Objects:\n");
        for (size_t i = 0; i < m_decalObjects.size(); ++i)
            Log(" '%s' Mat: '%s' RenderCount: %d\n",
                m_decalObjects[i]->GetName().c_str(),
                m_decalObjects[i]->GetMaterial()->GetName().c_str(),
                m_solidObjects[i]->GetRenderFrameCount());

        Log("Illum Objects:\n");
        for (size_t i = 0; i < m_illumObjects.size(); ++i)
            Log(" '%s' Mat: '%s' RenderCount: %d\n",
                m_illumObjects[i]->GetName().c_str(),
                m_illumObjects[i]->GetMaterial()->GetName().c_str(),
                m_solidObjects[i]->GetRenderFrameCount());

        Log("---------------------------------\n");
    }

    bool cRenderList::ArrayHasObjects(eRenderListType aType) {
        return m_sortedArrays[aType].empty() == false;
    }

    std::span<iRenderable*> cRenderList::GetRenderableItems(eRenderListType aType) {
		if(m_sortedArrays[aType].empty()) {
			return std::span<iRenderable*>();
		}
        return std::span<iRenderable*>(m_sortedArrays[aType]);
    }

    std::span<iRenderable*> cRenderList::GetOcclusionQueryItems() {
        return std::span<iRenderable*>(m_occlusionQueryObjects);
    }

    std::span<cFogArea*> cRenderList::GetFogAreas() {
		if(m_fogAreas.empty()) {
			return std::span<cFogArea*>();
		}
        return std::span<cFogArea*>(m_fogAreas);
    }

    std::span<iLight*> cRenderList::GetLights() {
        if(m_lights.empty()) {
			return std::span<iLight*>();
		}
        return std::span<iLight*>(m_lights);
    }
} // namespace hpl
