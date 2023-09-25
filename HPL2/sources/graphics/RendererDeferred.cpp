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

#include "graphics/RendererDeferred.h"


#include "engine/Event.h"
#include "engine/Interface.h"
#include "graphics/ForgeHandles.h"
#include "graphics/ImmediateDrawBatch.h"
#include "graphics/MaterialResource.h"
#include "math/cFrustum.h"
#include "scene/ParticleEmitter.h"
#include "scene/Viewport.h"
#include "windowing/NativeWindow.h"

#include <cstdint>
#include <graphics/Enum.h>

#include "graphics/GraphicsTypes.h"
#include "graphics/Image.h"
#include "graphics/RenderTarget.h"
#include "impl/LegacyVertexBuffer.h"
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
#include "graphics/RenderList.h"
#include "graphics/Renderable.h"
#include "graphics/SubMesh.h"
#include "graphics/Texture.h"
#include "graphics/TextureCreator.h"
#include "graphics/VertexBuffer.h"

#include "resources/MeshManager.h"
#include "resources/Resources.h"
#include "resources/TextureManager.h"

#include "scene/BillBoard.h"
#include "scene/Camera.h"
#include "scene/FogArea.h"
#include "scene/Light.h"
#include "scene/LightBox.h"
#include "scene/LightSpot.h"
#include "scene/MeshEntity.h"
#include "scene/RenderableContainer.h"
#include "scene/World.h"

#include <algorithm>
#include <array>
#include <functional>
#include <iterator>
#include <memory>
#include <set>
#include <unordered_map>
#include <utility>

#include "Common_3/Utilities/Math/MathTypes.h"
#include "Common_3/Resources/ResourceLoader/Interfaces/IResourceLoader.h"
#include "Common_3/Graphics/Interfaces/IGraphics.h"
#include "FixPreprocessor.h"
#include <folly/FixedString.h>
#include <folly/hash/Hash.h>
#include <folly/small_vector.h>

namespace hpl {

    static constexpr float kLightRadiusMul_High = 1.08f;
    static constexpr float kLightRadiusMul_Medium = 1.12f;
    static constexpr float kLightRadiusMul_Low = 1.2f;
    static constexpr uint32_t kMaxStencilBitsUsed = 8;
    static constexpr uint32_t kStartStencilBit = 0;
    static constexpr uint32_t MinLargeLightNormalizedArea = 0.2f * 0.2f;

    static constexpr uint32_t SSAOImageSizeDiv = 2;
    static constexpr uint32_t SSAONumOfSamples = 8;

    namespace detail {
        void cmdDefaultLegacyGeomBinding(Cmd* cmd, const ForgeRenderer::Frame& frame, LegacyVertexBuffer::GeometryBinding& binding) {
            folly::small_vector<Buffer*, 16> vbBuffer;
            folly::small_vector<uint64_t, 16> vbOffsets;
            folly::small_vector<uint32_t, 16> vbStride;

            for (auto& element : binding.m_vertexElement) {
                vbBuffer.push_back(element.element->m_buffer.m_handle);
                vbOffsets.push_back(element.offset);
                vbStride.push_back(element.element->Stride());
                frame.m_resourcePool->Push(element.element->m_buffer);
            }
            frame.m_resourcePool->Push(*binding.m_indexBuffer.element);

            cmdBindVertexBuffer(cmd, binding.m_vertexElement.size(), vbBuffer.data(), vbStride.data(), vbOffsets.data());
            cmdBindIndexBuffer(cmd, binding.m_indexBuffer.element->m_handle, INDEX_TYPE_UINT32, binding.m_indexBuffer.offset);
        }

        uint32_t resolveMaterialID(cMaterial::TextureAntistropy anisotropy, eTextureWrap wrap, eTextureFilter filter) {
            const uint32_t anisotropyGroup =
                (static_cast<uint32_t>(eTextureFilter_LastEnum) * static_cast<uint32_t>(eTextureWrap_LastEnum)) *
                static_cast<uint32_t>(anisotropy);
            return anisotropyGroup +
                ((static_cast<uint32_t>(wrap) * static_cast<uint32_t>(eTextureFilter_LastEnum)) + static_cast<uint32_t>(filter));
        }

        struct DeferredLight {
        public:
            DeferredLight() = default;
            cRect2l m_clipRect;
            cMatrixf m_mtxViewSpaceRender;
            cMatrixf m_mtxViewSpaceTransform;
            cRendererDeferred::ShadowMapData* m_shadowMapData = nullptr;
            iLight* m_light = nullptr;
            bool m_insideNearPlane = false;

            inline float getArea() {
                return m_clipRect.w * m_clipRect.h;
            }
        };

        static inline std::vector<cRendererDeferred::FogRendererData> createFogRenderData(
            std::span<cFogArea*> fogAreas, cFrustum* apFrustum) {
            std::vector<cRendererDeferred::FogRendererData> fogRenderData;
            fogRenderData.reserve(fogAreas.size());
            for (const auto& fogArea : fogAreas) {
                auto& fogData = fogRenderData.emplace_back(
                    cRendererDeferred::FogRendererData{ fogArea, false, cVector3f(0.0f), cMatrixf(cMatrixf::Identity) });
                fogData.m_fogArea = fogArea;
                fogData.m_mtxInvBoxSpace = cMath::MatrixInverse(*fogArea->GetModelMatrixPtr());
                fogData.m_boxSpaceFrustumOrigin = cMath::MatrixMul(fogData.m_mtxInvBoxSpace, apFrustum->GetOrigin());
                fogData.m_insideNearFrustum = ([&]() {
                    std::array<cVector3f, 4> nearPlaneVtx;
                    cVector3f min(-0.5f);
                    cVector3f max(0.5f);

                    for (size_t i = 0; i < nearPlaneVtx.size(); ++i) {
                        nearPlaneVtx[i] = cMath::MatrixMul(fogData.m_mtxInvBoxSpace, apFrustum->GetVertex(i));
                    }
                    for (size_t i = 0; i < nearPlaneVtx.size(); ++i) {
                        if (cMath::CheckPointInAABBIntersection(nearPlaneVtx[i], min, max)) {
                            return true;
                        }
                    }
                    //////////////////////////////
                    // Check if near plane points intersect with box
                    if (cMath::CheckPointsAABBPlanesCollision(nearPlaneVtx.data(), 4, min, max) != eCollision_Outside) {
                        return true;
                    }
                    return false;
                })();
            }
            return fogRenderData;
        }

        static inline bool SetupShadowMapRendering(
            std::vector<iRenderable*>& shadowCasters, cWorld* world, cFrustum* frustum, iLight* light, std::span<cPlanef> clipPlanes) {
            /////////////////////////
            // Get light data
            if (light->GetLightType() != eLightType_Spot)
                return false; // Only support spot lights for now...

            cLightSpot* pSpotLight = static_cast<cLightSpot*>(light);
            cFrustum* pLightFrustum = pSpotLight->GetFrustum();

            std::function<void(iRenderableContainerNode * apNode, eCollision aPrevCollision)> walkShadowCasters;
            walkShadowCasters = [&](iRenderableContainerNode* apNode, eCollision aPrevCollision) {
                ///////////////////////////////////////
                // Get frustum collision, if previous was inside, then this is too!
                eCollision frustumCollision = aPrevCollision == eCollision_Inside ? aPrevCollision : pLightFrustum->CollideNode(apNode);

                ///////////////////////////////////
                // Check if visible but always iterate the root node!
                if (apNode->GetParent()) {
                    if (frustumCollision == eCollision_Outside) {
                        return;
                    }
                    if (rendering::detail::IsRenderableNodeIsVisible(apNode, clipPlanes) == false) {
                        return;
                    }
                }

                ////////////////////////
                // Iterate children
                for (auto& childNode : apNode->GetChildNodes()) {
                    walkShadowCasters(childNode, frustumCollision);
                }

                /////////////////////////////
                // Iterate objects
                for (auto& object : apNode->GetObjects()) {
                    // Check so visible and shadow caster
                    if (rendering::detail::IsObjectIsVisible(object, eRenderableFlag_ShadowCaster, clipPlanes) == false ||
                        object->GetMaterial() == NULL || cMaterial::IsTranslucent(object->GetMaterial()->Descriptor().m_id)) {
                        continue;
                    }

                    /////////
                    // Check if in frustum
                    if (frustumCollision != eCollision_Inside &&
                        frustum->CollideBoundingVolume(object->GetBoundingVolume()) == eCollision_Outside) {
                        continue;
                    }

                    // Calculate the view space Z (just a squared distance)
                    object->SetViewSpaceZ(cMath::Vector3DistSqr(object->GetBoundingVolume()->GetWorldCenter(), frustum->GetOrigin()));

                    // Add to list
                    shadowCasters.push_back(object);
                }
            };

            /////////////////////////
            // If culling by occlusion, skip rest of function
            if (light->GetOcclusionCullShadowCasters()) {
                return true;
            }

            /////////////////////////
            // Get objects to render

            // Clear list
            shadowCasters.resize(0); // No clear, so we keep all in memory.

            // Get the objects
            if (light->GetShadowCastersAffected() & eObjectVariabilityFlag_Dynamic) {
                auto container = world->GetRenderableContainer(eWorldContainerType_Dynamic);
                container->UpdateBeforeRendering();
                walkShadowCasters(container->GetRoot(), eCollision_Outside);
            }

            if (light->GetShadowCastersAffected() & eObjectVariabilityFlag_Static) {
                auto container = world->GetRenderableContainer(eWorldContainerType_Static);
                container->UpdateBeforeRendering();
                walkShadowCasters(container->GetRoot(), eCollision_Outside);
            }

            // See if any objects where added.
            if (shadowCasters.empty())
                return false;

            // Sort the list
            std::sort(shadowCasters.begin(), shadowCasters.end(), [](iRenderable* a, iRenderable* b) {
                cMaterial* pMatA = a->GetMaterial();
                cMaterial* pMatB = b->GetMaterial();

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
                // View space z is really just BB dist dis squared, so use "<"
                return a->GetViewSpaceZ() < b->GetViewSpaceZ();
            });

            return true;
        }

        static inline cMatrixf GetLightMtx(const DeferredLight& light) {
            switch (light.m_light->GetLightType()) {
            case eLightType_Point:
                return cMath::MatrixScale(light.m_light->GetRadius() * kLightRadiusMul_Medium);
            case eLightType_Spot:
                {
                    cLightSpot* pLightSpot = static_cast<cLightSpot*>(light.m_light);

                    float fFarHeight = pLightSpot->GetTanHalfFOV() * pLightSpot->GetRadius() * 2.0f;
                    // Note: Aspect might be wonky if there is no gobo.
                    float fFarWidth = fFarHeight * pLightSpot->GetAspect();

                    return cMath::MatrixScale(
                        cVector3f(fFarWidth, fFarHeight, light.m_light->GetRadius())); // x and y = "far plane", z = radius
                }
            case eLightType_Box:
                {
                    cLightBox* pLightBox = static_cast<cLightBox*>(light.m_light);
                    auto mtx = cMath::MatrixScale(pLightBox->GetSize());
                    mtx.SetTranslation(pLightBox->GetWorldPosition());
                    return mtx;
                }
            default:
                break;
            }

            return cMatrixf::Identity;
        }

        static inline bool SortDeferredLightDefault(const DeferredLight* a, const DeferredLight* b) {
            iLight* pLightA = a->m_light;
            iLight* pLightB = b->m_light;

            //////////////////////////
            // Type
            if (pLightA->GetLightType() != pLightB->GetLightType()) {
                return pLightA->GetLightType() < pLightB->GetLightType();
            }

            //////////////////////////
            // Specular
            int lHasSpecularA = pLightA->GetDiffuseColor().a > 0 ? 1 : 0;
            int lHasSpecularB = pLightB->GetDiffuseColor().a > 0 ? 1 : 0;
            if (lHasSpecularA != lHasSpecularB) {
                return lHasSpecularA < lHasSpecularB;
            }

            ////////////////////////////////
            // Point inside near plane
            if (pLightA->GetLightType() == eLightType_Point) {
                return a->m_insideNearPlane < b->m_insideNearPlane;
            }

            //////////////////////////
            // Gobo
            if (pLightA->GetGoboTexture() != pLightB->GetGoboTexture()) {
                return pLightA->GetGoboTexture() < pLightB->GetGoboTexture();
            }

            //////////////////////////
            // Attenuation
            if (pLightA->GetFalloffMap() != pLightB->GetFalloffMap()) {
                return pLightA->GetFalloffMap() < pLightB->GetFalloffMap();
            }

            //////////////////////////
            // Spot falloff
            if (pLightA->GetLightType() == eLightType_Spot) {
                cLightSpot* pLightSpotA = static_cast<cLightSpot*>(pLightA);
                cLightSpot* pLightSpotB = static_cast<cLightSpot*>(pLightB);

                if (pLightSpotA->GetSpotFalloffMap() != pLightSpotB->GetSpotFalloffMap()) {
                    return pLightSpotA->GetSpotFalloffMap() < pLightSpotB->GetSpotFalloffMap();
                }
            }
            if (pLightA->GetLightType() == eLightType_Box) {
                cLightBox* pBoxLightA = static_cast<cLightBox*>(pLightA);
                cLightBox* pBoxLightB = static_cast<cLightBox*>(pLightB);
                if (pBoxLightA->GetBoxLightPrio() != pBoxLightB->GetBoxLightPrio()) {
                    return pBoxLightA->GetBoxLightPrio() < pBoxLightB->GetBoxLightPrio();
                }
            }

            //////////////////////////
            // Pointer
            return pLightA < pLightB;
        }

        static inline float GetFogAreaVisibilityForObject(
            const cRendererDeferred::FogRendererData& fogData, cFrustum& frustum, iRenderable* apObject) {
            cFogArea* pFogArea = fogData.m_fogArea;

            cVector3f vObjectPos = apObject->GetBoundingVolume()->GetWorldCenter();
            cVector3f vRayDir = vObjectPos - frustum.GetOrigin();
            float fCameraDistance = vRayDir.Length();
            vRayDir = vRayDir / fCameraDistance;

            float fEntryDist, fExitDist;

            auto checkFogAreaIntersection =
                [&fEntryDist,
                 &fExitDist](const cMatrixf& a_mtxInvBoxModelMatrix, const cVector3f& avBoxSpaceRayStart, const cVector3f& avRayDir) {
                    cVector3f vBoxSpaceDir = cMath::MatrixMul3x3(a_mtxInvBoxModelMatrix, avRayDir);

                    bool bFoundIntersection = false;
                    fExitDist = 0;

                    std::array<cVector3f, 6> fBoxPlaneNormals = { {
                        cVector3f(-1, 0, 0), // Left
                        cVector3f(1, 0, 0), // Right

                        cVector3f(0, -1, 0), // Bottom
                        cVector3f(0, 1, 0), // Top

                        cVector3f(0, 0, -1), // Back
                        cVector3f(0, 0, 1), // Front
                    } };

                    ///////////////////////////////////
                    // Iterate the sides of the cube
                    for (auto& planeNormal : fBoxPlaneNormals) {
                        ///////////////////////////////////
                        // Calculate plane intersection
                        float fMul = cMath::Vector3Dot(planeNormal, vBoxSpaceDir);
                        if (fabs(fMul) < 0.0001f) {
                            continue;
                        }
                        float fNegDist = -(cMath::Vector3Dot(planeNormal, avBoxSpaceRayStart) + 0.5f);

                        float fT = fNegDist / fMul;
                        if (fT < 0)
                            continue;
                        cVector3f vAbsNrmIntersect = cMath::Vector3Abs(vBoxSpaceDir * fT + avBoxSpaceRayStart);

                        ///////////////////////////////////
                        // Check if the intersection is inside the cube
                        if (cMath::Vector3LessEqual(vAbsNrmIntersect, cVector3f(0.5001f))) {
                            //////////////////////
                            // First intersection
                            if (bFoundIntersection == false) {
                                fEntryDist = fT;
                                fExitDist = fT;
                                bFoundIntersection = true;
                            }
                            //////////////////////
                            // There has already been a intersection.
                            else {
                                fEntryDist = cMath::Min(fEntryDist, fT);
                                fExitDist = cMath::Max(fExitDist, fT);
                            }
                        }
                    }

                    if (fExitDist < 0)
                        return false;

                    return bFoundIntersection;
                };
            if (checkFogAreaIntersection(fogData.m_mtxInvBoxSpace, fogData.m_boxSpaceFrustumOrigin, vRayDir) == false) {
                return 1.0f;
            }

            if (fogData.m_insideNearFrustum == false && fCameraDistance < fEntryDist) {
                return 1.0f;
            }

            //////////////////////////////
            // Calculate the distance the ray travels in the fog
            float fFogDist;
            if (fogData.m_insideNearFrustum) {
                if (pFogArea->GetShowBacksideWhenInside())
                    fFogDist = cMath::Min(fExitDist, fCameraDistance);
                else
                    fFogDist = fCameraDistance;
            } else {
                if (pFogArea->GetShowBacksideWhenOutside())
                    fFogDist = cMath::Min(fExitDist - fEntryDist, fCameraDistance - fEntryDist);
                else
                    fFogDist = fCameraDistance - fEntryDist;
            }

            //////////////////////////////
            // Calculate the alpha
            if (fFogDist <= 0)
                return 1.0f;

            float fFogStart = pFogArea->GetStart();
            float fFogEnd = pFogArea->GetEnd();
            float fFogAlpha = 1 - pFogArea->GetColor().a;

            if (fFogDist < fFogStart)
                return 1.0f;

            if (fFogDist > fFogEnd)
                return fFogAlpha;

            float fAlpha = (fFogDist - fFogStart) / (fFogEnd - fFogStart);
            if (pFogArea->GetFalloffExp() != 1)
                fAlpha = powf(fAlpha, pFogArea->GetFalloffExp());

            return (1.0f - fAlpha) + fFogAlpha * fAlpha;
        }
        /**
         * Calculates matrices for both rendering shape and the transformation
         * \param a_mtxDestRender This has a scale based on radius and radius mul. The mul is to make sure that the shape covers the while
         * light. \param a_mtxDestTransform A simple view space transform for the light. \param afRadiusMul
         */
        static inline void SetupLightMatrix(
            cMatrixf& a_mtxDestRender, cMatrixf& a_mtxDestTransform, iLight* apLight, cFrustum* apFrustum, float afRadiusMul) {
            ////////////////////////////
            // Point Light
            if (apLight->GetLightType() == eLightType_Point) {
                a_mtxDestRender =
                    cMath::MatrixScale(apLight->GetRadius() * afRadiusMul); // kLightRadiusMul = make sure it encapsulates the light.
                a_mtxDestTransform = cMath::MatrixMul(apFrustum->GetViewMatrix(), apLight->GetWorldMatrix());
                a_mtxDestRender = cMath::MatrixMul(a_mtxDestTransform, a_mtxDestRender);
            }
            ////////////////////////////
            // Spot Light
            else if (apLight->GetLightType() == eLightType_Spot) {
                cLightSpot* pLightSpot = static_cast<cLightSpot*>(apLight);

                float fFarHeight = pLightSpot->GetTanHalfFOV() * pLightSpot->GetRadius() * 2.0f;
                // Note: Aspect might be wonky if there is no gobo.
                float fFarWidth = fFarHeight * pLightSpot->GetAspect();

                a_mtxDestRender =
                    cMath::MatrixScale(cVector3f(fFarWidth, fFarHeight, apLight->GetRadius())); // x and y = "far plane", z = radius
                a_mtxDestTransform = cMath::MatrixMul(apFrustum->GetViewMatrix(), apLight->GetWorldMatrix());
                a_mtxDestRender = cMath::MatrixMul(a_mtxDestTransform, a_mtxDestRender);
            }
        }
    } // namespace detail

    enum eDefferredProgramMode { eDefferredProgramMode_Lights, eDefferredProgramMode_Misc, eDefferredProgramMode_LastEnum };

    cRendererDeferred::cRendererDeferred(cGraphics* apGraphics, cResources* apResources)
        : iRenderer("Deferred", apGraphics, apResources) {
        m_hbaoPlusPipeline = std::make_unique<renderer::PassHBAOPlus>();
        ////////////////////////////////////
        // Set up render specific things

        UVector3 shadowSizes[] = {
            UVector3(128, 128, 1), UVector3(256, 256, 1), UVector3(256, 256, 1), UVector3(512, 512, 1), UVector3(1024, 1024, 1)
        };
        int startSize = 2;
        if (mShadowMapResolution == eShadowMapResolution_Medium) {
            startSize = 1;
        } else if (mShadowMapResolution == eShadowMapResolution_Low) {
            startSize = 0;
        }

        m_dissolveImage = mpResources->GetTextureManager()->Create2DImage("core_dissolve.tga", false);
        auto* forgeRenderer = Interface<ForgeRenderer>::Get();

        m_occlusionUniformBuffer.Load([&](Buffer** buffer) {
            BufferLoadDesc desc = {};
            desc.mDesc.mDescriptors = DESCRIPTOR_TYPE_BUFFER;
            desc.mDesc.mMemoryUsage = RESOURCE_MEMORY_USAGE_CPU_TO_GPU;
            desc.mDesc.mFlags = BUFFER_CREATION_FLAG_PERSISTENT_MAP_BIT;
            desc.mDesc.mFirstElement = 0;
            desc.mDesc.mElementCount = MaxOcclusionDescSize;
            desc.mDesc.mStructStride = sizeof(mat4);
            desc.mDesc.mSize = desc.mDesc.mElementCount * desc.mDesc.mStructStride;
            desc.ppBuffer = buffer;
            addResource(&desc, nullptr);
            return true;
        });
        m_materialSet.m_materialUniformBuffer.Load([&](Buffer** buffer) {
            BufferLoadDesc desc = {};
            desc.mDesc.mDescriptors = DESCRIPTOR_TYPE_BUFFER;
            desc.mDesc.mMemoryUsage = RESOURCE_MEMORY_USAGE_CPU_TO_GPU;
            desc.mDesc.mFlags = BUFFER_CREATION_FLAG_PERSISTENT_MAP_BIT;
            desc.mDesc.mFirstElement = 0;
            desc.mDesc.mElementCount = cMaterial::MaxMaterialID;
            desc.mDesc.mStructStride = sizeof(material::UniformMaterialBlock);
            desc.mDesc.mSize = desc.mDesc.mElementCount * desc.mDesc.mStructStride;
            desc.ppBuffer = buffer;
            addResource(&desc, nullptr);
            return true;
        });

        for (auto& buffer : m_objectUniformBuffer) {
            buffer.Load([&](Buffer** buffer) {
                BufferLoadDesc desc = {};
                desc.mDesc.mDescriptors = DESCRIPTOR_TYPE_BUFFER;
                desc.mDesc.mMemoryUsage = RESOURCE_MEMORY_USAGE_CPU_TO_GPU;
                desc.mDesc.mFlags = BUFFER_CREATION_FLAG_PERSISTENT_MAP_BIT;
                desc.mDesc.mFirstElement = 0;
                desc.mDesc.mElementCount = MaxObjectUniforms;
                desc.mDesc.mStructStride = sizeof(cRendererDeferred::UniformObject);
                desc.mDesc.mSize = desc.mDesc.mElementCount * desc.mDesc.mStructStride;
                desc.ppBuffer = buffer;
                addResource(&desc, nullptr);
                return true;
            });
        }

        m_shadowCmpSampler.Load(forgeRenderer->Rend(), [&](Sampler** handle) {
            SamplerDesc miplessLinearSamplerDesc = {};
            miplessLinearSamplerDesc.mMinFilter = FILTER_LINEAR;
            miplessLinearSamplerDesc.mMagFilter = FILTER_LINEAR;
            miplessLinearSamplerDesc.mMipLodBias = 0.f;
            miplessLinearSamplerDesc.mMaxAnisotropy = 0.f;
            miplessLinearSamplerDesc.mCompareFunc = CompareMode::CMP_LEQUAL;
            miplessLinearSamplerDesc.mMipMapMode = MIPMAP_MODE_LINEAR;
            miplessLinearSamplerDesc.mAddressU = ADDRESS_MODE_CLAMP_TO_BORDER;
            miplessLinearSamplerDesc.mAddressV = ADDRESS_MODE_CLAMP_TO_BORDER;
            miplessLinearSamplerDesc.mAddressW = ADDRESS_MODE_CLAMP_TO_BORDER;
            addSampler(forgeRenderer->Rend(), &miplessLinearSamplerDesc, handle);
            return true;
        });
        m_goboSampler.Load(forgeRenderer->Rend(), [&](Sampler** sampler) {
            SamplerDesc samplerDesc = {};
            samplerDesc.mMinFilter = FILTER_LINEAR;
            samplerDesc.mMagFilter = FILTER_LINEAR;
            samplerDesc.mMipLodBias = 0.f;
            samplerDesc.mMaxAnisotropy = 16.f;
            samplerDesc.mMipMapMode = MIPMAP_MODE_LINEAR;
            samplerDesc.mAddressU = ADDRESS_MODE_CLAMP_TO_BORDER;
            samplerDesc.mAddressV = ADDRESS_MODE_CLAMP_TO_BORDER;
            samplerDesc.mAddressW = ADDRESS_MODE_CLAMP_TO_BORDER;
            addSampler(forgeRenderer->Rend(), &samplerDesc, sampler);
            return true;
        });

        m_bilinearSampler.Load(forgeRenderer->Rend(), [&](Sampler** sampler) {
            SamplerDesc bilinearClampDesc = { FILTER_NEAREST,
                                              FILTER_NEAREST,
                                              MIPMAP_MODE_NEAREST,
                                              ADDRESS_MODE_CLAMP_TO_EDGE,
                                              ADDRESS_MODE_CLAMP_TO_EDGE,
                                              ADDRESS_MODE_CLAMP_TO_EDGE };
            addSampler(forgeRenderer->Rend(), &bilinearClampDesc, sampler);
            return true;
        });
        m_samplerPointClampToBorder.Load(forgeRenderer->Rend(), [&](Sampler** sampler) {
            SamplerDesc pointSamplerDesc = {};
            pointSamplerDesc.mMinFilter = FILTER_NEAREST;
            pointSamplerDesc.mMagFilter = FILTER_NEAREST;
            pointSamplerDesc.mMipMapMode = MIPMAP_MODE_NEAREST;
            pointSamplerDesc.mAddressU = ADDRESS_MODE_CLAMP_TO_BORDER;
            pointSamplerDesc.mAddressV = ADDRESS_MODE_CLAMP_TO_BORDER;
            pointSamplerDesc.mAddressW = ADDRESS_MODE_CLAMP_TO_BORDER;
            addSampler(forgeRenderer->Rend(), &pointSamplerDesc, sampler);
            return true;
        });

        m_samplerPointClampToEdge.Load(forgeRenderer->Rend(), [&](Sampler** sampler) {
            SamplerDesc pointSamplerDesc = {};
            pointSamplerDesc.mMinFilter = FILTER_NEAREST;
            pointSamplerDesc.mMagFilter = FILTER_NEAREST;
            pointSamplerDesc.mMipMapMode = MIPMAP_MODE_NEAREST;
            pointSamplerDesc.mAddressU = ADDRESS_MODE_CLAMP_TO_EDGE;
            pointSamplerDesc.mAddressV = ADDRESS_MODE_CLAMP_TO_EDGE;
            pointSamplerDesc.mAddressW = ADDRESS_MODE_CLAMP_TO_EDGE;
            addSampler(forgeRenderer->Rend(), &pointSamplerDesc, sampler);
            return true;
        });
        m_samplerPointWrap.Load(forgeRenderer->Rend(), [&](Sampler** sampler) {
            SamplerDesc pointSamplerDesc = {};
            pointSamplerDesc.mMinFilter = FILTER_NEAREST;
            pointSamplerDesc.mMagFilter = FILTER_NEAREST;
            pointSamplerDesc.mMipMapMode = MIPMAP_MODE_NEAREST;
            pointSamplerDesc.mAddressU = ADDRESS_MODE_REPEAT;
            pointSamplerDesc.mAddressV = ADDRESS_MODE_REPEAT;
            pointSamplerDesc.mAddressW = ADDRESS_MODE_REPEAT;
            addSampler(forgeRenderer->Rend(), &pointSamplerDesc, sampler);
            return true;
        });
        for (auto& buffer : m_perFrameBuffer) {
            buffer.Load([&](Buffer** buffer) {
                BufferLoadDesc desc = {};
                desc.mDesc.mDescriptors = DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                desc.mDesc.mMemoryUsage = RESOURCE_MEMORY_USAGE_CPU_TO_GPU;
                desc.mDesc.mElementCount = 1;
                desc.mDesc.mStructStride = sizeof(cRendererDeferred::UniformPerFrameData);
                desc.mDesc.mSize = sizeof(cRendererDeferred::UniformPerFrameData);
                desc.mDesc.mFlags = BUFFER_CREATION_FLAG_PERSISTENT_MAP_BIT;
                desc.pData = nullptr;
                desc.ppBuffer = buffer;
                addResource(&desc, nullptr);
                return true;
            });
        }

        // prepass
        m_prePassPool.Load(forgeRenderer->Rend(), [&](CmdPool** pool) {
            CmdPoolDesc cmdPoolDesc = {};
            cmdPoolDesc.pQueue = forgeRenderer->GetGraphicsQueue();
            cmdPoolDesc.mTransient = true;
            addCmdPool(forgeRenderer->Rend(), &cmdPoolDesc, pool);
            return true;
        });
        m_prePassCmd.Load(forgeRenderer->Rend(), [&](Cmd** cmd) {
            CmdDesc cmdDesc = {};
            cmdDesc.pPool = m_prePassPool.m_handle;
            addCmd(forgeRenderer->Rend(), &cmdDesc, cmd);
            return true;
        });
        m_prePassFence.Load(forgeRenderer->Rend(), [&](Fence** fence) {
            addFence(forgeRenderer->Rend(), fence);
            return true;
        });

        // hi-z
        {
            m_ShaderHIZGenerate.Load(forgeRenderer->Rend(), [&](Shader** shader) {
                ShaderLoadDesc loadDesc = {};
                loadDesc.mStages[0].pFileName = "generate_hi_z.comp";
                addShader(forgeRenderer->Rend(), &loadDesc, shader);
                return true;
            });

            m_shaderTestOcclusion.Load(forgeRenderer->Rend(), [&](Shader** shader) {
                ShaderLoadDesc loadDesc = {};
                loadDesc.mStages[0].pFileName = "test_AABB_hi_z.comp";
                addShader(forgeRenderer->Rend(), &loadDesc, shader);
                return true;
            });

            m_copyDepthShader.Load(forgeRenderer->Rend(), [&](Shader** shader) {
                ShaderLoadDesc loadDesc = {};
                loadDesc.mStages[0].pFileName = "fullscreen.vert";
                loadDesc.mStages[1].pFileName = "copy_hi_z.frag";
                addShader(forgeRenderer->Rend(), &loadDesc, shader);
                return true;
            });

            m_hiZOcclusionUniformBuffer.Load([&](Buffer** buf) {
                BufferLoadDesc desc = {};
                desc.mDesc.mDescriptors = DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                desc.mDesc.mMemoryUsage = RESOURCE_MEMORY_USAGE_CPU_TO_GPU;
                desc.mDesc.mSize = sizeof(cRendererDeferred::UniformPropBlock);
                desc.mDesc.mFlags = BUFFER_CREATION_FLAG_PERSISTENT_MAP_BIT;
                desc.pData = nullptr;
                desc.ppBuffer = buf;
                addResource(&desc, nullptr);
                return true;
            });
            m_hiZBoundBoxBuffer.Load([&](Buffer** buf) {
                BufferLoadDesc desc = {};
                desc.mDesc.mDescriptors = DESCRIPTOR_TYPE_BUFFER;
                desc.mDesc.mMemoryUsage = RESOURCE_MEMORY_USAGE_CPU_TO_GPU;
                desc.mDesc.mElementCount = cRendererDeferred::MaxObjectTest;
                desc.mDesc.mStructStride = sizeof(float4);
                desc.mDesc.mSize = desc.mDesc.mStructStride * desc.mDesc.mElementCount;
                desc.mDesc.mFlags = BUFFER_CREATION_FLAG_PERSISTENT_MAP_BIT;
                desc.pData = nullptr;
                desc.ppBuffer = buf;
                addResource(&desc, nullptr);
                return true;
            });
            m_occlusionTestBuffer.Load([&](Buffer** buf) {
                BufferLoadDesc desc = {};
                desc.mDesc.mDescriptors = DESCRIPTOR_TYPE_RW_BUFFER;
                desc.mDesc.mMemoryUsage = RESOURCE_MEMORY_USAGE_GPU_TO_CPU;
                desc.mDesc.mElementCount = cRendererDeferred::MaxObjectTest;
                desc.mDesc.mStructStride = sizeof(uint32_t);
                desc.mDesc.mSize = sizeof(uint32_t) * cRendererDeferred::MaxObjectTest;
                desc.mDesc.mFlags = BUFFER_CREATION_FLAG_PERSISTENT_MAP_BIT;
                desc.pData = nullptr;
                desc.ppBuffer = buf;
                addResource(&desc, nullptr);
                return true;
            });
            m_rootSignatureHIZOcclusion.Load(forgeRenderer->Rend(), [&](RootSignature** sig) {
                std::array shaders = { m_ShaderHIZGenerate.m_handle, m_shaderTestOcclusion.m_handle };
                RootSignatureDesc rootSignatureDesc = {};
                const char* pStaticSamplers[] = { "depthSampler" };
                rootSignatureDesc.mStaticSamplerCount = 1;
                rootSignatureDesc.ppStaticSamplers = &m_bilinearSampler.m_handle;
                rootSignatureDesc.ppStaticSamplerNames = pStaticSamplers;
                rootSignatureDesc.ppShaders = shaders.data();
                rootSignatureDesc.mShaderCount = shaders.size();
                addRootSignature(forgeRenderer->Rend(), &rootSignatureDesc, sig);
                return true;
            });
            m_descriptorSetHIZGenerate.Load(forgeRenderer->Rend(), [&](DescriptorSet** set) {
                DescriptorSetDesc setDesc = { m_rootSignatureHIZOcclusion.m_handle,
                                              DESCRIPTOR_UPDATE_FREQ_PER_FRAME,
                                              cRendererDeferred::MaxHiZMipLevels };
                addDescriptorSet(forgeRenderer->Rend(), &setDesc, set);
                return true;
            });
            m_rootSignatureCopyDepth.Load(forgeRenderer->Rend(), [&](RootSignature** signature) {
                std::array shaders = { m_copyDepthShader.m_handle };
                RootSignatureDesc rootSignatureDesc = {};
                const char* pStaticSamplers[] = { "depthSampler" };

                rootSignatureDesc.ppShaders = shaders.data();
                rootSignatureDesc.mShaderCount = shaders.size();
                rootSignatureDesc.mStaticSamplerCount = 1;
                rootSignatureDesc.ppStaticSamplers = &m_samplerPointClampToBorder.m_handle;
                rootSignatureDesc.ppStaticSamplerNames = pStaticSamplers;
                addRootSignature(forgeRenderer->Rend(), &rootSignatureDesc, signature);
                return true;
            });

            m_descriptorCopyDepth.Load(forgeRenderer->Rend(), [&](DescriptorSet** handle) {
                DescriptorSetDesc setDesc = { m_rootSignatureCopyDepth.m_handle, DESCRIPTOR_UPDATE_FREQ_PER_FRAME, 1 };
                addDescriptorSet(forgeRenderer->Rend(), &setDesc, handle);
                return true;
            });
            m_pipelineHIZGenerate.Load(forgeRenderer->Rend(), [&](Pipeline** pipeline) {
                PipelineDesc pipelineDesc = {};
                pipelineDesc.mType = PIPELINE_TYPE_COMPUTE;
                ComputePipelineDesc& computePipelineDesc = pipelineDesc.mComputeDesc;
                computePipelineDesc.pShaderProgram = m_ShaderHIZGenerate.m_handle;
                computePipelineDesc.pRootSignature = m_rootSignatureHIZOcclusion.m_handle;
                addPipeline(forgeRenderer->Rend(), &pipelineDesc, pipeline);
                return true;
            });

            m_pipelineAABBOcclusionTest.Load(forgeRenderer->Rend(), [&](Pipeline** handle) {
                PipelineDesc pipelineDesc = {};
                pipelineDesc.mType = PIPELINE_TYPE_COMPUTE;
                ComputePipelineDesc& computePipelineDesc = pipelineDesc.mComputeDesc;
                computePipelineDesc.pShaderProgram = m_shaderTestOcclusion.m_handle;
                computePipelineDesc.pRootSignature = m_rootSignatureHIZOcclusion.m_handle;
                addPipeline(forgeRenderer->Rend(), &pipelineDesc, handle);
                return true;
            });

            m_descriptorAABBOcclusionTest.Load(forgeRenderer->Rend(), [&](DescriptorSet** handle) {
                DescriptorSetDesc setDesc = { m_rootSignatureHIZOcclusion.m_handle, DESCRIPTOR_UPDATE_FREQ_PER_FRAME, 1 };
                addDescriptorSet(forgeRenderer->Rend(), &setDesc, handle);
                return true;
            });

            m_pipelineCopyDepth.Load(forgeRenderer->Rend(), [&](Pipeline** handle) {
                RasterizerStateDesc rasterStateNoneDesc = {};
                rasterStateNoneDesc.mCullMode = CULL_MODE_NONE;

                DepthStateDesc depthStateDisabledDesc = {};
                depthStateDisabledDesc.mDepthWrite = false;
                depthStateDisabledDesc.mDepthTest = false;

                std::array imageTargets = { TinyImageFormat_R32_SFLOAT };
                PipelineDesc pipelineDesc = {};
                pipelineDesc.mType = PIPELINE_TYPE_GRAPHICS;
                GraphicsPipelineDesc& graphicsPipelineDesc = pipelineDesc.mGraphicsDesc;
                graphicsPipelineDesc.mPrimitiveTopo = PRIMITIVE_TOPO_TRI_LIST;
                graphicsPipelineDesc.mRenderTargetCount = imageTargets.size();
                graphicsPipelineDesc.pColorFormats = imageTargets.data();
                graphicsPipelineDesc.pShaderProgram = m_copyDepthShader.m_handle;
                graphicsPipelineDesc.pRootSignature = m_rootSignatureCopyDepth.m_handle;
                graphicsPipelineDesc.mRenderTargetCount = 1;
                graphicsPipelineDesc.mDepthStencilFormat = TinyImageFormat_UNDEFINED;
                graphicsPipelineDesc.pVertexLayout = NULL;
                graphicsPipelineDesc.mSampleCount = SAMPLE_COUNT_1;
                graphicsPipelineDesc.pRasterizerState = &rasterStateNoneDesc;
                graphicsPipelineDesc.pDepthState = &depthStateDisabledDesc;
                graphicsPipelineDesc.pBlendState = NULL;
                addPipeline(forgeRenderer->Rend(), &pipelineDesc, handle);
                return true;
            });
        }
        {
            m_occlusionReadBackBuffer.Load([&](Buffer** buf) {
                BufferLoadDesc desc = {};
                desc.mDesc.mDescriptors = DESCRIPTOR_TYPE_RW_BUFFER;
                desc.mDesc.mMemoryUsage = RESOURCE_MEMORY_USAGE_GPU_TO_CPU;
                desc.mDesc.mFlags = BUFFER_CREATION_FLAG_PERSISTENT_MAP_BIT;
                desc.mDesc.mElementCount = cRendererDeferred::MaxObjectTest;
                desc.mDesc.mStructStride = sizeof(uint64_t);
                desc.mDesc.mSize = desc.mDesc.mStructStride * desc.mDesc.mElementCount;
                desc.pData = nullptr;
                desc.ppBuffer = buf;
                addResource(&desc, nullptr);
                return true;
            });

            m_shaderOcclusionQuery.Load(forgeRenderer->Rend(), [&](Shader** handle) {
                ShaderLoadDesc loadDesc = {};
                loadDesc.mStages[0].pFileName = "occlusion_empty.vert";
                loadDesc.mStages[1].pFileName = "occlusion_empty.frag";
                addShader(forgeRenderer->Rend(), &loadDesc, handle);
                return true;
            });
            {
                m_rootSignatureOcclusuion.Load(forgeRenderer->Rend(), [&](RootSignature** signature) {
                    std::array shaders = { m_shaderOcclusionQuery.m_handle };
                    RootSignatureDesc rootSignatureDesc = {};
                    rootSignatureDesc.ppShaders = shaders.data();
                    rootSignatureDesc.mShaderCount = shaders.size();
                    addRootSignature(forgeRenderer->Rend(), &rootSignatureDesc, signature);
                    return true;
                });

                m_descriptorOcclusionConstSet.Load(forgeRenderer->Rend(), [&](DescriptorSet** handle) {
                    DescriptorSetDesc setDesc = { m_rootSignatureOcclusuion.m_handle, DESCRIPTOR_UPDATE_FREQ_NONE, 1 };
                    addDescriptorSet(forgeRenderer->Rend(), &setDesc, handle);
                    return true;
                });

                std::array<DescriptorData, 1> params = {};
                params[0].pName = "occlusionObjectBuffer";
                params[0].ppBuffers = &m_occlusionUniformBuffer.m_handle;
                updateDescriptorSet(forgeRenderer->Rend(), 0, m_descriptorOcclusionConstSet.m_handle, params.size(), params.data());
            }

            {
                VertexLayout vertexLayout = {};
#ifndef USE_THE_FORGE_LEGACY
                vertexLayout.mBindingCount = 1;
                vertexLayout.mBindings[0].mStride = sizeof(float3);
#endif
                vertexLayout.mAttribCount = 1;
                vertexLayout.mAttribs[0].mSemantic = SEMANTIC_POSITION;
                vertexLayout.mAttribs[0].mFormat = TinyImageFormat_R32G32B32_SFLOAT;
                vertexLayout.mAttribs[0].mBinding = 0;
                vertexLayout.mAttribs[0].mLocation = 0;
                vertexLayout.mAttribs[0].mOffset = 0;

                RasterizerStateDesc rasterStateNoneDesc = {};
                rasterStateNoneDesc.mCullMode = CULL_MODE_NONE;

                DepthStateDesc depthStateDesc = {};
                depthStateDesc.mDepthWrite = false;
                depthStateDesc.mDepthTest = true;

                PipelineDesc pipelineDesc = {};
                pipelineDesc.mType = PIPELINE_TYPE_GRAPHICS;
                GraphicsPipelineDesc& graphicsPipelineDesc = pipelineDesc.mGraphicsDesc;
                graphicsPipelineDesc.mPrimitiveTopo = PRIMITIVE_TOPO_TRI_LIST;
                graphicsPipelineDesc.pShaderProgram = m_shaderOcclusionQuery.m_handle;
                graphicsPipelineDesc.pRootSignature = m_rootSignatureOcclusuion.m_handle;
                graphicsPipelineDesc.mDepthStencilFormat = DepthBufferFormat;
                graphicsPipelineDesc.pVertexLayout = NULL;
                graphicsPipelineDesc.mSampleCount = SAMPLE_COUNT_1;
                graphicsPipelineDesc.mDepthStencilFormat = DepthBufferFormat;
                graphicsPipelineDesc.pRasterizerState = &rasterStateNoneDesc;
                graphicsPipelineDesc.pDepthState = &depthStateDesc;
                graphicsPipelineDesc.pBlendState = NULL;
                graphicsPipelineDesc.pVertexLayout = &vertexLayout;

                depthStateDesc.mDepthFunc = CMP_ALWAYS;
                m_pipelineMaxOcclusionQuery.Load(forgeRenderer->Rend(), [&](Pipeline** handle) {
                    addPipeline(forgeRenderer->Rend(), &pipelineDesc, handle);
                    return true;
                });
                depthStateDesc.mDepthFunc = CMP_LEQUAL;
                m_pipelineOcclusionQuery.Load(forgeRenderer->Rend(), [&](Pipeline** handle) {
                    addPipeline(forgeRenderer->Rend(), &pipelineDesc, handle);
                    return true;
                });
            }
            m_occlusionQuery.Load(forgeRenderer->Rend(), [&](QueryPool** pool) {
                QueryPoolDesc queryPoolDesc = {};
                queryPoolDesc.mType = QUERY_TYPE_OCCLUSION;
                queryPoolDesc.mQueryCount = MaxQueryPoolSize;
                addQueryPool(forgeRenderer->Rend(), &queryPoolDesc, pool);
                return true;
            });
        }
        // -------------- Fog ----------------------------
        {
            folly::small_vector<Shader*, 10> shaders = {};

            {
                m_fogPass.m_shader.Load(forgeRenderer->Rend(), [&](Shader** shader) {
                    ShaderLoadDesc loadDesc = {};
                    loadDesc.mStages[0].pFileName = "deferred_fog.vert";
                    loadDesc.mStages[1].pFileName = "deferred_fog.frag";
                    addShader(forgeRenderer->Rend(), &loadDesc, shader);
                    return true;
                });

                shaders.push_back(m_fogPass.m_shader.m_handle);
                ASSERT(m_fogPass.m_shader.m_handle && "Shader not loaded");
            }
            {
                m_fogPass.m_fullScreenShader.Load(forgeRenderer->Rend(), [&](Shader** shader) {
                    ShaderLoadDesc loadDesc = {};
                    loadDesc.mStages[0].pFileName = "fullscreen.vert";
                    loadDesc.mStages[1].pFileName = "deferred_fog_fullscreen.frag";
                    addShader(forgeRenderer->Rend(), &loadDesc, shader);
                    return true;
                });
                ASSERT(m_fogPass.m_fullScreenShader.m_handle && "Shader not loaded");
                shaders.push_back(m_fogPass.m_fullScreenShader.m_handle);
            }

            m_fogPass.m_fogRootSignature.Load(forgeRenderer->Rend(), [&](RootSignature** signature) {
                std::array samplerNames = { "nearestSampler" };
                std::array samplers = { m_samplerPointClampToBorder.m_handle };
                RootSignatureDesc rootSignatureDesc = {};
                rootSignatureDesc.ppShaders = shaders.data();
                rootSignatureDesc.mShaderCount = shaders.size();
                rootSignatureDesc.mStaticSamplerCount = samplers.size();
                rootSignatureDesc.ppStaticSamplerNames = samplerNames.data();
                rootSignatureDesc.ppStaticSamplers = samplers.data();
                addRootSignature(forgeRenderer->Rend(), &rootSignatureDesc, signature);
                return true;
            });

            VertexLayout vertexLayout = {};
#ifndef USE_THE_FORGE_LEGACY
            vertexLayout.mBindingCount = 1;
            vertexLayout.mBindings[0].mStride = sizeof(float3);
#endif

            vertexLayout.mAttribCount = 1;
            vertexLayout.mAttribs[0].mSemantic = SEMANTIC_POSITION;
            vertexLayout.mAttribs[0].mFormat = TinyImageFormat_R32G32B32_SFLOAT;
            vertexLayout.mAttribs[0].mBinding = 0;
            vertexLayout.mAttribs[0].mLocation = 0;
            vertexLayout.mAttribs[0].mOffset = 0;

            std::array colorFormats = { ColorBufferFormat };
            {
                RasterizerStateDesc rasterizerStateDesc = {};
                DepthStateDesc depthStateDesc = {};
                depthStateDesc.mDepthTest = true;
                depthStateDesc.mDepthWrite = false;

                BlendStateDesc blendStateDesc{};
                blendStateDesc.mSrcFactors[0] = BC_SRC_ALPHA;
                blendStateDesc.mDstFactors[0] = BC_ONE_MINUS_SRC_ALPHA;
                blendStateDesc.mBlendModes[0] = BM_ADD;
                blendStateDesc.mSrcAlphaFactors[0] = BC_SRC_ALPHA;
                blendStateDesc.mDstAlphaFactors[0] = BC_ONE_MINUS_SRC_ALPHA;
                blendStateDesc.mBlendAlphaModes[0] = BM_ADD;
#ifdef USE_THE_FORGE_LEGACY
                blendStateDesc.mMasks[0] = RED | GREEN | BLUE;
#else
                blendStateDesc.mColorWriteMasks[0] = ColorMask::COLOR_MASK_RED | ColorMask::COLOR_MASK_GREEN | ColorMask::COLOR_MASK_BLUE;
#endif
                blendStateDesc.mRenderTargetMask = BLEND_STATE_TARGET_0;
                blendStateDesc.mIndependentBlend = false;

                rasterizerStateDesc.mFrontFace = FrontFace::FRONT_FACE_CCW;
                depthStateDesc.mDepthFunc = CMP_LEQUAL;
                rasterizerStateDesc.mCullMode = CULL_MODE_FRONT;

                PipelineDesc pipelineDesc = {};
                pipelineDesc.mType = PIPELINE_TYPE_GRAPHICS;
                auto& pipelineSettings = pipelineDesc.mGraphicsDesc;
                pipelineSettings.mPrimitiveTopo = PRIMITIVE_TOPO_TRI_LIST;
                pipelineSettings.mRenderTargetCount = colorFormats.size();
                pipelineSettings.pColorFormats = colorFormats.data();
                pipelineSettings.pDepthState = &depthStateDesc;
                pipelineSettings.pBlendState = &blendStateDesc;
                pipelineSettings.mSampleCount = SAMPLE_COUNT_1;
                pipelineSettings.mDepthStencilFormat = DepthBufferFormat;
                pipelineSettings.mSampleQuality = 0;
                pipelineSettings.pRootSignature = m_fogPass.m_fogRootSignature.m_handle;
                pipelineSettings.pShaderProgram = m_fogPass.m_shader.m_handle;
                pipelineSettings.pRasterizerState = &rasterizerStateDesc;
                pipelineSettings.pVertexLayout = &vertexLayout;
                m_fogPass.m_pipeline.Load(forgeRenderer->Rend(), [&](Pipeline** pipline) {
                    addPipeline(forgeRenderer->Rend(), &pipelineDesc, pipline);
                    return true;
                });

                rasterizerStateDesc.mFrontFace = FrontFace::FRONT_FACE_CW;
                depthStateDesc.mDepthFunc = CMP_ALWAYS;
                m_fogPass.m_pipelineInsideNearFrustum.Load(forgeRenderer->Rend(), [&](Pipeline** pipline) {
                    addPipeline(forgeRenderer->Rend(), &pipelineDesc, pipline);
                    return true;
                });
            }
            m_fogPass.m_fullScreenPipeline.Load(forgeRenderer->Rend(), [&](Pipeline** pipline) {
                DepthStateDesc depthStateDisabledDesc = {};
                depthStateDisabledDesc.mDepthWrite = false;
                depthStateDisabledDesc.mDepthTest = false;

                BlendStateDesc blendStateDesc{};
                blendStateDesc.mSrcFactors[0] = BC_SRC_ALPHA;
                blendStateDesc.mDstFactors[0] = BC_ONE_MINUS_SRC_ALPHA;
                blendStateDesc.mBlendModes[0] = BM_ADD;
                blendStateDesc.mSrcAlphaFactors[0] = BC_SRC_ALPHA;
                blendStateDesc.mDstAlphaFactors[0] = BC_ONE_MINUS_SRC_ALPHA;
                blendStateDesc.mBlendAlphaModes[0] = BM_ADD;
#ifdef USE_THE_FORGE_LEGACY
                blendStateDesc.mMasks[0] = RED | GREEN | BLUE;
#else
                    blendStateDesc.mColorWriteMasks[0] = ColorMask::COLOR_MASK_RED | ColorMask::COLOR_MASK_GREEN | ColorMask::COLOR_MASK_BLUE;
#endif
                blendStateDesc.mRenderTargetMask = BLEND_STATE_TARGET_0;
                blendStateDesc.mIndependentBlend = false;

                RasterizerStateDesc rasterStateNoneDesc = {};
                rasterStateNoneDesc.mCullMode = CULL_MODE_NONE;

                PipelineDesc pipelineDesc = {};
                pipelineDesc.mType = PIPELINE_TYPE_GRAPHICS;
                GraphicsPipelineDesc& graphicsPipelineDesc = pipelineDesc.mGraphicsDesc;
                graphicsPipelineDesc.mPrimitiveTopo = PRIMITIVE_TOPO_TRI_LIST;
                graphicsPipelineDesc.pShaderProgram = m_fogPass.m_fullScreenShader.m_handle;
                graphicsPipelineDesc.pRootSignature = m_fogPass.m_fogRootSignature.m_handle;
                graphicsPipelineDesc.mRenderTargetCount = 1;
                graphicsPipelineDesc.mDepthStencilFormat = TinyImageFormat_UNDEFINED;
                graphicsPipelineDesc.pVertexLayout = NULL;
                graphicsPipelineDesc.pRasterizerState = &rasterStateNoneDesc;
                graphicsPipelineDesc.pDepthState = &depthStateDisabledDesc;
                graphicsPipelineDesc.pBlendState = NULL;
                graphicsPipelineDesc.mSampleCount = SAMPLE_COUNT_1;
                graphicsPipelineDesc.mSampleQuality = 0;
                graphicsPipelineDesc.pBlendState = &blendStateDesc;
                graphicsPipelineDesc.pColorFormats = colorFormats.data();
                addPipeline(forgeRenderer->Rend(), &pipelineDesc, pipline);
                return true;
            });

            for (auto& buffer : m_fogPass.m_fogUniformBuffer) {
                buffer.Load([&](Buffer** buffer) {
                    BufferLoadDesc desc = {};
                    desc.mDesc.mDescriptors = DESCRIPTOR_TYPE_BUFFER;
                    desc.mDesc.mMemoryUsage = RESOURCE_MEMORY_USAGE_CPU_TO_GPU;
                    desc.mDesc.mFlags = BUFFER_CREATION_FLAG_PERSISTENT_MAP_BIT;
                    desc.mDesc.mFirstElement = 0;
                    desc.mDesc.mElementCount = Fog::MaxFogCount;
                    desc.mDesc.mStructStride = sizeof(UniformFogData);
                    desc.mDesc.mSize = desc.mDesc.mElementCount * desc.mDesc.mStructStride;
                    desc.ppBuffer = buffer;
                    addResource(&desc, nullptr);
                    return true;
                });
            }
            for (auto& buffer : m_fogPass.m_fogFullscreenUniformBuffer) {
                buffer.Load([&](Buffer** buffer) {
                    BufferLoadDesc desc = {};
                    desc.mDesc.mDescriptors = DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    desc.mDesc.mMemoryUsage = RESOURCE_MEMORY_USAGE_CPU_TO_GPU;
                    desc.mDesc.mFlags = BUFFER_CREATION_FLAG_PERSISTENT_MAP_BIT;
                    desc.mDesc.mElementCount = 1;
                    desc.mDesc.mStructStride = sizeof(UniformFullscreenFogData);
                    desc.mDesc.mSize = sizeof(UniformFullscreenFogData);
                    desc.ppBuffer = buffer;
                    addResource(&desc, nullptr);
                    return true;
                });
            }
            DescriptorSetDesc perFrameDescSet{ m_fogPass.m_fogRootSignature.m_handle, DESCRIPTOR_UPDATE_FREQ_PER_FRAME, 1 };
            for (size_t setIndex = 0; setIndex < m_fogPass.m_perFrameSet.size(); setIndex++) {
                addDescriptorSet(forgeRenderer->Rend(), &perFrameDescSet, &m_fogPass.m_perFrameSet[setIndex]);

                std::array<DescriptorData, 2> params = {};
                params[0].pName = "uniformFogBuffer";
                params[0].ppBuffers = &m_fogPass.m_fogUniformBuffer[setIndex].m_handle;
                params[1].pName = "uniformFogFullscreen";
                params[1].ppBuffers = &m_fogPass.m_fogFullscreenUniformBuffer[setIndex].m_handle;
                updateDescriptorSet(forgeRenderer->Rend(), 0, m_fogPass.m_perFrameSet[setIndex], params.size(), params.data());
            }

            // DescriptorSetDesc perObjectSet{ m_fogPass.m_fogRootSignature, DESCRIPTOR_UPDATE_FREQ_PER_DRAW, Fog::MaxFogCount };
            // for (auto& objectSet : m_fogPass.m_perObjectSet) {
            //     addDescriptorSet(forgeRenderer->Rend(), &perObjectSet, &objectSet);
            // }

            // addUniformGPURingBuffer(
            //     forgeRenderer->Rend(), sizeof(Fog::UniformFogData) * Fog::MaxFogCount, &m_fogPass.m_fogUniformBuffer, true);
        }
        //---------------- Diffuse Pipeline  ------------------------
        {
            // z pass
            m_zPassShader.Load(forgeRenderer->Rend(), [&](Shader** shader) {
                ShaderLoadDesc loadDesc = {};
                loadDesc.mStages[0].pFileName = "solid_z.vert";
                loadDesc.mStages[1].pFileName = "solid_z.frag";
                addShader(forgeRenderer->Rend(), &loadDesc, shader);
                return true;
            });
            // diffuse pipeline
            m_materialSolidPass.m_solidDiffuseShader.Load(forgeRenderer->Rend(), [&](Shader** shader) {
                ShaderLoadDesc loadDesc = {};
                loadDesc.mStages[0].pFileName = "solid_diffuse.vert";
                loadDesc.mStages[1].pFileName = "solid_diffuse.frag";
                addShader(forgeRenderer->Rend(), &loadDesc, shader);
                return true;
            });

            m_materialSolidPass.m_solidDiffuseParallaxShader.Load(forgeRenderer->Rend(), [&](Shader** shader) {
                ShaderLoadDesc loadDesc = {};
                loadDesc.mStages[0].pFileName = "solid_diffuse.vert";
                loadDesc.mStages[1].pFileName = "solid_diffuse_parallax.frag";
                addShader(forgeRenderer->Rend(), &loadDesc, shader);
                return true;
            });

            m_solidIlluminationShader.Load(forgeRenderer->Rend(), [&](Shader** shader) {
                ShaderLoadDesc loadDesc = {};
                loadDesc.mStages[0].pFileName = "solid_illumination.vert";
                loadDesc.mStages[1].pFileName = "solid_illumination.frag";
                addShader(forgeRenderer->Rend(), &loadDesc, shader);
                return true;
            });

            // decal pass
            m_decalShader.Load(forgeRenderer->Rend(), [&](Shader** shader) {
                ShaderLoadDesc loadDesc = {};
                loadDesc.mStages[0].pFileName = "decal.vert";
                loadDesc.mStages[1].pFileName = "decal.frag";
                addShader(forgeRenderer->Rend(), &loadDesc, shader);
                return true;
            });
            // translucency
            {
                m_materialTranslucencyPass.m_shader.Load(forgeRenderer->Rend(), [&](Shader** handle) {
                    ShaderLoadDesc loadDesc = {};
                    loadDesc.mStages[0].pFileName = "translucency.vert";
                    loadDesc.mStages[1].pFileName = "translucency.frag";
                    addShader(forgeRenderer->Rend(), &loadDesc, handle);
                    return true;
                });

                m_materialTranslucencyPass.m_particleShader.Load(forgeRenderer->Rend(), [&](Shader** handle) {
                    ShaderLoadDesc loadDesc = {};
                    loadDesc.mStages[0].pFileName = "translucency_particle.vert";
                    loadDesc.mStages[1].pFileName = "translucency_particle.frag";
                    addShader(forgeRenderer->Rend(), &loadDesc, handle);
                    return true;
                });

                m_materialTranslucencyPass.m_waterShader.Load(forgeRenderer->Rend(), [&](Shader** handle) {
                    ShaderLoadDesc loadDesc = {};
                    loadDesc.mStages[0].pFileName = "translucency.vert";
                    loadDesc.mStages[1].pFileName = "translucency_water.frag";
                    addShader(forgeRenderer->Rend(), &loadDesc, handle);
                    return true;
                });
            }

            {
                m_materialTranslucencyPass.m_copyRefraction.Load(forgeRenderer->Rend(), [&](Shader** shader) {
                    ShaderLoadDesc loadDesc = {};
                    loadDesc.mStages[0].pFileName = "copy_channel_4.comp";
                    addShader(forgeRenderer->Rend(), &loadDesc, shader);
                    return true;
                });

                m_materialTranslucencyPass.m_refractionCopyRootSignature.Load(forgeRenderer->Rend(), [&](RootSignature** signature) {
                    RootSignatureDesc refractionCopyRootDesc = {};
                    refractionCopyRootDesc.mShaderCount = 1;
                    refractionCopyRootDesc.ppShaders = &m_materialTranslucencyPass.m_copyRefraction.m_handle;
                    addRootSignature(forgeRenderer->Rend(), &refractionCopyRootDesc, signature);
                    return true;
                });

                DescriptorSetDesc setDesc = { m_materialTranslucencyPass.m_refractionCopyRootSignature.m_handle,
                                              DESCRIPTOR_UPDATE_FREQ_PER_FRAME,
                                              1 };
                for (auto& frameset : m_materialTranslucencyPass.m_refractionPerFrameSet) {
                    frameset.Load(forgeRenderer->Rend(), [&](DescriptorSet** descriptor) {
                        addDescriptorSet(forgeRenderer->Rend(), &setDesc, descriptor);
                        return true;
                    });
                }
                m_materialTranslucencyPass.m_refractionCopyPipeline.Load(forgeRenderer->Rend(), [&](Pipeline** pipeline) {
                    PipelineDesc desc = {};
                    desc.mType = PIPELINE_TYPE_COMPUTE;
                    ComputePipelineDesc& pipelineSettings = desc.mComputeDesc;
                    pipelineSettings.pShaderProgram = m_materialTranslucencyPass.m_copyRefraction.m_handle;
                    pipelineSettings.pRootSignature = m_materialTranslucencyPass.m_refractionCopyRootSignature.m_handle;
                    addPipeline(forgeRenderer->Rend(), &desc, pipeline);
                    return true;
                });
            }

            folly::small_vector<Shader*, 64> shaders{ m_materialSolidPass.m_solidDiffuseShader.m_handle,
                                                      m_materialSolidPass.m_solidDiffuseParallaxShader.m_handle,
                                                      m_zPassShader.m_handle,
                                                      m_decalShader.m_handle,
                                                      m_solidIlluminationShader.m_handle };
            shaders.push_back(m_materialTranslucencyPass.m_shader.m_handle);
            shaders.push_back(m_materialTranslucencyPass.m_particleShader.m_handle);
            shaders.push_back(m_materialTranslucencyPass.m_waterShader.m_handle);

            m_materialRootSignature.Load(forgeRenderer->Rend(), [&](RootSignature** signature) {
                RootSignatureDesc rootSignatureDesc = {};
                const char* pStaticSamplersNames[] = { "nearestSampler", "refractionSampler", "dissolveSampler" };
                Sampler* pStaticSampler[] = { m_samplerPointClampToBorder.m_handle, m_samplerPointClampToEdge.m_handle, m_samplerPointWrap.m_handle };
                rootSignatureDesc.mStaticSamplerCount = std::size(pStaticSamplersNames);
                rootSignatureDesc.ppStaticSamplerNames = pStaticSamplersNames;
                rootSignatureDesc.ppStaticSamplers = pStaticSampler;
                rootSignatureDesc.ppShaders = shaders.data();
                rootSignatureDesc.mShaderCount = shaders.size();
                addRootSignature(forgeRenderer->Rend(), &rootSignatureDesc, signature);
                return true;
            });

            // diffuse material pass
            {
                VertexLayout vertexLayout = {};
#ifndef USE_THE_FORGE_LEGACY
                vertexLayout.mBindingCount = 4;
                vertexLayout.mBindings[0].mStride = sizeof(float3);
                vertexLayout.mBindings[1].mStride = sizeof(float2);
                vertexLayout.mBindings[2].mStride = sizeof(float3);
                vertexLayout.mBindings[3].mStride = sizeof(float3);
#endif
                vertexLayout.mAttribCount = 4;
                vertexLayout.mAttribs[0].mSemantic = SEMANTIC_POSITION;
                vertexLayout.mAttribs[0].mFormat = TinyImageFormat_R32G32B32_SFLOAT;
                vertexLayout.mAttribs[0].mBinding = 0;
                vertexLayout.mAttribs[0].mLocation = 0;
                vertexLayout.mAttribs[0].mOffset = 0;

                vertexLayout.mAttribs[1].mSemantic = SEMANTIC_TEXCOORD0;
                vertexLayout.mAttribs[1].mFormat = TinyImageFormat_R32G32_SFLOAT;
                vertexLayout.mAttribs[1].mBinding = 1;
                vertexLayout.mAttribs[1].mLocation = 1;
                vertexLayout.mAttribs[1].mOffset = 0;

                vertexLayout.mAttribs[2].mSemantic = SEMANTIC_NORMAL;
                vertexLayout.mAttribs[2].mFormat = TinyImageFormat_R32G32B32_SFLOAT;
                vertexLayout.mAttribs[2].mBinding = 2;
                vertexLayout.mAttribs[2].mLocation = 2;
                vertexLayout.mAttribs[2].mOffset = 0;

                vertexLayout.mAttribs[3].mSemantic = SEMANTIC_TANGENT;
                vertexLayout.mAttribs[3].mFormat = TinyImageFormat_R32G32B32_SFLOAT;
                vertexLayout.mAttribs[3].mBinding = 3;
                vertexLayout.mAttribs[3].mLocation = 3;
                vertexLayout.mAttribs[3].mOffset = 0;

                RasterizerStateDesc rasterizerStateDesc = {};
                rasterizerStateDesc.mCullMode = CULL_MODE_FRONT;
                rasterizerStateDesc.mFrontFace = FRONT_FACE_CCW;

                DepthStateDesc depthStateDesc = {};
                depthStateDesc.mDepthTest = true;
                depthStateDesc.mDepthWrite = false;
                depthStateDesc.mDepthFunc = CMP_EQUAL;

                std::array colorFormats = { ColorBufferFormat, NormalBufferFormat, PositionBufferFormat, SpecularBufferFormat };

                PipelineDesc pipelineDesc = {};
                pipelineDesc.mType = PIPELINE_TYPE_GRAPHICS;
                auto& pipelineSettings = pipelineDesc.mGraphicsDesc;
                pipelineSettings.mPrimitiveTopo = PRIMITIVE_TOPO_TRI_LIST;
                pipelineSettings.mRenderTargetCount = colorFormats.size();
                pipelineSettings.pColorFormats = colorFormats.data();
                pipelineSettings.pDepthState = &depthStateDesc;
                pipelineSettings.mSampleCount = SAMPLE_COUNT_1;
                pipelineSettings.mDepthStencilFormat = DepthBufferFormat;
                pipelineSettings.mSampleQuality = 0;
                pipelineSettings.pRootSignature = m_materialRootSignature.m_handle;
                pipelineSettings.pShaderProgram = m_materialSolidPass.m_solidDiffuseShader.m_handle;
                pipelineSettings.pRasterizerState = &rasterizerStateDesc;
                pipelineSettings.pVertexLayout = &vertexLayout;
                m_materialSolidPass.m_solidDiffusePipeline.Load(forgeRenderer->Rend(), [&](Pipeline** pipeline) {
                    addPipeline(forgeRenderer->Rend(), &pipelineDesc, pipeline);
                    return true;
                });

                pipelineSettings.pShaderProgram = m_materialSolidPass.m_solidDiffuseParallaxShader.m_handle;
                m_materialSolidPass.m_solidDiffuseParallaxPipeline.Load(forgeRenderer->Rend(), [&](Pipeline** pipeline) {
                    addPipeline(forgeRenderer->Rend(), &pipelineDesc, pipeline);
                    return true;
                });

                rasterizerStateDesc.mFrontFace = FRONT_FACE_CW;
                m_materialSolidPass.m_solidDiffusePipelineCW.Load(forgeRenderer->Rend(), [&](Pipeline** pipeline) {
                    addPipeline(forgeRenderer->Rend(), &pipelineDesc, pipeline);
                    return true;
                });

                pipelineSettings.pShaderProgram = m_materialSolidPass.m_solidDiffuseParallaxShader.m_handle;
                m_materialSolidPass.m_solidDiffuseParallaxPipelineCW.Load(forgeRenderer->Rend(), [&](Pipeline** pipeline) {
                    addPipeline(forgeRenderer->Rend(), &pipelineDesc, pipeline);
                    return true;
                });
            }
            // decal material pass
            {
                VertexLayout vertexLayout = {};
#ifndef USE_THE_FORGE_LEGACY
                vertexLayout.mBindingCount = 3;
                vertexLayout.mBindings[0].mStride = sizeof(float3);
                vertexLayout.mBindings[1].mStride = sizeof(float2);
                vertexLayout.mBindings[2].mStride = sizeof(float4);
#endif
                vertexLayout.mAttribCount = 3;
                vertexLayout.mAttribs[0].mSemantic = SEMANTIC_POSITION;
                vertexLayout.mAttribs[0].mFormat = TinyImageFormat_R32G32B32_SFLOAT;
                vertexLayout.mAttribs[0].mBinding = 0;
                vertexLayout.mAttribs[0].mLocation = 0;
                vertexLayout.mAttribs[0].mOffset = 0;

                vertexLayout.mAttribs[1].mSemantic = SEMANTIC_TEXCOORD0;
                vertexLayout.mAttribs[1].mFormat = TinyImageFormat_R32G32_SFLOAT;
                vertexLayout.mAttribs[1].mBinding = 1;
                vertexLayout.mAttribs[1].mLocation = 1;
                vertexLayout.mAttribs[1].mOffset = 0;

                vertexLayout.mAttribs[2].mSemantic = SEMANTIC_COLOR;
                vertexLayout.mAttribs[2].mFormat = TinyImageFormat_R32G32B32A32_SFLOAT;
                vertexLayout.mAttribs[2].mBinding = 2;
                vertexLayout.mAttribs[2].mLocation = 2;
                vertexLayout.mAttribs[2].mOffset = 0;

                RasterizerStateDesc rasterizerStateDesc = {};
                rasterizerStateDesc.mCullMode = CULL_MODE_FRONT;


                DepthStateDesc depthStateDesc = {};
                depthStateDesc.mDepthTest = true;
                depthStateDesc.mDepthWrite = false;
                depthStateDesc.mDepthFunc = CMP_LEQUAL;

                std::array colorFormats = { ColorBufferFormat };
                PipelineDesc pipelineDesc = {};
                pipelineDesc.mType = PIPELINE_TYPE_GRAPHICS;
                auto& pipelineSettings = pipelineDesc.mGraphicsDesc;
                pipelineSettings.mPrimitiveTopo = PRIMITIVE_TOPO_TRI_LIST;
                pipelineSettings.mRenderTargetCount = colorFormats.size();
                pipelineSettings.pColorFormats = colorFormats.data();
                pipelineSettings.pDepthState = &depthStateDesc;
                pipelineSettings.mSampleCount = SAMPLE_COUNT_1;
                pipelineSettings.mDepthStencilFormat = DepthBufferFormat;
                pipelineSettings.mSampleQuality = 0;
                pipelineSettings.pRootSignature = m_materialRootSignature.m_handle;
                pipelineSettings.pShaderProgram = m_decalShader.m_handle;
                pipelineSettings.pRasterizerState = &rasterizerStateDesc;
                pipelineSettings.pVertexLayout = &vertexLayout;

                for (size_t blendMode = 0; blendMode < eMaterialBlendMode_LastEnum; ++blendMode) {
                    BlendStateDesc blendStateDesc{};

                    blendStateDesc.mSrcFactors[0] = hpl::HPL2BlendTable[blendMode].src;
                    blendStateDesc.mDstFactors[0] = hpl::HPL2BlendTable[blendMode].dst;
                    blendStateDesc.mBlendModes[0] = hpl::HPL2BlendTable[blendMode].mode;

                    blendStateDesc.mSrcAlphaFactors[0] = hpl::HPL2BlendTable[blendMode].srcAlpha;
                    blendStateDesc.mDstAlphaFactors[0] = hpl::HPL2BlendTable[blendMode].dstAlpha;
                    blendStateDesc.mBlendAlphaModes[0] = hpl::HPL2BlendTable[blendMode].alphaMode;
#ifdef USE_THE_FORGE_LEGACY
                    blendStateDesc.mMasks[0] = RED | GREEN | BLUE;
#else
                    blendStateDesc.mColorWriteMasks[0] =
                        ColorMask::COLOR_MASK_RED | ColorMask::COLOR_MASK_GREEN | ColorMask::COLOR_MASK_BLUE;
#endif
                    blendStateDesc.mRenderTargetMask = BLEND_STATE_TARGET_0;
                    pipelineSettings.pBlendState = &blendStateDesc;
                    rasterizerStateDesc.mFrontFace = FRONT_FACE_CCW;
                    m_decalPipeline[blendMode].Load(forgeRenderer->Rend(), [&](Pipeline** pipline) {
                        addPipeline(forgeRenderer->Rend(), &pipelineDesc, pipline);
                        return true;
                    });
                    rasterizerStateDesc.mFrontFace = FRONT_FACE_CW;
                    m_decalPipelineCW[blendMode].Load(forgeRenderer->Rend(), [&](Pipeline** pipline) {
                        addPipeline(forgeRenderer->Rend(), &pipelineDesc, pipline);
                        return true;
                    });
                }
            }
            // z pass material
            {
                // layout and pipeline for sphere draw
                VertexLayout vertexLayout = {};
                vertexLayout.mAttribCount = 4;

#ifndef USE_THE_FORGE_LEGACY
                vertexLayout.mBindingCount = 4;
                vertexLayout.mBindings[0].mStride = sizeof(float3);
                vertexLayout.mBindings[1].mStride = sizeof(float2);
                vertexLayout.mBindings[2].mStride = sizeof(float3);
                vertexLayout.mBindings[3].mStride = sizeof(float3);
#endif
                vertexLayout.mAttribs[0].mSemantic = SEMANTIC_POSITION;
                vertexLayout.mAttribs[0].mFormat = TinyImageFormat_R32G32B32_SFLOAT;
                vertexLayout.mAttribs[0].mBinding = 0;
                vertexLayout.mAttribs[0].mLocation = 0;
                vertexLayout.mAttribs[0].mOffset = 0;

                vertexLayout.mAttribs[1].mSemantic = SEMANTIC_TEXCOORD0;
                vertexLayout.mAttribs[1].mFormat = TinyImageFormat_R32G32_SFLOAT;
                vertexLayout.mAttribs[1].mBinding = 1;
                vertexLayout.mAttribs[1].mLocation = 1;
                vertexLayout.mAttribs[1].mOffset = 0;

                vertexLayout.mAttribs[2].mSemantic = SEMANTIC_NORMAL;
                vertexLayout.mAttribs[2].mFormat = TinyImageFormat_R32G32B32_SFLOAT;
                vertexLayout.mAttribs[2].mBinding = 2;
                vertexLayout.mAttribs[2].mLocation = 2;
                vertexLayout.mAttribs[2].mOffset = 0;

                vertexLayout.mAttribs[3].mSemantic = SEMANTIC_TANGENT;
                vertexLayout.mAttribs[3].mFormat = TinyImageFormat_R32G32B32_SFLOAT;
                vertexLayout.mAttribs[3].mBinding = 3;
                vertexLayout.mAttribs[3].mLocation = 3;
                vertexLayout.mAttribs[3].mOffset = 0;

                m_zPassPipelineCCW.Load(forgeRenderer->Rend(), [&](Pipeline** pipeline) {
                    RasterizerStateDesc rasterizerStateDesc = {};
                    rasterizerStateDesc.mCullMode = CULL_MODE_FRONT;
                    rasterizerStateDesc.mFrontFace = FrontFace::FRONT_FACE_CCW;

                    DepthStateDesc depthStateDesc = {};
                    depthStateDesc.mDepthTest = true;
                    depthStateDesc.mDepthWrite = true;
                    depthStateDesc.mDepthFunc = CMP_LEQUAL;

                    PipelineDesc pipelineDesc = {};
                    pipelineDesc.mType = PIPELINE_TYPE_GRAPHICS;
                    auto& pipelineSettings = pipelineDesc.mGraphicsDesc;
                    pipelineSettings.mPrimitiveTopo = PRIMITIVE_TOPO_TRI_LIST;
                    pipelineSettings.mRenderTargetCount = 0;
                    pipelineSettings.pDepthState = &depthStateDesc;
                    pipelineSettings.pColorFormats = NULL;
                    pipelineSettings.mSampleCount = SAMPLE_COUNT_1;
                    pipelineSettings.mSampleQuality = 0;
                    pipelineSettings.mDepthStencilFormat = DepthBufferFormat;
                    pipelineSettings.pRootSignature = m_materialRootSignature.m_handle;
                    pipelineSettings.pShaderProgram = m_zPassShader.m_handle;
                    pipelineSettings.pRasterizerState = &rasterizerStateDesc;
                    pipelineSettings.pVertexLayout = &vertexLayout;
                    addPipeline(forgeRenderer->Rend(), &pipelineDesc, pipeline);
                    return true;
                });
                m_zPassPipelineCW.Load(forgeRenderer->Rend(), [&](Pipeline** pipeline) {
                    RasterizerStateDesc rasterizerStateDesc = {};
                    rasterizerStateDesc.mCullMode = CULL_MODE_FRONT;
                    rasterizerStateDesc.mFrontFace = FrontFace::FRONT_FACE_CW;

                    DepthStateDesc depthStateDesc = {};
                    depthStateDesc.mDepthTest = true;
                    depthStateDesc.mDepthWrite = true;
                    depthStateDesc.mDepthFunc = CMP_LEQUAL;

                    PipelineDesc pipelineDesc = {};
                    pipelineDesc.mType = PIPELINE_TYPE_GRAPHICS;
                    auto& pipelineSettings = pipelineDesc.mGraphicsDesc;
                    pipelineSettings.mPrimitiveTopo = PRIMITIVE_TOPO_TRI_LIST;
                    pipelineSettings.mRenderTargetCount = 0;
                    pipelineSettings.pDepthState = &depthStateDesc;
                    pipelineSettings.pColorFormats = NULL;
                    pipelineSettings.mSampleCount = SAMPLE_COUNT_1;
                    pipelineSettings.mSampleQuality = 0;
                    pipelineSettings.mDepthStencilFormat = DepthBufferFormat;
                    pipelineSettings.pRootSignature = m_materialRootSignature.m_handle;
                    pipelineSettings.pShaderProgram = m_zPassShader.m_handle;
                    pipelineSettings.pRasterizerState = &rasterizerStateDesc;
                    pipelineSettings.pVertexLayout = &vertexLayout;
                    addPipeline(forgeRenderer->Rend(), &pipelineDesc, pipeline);
                    return true;
                });
                m_zPassShadowPipelineCW.Load(forgeRenderer->Rend(), [&](Pipeline** pipeline) {
                    RasterizerStateDesc rasterizerStateDesc = {};
                    rasterizerStateDesc.mFrontFace = FRONT_FACE_CW;
                    rasterizerStateDesc.mCullMode = CULL_MODE_FRONT;

                    DepthStateDesc depthStateDesc = {};
                    depthStateDesc.mDepthTest = true;
                    depthStateDesc.mDepthWrite = true;
                    depthStateDesc.mDepthFunc = CMP_LEQUAL;

                    PipelineDesc pipelineDesc = {};
                    pipelineDesc.mType = PIPELINE_TYPE_GRAPHICS;
                    auto& pipelineSettings = pipelineDesc.mGraphicsDesc;
                    pipelineSettings.mPrimitiveTopo = PRIMITIVE_TOPO_TRI_LIST;
                    pipelineSettings.mRenderTargetCount = 0;
                    pipelineSettings.pDepthState = &depthStateDesc;
                    pipelineSettings.pColorFormats = NULL;
                    pipelineSettings.mSampleCount = SAMPLE_COUNT_1;
                    pipelineSettings.mDepthStencilFormat = ShadowDepthBufferFormat;
                    pipelineSettings.mSampleQuality = 0;
                    pipelineSettings.pRootSignature = m_materialRootSignature.m_handle;
                    pipelineSettings.pShaderProgram = m_zPassShader.m_handle;
                    pipelineSettings.pRasterizerState = &rasterizerStateDesc;
                    pipelineSettings.pVertexLayout = &vertexLayout;
                    addPipeline(forgeRenderer->Rend(), &pipelineDesc, pipeline);
                    return true;
                });
                m_zPassShadowPipelineCCW.Load(forgeRenderer->Rend(), [&](Pipeline** pipeline) {
                    RasterizerStateDesc rasterizerStateDesc = {};
                    rasterizerStateDesc.mFrontFace = FRONT_FACE_CW;
                    rasterizerStateDesc.mCullMode = CULL_MODE_FRONT;

                    DepthStateDesc depthStateDesc = {};
                    depthStateDesc.mDepthTest = true;
                    depthStateDesc.mDepthWrite = true;
                    depthStateDesc.mDepthFunc = CMP_LEQUAL;

                    PipelineDesc pipelineDesc = {};
                    pipelineDesc.mType = PIPELINE_TYPE_GRAPHICS;
                    auto& pipelineSettings = pipelineDesc.mGraphicsDesc;
                    pipelineSettings.mPrimitiveTopo = PRIMITIVE_TOPO_TRI_LIST;
                    pipelineSettings.mRenderTargetCount = 0;
                    pipelineSettings.pDepthState = &depthStateDesc;
                    pipelineSettings.pColorFormats = NULL;
                    pipelineSettings.mSampleCount = SAMPLE_COUNT_1;
                    pipelineSettings.mDepthStencilFormat = ShadowDepthBufferFormat;
                    pipelineSettings.mSampleQuality = 0;
                    pipelineSettings.pRootSignature = m_materialRootSignature.m_handle;
                    pipelineSettings.pShaderProgram = m_zPassShader.m_handle;
                    pipelineSettings.pRasterizerState = &rasterizerStateDesc;
                    pipelineSettings.pVertexLayout = &vertexLayout;
                    addPipeline(forgeRenderer->Rend(), &pipelineDesc, pipeline);
                    return true;
                });

                m_materialSet.m_materialConstSet.Load(forgeRenderer->Rend(), [&](DescriptorSet** handle) {
                    DescriptorSetDesc setDesc{ m_materialRootSignature.m_handle, DESCRIPTOR_UPDATE_FREQ_NONE, MaxMaterialSamplers };
                    addDescriptorSet(forgeRenderer->Rend(), &setDesc, handle);
                    return true;
                });
                for (size_t antistropy = 0; antistropy < cMaterial::Antistropy_Count; antistropy++) {
                    for (size_t textureWrap = 0; textureWrap < eTextureWrap_LastEnum; textureWrap++) {
                        for (size_t textureFilter = 0; textureFilter < eTextureFilter_LastEnum; textureFilter++) {
                            uint32_t materialID = detail::resolveMaterialID(
                                static_cast<cMaterial::TextureAntistropy>(antistropy),
                                static_cast<eTextureWrap>(textureWrap),
                                static_cast<eTextureFilter>(textureFilter));
                            m_materialSet.m_samplers[materialID].Load(forgeRenderer->Rend(), [&](Sampler** sampler) {
                                SamplerDesc samplerDesc = {};
                                switch (textureWrap) {
                                case eTextureWrap_Repeat:
                                    samplerDesc.mAddressU = ADDRESS_MODE_REPEAT;
                                    samplerDesc.mAddressV = ADDRESS_MODE_REPEAT;
                                    samplerDesc.mAddressW = ADDRESS_MODE_REPEAT;
                                    break;
                                case eTextureWrap_Clamp:
                                    samplerDesc.mAddressU = ADDRESS_MODE_CLAMP_TO_EDGE;
                                    samplerDesc.mAddressV = ADDRESS_MODE_CLAMP_TO_EDGE;
                                    samplerDesc.mAddressW = ADDRESS_MODE_CLAMP_TO_EDGE;
                                    break;
                                case eTextureWrap_ClampToBorder:
                                    samplerDesc.mAddressU = ADDRESS_MODE_CLAMP_TO_BORDER;
                                    samplerDesc.mAddressV = ADDRESS_MODE_CLAMP_TO_BORDER;
                                    samplerDesc.mAddressW = ADDRESS_MODE_CLAMP_TO_BORDER;
                                    break;
                                default:
                                    ASSERT(false && "Invalid wrap mode");
                                    break;
                                }
                                switch (textureFilter) {
                                case eTextureFilter_Nearest:
                                    samplerDesc.mMinFilter = FilterType::FILTER_NEAREST;
                                    samplerDesc.mMagFilter = FilterType::FILTER_NEAREST;
                                    samplerDesc.mMipMapMode = MipMapMode::MIPMAP_MODE_NEAREST;
                                    break;
                                case eTextureFilter_Bilinear:
                                    samplerDesc.mMinFilter = FilterType::FILTER_LINEAR;
                                    samplerDesc.mMagFilter = FilterType::FILTER_LINEAR;
                                    samplerDesc.mMipMapMode = MipMapMode::MIPMAP_MODE_NEAREST;
                                    break;
                                case eTextureFilter_Trilinear:
                                    samplerDesc.mMinFilter = FilterType::FILTER_LINEAR;
                                    samplerDesc.mMagFilter = FilterType::FILTER_LINEAR;
                                    samplerDesc.mMipMapMode = MipMapMode::MIPMAP_MODE_LINEAR;
                                    break;
                                default:
                                    ASSERT(false && "Invalid filter");
                                    break;
                                }
                                switch (antistropy) {
                                case cMaterial::Antistropy_8:
                                    samplerDesc.mMaxAnisotropy = 8.0f;
                                    break;
                                case cMaterial::Antistropy_16:
                                    samplerDesc.mMaxAnisotropy = 16.0f;
                                    break;
                                default:
                                    break;
                                }
                                addSampler(forgeRenderer->Rend(), &samplerDesc, sampler);
                                return true;
                            });
                            std::array<DescriptorData, 2> params{};
                            params[0].ppTextures = &m_dissolveImage->GetTexture().m_handle;
                            params[0].pName = "dissolveMap";
                            params[1].ppSamplers = &m_materialSet.m_samplers[materialID].m_handle;
                            params[1].pName = "materialSampler";
                            updateDescriptorSet(
                                forgeRenderer->Rend(), materialID, m_materialSet.m_materialConstSet.m_handle, params.size(), params.data());
                        }
                    }
                }
            }
            {
                // layout and pipeline for sphere draw
                VertexLayout vertexLayout = {};
#ifndef USE_THE_FORGE_LEGACY
                vertexLayout.mBindingCount = 2;
                vertexLayout.mBindings[0].mStride = sizeof(float3);
                vertexLayout.mBindings[1].mStride = sizeof(float2);
#endif
                vertexLayout.mAttribCount = 2;
                vertexLayout.mAttribs[0].mSemantic = SEMANTIC_POSITION;
                vertexLayout.mAttribs[0].mFormat = TinyImageFormat_R32G32B32_SFLOAT;
                vertexLayout.mAttribs[0].mBinding = 0;
                vertexLayout.mAttribs[0].mLocation = 0;
                vertexLayout.mAttribs[0].mOffset = 0;

                vertexLayout.mAttribs[1].mSemantic = SEMANTIC_TEXCOORD0;
                vertexLayout.mAttribs[1].mFormat = TinyImageFormat_R32G32_SFLOAT;
                vertexLayout.mAttribs[1].mBinding = 1;
                vertexLayout.mAttribs[1].mLocation = 1;
                vertexLayout.mAttribs[1].mOffset = 0;

                RasterizerStateDesc rasterizerStateDesc = {};
                rasterizerStateDesc.mCullMode = CULL_MODE_FRONT;
                rasterizerStateDesc.mFrontFace = FRONT_FACE_CCW;

                BlendStateDesc blendStateDesc{};
#ifdef USE_THE_FORGE_LEGACY
                blendStateDesc.mMasks[0] = ALL;
#else
                    blendStateDesc.mColorWriteMasks[0] = ColorMask::COLOR_MASK_ALL;
#endif
                blendStateDesc.mSrcFactors[0] = BC_ONE;
                blendStateDesc.mDstFactors[0] = BC_ONE;
                blendStateDesc.mBlendModes[0] = BM_ADD;
                blendStateDesc.mSrcAlphaFactors[0] = BC_ONE;
                blendStateDesc.mDstAlphaFactors[0] = BC_ONE;
                blendStateDesc.mBlendAlphaModes[0] = BM_ADD;
                blendStateDesc.mRenderTargetMask = BLEND_STATE_TARGET_0;

                DepthStateDesc depthStateDesc = {};
                depthStateDesc.mDepthTest = true;
                depthStateDesc.mDepthWrite = false;
                depthStateDesc.mDepthFunc = CMP_EQUAL;

                std::array colorFormats = { ColorBufferFormat };
                PipelineDesc pipelineDesc = {};
                pipelineDesc.mType = PIPELINE_TYPE_GRAPHICS;
                auto& pipelineSettings = pipelineDesc.mGraphicsDesc;
                pipelineSettings.mPrimitiveTopo = PRIMITIVE_TOPO_TRI_LIST;
                pipelineSettings.mRenderTargetCount = 0;
                pipelineSettings.pDepthState = &depthStateDesc;
                pipelineSettings.mRenderTargetCount = colorFormats.size();
                pipelineSettings.pColorFormats = colorFormats.data();
                pipelineSettings.mSampleCount = SAMPLE_COUNT_1;
                pipelineSettings.mDepthStencilFormat = DepthBufferFormat;
                pipelineSettings.mSampleQuality = 0;
                pipelineSettings.pBlendState = &blendStateDesc;
                pipelineSettings.pRootSignature = m_materialRootSignature.m_handle;
                pipelineSettings.pShaderProgram = m_solidIlluminationShader.m_handle;
                pipelineSettings.pRasterizerState = &rasterizerStateDesc;
                pipelineSettings.pVertexLayout = &vertexLayout;
                // illumination pass
                m_solidIlluminationPipelineCCW.Load(forgeRenderer->Rend(), [&](Pipeline** pipline) {
                    addPipeline(forgeRenderer->Rend(), &pipelineDesc, pipline);
                    return true;
                });

                rasterizerStateDesc.mFrontFace = FRONT_FACE_CW;
                m_solidIlluminationPipelineCW.Load(forgeRenderer->Rend(), [&](Pipeline** pipline) {
                    addPipeline(forgeRenderer->Rend(), &pipelineDesc, pipline);
                    return true;
                });

            }

            // translucency pass
            {
                VertexLayout particleVertexLayout = {};
#ifndef USE_THE_FORGE_LEGACY
                particleVertexLayout.mBindingCount = 3;
                particleVertexLayout.mBindings[0].mStride = sizeof(float3);
                particleVertexLayout.mBindings[1].mStride = sizeof(float2);
                particleVertexLayout.mBindings[2].mStride = sizeof(float4);
#endif

                particleVertexLayout.mAttribCount = 3;
                particleVertexLayout.mAttribs[0].mSemantic = SEMANTIC_POSITION;
                particleVertexLayout.mAttribs[0].mFormat = TinyImageFormat_R32G32B32_SFLOAT;
                particleVertexLayout.mAttribs[0].mBinding = 0;
                particleVertexLayout.mAttribs[0].mLocation = 0;
                particleVertexLayout.mAttribs[0].mOffset = 0;

                particleVertexLayout.mAttribs[1].mSemantic = SEMANTIC_TEXCOORD0;
                particleVertexLayout.mAttribs[1].mFormat = TinyImageFormat_R32G32_SFLOAT;
                particleVertexLayout.mAttribs[1].mBinding = 1;
                particleVertexLayout.mAttribs[1].mLocation = 1;
                particleVertexLayout.mAttribs[1].mOffset = 0;

                particleVertexLayout.mAttribs[2].mSemantic = SEMANTIC_COLOR;
                particleVertexLayout.mAttribs[2].mFormat = TinyImageFormat_R32G32B32A32_SFLOAT;
                particleVertexLayout.mAttribs[2].mBinding = 2;
                particleVertexLayout.mAttribs[2].mLocation = 2;
                particleVertexLayout.mAttribs[2].mOffset = 0;

                VertexLayout vertexLayout = {};

#ifndef USE_THE_FORGE_LEGACY
                vertexLayout.mBindingCount = 5;
                vertexLayout.mBindings[0].mStride = sizeof(float3);
                vertexLayout.mBindings[1].mStride = sizeof(float2);
                vertexLayout.mBindings[2].mStride = sizeof(float3);
                vertexLayout.mBindings[3].mStride = sizeof(float3);
                vertexLayout.mBindings[4].mStride = sizeof(float3);
#endif
                vertexLayout.mAttribCount = 5;
                vertexLayout.mAttribs[0].mSemantic = SEMANTIC_POSITION;
                vertexLayout.mAttribs[0].mFormat = TinyImageFormat_R32G32B32_SFLOAT;
                vertexLayout.mAttribs[0].mBinding = 0;
                vertexLayout.mAttribs[0].mLocation = 0;
                vertexLayout.mAttribs[0].mOffset = 0;

                vertexLayout.mAttribs[1].mSemantic = SEMANTIC_TEXCOORD0;
                vertexLayout.mAttribs[1].mFormat = TinyImageFormat_R32G32_SFLOAT;
                vertexLayout.mAttribs[1].mBinding = 1;
                vertexLayout.mAttribs[1].mLocation = 1;
                vertexLayout.mAttribs[1].mOffset = 0;

                vertexLayout.mAttribs[2].mSemantic = SEMANTIC_NORMAL;
                vertexLayout.mAttribs[2].mFormat = TinyImageFormat_R32G32B32_SFLOAT;
                vertexLayout.mAttribs[2].mBinding = 2;
                vertexLayout.mAttribs[2].mLocation = 2;
                vertexLayout.mAttribs[2].mOffset = 0;

                vertexLayout.mAttribs[3].mSemantic = SEMANTIC_TANGENT;
                vertexLayout.mAttribs[3].mFormat = TinyImageFormat_R32G32B32_SFLOAT;
                vertexLayout.mAttribs[3].mBinding = 3;
                vertexLayout.mAttribs[3].mLocation = 3;
                vertexLayout.mAttribs[3].mOffset = 0;

                vertexLayout.mAttribs[4].mSemantic = SEMANTIC_COLOR;
                vertexLayout.mAttribs[4].mFormat = TinyImageFormat_R32G32B32A32_SFLOAT;
                vertexLayout.mAttribs[4].mBinding = 4;
                vertexLayout.mAttribs[4].mLocation = 4;
                vertexLayout.mAttribs[4].mOffset = 0;

                std::array colorFormats = { ColorBufferFormat };
                std::array<eMaterialBlendMode, TranslucencyPipeline::BlendModeCount> blendMapping = {};
                blendMapping[TranslucencyPipeline::BlendAdd] = eMaterialBlendMode_Add;
                blendMapping[TranslucencyPipeline::BlendMul] = eMaterialBlendMode_Mul;
                blendMapping[TranslucencyPipeline::BlendMulX2] = eMaterialBlendMode_MulX2;
                blendMapping[TranslucencyPipeline::BlendAlpha] = eMaterialBlendMode_Alpha;
                blendMapping[TranslucencyPipeline::BlendPremulAlpha] = eMaterialBlendMode_PremulAlpha;

                // create translucent pipelines
                for (size_t transBlend = 0; transBlend < TranslucencyPipeline::TranslucencyBlend::BlendModeCount; transBlend++) {
                    auto& pipelineBlendGroup = m_materialTranslucencyPass.m_pipelines[transBlend];
                    for (size_t pipelineKey = 0; pipelineKey < pipelineBlendGroup.size(); pipelineKey++) {
                        TranslucencyPipeline::TranslucencyKey key = {};
                        key.m_id = pipelineKey;

                        pipelineBlendGroup[key.m_id].Load(forgeRenderer->Rend(), [&](Pipeline** pipeline) {
                            BlendStateDesc blendStateDesc{};
#ifdef USE_THE_FORGE_LEGACY
                            blendStateDesc.mMasks[0] = ALL;
#else
                                blendStateDesc.mColorWriteMasks[0] = ColorMask::COLOR_MASK_ALL;
#endif
                            blendStateDesc.mRenderTargetMask = BLEND_STATE_TARGET_0;
                            blendStateDesc.mIndependentBlend = false;

                            PipelineDesc pipelineDesc = {};
                            pipelineDesc.mType = PIPELINE_TYPE_GRAPHICS;
                            auto& pipelineSettings = pipelineDesc.mGraphicsDesc;
                            pipelineSettings.mPrimitiveTopo = PRIMITIVE_TOPO_TRI_LIST;
                            pipelineSettings.mRenderTargetCount = colorFormats.size();
                            pipelineSettings.pColorFormats = colorFormats.data();
                            pipelineSettings.mSampleCount = SAMPLE_COUNT_1;
                            pipelineSettings.mSampleQuality = 0;
                            pipelineSettings.pBlendState = &blendStateDesc;
                            pipelineSettings.mDepthStencilFormat = DepthBufferFormat;
                            pipelineSettings.pRootSignature = m_materialRootSignature.m_handle;
                            pipelineSettings.pVertexLayout = &vertexLayout;

                            RasterizerStateDesc rasterizerStateDesc = {};
                            rasterizerStateDesc.mCullMode = CULL_MODE_FRONT;
                            rasterizerStateDesc.mFrontFace = FRONT_FACE_CCW;
                            pipelineSettings.pRasterizerState = &rasterizerStateDesc;

                            DepthStateDesc depthStateDesc = {};
                            depthStateDesc.mDepthWrite = false;
                            if (key.m_field.m_hasDepthTest) {
                                depthStateDesc.mDepthTest = true;
                                depthStateDesc.mDepthFunc = CMP_LEQUAL;
                            }
                            pipelineSettings.pDepthState = &depthStateDesc;

                            blendStateDesc.mSrcFactors[0] = hpl::HPL2BlendTable[blendMapping[transBlend]].src;
                            blendStateDesc.mDstFactors[0] = hpl::HPL2BlendTable[blendMapping[transBlend]].dst;
                            blendStateDesc.mBlendModes[0] = hpl::HPL2BlendTable[blendMapping[transBlend]].mode;

                            blendStateDesc.mSrcAlphaFactors[0] = hpl::HPL2BlendTable[blendMapping[transBlend]].srcAlpha;
                            blendStateDesc.mDstAlphaFactors[0] = hpl::HPL2BlendTable[blendMapping[transBlend]].dstAlpha;
                            blendStateDesc.mBlendAlphaModes[0] = hpl::HPL2BlendTable[blendMapping[transBlend]].alphaMode;
                            pipelineSettings.pShaderProgram = m_materialTranslucencyPass.m_shader.m_handle;

                            addPipeline(forgeRenderer->Rend(), &pipelineDesc, pipeline);
                            return true;
                        });
                    }
                }
                {
                    auto& pipelineBlendGroup = m_materialTranslucencyPass.m_waterPipeline;
                    for (size_t pipelineKey = 0; pipelineKey < pipelineBlendGroup.size(); pipelineKey++) {
                        TranslucencyPipeline::TranslucencyWaterKey key = {};
                        key.m_id = pipelineKey;

                        pipelineBlendGroup[key.m_id].Load(forgeRenderer->Rend(), [&](Pipeline** pipeline) {
                            BlendStateDesc blendStateDesc{};
#ifdef USE_THE_FORGE_LEGACY
                            blendStateDesc.mMasks[0] = RED | GREEN | BLUE;
#else
                                blendStateDesc.mColorWriteMasks[0] = ColorMask::COLOR_MASK_RED | ColorMask::COLOR_MASK_GREEN | ColorMask::COLOR_MASK_BLUE;
#endif
                            blendStateDesc.mRenderTargetMask = BLEND_STATE_TARGET_0;
                            blendStateDesc.mIndependentBlend = false;

                            PipelineDesc pipelineDesc = {};
                            pipelineDesc.mType = PIPELINE_TYPE_GRAPHICS;
                            auto& pipelineSettings = pipelineDesc.mGraphicsDesc;
                            pipelineSettings.mPrimitiveTopo = PRIMITIVE_TOPO_TRI_LIST;
                            pipelineSettings.mRenderTargetCount = colorFormats.size();
                            pipelineSettings.pColorFormats = colorFormats.data();
                            pipelineSettings.mSampleCount = SAMPLE_COUNT_1;
                            pipelineSettings.mSampleQuality = 0;
                            pipelineSettings.pBlendState = &blendStateDesc;
                            pipelineSettings.mDepthStencilFormat = DepthBufferFormat;
                            pipelineSettings.pRootSignature = m_materialRootSignature.m_handle;
                            pipelineSettings.pVertexLayout = &vertexLayout;

                            RasterizerStateDesc rasterizerStateDesc = {};
                            rasterizerStateDesc.mCullMode = CULL_MODE_FRONT;
                            rasterizerStateDesc.mFrontFace = FRONT_FACE_CCW;
                            pipelineSettings.pRasterizerState = &rasterizerStateDesc;

                            DepthStateDesc depthStateDesc = {};
                            depthStateDesc.mDepthWrite = false;
                            if (key.m_field.m_hasDepthTest) {
                                depthStateDesc.mDepthTest = true;
                                depthStateDesc.mDepthFunc = CMP_LEQUAL;
                            }
                            pipelineSettings.pDepthState = &depthStateDesc;

                            blendStateDesc.mSrcFactors[0] = BC_ONE;
                            blendStateDesc.mDstFactors[0] = BC_ZERO;
                            blendStateDesc.mBlendModes[0] = BlendMode::BM_ADD;
                            pipelineSettings.pShaderProgram = m_materialTranslucencyPass.m_waterShader.m_handle;

                            addPipeline(forgeRenderer->Rend(), &pipelineDesc, pipeline);
                            return true;
                        });
                    }
                }

                for (size_t transBlend = 0; transBlend < TranslucencyPipeline::TranslucencyBlend::BlendModeCount; transBlend++) {
                    auto& pipelineBlendGroup = m_materialTranslucencyPass.m_particlePipelines[transBlend];
                    for (size_t pipelineKey = 0; pipelineKey < pipelineBlendGroup.size(); pipelineKey++) {
                        TranslucencyPipeline::TranslucencyKey key = {};
                        key.m_id = pipelineKey;
                        pipelineBlendGroup[key.m_id].Load(forgeRenderer->Rend(), [&](Pipeline** pipeline) {
                            BlendStateDesc blendStateDesc{};
#ifdef USE_THE_FORGE_LEGACY
                            blendStateDesc.mMasks[0] = ALL;
#else
                                blendStateDesc.mColorWriteMasks[0] = ColorMask::COLOR_MASK_ALL;
#endif
                            blendStateDesc.mRenderTargetMask = BLEND_STATE_TARGET_0;
                            blendStateDesc.mIndependentBlend = false;

                            PipelineDesc pipelineDesc = {};
                            pipelineDesc.mType = PIPELINE_TYPE_GRAPHICS;
                            auto& pipelineSettings = pipelineDesc.mGraphicsDesc;
                            pipelineSettings.mPrimitiveTopo = PRIMITIVE_TOPO_TRI_LIST;
                            pipelineSettings.mRenderTargetCount = colorFormats.size();
                            pipelineSettings.pColorFormats = colorFormats.data();
                            pipelineSettings.mSampleCount = SAMPLE_COUNT_1;
                            pipelineSettings.mSampleQuality = 0;
                            pipelineSettings.pBlendState = &blendStateDesc;
                            pipelineSettings.mDepthStencilFormat = DepthBufferFormat;
                            pipelineSettings.pRootSignature = m_materialRootSignature.m_handle;
                            pipelineSettings.pVertexLayout = &particleVertexLayout;

                            RasterizerStateDesc rasterizerStateDesc = {};
                            rasterizerStateDesc.mCullMode = CULL_MODE_FRONT;
                            rasterizerStateDesc.mFrontFace = FRONT_FACE_CCW;
                            pipelineSettings.pRasterizerState = &rasterizerStateDesc;

                            DepthStateDesc depthStateDesc = {};
                            depthStateDesc.mDepthWrite = false;
                            if (key.m_field.m_hasDepthTest) {
                                depthStateDesc.mDepthTest = true;
                                depthStateDesc.mDepthFunc = CMP_LEQUAL;
                            }
                            pipelineSettings.pDepthState = &depthStateDesc;
                            pipelineSettings.pShaderProgram = m_materialTranslucencyPass.m_particleShader.m_handle;

                            blendStateDesc.mSrcFactors[0] = hpl::HPL2BlendTable[blendMapping[transBlend]].src;
                            blendStateDesc.mDstFactors[0] = hpl::HPL2BlendTable[blendMapping[transBlend]].dst;
                            blendStateDesc.mBlendModes[0] = hpl::HPL2BlendTable[blendMapping[transBlend]].mode;

                            blendStateDesc.mSrcAlphaFactors[0] = hpl::HPL2BlendTable[blendMapping[transBlend]].srcAlpha;
                            blendStateDesc.mDstAlphaFactors[0] = hpl::HPL2BlendTable[blendMapping[transBlend]].dstAlpha;
                            blendStateDesc.mBlendAlphaModes[0] = hpl::HPL2BlendTable[blendMapping[transBlend]].alphaMode;

                            addPipeline(forgeRenderer->Rend(), &pipelineDesc, pipeline);
                            return true;
                        });
                    }
                }

                {
                    for (size_t pipelineKey = 0; pipelineKey < m_materialTranslucencyPass.m_refractionPipeline.size(); pipelineKey++) {
                        TranslucencyPipeline::TranslucencyKey key = {};
                        key.m_id = pipelineKey;
                        m_materialTranslucencyPass.m_refractionPipeline[key.m_id].Load(forgeRenderer->Rend(), [&](Pipeline** pipeline) {
                            BlendStateDesc blendStateDesc{};
#ifdef USE_THE_FORGE_LEGACY
                            blendStateDesc.mMasks[0] = ALL;
#else
                                blendStateDesc.mColorWriteMasks[0] = ColorMask::COLOR_MASK_ALL;
#endif
                            blendStateDesc.mRenderTargetMask = BLEND_STATE_TARGET_0;
                            blendStateDesc.mIndependentBlend = false;

                            PipelineDesc pipelineDesc = {};
                            pipelineDesc.mType = PIPELINE_TYPE_GRAPHICS;
                            auto& pipelineSettings = pipelineDesc.mGraphicsDesc;
                            pipelineSettings.mPrimitiveTopo = PRIMITIVE_TOPO_TRI_LIST;
                            pipelineSettings.mRenderTargetCount = colorFormats.size();
                            pipelineSettings.pColorFormats = colorFormats.data();
                            pipelineSettings.mSampleCount = SAMPLE_COUNT_1;
                            pipelineSettings.mSampleQuality = 0;
                            pipelineSettings.pBlendState = &blendStateDesc;
                            pipelineSettings.mDepthStencilFormat = DepthBufferFormat;
                            pipelineSettings.pRootSignature = m_materialRootSignature.m_handle;
                            pipelineSettings.pVertexLayout = &vertexLayout;

                            RasterizerStateDesc rasterizerStateDesc = {};
                            rasterizerStateDesc.mCullMode = CULL_MODE_FRONT;
                            rasterizerStateDesc.mFrontFace = FRONT_FACE_CCW;
                            pipelineSettings.pRasterizerState = &rasterizerStateDesc;

                            DepthStateDesc depthStateDesc = {};
                            depthStateDesc.mDepthWrite = false;
                            if (key.m_field.m_hasDepthTest) {
                                depthStateDesc.mDepthTest = true;
                                depthStateDesc.mDepthFunc = CMP_LEQUAL;
                            }
                            pipelineSettings.pDepthState = &depthStateDesc;

                            blendStateDesc.mSrcFactors[0] = BlendConstant::BC_ONE;
                            blendStateDesc.mDstFactors[0] = BlendConstant::BC_SRC_ALPHA;
                            blendStateDesc.mBlendModes[0] = BlendMode::BM_ADD;

                            blendStateDesc.mSrcAlphaFactors[0] = hpl::HPL2BlendTable[eMaterialBlendMode_Add].srcAlpha;
                            blendStateDesc.mDstAlphaFactors[0] = hpl::HPL2BlendTable[eMaterialBlendMode_Add].dstAlpha;
                            blendStateDesc.mBlendAlphaModes[0] = hpl::HPL2BlendTable[eMaterialBlendMode_Add].alphaMode;

                            pipelineSettings.pShaderProgram = m_materialTranslucencyPass.m_shader.m_handle;
                            addPipeline(forgeRenderer->Rend(), &pipelineDesc, pipeline);
                            return true;
                        });
                    }
                }
            }

            DescriptorSetDesc perFrameDescSet{ m_materialRootSignature.m_handle,
                                               DESCRIPTOR_UPDATE_FREQ_PER_FRAME,
                                               MaxViewportFrameDescriptors };
            for (size_t setIndex = 0; setIndex < m_materialSet.m_frameSet.size(); setIndex++) {
                m_materialSet.m_frameSet[setIndex].Load(forgeRenderer->Rend(), [&](DescriptorSet** descSet) {
                    addDescriptorSet(forgeRenderer->Rend(), &perFrameDescSet, descSet);
                    return true;
                });

                for (size_t i = 0; i < MaxViewportFrameDescriptors; i++) {
                    std::array<DescriptorData, 3> params = {};
                    params[0].pName = "perFrameConstants";
                    params[0].ppBuffers = &m_perFrameBuffer[i].m_handle;
                    params[1].pName = "uniformObjectBuffer";
                    params[1].ppBuffers = &m_objectUniformBuffer[setIndex].m_handle;
                    params[2].pName = "uniformMaterialBuffer";
                    params[2].ppBuffers = &m_materialSet.m_materialUniformBuffer.m_handle;
                    updateDescriptorSet(
                        forgeRenderer->Rend(), i, m_materialSet.m_frameSet[setIndex].m_handle, params.size(), params.data());
                }
            }

            for (size_t setIndex = 0; setIndex < m_materialSet.m_perBatchSet.size(); setIndex++) {
                m_materialSet.m_perBatchSet[setIndex].Load(forgeRenderer->Rend(), [&](DescriptorSet** descSet) {
                    DescriptorSetDesc batchDescriptorSet{ m_materialRootSignature.m_handle,
                                                          DESCRIPTOR_UPDATE_FREQ_PER_BATCH,
                                                          cMaterial::MaxMaterialID };
                    addDescriptorSet(forgeRenderer->Rend(), &batchDescriptorSet, descSet);
                    return true;
                });
            }
        }

        // ------------------------ Light Pass -----------------------------------------------------------------
        {
            for (auto& buffer : m_lightPassBuffer) {
                buffer.Load([&](Buffer** buffer) {
                    BufferLoadDesc desc = {};
                    desc.mDesc.mDescriptors = DESCRIPTOR_TYPE_BUFFER;
                    desc.mDesc.mMemoryUsage = RESOURCE_MEMORY_USAGE_CPU_TO_GPU;
                    desc.mDesc.mFlags = BUFFER_CREATION_FLAG_PERSISTENT_MAP_BIT;
                    desc.mDesc.mFirstElement = 0;
                    desc.mDesc.mElementCount = MaxLightUniforms;
                    desc.mDesc.mStructStride = sizeof(UniformLightData);
                    desc.mDesc.mSize = desc.mDesc.mElementCount * desc.mDesc.mStructStride;
                    desc.ppBuffer = buffer;
                    addResource(&desc, nullptr);
                    return true;
                });
            }

            m_pointLightShader.Load(forgeRenderer->Rend(), [&](Shader** handle) {
                ShaderLoadDesc loadDesc = {};
                loadDesc.mStages[0].pFileName = "deferred_light.vert";
                loadDesc.mStages[1].pFileName = "deferred_light_pointlight.frag";
                addShader(forgeRenderer->Rend(), &loadDesc, handle);
                return true;
            });

            // High
            int shadowMapJitterSize = 0;
            int shadowMapJitterSamples = 0;
            if (mShadowMapQuality == eShadowMapQuality_High) {
                shadowMapJitterSize = 64;
                shadowMapJitterSamples = 32; // 64 here instead? I mean, ATI has to deal with medium has max? or different max for ATI?

                m_spotLightShader.Load(forgeRenderer->Rend(), [&](Shader** handle) {
                    ShaderLoadDesc loadDesc = {};
                    loadDesc.mStages[0].pFileName = "deferred_light.vert";
                    loadDesc.mStages[1].pFileName = "deferred_light_spotlight_high.frag";
                    addShader(forgeRenderer->Rend(), &loadDesc, handle);
                    return true;
                });
            }
            // Medium
            else if (mShadowMapQuality == eShadowMapQuality_Medium) {
                shadowMapJitterSize = 32;
                shadowMapJitterSamples = 16;
                m_spotLightShader.Load(forgeRenderer->Rend(), [&](Shader** handle) {
                    ShaderLoadDesc loadDesc = {};
                    loadDesc.mStages[0].pFileName = "deferred_light.vert";
                    loadDesc.mStages[1].pFileName = "deferred_light_spotlight_med.frag";
                    addShader(forgeRenderer->Rend(), &loadDesc, handle);
                    return true;
                });
            }
            // Low
            else {
                shadowMapJitterSize = 0;
                shadowMapJitterSamples = 0;
                m_spotLightShader.Load(forgeRenderer->Rend(), [&](Shader** handle) {
                    ShaderLoadDesc loadDesc = {};
                    loadDesc.mStages[0].pFileName = "deferred_light.vert";
                    loadDesc.mStages[1].pFileName = "deferred_light_spotlight.frag";
                    addShader(forgeRenderer->Rend(), &loadDesc, handle);
                    return true;
                });
            }

            if (mShadowMapQuality != eShadowMapQuality_Low) {
                m_shadowJitterTexture.Load([&](Texture** texture) {
                    TextureCreator::GenerateScatterDiskMap2D(shadowMapJitterSize, shadowMapJitterSamples, true, texture);
                    return true;
                });
            }

            m_stencilLightShader.Load(forgeRenderer->Rend(), [&](Shader** handle) {
                ShaderLoadDesc loadDesc = {};
                loadDesc.mStages[0].pFileName = "deferred_light.vert";
                loadDesc.mStages[1].pFileName = "deferred_light_stencil.frag";
                addShader(forgeRenderer->Rend(), &loadDesc, handle);

                return true;
            });

            m_boxLightShader.Load(forgeRenderer->Rend(), [&](Shader** handle) {
                ShaderLoadDesc loadDesc = {};
                loadDesc.mStages[0].pFileName = "deferred_light.vert";
                loadDesc.mStages[1].pFileName = "deferred_light_box.frag";
                addShader(forgeRenderer->Rend(), &loadDesc, handle);
                return true;
            });
            Sampler* vbShadeSceneSamplers[] = { m_shadowCmpSampler.m_handle,
                                                m_samplerPointClampToBorder.m_handle,
                                                m_goboSampler.m_handle,
                                                m_samplerPointClampToBorder.m_handle };
            const char* vbShadeSceneSamplersNames[] = { "shadowCmpSampler", "pointSampler", "goboSampler", "nearestSampler" };
            Shader* shaders[] = {
                m_pointLightShader.m_handle, m_stencilLightShader.m_handle, m_boxLightShader.m_handle, m_spotLightShader.m_handle
            };
            m_lightPassRootSignature.Load(forgeRenderer->Rend(), [&](RootSignature** signature) {
                RootSignatureDesc rootSignatureDesc = {};
                rootSignatureDesc.ppShaders = shaders;
                rootSignatureDesc.mShaderCount = std::size(shaders);
                rootSignatureDesc.mStaticSamplerCount = std::size(vbShadeSceneSamplers);
                rootSignatureDesc.ppStaticSamplers = vbShadeSceneSamplers;
                rootSignatureDesc.ppStaticSamplerNames = vbShadeSceneSamplersNames;
                addRootSignature(forgeRenderer->Rend(), &rootSignatureDesc, signature);
                return true;
            });

            VertexLayout vertexLayout = {};
#ifndef USE_THE_FORGE_LEGACY
            vertexLayout.mBindingCount = 1;
            vertexLayout.mBindings[0].mStride = sizeof(float3);
#endif
            vertexLayout.mAttribCount = 1;
            vertexLayout.mAttribs[0].mSemantic = SEMANTIC_POSITION;
            vertexLayout.mAttribs[0].mFormat = TinyImageFormat_R32G32B32_SFLOAT;
            vertexLayout.mAttribs[0].mBinding = 0;
            vertexLayout.mAttribs[0].mLocation = 0;
            vertexLayout.mAttribs[0].mOffset = 0;

            auto createPipelineVariants = [&](PipelineDesc& pipelineDesc,
                                              std::array<SharedPipeline, LightPipelineVariant_Size>& pipelines) {
                for (uint32_t i = 0; i < LightPipelineVariants::LightPipelineVariant_Size; ++i) {
                    DepthStateDesc depthStateDec = {};
                    depthStateDec.mDepthTest = true;
                    depthStateDec.mDepthWrite = false;
                    depthStateDec.mDepthFunc = CMP_GEQUAL;
                    if ((LightPipelineVariants::LightPipelineVariant_StencilTest & i) > 0) {
                        depthStateDec.mStencilTest = true;
                        depthStateDec.mStencilFrontFunc = CMP_EQUAL;
                        depthStateDec.mStencilBackFunc = CMP_EQUAL;
                        depthStateDec.mStencilFrontPass = STENCIL_OP_SET_ZERO;
                        depthStateDec.mStencilFrontFail = STENCIL_OP_SET_ZERO;
                        depthStateDec.mStencilBackPass = STENCIL_OP_SET_ZERO;
                        depthStateDec.mStencilBackFail = STENCIL_OP_SET_ZERO;
                        depthStateDec.mDepthFrontFail = STENCIL_OP_SET_ZERO;
                        depthStateDec.mDepthBackFail = STENCIL_OP_SET_ZERO;
                        depthStateDec.mStencilWriteMask = 0xff;
                        depthStateDec.mStencilReadMask = 0xff;
                    }

                    RasterizerStateDesc rasterizerStateDesc = {};
                    if ((LightPipelineVariants::LightPipelineVariant_CCW & i) > 0) {
                        rasterizerStateDesc.mFrontFace = FrontFace::FRONT_FACE_CCW;
                        rasterizerStateDesc.mCullMode = CULL_MODE_FRONT;
                    } else {
                        rasterizerStateDesc.mFrontFace = FrontFace::FRONT_FACE_CW;
                        rasterizerStateDesc.mCullMode = CULL_MODE_FRONT;
                    }
                    auto& pipelineSettings = pipelineDesc.mGraphicsDesc;
                    pipelineSettings.pDepthState = &depthStateDec;
                    pipelineSettings.pRasterizerState = &rasterizerStateDesc;
                    pipelines[i].Load(forgeRenderer->Rend(), [&](Pipeline** pipeline) {
                        addPipeline(forgeRenderer->Rend(), &pipelineDesc, pipeline);
                        return true;
                    });
                }
            };

            BlendStateDesc blendStateDesc{};
            blendStateDesc.mSrcFactors[0] = BC_ONE;
            blendStateDesc.mDstFactors[0] = BC_ONE;
            blendStateDesc.mBlendModes[0] = BM_ADD;
            blendStateDesc.mSrcAlphaFactors[0] = BC_ONE;
            blendStateDesc.mDstAlphaFactors[0] = BC_ONE;
            blendStateDesc.mBlendAlphaModes[0] = BM_ADD;
#ifdef USE_THE_FORGE_LEGACY
            blendStateDesc.mMasks[0] = RED | GREEN | BLUE;
#else
            blendStateDesc.mColorWriteMasks[0] = ColorMask::COLOR_MASK_RED | ColorMask::COLOR_MASK_GREEN | ColorMask::COLOR_MASK_BLUE;
#endif
            blendStateDesc.mRenderTargetMask = BLEND_STATE_TARGET_0;
            blendStateDesc.mIndependentBlend = false;

            std::array colorFormats = { ColorBufferFormat };
            {
                PipelineDesc pipelineDesc = {};
                pipelineDesc.mType = PIPELINE_TYPE_GRAPHICS;
                auto& pipelineSettings = pipelineDesc.mGraphicsDesc;
                pipelineSettings.mPrimitiveTopo = PRIMITIVE_TOPO_TRI_LIST;
                pipelineSettings.mRenderTargetCount = colorFormats.size();
                pipelineSettings.pColorFormats = colorFormats.data();
                pipelineSettings.mSampleCount = SAMPLE_COUNT_1;
                pipelineSettings.mSampleQuality = 0;
                pipelineSettings.pBlendState = &blendStateDesc;
                pipelineSettings.mDepthStencilFormat = DepthBufferFormat;
                pipelineSettings.pRootSignature = m_lightPassRootSignature.m_handle;
                pipelineSettings.pVertexLayout = &vertexLayout;
                pipelineSettings.pShaderProgram = m_pointLightShader.m_handle;
                createPipelineVariants(pipelineDesc, m_pointLightPipeline);
            }

            {
                PipelineDesc pipelineDesc = {};
                pipelineDesc.mType = PIPELINE_TYPE_GRAPHICS;
                auto& pipelineSettings = pipelineDesc.mGraphicsDesc;
                pipelineSettings.mPrimitiveTopo = PRIMITIVE_TOPO_TRI_LIST;
                pipelineSettings.mRenderTargetCount = colorFormats.size();
                pipelineSettings.pColorFormats = colorFormats.data();
                pipelineSettings.mSampleCount = SAMPLE_COUNT_1;
                pipelineSettings.mSampleQuality = 0;
                pipelineSettings.pBlendState = &blendStateDesc;
                pipelineSettings.mDepthStencilFormat = DepthBufferFormat;
                pipelineSettings.pRootSignature = m_lightPassRootSignature.m_handle;
                pipelineSettings.pVertexLayout = &vertexLayout;
                pipelineSettings.pShaderProgram = m_boxLightShader.m_handle;
                createPipelineVariants(pipelineDesc, m_boxLightPipeline);
            }

            {
                PipelineDesc pipelineDesc = {};
                pipelineDesc.mType = PIPELINE_TYPE_GRAPHICS;
                auto& pipelineSettings = pipelineDesc.mGraphicsDesc;
                pipelineSettings.mPrimitiveTopo = PRIMITIVE_TOPO_TRI_LIST;
                pipelineSettings.mRenderTargetCount = colorFormats.size();
                pipelineSettings.pColorFormats = colorFormats.data();
                pipelineSettings.mSampleCount = SAMPLE_COUNT_1;
                pipelineSettings.mSampleQuality = 0;
                pipelineSettings.pBlendState = &blendStateDesc;
                pipelineSettings.mDepthStencilFormat = DepthBufferFormat;
                pipelineSettings.pRootSignature = m_lightPassRootSignature.m_handle;
                pipelineSettings.pVertexLayout = &vertexLayout;
                pipelineSettings.pShaderProgram = m_spotLightShader.m_handle;

                createPipelineVariants(pipelineDesc, m_spotLightPipeline);
            }
            {
                DepthStateDesc stencilDepthTest = {};
                stencilDepthTest.mDepthTest = true;
                stencilDepthTest.mDepthWrite = false;

                stencilDepthTest.mStencilTest = true;
                stencilDepthTest.mDepthFunc = CMP_GEQUAL;
                stencilDepthTest.mStencilFrontFunc = CMP_ALWAYS;
                stencilDepthTest.mStencilFrontPass = STENCIL_OP_KEEP;
                stencilDepthTest.mStencilFrontFail = STENCIL_OP_KEEP;
                stencilDepthTest.mDepthFrontFail = STENCIL_OP_REPLACE;
                stencilDepthTest.mStencilBackFunc = CMP_ALWAYS;
                stencilDepthTest.mStencilBackPass = STENCIL_OP_KEEP;
                stencilDepthTest.mStencilBackFail = STENCIL_OP_KEEP;
                stencilDepthTest.mDepthBackFail = STENCIL_OP_REPLACE;
                stencilDepthTest.mStencilWriteMask = 0xff;
                stencilDepthTest.mStencilReadMask = 0xff;

                RasterizerStateDesc rasterizerStateDesc{};
                rasterizerStateDesc.mFrontFace = FrontFace::FRONT_FACE_CCW;
                rasterizerStateDesc.mCullMode = CULL_MODE_FRONT;
                rasterizerStateDesc.mFillMode = FILL_MODE_SOLID;

                PipelineDesc pipelineDesc{};
                pipelineDesc.mType = PIPELINE_TYPE_GRAPHICS;
                auto& pipelineSettings = pipelineDesc.mGraphicsDesc;
                pipelineSettings.mPrimitiveTopo = PRIMITIVE_TOPO_TRI_LIST;
                pipelineSettings.mRenderTargetCount = colorFormats.size();
                pipelineSettings.pColorFormats = colorFormats.data();
                pipelineSettings.mSampleCount = SAMPLE_COUNT_1;
                pipelineSettings.mSampleQuality = 0;
                pipelineSettings.pBlendState = &blendStateDesc;
                pipelineSettings.mDepthStencilFormat = DepthBufferFormat;
                pipelineSettings.pRootSignature = m_lightPassRootSignature.m_handle;
                pipelineSettings.pVertexLayout = &vertexLayout;
                pipelineSettings.pDepthState = &stencilDepthTest;
                pipelineSettings.pRasterizerState = &rasterizerStateDesc;
                pipelineSettings.pShaderProgram = m_stencilLightShader.m_handle;
                m_lightStencilPipelineCCW.Load(forgeRenderer->Rend(), [&](Pipeline** handle) {
                    addPipeline(forgeRenderer->Rend(), &pipelineDesc, handle);
                    return true;
                });

                rasterizerStateDesc.mFrontFace = FrontFace::FRONT_FACE_CW;
                m_lightStencilPipelineCW.Load(forgeRenderer->Rend(), [&](Pipeline** handle) {
                    addPipeline(forgeRenderer->Rend(), &pipelineDesc, handle);
                    return true;
                });
            }

            for (auto& lightSet : m_lightPerLightSet) {
                lightSet.Load(forgeRenderer->Rend(), [&](DescriptorSet** set) {
                    DescriptorSetDesc batchDescriptorSet{ m_lightPassRootSignature.m_handle,
                                                          DESCRIPTOR_UPDATE_FREQ_PER_BATCH,
                                                          cRendererDeferred::MaxLightUniforms };
                    addDescriptorSet(forgeRenderer->Rend(), &batchDescriptorSet, set);
                    return true;
                });
            }
            for (size_t setIndex = 0; setIndex < m_lightPerFrameSet.size(); setIndex++) {
                m_lightPerFrameSet[setIndex].Load(forgeRenderer->Rend(), [&](DescriptorSet** set) {
                    DescriptorSetDesc perFrameDescSet{ m_lightPassRootSignature.m_handle,
                                                       DESCRIPTOR_UPDATE_FREQ_PER_FRAME,
                                                       MaxViewportFrameDescriptors  };
                    addDescriptorSet(forgeRenderer->Rend(), &perFrameDescSet, set);
                    return true;
                });
                for(uint32_t i = 0; i < MaxViewportFrameDescriptors; i++) {
                    std::array<DescriptorData, 1> params = {};
                    params[0].pName = "lightObjectBuffer";
                    params[0].ppBuffers = &m_lightPassBuffer[setIndex].m_handle;
                    updateDescriptorSet(forgeRenderer->Rend(), i, m_lightPerFrameSet[setIndex].m_handle, params.size(), params.data());
                }
            }
        }

        auto createShadowMap = [&](const UVector3& avSize) -> ShadowMapData {
            ShadowMapData shadowMapData = {};
            shadowMapData.m_target.Load(forgeRenderer->Rend(), [&](RenderTarget** target) {
                RenderTargetDesc renderTarget = {};
                renderTarget.mArraySize = 1;
                renderTarget.mClearValue.depth = 1.0f;
                renderTarget.mDepth = 1;
                renderTarget.mFormat = ShadowDepthBufferFormat;
                renderTarget.mWidth = avSize.getX();
                renderTarget.mHeight = avSize.getY();
                renderTarget.mDescriptors = DESCRIPTOR_TYPE_TEXTURE;
                renderTarget.mSampleCount = SAMPLE_COUNT_1;
                renderTarget.mSampleQuality = 0;
                renderTarget.mStartState = RESOURCE_STATE_SHADER_RESOURCE;
                renderTarget.pName = "ShadowMaps RTs";
                addRenderTarget(forgeRenderer->Rend(), &renderTarget, target);
                return true;
            });

            shadowMapData.m_transformCount = -1;
            shadowMapData.m_frameCount = -1;
            shadowMapData.m_radius = 0;
            shadowMapData.m_fov = 0;
            shadowMapData.m_aspect = 0;
            shadowMapData.m_light = nullptr;

            shadowMapData.m_pool.Load(forgeRenderer->Rend(), [&](CmdPool** pool) {
                CmdPoolDesc cmdPoolDesc = {};
                cmdPoolDesc.pQueue = forgeRenderer->GetGraphicsQueue();
                addCmdPool(forgeRenderer->Rend(), &cmdPoolDesc, pool);
                return true;
            });

            shadowMapData.m_cmd.Load(forgeRenderer->Rend(),  [&](Cmd** cmd) {
                CmdDesc cmdDesc = {};
                cmdDesc.pPool = shadowMapData.m_pool.m_handle;
                addCmd(forgeRenderer->Rend(), &cmdDesc, cmd);
                return true;
            });
            shadowMapData.m_shadowFence.Load(forgeRenderer->Rend(), [&](Fence**  fence) {
                addFence(forgeRenderer->Rend(), fence);
                return true;
            });
            return shadowMapData;
        };

        for (size_t i = 0; i < 32; ++i) {
            m_shadowMapData[eShadowMapResolution_High].emplace_back(createShadowMap(shadowSizes[startSize + eShadowMapResolution_High]));
        }
        for (size_t i = 0; i < 32; ++i) {
            m_shadowMapData[eShadowMapResolution_Medium].emplace_back(createShadowMap(shadowSizes[startSize + eShadowMapResolution_Medium]));
        }
        for (size_t i = 0; i < 32; ++i) {
            m_shadowMapData[eShadowMapResolution_Low].emplace_back(createShadowMap(shadowSizes[startSize + eShadowMapResolution_Low]));
        }

        ////////////////////////////////////
        // Create light shapes
        tFlag lVtxFlag = eVertexElementFlag_Position | eVertexElementFlag_Color0 | eVertexElementFlag_Texture0;
        auto loadVertexBufferFromMesh = [&](const tString& meshName, tVertexElementFlag vtxFlag) {
            iVertexBuffer* pVtxBuffer = mpResources->GetMeshManager()->CreateVertexBufferFromMesh(meshName, vtxFlag);
            if (pVtxBuffer == NULL) {
                FatalError("Could not load vertex buffer from mesh '%s'\n", meshName.c_str());
            }
            return pVtxBuffer;
        };
        m_shapeSphere[eDeferredShapeQuality_High] =
            std::unique_ptr<iVertexBuffer>(loadVertexBufferFromMesh("core_12_12_sphere.dae", lVtxFlag));
        m_shapeSphere[eDeferredShapeQuality_Medium] =
            std::unique_ptr<iVertexBuffer>(loadVertexBufferFromMesh("core_7_7_sphere.dae", lVtxFlag));
        m_shapeSphere[eDeferredShapeQuality_Low] =
            std::unique_ptr<iVertexBuffer>(loadVertexBufferFromMesh("core_5_5_sphere.dae", lVtxFlag));
        m_shapePyramid = std::unique_ptr<iVertexBuffer>(loadVertexBufferFromMesh("core_pyramid.dae", lVtxFlag));
        m_box = std::unique_ptr<iVertexBuffer>(loadVertexBufferFromMesh("core_box.dae", lVtxFlag));
        ////////////////////////////////////
        // Batch vertex buffer
    }

    cRendererDeferred::~cRendererDeferred() {
    }

    bool cRendererDeferred::LoadData() {
        return true;
    }

    void cRendererDeferred::DestroyData() {
    }

    uint32_t cRendererDeferred::updateFrameDescriptor(
        const ForgeRenderer::Frame& frame, Cmd* cmd, cWorld* apWorld, const PerFrameOption& options) {
        uint32_t index = m_materialSet.m_frameIndex;
        BufferUpdateDesc updatePerFrameConstantsDesc = { m_perFrameBuffer[index].m_handle, 0 };
        beginUpdateResource(&updatePerFrameConstantsDesc);
        auto* uniformFrameData = reinterpret_cast<UniformPerFrameData*>(updatePerFrameConstantsDesc.pMappedData);
        uniformFrameData->m_viewMatrix = cMath::ToForgeMat(options.m_viewMat.GetTranspose());
        uniformFrameData->m_invViewMatrix = cMath::ToForgeMat(cMath::MatrixInverse(options.m_viewMat).GetTranspose());
        uniformFrameData->m_projectionMatrix = cMath::ToForgeMat(options.m_projectionMat.GetTranspose());
        uniformFrameData->m_viewProjectionMatrix = cMath::ToForgeMat(cMath::MatrixMul(options.m_projectionMat, options.m_viewMat).GetTranspose());

        uniformFrameData->worldFogStart = apWorld->GetFogStart();
        uniformFrameData->worldFogLength = apWorld->GetFogEnd() - apWorld->GetFogStart();
        uniformFrameData->oneMinusFogAlpha = 1.0f - apWorld->GetFogColor().a;
        uniformFrameData->fogFalloffExp = apWorld->GetFogFalloffExp();

        uniformFrameData->m_invViewRotation = cMath::ToForgeMat((options.m_viewMat.GetTranspose()).GetRotation().GetTranspose());
        uniformFrameData->viewTexel = float2(1.0f / options.m_size.x, 1.0f / options.m_size.y);
        uniformFrameData->viewportSize = float2(options.m_size.x, options.m_size.y);
        const auto fogColor = apWorld->GetFogColor();
        uniformFrameData->fogColor = float4(fogColor.r, fogColor.g, fogColor.b, fogColor.a);
        endUpdateResource(&updatePerFrameConstantsDesc, NULL);

        m_materialSet.m_frameIndex = (m_materialSet.m_frameIndex + 1) % MaxViewportFrameDescriptors;
        return index;
    }

    void cRendererDeferred::cmdIlluminationPass(Cmd* cmd,
        const ForgeRenderer::Frame& frame,
        cRenderList& renderList,
        uint32_t frameDescriptorIndex,
        RenderTarget* depthBuffer,
        RenderTarget* outputBuffer,
        AdditionalIlluminationPassOptions options
    ) {
        uint32_t materialObjectIndex = getDescriptorIndexFromName(m_materialRootSignature.m_handle, "materialRootConstant");
        LoadActionsDesc loadActions = {};
        loadActions.mLoadActionsColor[0] = LOAD_ACTION_LOAD;
        loadActions.mLoadActionDepth = LOAD_ACTION_LOAD;
        std::array targets = {
            outputBuffer,
        };
        cmdBindRenderTargets(cmd, targets.size(), targets.data(), depthBuffer, &loadActions, nullptr, nullptr, -1, -1);
        cmdSetViewport(cmd, 0.0f, 0.0f, outputBuffer->mWidth, outputBuffer->mHeight, 0.0f, 1.0f);
        cmdSetScissor(cmd, 0, 0, outputBuffer->mWidth, outputBuffer->mHeight);
        cmdBindPipeline(cmd, options.m_invert ? m_solidIlluminationPipelineCW.m_handle: m_solidIlluminationPipelineCCW.m_handle);


        // for DirectX12 these frame descriptors are not used so its ommitied for the draw
//#if defined(VULKAN)
//	    if (frame.m_renderer->GetApi() == RENDERER_API_VULKAN) {
            cmdBindDescriptorSet(cmd, frameDescriptorIndex, m_materialSet.m_frameSet[frame.m_frameIndex].m_handle);
//        }
//#endif
        for (auto& illuminationItem : m_rendererList.GetRenderableItems(eRenderListType_Illumination)) {
            cMaterial* pMaterial = illuminationItem->GetMaterial();
            iVertexBuffer* vertexBuffer = illuminationItem->GetVertexBuffer();
            if (pMaterial == nullptr || vertexBuffer == nullptr) {
                continue;
            }
            ASSERT(pMaterial->Descriptor().m_id == MaterialID::SolidDiffuse && "Invalid material type");
            MaterialRootConstant materialConst = {};
            uint32_t instance = cmdBindMaterialAndObject(cmd, frame, pMaterial, illuminationItem);
            materialConst.objectId = instance;
            materialConst.m_afT = illuminationItem->GetIlluminationAmount();
            std::array targets = {
                eVertexBufferElement_Position,
                eVertexBufferElement_Texture0,
            };
            LegacyVertexBuffer::GeometryBinding binding;
            static_cast<LegacyVertexBuffer*>(vertexBuffer)->resolveGeometryBinding(frame.m_currentFrame, targets, &binding);
            detail::cmdDefaultLegacyGeomBinding(cmd, frame, binding);
            cmdBindPushConstants(cmd, m_materialRootSignature.m_handle, materialObjectIndex, &materialConst);
            cmdDrawIndexed(cmd, binding.m_indexBuffer.numIndicies, 0, 0);
        }
    }
    void cRendererDeferred::cmdLightPass(
        Cmd* cmd,
        const ForgeRenderer::Frame& frame,
        cWorld* apWorld,
        cFrustum* apFrustum,
        cRenderList& renderList,
        uint32_t frameDescriptorIndex,
        RenderTarget* colorBuffer,
        RenderTarget* normalBuffer,
        RenderTarget* positionBuffer,
        RenderTarget* specularBuffer,
        RenderTarget* depthBuffer,
        RenderTarget* outputBuffer,
        cMatrixf viewMat,
        cMatrixf invViewMat,
        cMatrixf projectionMat,
        AdditionalLightPassOptions options) {
        uint32_t materialObjectIndex = getDescriptorIndexFromName(m_materialRootSignature.m_handle, "materialRootConstant");

        folly::small_vector<cPlanef, 3> occlusionPlanes;
        if(apWorld->GetFogActive() && apWorld->GetFogColor().a >= 1.0f && apWorld->GetFogCulling()) {
            cPlanef fogPlane;
            fogPlane.FromNormalPoint(apFrustum->GetForward(), apFrustum->GetOrigin() + apFrustum->GetForward() * -apWorld->GetFogEnd());
            occlusionPlanes.push_back(fogPlane);
        }

        // --------------------------------------------------------------------
        // Render Light Pass
        // --------------------------------------------------------------------
        {

            float fScreenArea = (float)(outputBuffer->mWidth * outputBuffer->mHeight);
            int mlMinLargeLightArea = (int)(MinLargeLightNormalizedArea * fScreenArea);

            // std::array<std::vector<detail::DeferredLight*>, eDeferredLightList_LastEnum> sortedLights;
            // DON'T touch deferredLights after this point
            std::vector<detail::DeferredLight> deferredLights;
            std::vector<detail::DeferredLight*> deferredLightRenderBack;
            std::vector<detail::DeferredLight*> deferredLightStencilFront;

            for (auto& light : renderList.GetLights()) {
                auto lightType = light->GetLightType();
                auto& deferredLightData = deferredLights.emplace_back(detail::DeferredLight());
                deferredLightData.m_light = light;
                if (lightType == eLightType_Box) {
                    continue;
                }
                switch (lightType) {
                case eLightType_Point:
                    {
                        deferredLightData.m_insideNearPlane = apFrustum->CheckSphereNearPlaneIntersection(
                            light->GetWorldPosition(), light->GetRadius() * kLightRadiusMul_Low);
                        detail::SetupLightMatrix(
                            deferredLightData.m_mtxViewSpaceRender,
                            deferredLightData.m_mtxViewSpaceTransform,
                            light,
                            apFrustum,
                            kLightRadiusMul_Medium);
                        deferredLightData.m_clipRect = cMath::GetClipRectFromSphere(
                            deferredLightData.m_mtxViewSpaceRender.GetTranslation(), light->GetRadius(), apFrustum, cVector2l(outputBuffer->mWidth, outputBuffer->mHeight), true, 0);
                        break;
                    }
                case eLightType_Spot:
                    {
                        cLightSpot* lightSpot = static_cast<cLightSpot*>(light);
                        deferredLightData.m_insideNearPlane = apFrustum->CheckFrustumNearPlaneIntersection(lightSpot->GetFrustum());
                        detail::SetupLightMatrix(
                            deferredLightData.m_mtxViewSpaceRender,
                            deferredLightData.m_mtxViewSpaceTransform,
                            light,
                            apFrustum,
                            kLightRadiusMul_Medium);
                        cMath::GetClipRectFromBV(deferredLightData.m_clipRect, *light->GetBoundingVolume(), apFrustum, cVector2l(outputBuffer->mWidth, outputBuffer->mHeight), 0);
                        break;
                    }
                default:
                    break;
                }
            }

            for (auto& deferredLight : deferredLights) {
                auto light = deferredLight.m_light;
                auto lightType = deferredLight.m_light->GetLightType();

                if (lightType == eLightType_Spot && light->GetCastShadows() && mpCurrentSettings->mbRenderShadows) {
                    cLightSpot* pLightSpot = static_cast<cLightSpot*>(light);

                    bool castShadow = false;
                    eShadowMapResolution shadowMapResolution = eShadowMapResolution_Low;
                    ////////////////////////
                    // Inside near plane, use max resolution
                    if (deferredLight.m_insideNearPlane) {
                        castShadow = true;

                        shadowMapResolution = rendering::detail::GetShadowMapResolution(
                            light->GetShadowMapResolution(), mpCurrentSettings->mMaxShadowMapResolution);
                    } else {
                        cVector3f vIntersection = pLightSpot->GetFrustum()->GetOrigin();
                        pLightSpot->GetFrustum()->CheckLineIntersection(
                            apFrustum->GetOrigin(), light->GetBoundingVolume()->GetWorldCenter(), vIntersection);

                        float fDistToLight = cMath::Vector3Dist(apFrustum->GetOrigin(), vIntersection);

                        castShadow = true;
                        shadowMapResolution = rendering::detail::GetShadowMapResolution(
                            light->GetShadowMapResolution(), mpCurrentSettings->mMaxShadowMapResolution);

                        ///////////////////////
                        // Skip shadow
                        if (fDistToLight > ShadowDistanceNone) {
                            castShadow = false;
                        }
                        ///////////////////////
                        // Use Low
                        else if (fDistToLight > ShadowDistanceLow) {
                            if (shadowMapResolution == eShadowMapResolution_Low) {
                                castShadow = false;
                            }
                            shadowMapResolution = eShadowMapResolution_Low;
                        }
                        ///////////////////////
                        // Use Medium
                        else if (fDistToLight > ShadowDistanceMedium) {
                            if (shadowMapResolution == eShadowMapResolution_High) {
                                shadowMapResolution = eShadowMapResolution_Medium;
                            } else {
                                shadowMapResolution = eShadowMapResolution_Low;
                            }
                        }
                    }

                    cFrustum* pLightFrustum = pLightSpot->GetFrustum();
                    std::vector<iRenderable*> shadowCasters;
                    if (castShadow &&
                        detail::SetupShadowMapRendering(shadowCasters, apWorld, pLightFrustum, pLightSpot, occlusionPlanes)) {
                        auto findBestShadowMap = [&](eShadowMapResolution resolution, iLight* light) -> cRendererDeferred::ShadowMapData* {
                            auto& shadowMapVec = m_shadowMapData[resolution];
                            uint32_t maxFrameDistance = 0;
                            size_t bestIndex = 0;
                            for (size_t i = 0; i < shadowMapVec.size(); ++i) {
                                auto& shadowMap = shadowMapVec[i];
                                if (shadowMap.m_light == light) {
                                    shadowMap.m_frameCount = frame.m_currentFrame;
                                    return &shadowMap;
                                }

                                const uint32_t frameDist = frame.m_currentFrame - shadowMap.m_frameCount;
                                if (frameDist > maxFrameDistance) {
                                    maxFrameDistance = frameDist;
                                    bestIndex = i;
                                }
                            }
                            shadowMapVec[bestIndex].m_frameCount = frame.m_currentFrame;
                            return &shadowMapVec[bestIndex];
                        };
                        auto* shadowMapData = findBestShadowMap(shadowMapResolution, pLightSpot);
                        if (shadowMapData) {
                            deferredLight.m_shadowMapData = shadowMapData;
                            // testing if the shadow map needs to be updated
                            if (shadowMapData->m_transformCount != pLightSpot->GetTransformUpdateCount() || [&]() -> bool {
                                    // Check if texture map and light are valid
                                    if (pLightSpot->GetOcclusionCullShadowCasters()) {
                                        return true;
                                    }

                                    if (pLightSpot->GetLightType() == eLightType_Spot &&
                                        (pLightSpot->GetAspect() != shadowMapData->m_aspect ||
                                         pLightSpot->GetFOV() != shadowMapData->m_fov)) {
                                        return true;
                                    }
                                    return !pLightSpot->ShadowCastersAreUnchanged(shadowCasters);
                                }()) {
                                shadowMapData->m_light = pLightSpot;
                                shadowMapData->m_transformCount = pLightSpot->GetTransformUpdateCount();
                                shadowMapData->m_radius = pLightSpot->GetRadius();
                                shadowMapData->m_aspect = pLightSpot->GetAspect();
                                shadowMapData->m_fov = pLightSpot->GetFOV();

                                pLightSpot->SetShadowCasterCacheFromVec(shadowCasters);

                                FenceStatus fenceStatus;
                                getFenceStatus(frame.m_renderer->Rend(), shadowMapData->m_shadowFence.m_handle, &fenceStatus);
                                if (fenceStatus == FENCE_STATUS_INCOMPLETE)
                                    waitForFences(frame.m_renderer->Rend(), 1, &shadowMapData->m_shadowFence.m_handle);
                                resetCmdPool(frame.m_renderer->Rend(), shadowMapData->m_pool.m_handle);

                                beginCmd(shadowMapData->m_cmd.m_handle);
                                {
                                    cmdBindRenderTargets(shadowMapData->m_cmd.m_handle, 0, NULL, NULL, NULL, NULL, NULL, -1, -1);
                                    std::array rtBarriers = {
                                        RenderTargetBarrier{
                                            shadowMapData->m_target.m_handle, RESOURCE_STATE_SHADER_RESOURCE, RESOURCE_STATE_DEPTH_WRITE },
                                    };
                                    cmdResourceBarrier(shadowMapData->m_cmd.m_handle, 0, NULL, 0, NULL, rtBarriers.size(), rtBarriers.data());
                                }

                                LoadActionsDesc loadActions = {};
                                loadActions.mLoadActionDepth = LOAD_ACTION_CLEAR;
                                loadActions.mLoadActionStencil = LOAD_ACTION_DONTCARE;
                                loadActions.mClearDepth = { .depth = 1.0f, .stencil = 0 };
                                cmdBindRenderTargets(
                                    shadowMapData->m_cmd.m_handle, 0, NULL, shadowMapData->m_target.m_handle, &loadActions, NULL, NULL, -1, -1);
                                cmdSetViewport(
                                    shadowMapData->m_cmd.m_handle,
                                    0.0f,
                                    static_cast<float>(shadowMapData->m_target.m_handle->mHeight),
                                    static_cast<float>(shadowMapData->m_target.m_handle->mWidth),
                                    -static_cast<float>(shadowMapData->m_target.m_handle->mHeight),
                                    0.0f,
                                    1.0f);
                                cmdSetScissor(
                                    shadowMapData->m_cmd.m_handle,
                                    0,
                                    0,
                                    shadowMapData->m_target.m_handle->mWidth,
                                    shadowMapData->m_target.m_handle->mHeight);
                                cmdBindPipeline(shadowMapData->m_cmd.m_handle, options.m_invert ? m_zPassShadowPipelineCCW.m_handle : m_zPassShadowPipelineCW.m_handle);

                                uint32_t shadowFrameIndex = updateFrameDescriptor(
                                    frame,
                                    shadowMapData->m_cmd.m_handle,
                                    apWorld,
                                    { .m_size = float2(outputBuffer->mWidth, outputBuffer->mHeight),
                                      .m_viewMat = pLightFrustum->GetViewMatrix(),
                                      .m_projectionMat = pLightFrustum->GetProjectionMatrix() });
                                cmdBindDescriptorSet(
                                    shadowMapData->m_cmd.m_handle, shadowFrameIndex, m_materialSet.m_frameSet[frame.m_frameIndex].m_handle);
                                for (auto& pObject : shadowCasters) {
                                    eMaterialRenderMode renderMode =
                                        pObject->GetCoverageAmount() >= 1 ? eMaterialRenderMode_Z : eMaterialRenderMode_Z_Dissolve;
                                    cMaterial* pMaterial = pObject->GetMaterial();
                                    const MaterialDescriptor& descriptor = pMaterial->Descriptor();
                                    iVertexBuffer* vertexBuffer = pObject->GetVertexBuffer();
                                    if (vertexBuffer == nullptr || descriptor.m_id == MaterialID::Unknown) {
                                        return;
                                    }
                                    MaterialRootConstant materialConst = {};
                                    uint32_t instance = cmdBindMaterialAndObject(shadowMapData->m_cmd.m_handle, frame, pMaterial, pObject);
                                    materialConst.objectId = instance;
                                    std::array targets = { eVertexBufferElement_Position,
                                                           eVertexBufferElement_Texture0,
                                                           eVertexBufferElement_Normal,
                                                           eVertexBufferElement_Texture1Tangent };
                                    LegacyVertexBuffer::GeometryBinding binding{};
                                    static_cast<LegacyVertexBuffer*>(vertexBuffer)
                                        ->resolveGeometryBinding(frame.m_currentFrame, targets, &binding);
                                    detail::cmdDefaultLegacyGeomBinding(shadowMapData->m_cmd.m_handle, frame, binding);
                                    cmdBindPushConstants(
                                        shadowMapData->m_cmd.m_handle, m_materialRootSignature.m_handle, materialObjectIndex, &materialConst);
                                    cmdDrawIndexed(shadowMapData->m_cmd.m_handle, binding.m_indexBuffer.numIndicies, 0, 0);
                                }
                                {
                                    cmdBindRenderTargets(shadowMapData->m_cmd.m_handle, 0, NULL, NULL, NULL, NULL, NULL, -1, -1);
                                    std::array rtBarriers = {
                                        RenderTargetBarrier{
                                            shadowMapData->m_target.m_handle, RESOURCE_STATE_DEPTH_WRITE, RESOURCE_STATE_SHADER_RESOURCE },
                                    };
                                    cmdResourceBarrier(shadowMapData->m_cmd.m_handle, 0, NULL, 0, NULL, rtBarriers.size(), rtBarriers.data());
                                }
                                endCmd(shadowMapData->m_cmd.m_handle);
                                QueueSubmitDesc submitDesc = {};
                                submitDesc.mCmdCount = 1;
                                submitDesc.ppCmds = &shadowMapData->m_cmd.m_handle;
                                submitDesc.pSignalFence = shadowMapData->m_shadowFence.m_handle;
                                submitDesc.mSubmitDone = true;
                                queueSubmit(frame.m_renderer->GetGraphicsQueue(), &submitDesc);
                            }
                        }
                    }
                    // render shadow map
                }
            }

            mpCurrentSettings->mlNumberOfLightsRendered = 0;
            for (auto& deferredLight : deferredLights) {
                // cDeferredLight* pLightData =  mvTempDeferredLights[i];
                iLight* pLight = deferredLight.m_light;
                eLightType lightType = pLight->GetLightType();

                ////////////////////////
                // If box, we have special case...
                if (lightType == eLightType_Box) {
                    cLightBox* pLightBox = static_cast<cLightBox*>(pLight);

                    // Set up matrix
                    deferredLight.m_mtxViewSpaceRender = cMath::MatrixScale(pLightBox->GetSize());
                    deferredLight.m_mtxViewSpaceRender.SetTranslation(pLightBox->GetWorldPosition());
                    deferredLight.m_mtxViewSpaceRender = cMath::MatrixMul(apFrustum->GetViewMatrix(), deferredLight.m_mtxViewSpaceRender);

                    mpCurrentSettings->mlNumberOfLightsRendered++;

                    // Check if near plane is inside box. If so only render back
                    if (apFrustum->CheckBVNearPlaneIntersection(pLight->GetBoundingVolume())) {
                        deferredLightRenderBack.push_back(&deferredLight);
                    } else {
                        deferredLightStencilFront.push_back(&deferredLight);
                    }

                    continue;
                }

                mpCurrentSettings->mlNumberOfLightsRendered++;

                deferredLightRenderBack.emplace_back(&deferredLight);
            }
            std::sort(deferredLightStencilFront.begin(), deferredLightStencilFront.end(), detail::SortDeferredLightDefault);
            std::sort(deferredLightRenderBack.begin(), deferredLightRenderBack.end(), detail::SortDeferredLightDefault);

            {
                // updates and binds the light data
                auto cmdBindLightDescriptor = [&](detail::DeferredLight* light) {
                    DescriptorData params[10] = {};
                    size_t paramCount = 0;
                    UniformLightData uniformObjectData = {};

                    const auto modelViewMtx = cMath::MatrixMul(apFrustum->GetViewMatrix(), light->m_light->GetWorldMatrix());
                    const auto viewProjectionMat = cMath::MatrixMul(projectionMat, viewMat);

                    switch (light->m_light->GetLightType()) {
                    case eLightType_Point:
                        {
                            uniformObjectData.m_common.m_mvp = cMath::ToForgeMat4(
                                cMath::MatrixMul(
                                    viewProjectionMat, cMath::MatrixMul(light->m_light->GetWorldMatrix(), detail::GetLightMtx(*light)))
                                    .GetTranspose());

                            cVector3f lightViewPos = cMath::MatrixMul(modelViewMtx, detail::GetLightMtx(*light)).GetTranslation();
                            const auto color = light->m_light->GetDiffuseColor();

                            if (light->m_light->GetGoboTexture()) {
                                uniformObjectData.m_common.m_config |= LightConfiguration::HasGoboMap;
                                params[paramCount].pName = "goboCubeMap";
                                params[paramCount++].ppTextures = &light->m_light->GetGoboTexture()->GetTexture().m_handle;
                                m_lightResources[frame.m_frameIndex][m_lightIndex].m_goboCubeMap =
                                    light->m_light->GetGoboTexture()->GetTexture();
                            }

                            auto falloffMap = light->m_light->GetFalloffMap();
                            ASSERT(falloffMap && "Point light needs a falloff map");
                            params[paramCount].pName = "attenuationLightMap";
                            params[paramCount++].ppTextures = &falloffMap->GetTexture().m_handle;
                            m_lightResources[frame.m_frameIndex][m_lightIndex].m_attenuationLightMap = falloffMap->GetTexture();

                            uniformObjectData.m_pointLight.m_radius = light->m_light->GetRadius();
                            uniformObjectData.m_pointLight.m_lightPos = float3(lightViewPos.x, lightViewPos.y, lightViewPos.z);
                            uniformObjectData.m_pointLight.m_lightColor = float4(color.r, color.g, color.b, color.a);
                            cMatrixf mtxInvViewRotation =
                                cMath::MatrixMul(light->m_light->GetWorldMatrix(), invViewMat).GetTranspose();
                            uniformObjectData.m_pointLight.m_invViewRotation = cMath::ToForgeMat4(mtxInvViewRotation);
                            break;
                        }
                    case eLightType_Spot:
                        {
                            cLightSpot* pLightSpot = static_cast<cLightSpot*>(light->m_light);

                            cMatrixf spotViewProj = cMath::MatrixMul(pLightSpot->GetViewProjMatrix(), invViewMat).GetTranspose();
                            cVector3f forward = cMath::MatrixMul3x3(light->m_mtxViewSpaceTransform, cVector3f(0, 0, 1));
                            cVector3f lightViewPos = cMath::MatrixMul(modelViewMtx, detail::GetLightMtx(*light)).GetTranslation();
                            const auto color = pLightSpot->GetDiffuseColor();

                            uniformObjectData.m_common.m_mvp = cMath::ToForgeMat4(
                                cMath::MatrixMul(
                                    viewProjectionMat, cMath::MatrixMul(light->m_light->GetWorldMatrix(), detail::GetLightMtx(*light)))
                                    .GetTranspose());

                            if (light->m_shadowMapData) {
                                uniformObjectData.m_common.m_config |= LightConfiguration::HasShadowMap;
                                params[paramCount].pName = "shadowMap";
                                params[paramCount++].ppTextures = &light->m_shadowMapData->m_target.m_handle->pTexture;
                            }
                            if (m_shadowJitterTexture.IsValid()) {
                                params[paramCount].pName = "shadowOffsetMap";
                                params[paramCount++].ppTextures = &m_shadowJitterTexture.m_handle;
                            }
                            uniformObjectData.m_spotLight.m_spotViewProj = cMath::ToForgeMat4(spotViewProj);
                            uniformObjectData.m_spotLight.m_oneMinusCosHalfSpotFOV = 1 - pLightSpot->GetCosHalfFOV();
                            uniformObjectData.m_spotLight.m_radius = light->m_light->GetRadius();
                            uniformObjectData.m_spotLight.m_forward = float3(forward.x, forward.y, forward.z);
                            uniformObjectData.m_spotLight.m_color = float4(color.r, color.g, color.b, color.a);
                            uniformObjectData.m_spotLight.m_pos = float3(lightViewPos.x, lightViewPos.y, lightViewPos.z);

                            auto goboImage = light->m_light->GetGoboTexture();
                            auto spotFallOffImage = pLightSpot->GetSpotFalloffMap();
                            auto spotAttenuationImage = pLightSpot->GetFalloffMap();
                            if (goboImage) {
                                uniformObjectData.m_common.m_config |= LightConfiguration::HasGoboMap;
                                params[paramCount].pName = "goboMap";
                                params[paramCount++].ppTextures = &light->m_light->GetGoboTexture()->GetTexture().m_handle;
                                frame.m_resourcePool->Push(light->m_light->GetGoboTexture()->GetTexture());
                                m_lightResources[frame.m_frameIndex][m_lightIndex].m_goboMap = light->m_light->GetGoboTexture()->GetTexture();
                            } else {
                                params[paramCount].pName = "falloffMap";
                                params[paramCount++].ppTextures = &spotFallOffImage->GetTexture().m_handle;
                                frame.m_resourcePool->Push(spotFallOffImage->GetTexture());
                                m_lightResources[frame.m_frameIndex][m_lightIndex].m_falloffMap = spotFallOffImage->GetTexture();
                            }
                            m_lightResources[frame.m_frameIndex][m_lightIndex].m_attenuationLightMap = spotAttenuationImage->GetTexture();
                            params[paramCount].pName = "attenuationLightMap";
                            params[paramCount++].ppTextures = &spotAttenuationImage->GetTexture().m_handle;
                            frame.m_resourcePool->Push(spotAttenuationImage->GetTexture());
                            break;
                        }
                    case eLightType_Box:
                        {
                            uniformObjectData.m_common.m_mvp =
                                cMath::ToForgeMat4(cMath::MatrixMul(viewProjectionMat, detail::GetLightMtx(*light)).GetTranspose());

                            cLightBox* pLightBox = static_cast<cLightBox*>(light->m_light);
                            const auto& color = light->m_light->GetDiffuseColor();
                            uniformObjectData.m_boxLight.m_lightColor = float4(color.r, color.g, color.b, color.a);
                            break;
                        }
                    default:
                        {
                            ASSERT(false && "Unsupported light type");
                            break;
                        }
                    }

                    BufferUpdateDesc updateDesc = { m_lightPassBuffer[frame.m_frameIndex].m_handle, m_lightIndex* sizeof(UniformLightData) };
                    beginUpdateResource(&updateDesc);
                    memcpy(updateDesc.pMappedData, &uniformObjectData, sizeof(UniformLightData));
                    endUpdateResource(&updateDesc, NULL);

                    updateDescriptorSet(
                        frame.m_renderer->Rend(), m_lightIndex, m_lightPerLightSet[frame.m_frameIndex].m_handle, paramCount, params);
                    cmdBindDescriptorSet(cmd, m_lightIndex, m_lightPerLightSet[frame.m_frameIndex].m_handle);
                    uint32_t index = m_lightIndex;
                    m_lightIndex++;
                    return index;
                };
                {
                    LoadActionsDesc loadActions = {};
                    loadActions.mLoadActionsColor[0] = LOAD_ACTION_CLEAR;
                    loadActions.mLoadActionDepth = LOAD_ACTION_LOAD;
                    loadActions.mLoadActionStencil = LOAD_ACTION_CLEAR;
                    std::array targets = {
                        outputBuffer,
                    };
                    cmdBindRenderTargets(
                        cmd,
                        targets.size(),
                        targets.data(),
                        depthBuffer,
                        &loadActions,
                        nullptr,
                        nullptr,
                        -1,
                        -1);
                }
                cmdSetViewport(cmd, 0.0f, 0.0f, outputBuffer->mWidth, outputBuffer->mHeight, 0.0f, 1.0f);
                cmdSetScissor(cmd, 0, 0, outputBuffer->mWidth, outputBuffer->mHeight);

                {
                    DescriptorData params[15] = {};
                    size_t paramCount = 0;

                    DescriptorDataRange range = { (uint32_t)(frame.m_frameIndex * sizeof(UniformLightPerFrameSet)),
                                                  sizeof(UniformLightPerFrameSet) };
                    params[paramCount].pName = "diffuseMap";
                    params[paramCount++].ppTextures = &colorBuffer->pTexture;
                    params[paramCount].pName = "normalMap";
                    params[paramCount++].ppTextures = &normalBuffer->pTexture;
                    params[paramCount].pName = "positionMap";
                    params[paramCount++].ppTextures = &positionBuffer->pTexture;
                    params[paramCount].pName = "specularMap";
                    params[paramCount++].ppTextures = &specularBuffer->pTexture;
                    updateDescriptorSet(frame.m_renderer->Rend(), frameDescriptorIndex, m_lightPerFrameSet[frame.m_frameIndex].m_handle, paramCount, params);
                }

                cmdBindDescriptorSet(cmd, frameDescriptorIndex, m_lightPerFrameSet[frame.m_frameIndex].m_handle);

                // --------------------------------------------------------
                // Draw Point Lights
                // Draw Spot Lights
                // Draw Box Lights
                // --------------------------------------------------------
                cmdBeginDebugMarker(cmd, 0, 1, 0, "Point Light Deferred Stencil front");
                uint32_t lightRootConstantIndex = getDescriptorIndexFromName(m_lightPassRootSignature.m_handle, "lightPushConstant");
                for (auto& light : deferredLightStencilFront) {
                    cmdSetStencilReferenceValue(cmd, 0xff);

                    std::array elements = { eVertexBufferElement_Position };
                    LegacyVertexBuffer::GeometryBinding binding{};
                    auto lightShape = GetLightShape(light->m_light, eDeferredShapeQuality_High);
                    ASSERT(lightShape && "Light shape not found");
                    static_cast<LegacyVertexBuffer*>(lightShape)->resolveGeometryBinding(frame.m_currentFrame, elements, &binding);
                    detail::cmdDefaultLegacyGeomBinding(cmd, frame, binding);

                    cmdBindPipeline(cmd, options.m_invert ? m_lightStencilPipelineCW.m_handle : m_lightStencilPipelineCCW.m_handle);
                    uint32_t instance = cmdBindLightDescriptor(light); // bind light descriptor light uniforms
                    cmdBindPushConstants(cmd, m_lightPassRootSignature.m_handle, lightRootConstantIndex, &instance);
                    cmdDrawIndexed(cmd, binding.m_indexBuffer.numIndicies, 0, 0);

                    switch (light->m_light->GetLightType()) {
                    case eLightType_Point:
                        cmdBindPipeline(
                            cmd,
                            m_pointLightPipeline
                                [(options.m_invert ? LightPipelineVariants::LightPipelineVariant_CCW : LightPipelineVariants::LightPipelineVariant_CW) | LightPipelineVariants::LightPipelineVariant_StencilTest]
                                    .m_handle);
                        break;
                    case eLightType_Spot:
                        cmdBindPipeline(
                            cmd,
                            m_spotLightPipeline
                                [(options.m_invert ? LightPipelineVariants::LightPipelineVariant_CCW : LightPipelineVariants::LightPipelineVariant_CW) | LightPipelineVariants::LightPipelineVariant_StencilTest]
                                    .m_handle);
                        break;
                    case eLightType_Box:
                        cmdBindPipeline(
                            cmd,
                            m_boxLightPipeline
                                [(options.m_invert ? LightPipelineVariants::LightPipelineVariant_CCW : LightPipelineVariants::LightPipelineVariant_CW) | LightPipelineVariants::LightPipelineVariant_StencilTest]
                                    .m_handle);
                        break;
                    default:
                        ASSERT(false && "Unsupported light type");
                        break;
                    }
                    cmdBindPushConstants(cmd, m_lightPassRootSignature.m_handle, lightRootConstantIndex, &instance);
                    cmdDrawIndexed(cmd, binding.m_indexBuffer.numIndicies, 0, 0);
                    {
                        LoadActionsDesc loadActions = {};
                        loadActions.mLoadActionsColor[0] = LOAD_ACTION_LOAD;
                        loadActions.mLoadActionDepth = LOAD_ACTION_LOAD;
                        loadActions.mLoadActionStencil = LOAD_ACTION_CLEAR;
                        std::array targets = {
                            outputBuffer,
                        };
                        cmdBindRenderTargets(cmd, 0, NULL, NULL, NULL, NULL, NULL, -1, -1);
                        {
                            std::array rtBarriers = {
                                RenderTargetBarrier{
                                    depthBuffer, RESOURCE_STATE_DEPTH_WRITE, RESOURCE_STATE_PRESENT },
                            };
                            cmdResourceBarrier(cmd, 0, NULL, 0, nullptr, rtBarriers.size(), rtBarriers.data());
                        }
                        {
                            std::array rtBarriers = {
                                RenderTargetBarrier{
                                   depthBuffer, RESOURCE_STATE_PRESENT, RESOURCE_STATE_DEPTH_WRITE },
                            };
                            cmdResourceBarrier(cmd, 0, NULL, 0, nullptr, rtBarriers.size(), rtBarriers.data());
                        }

                        cmdBindRenderTargets(
                            cmd,
                            targets.size(),
                            targets.data(),
                            depthBuffer,
                            &loadActions,
                            nullptr,
                            nullptr,
                            -1,
                            -1);
                    }
                }
                cmdEndDebugMarker(cmd);

                cmdBeginDebugMarker(cmd, 0, 1, 0, "Point Light Deferred Back");
                for (auto& light : deferredLightRenderBack) {
                    switch (light->m_light->GetLightType()) {
                    case eLightType_Point:
                        cmdBindPipeline(
                            cmd,
                            m_pointLightPipeline
                                [options.m_invert ? LightPipelineVariants::LightPipelineVariant_CCW
                                                  : LightPipelineVariants::LightPipelineVariant_CW]
                                    .m_handle);
                        break;
                    case eLightType_Spot:
                        cmdBindPipeline(cmd, m_spotLightPipeline[options.m_invert ? LightPipelineVariants::LightPipelineVariant_CCW
                                                  : LightPipelineVariants::LightPipelineVariant_CW].m_handle);
                        break;
                    case eLightType_Box:
                        cmdBindPipeline(cmd, m_boxLightPipeline[options.m_invert ? LightPipelineVariants::LightPipelineVariant_CCW
                                                  : LightPipelineVariants::LightPipelineVariant_CW].m_handle);
                        break;
                    default:
                        ASSERT(false && "Unsupported light type");
                        break;
                    }

                    std::array targets = { eVertexBufferElement_Position };
                    LegacyVertexBuffer::GeometryBinding binding{};
                    auto lightShape = GetLightShape(light->m_light, eDeferredShapeQuality_High);
                    ASSERT(lightShape && "Light shape not found");
                    static_cast<LegacyVertexBuffer*>(lightShape)->resolveGeometryBinding(frame.m_currentFrame, targets, &binding);

                    uint32_t instance = cmdBindLightDescriptor(light);
                    detail::cmdDefaultLegacyGeomBinding(cmd, frame, binding);
                    cmdBindPushConstants(cmd, m_lightPassRootSignature.m_handle, lightRootConstantIndex, &instance);
                    cmdDrawIndexed(cmd, binding.m_indexBuffer.numIndicies, 0, 0);
                }
                cmdEndDebugMarker(cmd);
            }
        }
    }

    void cRendererDeferred::cmdBuildPrimaryGBuffer(
        const ForgeRenderer::Frame& frame,
        Cmd* cmd,
        uint32_t frameDescriptorIndex,
        cRenderList& renderList,
        RenderTarget* colorBuffer,
        RenderTarget* normalBuffer,
        RenderTarget* positionBuffer,
        RenderTarget* specularBuffer,
        RenderTarget* depthBuffer,
        AdditionalGbufferPassOptions options) {
        uint32_t materialObjectIndex = getDescriptorIndexFromName(m_materialRootSignature.m_handle, "materialRootConstant");
        {
            cmdBeginDebugMarker(cmd, 0, 1, 0, "Build GBuffer");
            LoadActionsDesc loadActions = {};
            loadActions.mLoadActionsColor[0] = LOAD_ACTION_CLEAR;
            loadActions.mLoadActionsColor[1] = LOAD_ACTION_CLEAR;
            loadActions.mLoadActionsColor[2] = LOAD_ACTION_CLEAR;
            loadActions.mLoadActionsColor[3] = LOAD_ACTION_CLEAR;
            loadActions.mClearColorValues[0] = { .r = 0.0f, .g = 0.0f, .b = 0.0f, .a = 0.0f };
            loadActions.mLoadActionDepth = LOAD_ACTION_LOAD;
            std::array targets = { colorBuffer, normalBuffer, positionBuffer, specularBuffer };
            cmdBindRenderTargets(cmd, targets.size(), targets.data(), depthBuffer, &loadActions, NULL, NULL, -1, -1);
            cmdSetViewport(cmd, 0.0f, 0.0f, colorBuffer->mWidth, colorBuffer->mHeight, 0.0f, 1.0f);
            cmdSetScissor(cmd, 0, 0, colorBuffer->mWidth, colorBuffer->mHeight);
            cmdBindPipeline(cmd, options.m_invert ? m_materialSolidPass.m_solidDiffuseParallaxPipelineCW.m_handle: m_materialSolidPass.m_solidDiffuseParallaxPipeline.m_handle);

            cmdBindDescriptorSet(cmd, frameDescriptorIndex, m_materialSet.m_frameSet[frame.m_frameIndex].m_handle);

            for (auto& diffuseItem : renderList.GetRenderableItems(eRenderListType_Diffuse)) {
                cMaterial* pMaterial = diffuseItem->GetMaterial();
                iVertexBuffer* vertexBuffer = diffuseItem->GetVertexBuffer();
                if (pMaterial == nullptr || vertexBuffer == nullptr) {
                    continue;
                }

                ASSERT(pMaterial->Descriptor().m_id == MaterialID::SolidDiffuse && "Invalid material type");
                MaterialRootConstant materialConst = {};
                uint32_t instance = cmdBindMaterialAndObject(cmd, frame, pMaterial, diffuseItem);
                materialConst.objectId = instance;
                std::array targets = { eVertexBufferElement_Position,
                                       eVertexBufferElement_Texture0,
                                       eVertexBufferElement_Normal,
                                       eVertexBufferElement_Texture1Tangent };
                LegacyVertexBuffer::GeometryBinding binding;
                static_cast<LegacyVertexBuffer*>(vertexBuffer)->resolveGeometryBinding(frame.m_currentFrame, targets, &binding);
                detail::cmdDefaultLegacyGeomBinding(cmd, frame, binding);
                cmdBindPushConstants(cmd, m_materialRootSignature.m_handle, materialObjectIndex, &materialConst);
                cmdDrawIndexed(cmd, binding.m_indexBuffer.numIndicies, 0, 0);
            }
            cmdEndDebugMarker(cmd);
        }

        // ------------------------------------------------------------------------------------
        //  Render Decal Pass render to color and depth
        // ------------------------------------------------------------------------------------
        {
            cmdBeginDebugMarker(cmd, 0, 1, 0, "Build Decal");
            LoadActionsDesc loadActions = {};
            loadActions.mLoadActionsColor[0] = LOAD_ACTION_LOAD;
            loadActions.mLoadActionDepth = LOAD_ACTION_LOAD;
            std::array targets = {
                colorBuffer,
            };
            cmdBindRenderTargets(cmd, targets.size(), targets.data(), depthBuffer, &loadActions, NULL, NULL, -1, -1);
            cmdSetViewport(cmd, 0.0f, 0.0f, colorBuffer->mWidth, colorBuffer->mHeight, 0.0f, 1.0f);
            cmdSetScissor(cmd, 0, 0, colorBuffer->mWidth, colorBuffer->mHeight);

            cmdBindDescriptorSet(cmd, frameDescriptorIndex, m_materialSet.m_frameSet[frame.m_frameIndex].m_handle);
            for (auto& decalItem : m_rendererList.GetRenderableItems(eRenderListType_Decal)) {
                cMaterial* pMaterial = decalItem->GetMaterial();
                iVertexBuffer* vertexBuffer = decalItem->GetVertexBuffer();
                if (pMaterial == nullptr || vertexBuffer == nullptr) {
                    continue;
                }
                ASSERT(pMaterial->GetBlendMode() < eMaterialBlendMode_LastEnum && "Invalid blend mode");
                ASSERT(pMaterial->Descriptor().m_id == MaterialID::Decal && "Invalid material type");
                cmdBindPipeline(cmd, options.m_invert ? m_decalPipelineCW[pMaterial->GetBlendMode()].m_handle : m_decalPipeline[pMaterial->GetBlendMode()].m_handle);

                std::array targets = { eVertexBufferElement_Position, eVertexBufferElement_Texture0, eVertexBufferElement_Color0 };
                MaterialRootConstant materialConst = {};
                uint32_t instance = cmdBindMaterialAndObject(cmd, frame, pMaterial, decalItem);
                materialConst.objectId = instance;
                LegacyVertexBuffer::GeometryBinding binding;
                static_cast<LegacyVertexBuffer*>(vertexBuffer)->resolveGeometryBinding(frame.m_currentFrame, targets, &binding);
                detail::cmdDefaultLegacyGeomBinding(cmd, frame, binding);
                cmdBindPushConstants(cmd, m_materialRootSignature.m_handle, materialObjectIndex, &materialConst);
                cmdDrawIndexed(cmd, binding.m_indexBuffer.numIndicies, 0, 0);
            }
            cmdEndDebugMarker(cmd);
        }
    }

    void cRendererDeferred::cmdPreAndPostZ(
        Cmd* cmd,
        cWorld* apWorld,
        std::set<iRenderable*>& prePassRenderables,
        const ForgeRenderer::Frame& frame,
        cRenderList& renderList,
        float frameTime,
        RenderTarget* depthBuffer,
        RenderTarget* hiZBuffer,
        cFrustum* apFrustum,
        uint32_t frameDescriptorIndex,
        cMatrixf viewMat,
        cMatrixf projectionMat,
        AdditionalZPassOptions options) {

        ASSERT(depthBuffer && "Depth buffer not created");
        ASSERT(hiZBuffer && "Depth buffer not created");

        FenceStatus fenceStatus;
        getFenceStatus(frame.m_renderer->Rend(), m_prePassFence.m_handle, &fenceStatus);
        if (fenceStatus == FENCE_STATUS_INCOMPLETE) {
            waitForFences(frame.m_renderer->Rend(), 1, &m_prePassFence.m_handle);
        }
        resetCmdPool(frame.m_renderer->Rend(), m_prePassPool.m_handle);

        uint32_t materialObjectIndex = getDescriptorIndexFromName(m_materialRootSignature.m_handle, "materialRootConstant");

        struct RenderableContainer {
            iRenderableContainer* m_continer;
            eWorldContainerType m_type;
        };
        std::array worldContainers = {
            RenderableContainer{ apWorld->GetRenderableContainer(eWorldContainerType_Dynamic), eWorldContainerType_Dynamic },
            RenderableContainer{ apWorld->GetRenderableContainer(eWorldContainerType_Static), eWorldContainerType_Static }
        };

        for (auto& container : worldContainers) {
            container.m_continer->UpdateBeforeRendering();
        }
        std::vector<UniformTest> uniformTest;
        uint32_t queryIndex = 0;
        std::vector<OcclusionQueryAlpha> occlusionQueryAlpha;
        // start pre-z pass
        beginCmd(m_prePassCmd.m_handle);
        {
            std::function<void(iRenderableContainerNode * childNode, eWorldContainerType staticGeometry)> walkRenderables;
            walkRenderables = [&](iRenderableContainerNode* childNode, eWorldContainerType containerType) {
                childNode->UpdateBeforeUse();
                for (auto& childNode : childNode->GetChildNodes()) {
                    childNode->UpdateBeforeUse();
                    eCollision frustumCollision = apFrustum->CollideNode(childNode);
                    if (frustumCollision == eCollision_Outside) {
                        continue;
                    }
                    if (apFrustum->CheckAABBNearPlaneIntersection(childNode->GetMin(), childNode->GetMax())) {
                        cVector3f vViewSpacePos = cMath::MatrixMul(apFrustum->GetViewMatrix(), childNode->GetCenter());
                        childNode->SetViewDistance(vViewSpacePos.z);
                        childNode->SetInsideView(true);
                    } else {
                        // Frustum origin is outside of node. Do intersection test.
                        cVector3f vIntersection;
                        cMath::CheckAABBLineIntersection(
                            childNode->GetMin(), childNode->GetMax(), apFrustum->GetOrigin(), childNode->GetCenter(), &vIntersection, NULL);
                        cVector3f vViewSpacePos = cMath::MatrixMul(apFrustum->GetViewMatrix(), vIntersection);
                        childNode->SetViewDistance(vViewSpacePos.z);
                        childNode->SetInsideView(false);
                    }
                    walkRenderables(childNode, containerType);
                }
                for (auto& pObject : childNode->GetObjects()) {
                    if (!rendering::detail::IsObjectIsVisible(pObject, options.objectVisibilityFlags, options.clipPlanes)) {
                        continue;
                    }
                    const bool visibleLastFrame = (prePassRenderables.empty() || prePassRenderables.contains(pObject));

                    auto& test = uniformTest.emplace_back();
                    test.m_preZPass = visibleLastFrame;
                    test.m_renderable = pObject;

                    cMaterial* pMaterial = pObject->GetMaterial();
                    iVertexBuffer* vertexBuffer = pObject->GetVertexBuffer();

                    if (pObject && pObject->GetRenderFrameCount() != iRenderer::GetRenderFrameCount()) {
                        pObject->SetRenderFrameCount(iRenderer::GetRenderFrameCount());
                        pObject->UpdateGraphicsForFrame(frameTime);
                    }

                    if (pMaterial && pMaterial->GetRenderFrameCount() != iRenderer::GetRenderFrameCount()) {
                        pMaterial->SetRenderFrameCount(iRenderer::GetRenderFrameCount());
                        pMaterial->UpdateBeforeRendering(frameTime);
                    }
                    ////////////////////////////////////////
                    // Update per viewport specific and set amtrix point
                    // Skip this for non-decal translucent! This is because the water rendering might mess it up otherwise!
                    if (pMaterial == NULL || cMaterial::IsTranslucent(pMaterial->Descriptor().m_id) == false || pMaterial->Descriptor().m_id == MaterialID::Decal) {
                        // skip rendering if the update return false
                        if (pObject->UpdateGraphicsForViewport(apFrustum, frameTime) == false) {
                            return;
                        }

                        pObject->SetModelMatrixPtr(pObject->GetModelMatrix(apFrustum));
                    }
                    // Only set a matrix used for sorting. Calculate the proper in the trans rendering!
                    else {
                        pObject->SetModelMatrixPtr(pObject->GetModelMatrix(NULL));
                    }

                    if (visibleLastFrame && pObject->UsesOcclusionQuery() && !options.m_disableOcclusionQueries) {
                        auto& occlusionAlpha = occlusionQueryAlpha.emplace_back();
                        occlusionAlpha.m_renderable = pObject;
                    }
                }
            };
            for (auto& it : worldContainers) {
                iRenderableContainerNode* pNode = it.m_continer->GetRoot();
                pNode->UpdateBeforeUse(); // Make sure node is updated.
                pNode->SetInsideView(true); // We never want to check root! Assume player is inside.
                walkRenderables(pNode, it.m_type);
            }

            std::sort(uniformTest.begin(), uniformTest.end(), [&](UniformTest& apObjectA, UniformTest& apObjectB) {
                cMaterial* pMatA = apObjectA.m_renderable->GetMaterial();
                cMaterial* pMatB = apObjectB.m_renderable->GetMaterial();
                if (!(pMatA && pMatB)) {
                    return false;
                }

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
                return apObjectA.m_renderable->GetViewSpaceZ() > apObjectB.m_renderable->GetViewSpaceZ();
                // return apObjectA < apObjectB;
            });

            cmdBeginDebugMarker(m_prePassCmd.m_handle, 0, 1, 0, "Pre Z");
            LoadActionsDesc loadActions = {};
            loadActions.mLoadActionsColor[0] = LOAD_ACTION_DONTCARE;
            loadActions.mLoadActionDepth = LOAD_ACTION_CLEAR;
            loadActions.mLoadActionStencil = LOAD_ACTION_DONTCARE;
            loadActions.mClearDepth = { .depth = 1.0f, .stencil = 0 };

            cmdBindRenderTargets(m_prePassCmd.m_handle, 0, NULL, depthBuffer, &loadActions, NULL, NULL, -1, -1);

            cmdSetViewport(
                m_prePassCmd.m_handle,
                0.0f,
                0.0f,
                static_cast<float>(depthBuffer->mWidth),
                static_cast<float>(depthBuffer->mHeight),
                0.0f,
                1.0f);
            cmdSetScissor(
                m_prePassCmd.m_handle,
                0,
                0,
                static_cast<float>(depthBuffer->mWidth),
                static_cast<float>(depthBuffer->mHeight));
            cmdBindPipeline(m_prePassCmd.m_handle, options.m_invert ? m_zPassPipelineCW.m_handle : m_zPassPipelineCCW.m_handle);

            cmdBindDescriptorSet(m_prePassCmd.m_handle, frameDescriptorIndex, m_materialSet.m_frameSet[frame.m_frameIndex].m_handle);

            UniformPropBlock uniformPropBlock = {};
            for (size_t i = 0; i < uniformTest.size(); i++) {
                auto& test = uniformTest[i];
                cMaterial* pMaterial = test.m_renderable->GetMaterial();
                iVertexBuffer* vertexBuffer = test.m_renderable->GetVertexBuffer();

                ASSERT(uniformTest.size() < MaxObjectTest && "Too many renderables");
                auto* pBoundingVolume = test.m_renderable->GetBoundingVolume();

                BufferUpdateDesc updateDesc = { m_hiZBoundBoxBuffer.m_handle, i * (2 * sizeof(float4)), sizeof(float4) * 2 };
                auto boundBoxMin = pBoundingVolume->GetMin();
                auto boundBoxMax = pBoundingVolume->GetMax();
                beginUpdateResource(&updateDesc);
                reinterpret_cast<float4*>(updateDesc.pMappedData)[0] = float4(boundBoxMin.x, boundBoxMin.y, boundBoxMin.z, 0.0f);
                reinterpret_cast<float4*>(updateDesc.pMappedData)[1] = float4(boundBoxMax.x, boundBoxMax.y, boundBoxMax.z, 0.0f);
                endUpdateResource(&updateDesc, nullptr);

                if (!test.m_preZPass || !vertexBuffer || !pMaterial || cMaterial::IsTranslucent(pMaterial->Descriptor().m_id)) {
                    continue;
                }

                MaterialRootConstant materialConst = {};
                uint32_t instance = cmdBindMaterialAndObject(m_prePassCmd.m_handle, frame, pMaterial, test.m_renderable, {});
                materialConst.objectId = instance;
                std::array targets = { eVertexBufferElement_Position,
                                       eVertexBufferElement_Texture0,
                                       eVertexBufferElement_Normal,
                                       eVertexBufferElement_Texture1Tangent };
                LegacyVertexBuffer::GeometryBinding binding{};
                static_cast<LegacyVertexBuffer*>(vertexBuffer)->resolveGeometryBinding(frame.m_currentFrame, targets, &binding);
                detail::cmdDefaultLegacyGeomBinding(m_prePassCmd.m_handle, frame, binding);
                cmdBindPushConstants(m_prePassCmd.m_handle, m_materialRootSignature.m_handle, materialObjectIndex, &materialConst);
                cmdDrawIndexed(m_prePassCmd.m_handle, binding.m_indexBuffer.numIndicies, 0, 0);
            }

            uniformPropBlock.maxMipLevel = hiZBuffer->mMipLevels - 1;
            uniformPropBlock.depthDim = uint2(depthBuffer->mWidth, depthBuffer->mHeight);
            uniformPropBlock.numObjects = uniformTest.size();

            BufferUpdateDesc updateDesc = { m_hiZOcclusionUniformBuffer.m_handle, 0, sizeof(UniformPropBlock) };
            beginUpdateResource(&updateDesc);
            (*reinterpret_cast<UniformPropBlock*>(updateDesc.pMappedData)) = uniformPropBlock;
            endUpdateResource(&updateDesc, nullptr);
            cmdEndDebugMarker(m_prePassCmd.m_handle);
        }
        {
            cmdBeginDebugMarker(m_prePassCmd.m_handle, 1, 1, 0, "Occlusion Query");
            LegacyVertexBuffer::GeometryBinding binding{};
            std::array targets = { eVertexBufferElement_Position };
            static_cast<LegacyVertexBuffer*>(m_box.get())->resolveGeometryBinding(frame.m_currentFrame, targets, &binding);

            uint32_t occlusionObjectIndex = getDescriptorIndexFromName(m_rootSignatureOcclusuion.m_handle, "rootConstant");
            uint32_t occlusionIndex = 0;
            cMatrixf viewProj = cMath::MatrixMul(projectionMat, viewMat);

            cmdBindDescriptorSet(m_prePassCmd.m_handle, 0, m_descriptorOcclusionConstSet.m_handle);
            for (auto& query : occlusionQueryAlpha) {
                if (TypeInfo<hpl::cBillboard>::IsType(*query.m_renderable)) {
                    cBillboard* pBillboard = static_cast<cBillboard*>(query.m_renderable);

                    auto mvp = cMath::ToForgeMat(
                        cMath::MatrixMul(
                            viewProj, cMath::MatrixMul(pBillboard->GetWorldMatrix(), cMath::MatrixScale(pBillboard->GetHaloSourceSize())))
                            .GetTranspose());

                    BufferUpdateDesc updateDesc = { m_occlusionUniformBuffer.m_handle, m_occlusionIndex * sizeof(mat4) };
                    beginUpdateResource(&updateDesc);
                    (*reinterpret_cast<mat4*>(updateDesc.pMappedData)) = mvp;
                    endUpdateResource(&updateDesc, NULL);

                    cmdBindPushConstants(
                        m_prePassCmd.m_handle, m_rootSignatureOcclusuion.m_handle, occlusionObjectIndex, &m_occlusionIndex);

                    detail::cmdDefaultLegacyGeomBinding(m_prePassCmd.m_handle, frame, binding);

                    QueryDesc queryDesc = {};
                    queryDesc.mIndex = queryIndex++;
                    cmdBindPipeline(m_prePassCmd.m_handle, m_pipelineOcclusionQuery.m_handle);
                    cmdBeginQuery(m_prePassCmd.m_handle, m_occlusionQuery.m_handle, &queryDesc);
                    cmdDrawIndexed(m_prePassCmd.m_handle, binding.m_indexBuffer.numIndicies, 0, 0);
                    cmdEndQuery(m_prePassCmd.m_handle, m_occlusionQuery.m_handle, &queryDesc);
                    query.m_queryIndex = queryDesc.mIndex;

                    queryDesc.mIndex = queryIndex++;
                    cmdBindPipeline(m_prePassCmd.m_handle, m_pipelineMaxOcclusionQuery.m_handle);
                    cmdBeginQuery(m_prePassCmd.m_handle, m_occlusionQuery.m_handle, &queryDesc);
                    cmdDrawIndexed(m_prePassCmd.m_handle, binding.m_indexBuffer.numIndicies, 0, 0);
                    cmdEndQuery(m_prePassCmd.m_handle, m_occlusionQuery.m_handle, &queryDesc);
                    query.m_maxQueryIndex = queryDesc.mIndex;

                    m_occlusionIndex = (m_occlusionIndex + 1) % MaxOcclusionDescSize;
                }
            }
            cmdEndDebugMarker(m_prePassCmd.m_handle);
        }
        // hi-z generate pass
        {
            cmdBeginDebugMarker(m_prePassCmd.m_handle, 0, 0, 0, "Generate HI-Z");
            {
                cmdBindRenderTargets(m_prePassCmd.m_handle, 0, NULL, NULL, NULL, NULL, NULL, -1, -1);
                std::array rtBarriers = {
                    RenderTargetBarrier{ depthBuffer, RESOURCE_STATE_DEPTH_WRITE, RESOURCE_STATE_SHADER_RESOURCE },
                    RenderTargetBarrier{ hiZBuffer, RESOURCE_STATE_UNORDERED_ACCESS, RESOURCE_STATE_RENDER_TARGET },
                };
                cmdResourceBarrier(m_prePassCmd.m_handle, 0, NULL, 0, NULL, rtBarriers.size(), rtBarriers.data());
            }
            {
                LoadActionsDesc loadActions = {};
                loadActions.mLoadActionsColor[0] = LOAD_ACTION_LOAD;
                loadActions.mLoadActionDepth = LOAD_ACTION_DONTCARE;
                loadActions.mClearColorValues[0] = { .r = 1.0f, .g = 0.0f, .b = 0.0f, .a = 0.0f };
                cmdBindRenderTargets(m_prePassCmd.m_handle, 1, &hiZBuffer, NULL, &loadActions, NULL, NULL, -1, -1);

                std::array<DescriptorData, 1> params = {};
                params[0].pName = "sourceInput";
                params[0].ppTextures = &depthBuffer->pTexture;
                updateDescriptorSet(frame.m_renderer->Rend(), 0, m_descriptorCopyDepth.m_handle, params.size(), params.data());

                cmdSetViewport(m_prePassCmd.m_handle, 0.0f, 0.0f, depthBuffer->mWidth, depthBuffer->mHeight, 0.0f, 1.0f);
                cmdSetScissor(m_prePassCmd.m_handle, 0, 0, depthBuffer->mWidth, depthBuffer->mHeight);
                cmdBindPipeline(m_prePassCmd.m_handle, m_pipelineCopyDepth.m_handle);

                cmdBindDescriptorSet(m_prePassCmd.m_handle, 0, m_descriptorCopyDepth.m_handle);
                cmdDraw(m_prePassCmd.m_handle, 3, 0);
            }

            {
                cmdBindRenderTargets(m_prePassCmd.m_handle, 0, NULL, NULL, NULL, NULL, NULL, -1, -1);
                std::array rtBarriers = {
                    RenderTargetBarrier{ depthBuffer, RESOURCE_STATE_SHADER_RESOURCE, RESOURCE_STATE_DEPTH_WRITE },
                    RenderTargetBarrier{ hiZBuffer, RESOURCE_STATE_RENDER_TARGET, RESOURCE_STATE_UNORDERED_ACCESS },
                };
                cmdResourceBarrier(m_prePassCmd.m_handle, 0, NULL, 0, NULL, rtBarriers.size(), rtBarriers.data());
            }
            uint32_t rootConstantIndex = getDescriptorIndexFromName(m_rootSignatureHIZOcclusion.m_handle, "uRootConstants");
            uint32_t width = static_cast<float>(depthBuffer->mWidth);
            uint32_t height = static_cast<float>(depthBuffer->mHeight);
            for (uint32_t lod = 1; lod < hiZBuffer->mMipLevels; ++lod) {
                std::array<DescriptorData, 1> params = {};
                params[0].pName = "depthInput";
                params[0].mBindMipChain = true;
                params[0].ppTextures = &hiZBuffer->pTexture;
                updateDescriptorSet(frame.m_renderer->Rend(), lod, m_descriptorSetHIZGenerate.m_handle, params.size(), params.data());

                width /= 2;
                height /= 2;
                struct {
                    uint2 screenDim;
                    uint32_t mipLevel;
                } pushConstants = {};

                pushConstants.mipLevel = lod;
                pushConstants.screenDim = uint2(depthBuffer->mWidth, depthBuffer->mHeight);

                // bind lod to push constant
                cmdBindPushConstants(m_prePassCmd.m_handle, m_rootSignatureHIZOcclusion.m_handle, rootConstantIndex, &pushConstants);
                cmdBindDescriptorSet(m_prePassCmd.m_handle, lod, m_descriptorSetHIZGenerate.m_handle);
                cmdBindPipeline(m_prePassCmd.m_handle, m_pipelineHIZGenerate.m_handle);
                cmdDispatch(m_prePassCmd.m_handle, static_cast<uint32_t>(width / 16) + 1, static_cast<uint32_t>(height / 16) + 1, 1);

                std::array rtBarriers = {
                    RenderTargetBarrier{ hiZBuffer, RESOURCE_STATE_UNORDERED_ACCESS, RESOURCE_STATE_UNORDERED_ACCESS },
                };
                cmdResourceBarrier(m_prePassCmd.m_handle, 0, NULL, 0, NULL, rtBarriers.size(), rtBarriers.data());
            }
            {
                std::array rtBarriers = { RenderTargetBarrier { hiZBuffer, RESOURCE_STATE_UNORDERED_ACCESS, RESOURCE_STATE_SHADER_RESOURCE } };
                cmdResourceBarrier(m_prePassCmd.m_handle, 0, NULL, 0, NULL, rtBarriers.size(), rtBarriers.data());
            }
            cmdEndDebugMarker(m_prePassCmd.m_handle);

            cmdBeginDebugMarker(m_prePassCmd.m_handle, 0, 1, 0, "AABB Hi-Z");
            {
                std::array<DescriptorData, 5> params = {};
                params[0].pName = "perFrameConstants";
                params[0].ppBuffers = &m_perFrameBuffer[frameDescriptorIndex].m_handle;
                params[1].pName = "objectUniformBlock";
                params[1].ppBuffers = &m_hiZOcclusionUniformBuffer.m_handle;
                params[2].pName = "occlusionTest";
                params[2].ppBuffers = &m_occlusionTestBuffer.m_handle;
                params[3].pName = "occlusionBoxBuffer";
                params[3].ppBuffers = &m_hiZBoundBoxBuffer.m_handle;
                params[4].pName = "depthTest";
                params[4].ppTextures = &hiZBuffer->pTexture;
                updateDescriptorSet(frame.m_renderer->Rend(), 0, m_descriptorAABBOcclusionTest.m_handle, params.size(), params.data());

                cmdBindDescriptorSet(m_prePassCmd.m_handle, 0, m_descriptorAABBOcclusionTest.m_handle);
                cmdBindPipeline(m_prePassCmd.m_handle, m_pipelineAABBOcclusionTest.m_handle);
                cmdDispatch(m_prePassCmd.m_handle, static_cast<uint32_t>(uniformTest.size() / 128) + 1, 1, 1);
                {
                    std::array rtBarriers = { RenderTargetBarrier { hiZBuffer, RESOURCE_STATE_SHADER_RESOURCE, RESOURCE_STATE_UNORDERED_ACCESS } };
                    cmdResourceBarrier(m_prePassCmd.m_handle, 0, NULL, 0, NULL, rtBarriers.size(), rtBarriers.data());
                }
            }
            cmdEndDebugMarker(m_prePassCmd.m_handle);
            cmdResolveQuery(m_prePassCmd.m_handle, m_occlusionQuery.m_handle, m_occlusionReadBackBuffer.m_handle, 0, queryIndex);
            endCmd(m_prePassCmd.m_handle);


            // Submit the gpu work.
            QueueSubmitDesc submitDesc = {};
            submitDesc.mCmdCount = 1;
            submitDesc.ppCmds = &m_prePassCmd.m_handle;
            submitDesc.pSignalFence = m_prePassFence.m_handle;
            submitDesc.mSubmitDone = true;
            queueSubmit(frame.m_renderer->GetGraphicsQueue(), &submitDesc);

            FenceStatus fenceStatus;
            getFenceStatus(frame.m_renderer->Rend(), m_prePassFence.m_handle, &fenceStatus);
            if (fenceStatus == FENCE_STATUS_INCOMPLETE) {
                waitForFences(frame.m_renderer->Rend(), 1, &m_prePassFence.m_handle);
            }

            uint64_t* occlusionCount = reinterpret_cast<uint64_t*>(m_occlusionReadBackBuffer.m_handle->pCpuMappedAddress);
            for (auto& query : occlusionQueryAlpha) {
                if (TypeInfo<hpl::cBillboard>::IsType(*query.m_renderable)) {
                    cBillboard* pBillboard = static_cast<cBillboard*>(query.m_renderable);
                    float maxSamples = static_cast<float>(occlusionCount[query.m_maxQueryIndex]);
                    float samples = static_cast<float>(occlusionCount[query.m_queryIndex]);
                    float billboardCoverage = pBillboard->getAreaOfScreenSpace(apFrustum);
                    if (maxSamples > 0.0f) {
                        pBillboard->SetHaloAlpha(billboardCoverage * (samples / maxSamples));
                    } else {
                        pBillboard->SetHaloAlpha(0.0f);
                    }
                }
            }
        }

        prePassRenderables.clear();

        LoadActionsDesc loadActions = {};
        loadActions.mLoadActionsColor[0] = LOAD_ACTION_DONTCARE;
        loadActions.mLoadActionDepth = LOAD_ACTION_LOAD;
        loadActions.mLoadActionStencil = LOAD_ACTION_DONTCARE;
        loadActions.mClearDepth = { .depth = 1.0f, .stencil = 0 };

        cmdBeginDebugMarker(cmd, 0, 1, 0, "Post Z");
        cmdBindRenderTargets(cmd, 0, NULL, depthBuffer, &loadActions, NULL, NULL, -1, -1);
        cmdSetViewport(cmd, 0.0f, 0.0f, depthBuffer->mWidth, depthBuffer->mHeight, 0.0f, 1.0f);
        cmdSetScissor(cmd, 0, 0, depthBuffer->mWidth, depthBuffer->mHeight);

        cmdBindPipeline(cmd, options.m_invert ? m_zPassPipelineCW.m_handle : m_zPassPipelineCCW.m_handle);
        cmdBindDescriptorSet(cmd, frameDescriptorIndex, m_materialSet.m_frameSet[frame.m_frameIndex].m_handle);

        auto* testResult = reinterpret_cast<uint32_t*>(m_occlusionTestBuffer.m_handle->pCpuMappedAddress);
        for (size_t i = 0; i < uniformTest.size(); ++i) {
            if (testResult[i] == 1) {
                renderList.AddObject(uniformTest[i].m_renderable);
                prePassRenderables.insert(uniformTest[i].m_renderable);
                if (!uniformTest[i].m_preZPass) {
                    auto* renderable = uniformTest[i].m_renderable;
                    eMaterialRenderMode renderMode =
                        renderable->GetCoverageAmount() >= 1 ? eMaterialRenderMode_Z : eMaterialRenderMode_Z_Dissolve;

                    cMaterial* pMaterial = renderable->GetMaterial();
                    iVertexBuffer* vertexBuffer = renderable->GetVertexBuffer();
                    if (!vertexBuffer || !pMaterial || cMaterial::IsTranslucent(pMaterial->Descriptor().m_id)) {
                        continue;
                    }

                    MaterialRootConstant materialConst = {};
                    uint32_t instance = cmdBindMaterialAndObject(cmd, frame, pMaterial, renderable);
                    materialConst.objectId = instance;
                    std::array targets = { eVertexBufferElement_Position,
                                           eVertexBufferElement_Texture0,
                                           eVertexBufferElement_Normal,
                                           eVertexBufferElement_Texture1Tangent };
                    LegacyVertexBuffer::GeometryBinding binding{};
                    static_cast<LegacyVertexBuffer*>(vertexBuffer)->resolveGeometryBinding(frame.m_currentFrame, targets, &binding);
                    detail::cmdDefaultLegacyGeomBinding(cmd, frame, binding);
                    cmdBindPushConstants(cmd, m_materialRootSignature.m_handle, materialObjectIndex, &materialConst);
                    cmdDrawIndexed(cmd, binding.m_indexBuffer.numIndicies, 0, 0);
                }
            }
        }

        cmdEndDebugMarker(cmd);
    }

    uint32_t cRendererDeferred::cmdBindMaterialAndObject(
        Cmd* cmd, const ForgeRenderer::Frame& frame, cMaterial* apMaterial, iRenderable* apObject, std::optional<cMatrixf> modelMatrix) {
        uint32_t id = folly::hash::fnv32_buf(
            reinterpret_cast<const char*>(apObject),
            sizeof(apObject),
            modelMatrix.has_value() ? folly::hash::fnv32_buf(modelMatrix->v, sizeof(modelMatrix->v)) : folly::hash::fnv32_hash_start);

        auto* objectDescSet = m_materialSet.m_perObjectSet[frame.m_frameIndex].m_handle;
        auto objectLookup = m_materialSet.m_objectDescriptorLookup.find(apObject);
        auto& info = m_materialSet.m_materialInfo[apMaterial->Index()];
        auto& descInfo = info.m_materialDescInfo[frame.m_frameIndex];
        auto& materialDesc = apMaterial->Descriptor();


        auto metaInfo = std::find_if(cMaterial::MaterialMetaTable.begin(), cMaterial::MaterialMetaTable.end(), [&](auto& info) {
            return info.m_id == materialDesc.m_id;
        });

        if (descInfo.m_material != apMaterial || descInfo.m_version != apMaterial->Generation()) {
            descInfo.m_version = apMaterial->Generation();
            descInfo.m_material = apMaterial;

            BufferUpdateDesc updateDesc = { m_materialSet.m_materialUniformBuffer.m_handle,
                                            apMaterial->Index() * sizeof(hpl::material::UniformMaterialBlock) };
            beginUpdateResource(&updateDesc);
            (*reinterpret_cast<material::UniformMaterialBlock*>(updateDesc.pMappedData)) = material::UniformMaterialBlock::CreateFromMaterial(*apMaterial);
            endUpdateResource(&updateDesc, NULL);

            std::array<DescriptorData, 32> params{};
            size_t paramCount = 0;

            for (auto& supportedTexture : metaInfo->m_usedTextures) {
                static constexpr const char* TextureNameLookup[] = {
                    "diffuseMap", // eMaterialTexture_Diffuse
                    "normalMap", // eMaterialTexture_NMap
                    "specularMap", // eMaterialTexture_Specular
                    "alphaMap", // eMaterialTexture_Alpha
                    "heightMap", // eMaterialTexture_Height
                    "illuminationMap", // eMaterialTexture_Illumination
                    "cubeMap", // eMaterialTexture_CubeMap
                    "dissolveAlphaMap", // eMaterialTexture_DissolveAlpha
                    "cubeMapAlpha", // eMaterialTexture_CubeMapAlpha
                };

                auto* image = apMaterial->GetImage(supportedTexture);
                if (image) {
                    params[paramCount].pName = TextureNameLookup[supportedTexture];
                    params[paramCount++].ppTextures = &image->GetTexture().m_handle;
                    descInfo.m_textureHandles[supportedTexture] = image->GetTexture();
                }
            }
            updateDescriptorSet(
                frame.m_renderer->Rend(),
                apMaterial->Index(),
                m_materialSet.m_perBatchSet[frame.m_frameIndex].m_handle,
                paramCount,
                params.data());
        }

        const bool isFound = objectLookup != m_materialSet.m_objectDescriptorLookup.end();
        uint32_t index = isFound ? objectLookup->second : m_materialSet.m_objectIndex++;
        if (!isFound) {
            cMatrixf modelMat = modelMatrix.value_or(apObject->GetModelMatrixPtr() ? *apObject->GetModelMatrixPtr() : cMatrixf::Identity);

            cRendererDeferred::UniformObject uniformObjectData = {};
            uniformObjectData.m_dissolveAmount = apObject->GetCoverageAmount();
            uniformObjectData.m_materialIndex = apMaterial->Index();
            uniformObjectData.m_modelMat = cMath::ToForgeMat4(modelMat.GetTranspose());
            uniformObjectData.m_invModelMat = cMath::ToForgeMat4(cMath::MatrixInverse(modelMat).GetTranspose());
            if (apMaterial) {
                uniformObjectData.m_uvMat = cMath::ToForgeMat4(apMaterial->GetUvMatrix());
            }

            BufferUpdateDesc updateDesc = { m_objectUniformBuffer[frame.m_frameIndex].m_handle,
                                            sizeof(cRendererDeferred::UniformObject) * index };
            beginUpdateResource(&updateDesc);
            (*reinterpret_cast<cRendererDeferred::UniformObject*>(updateDesc.pMappedData)) = uniformObjectData;
            endUpdateResource(&updateDesc, NULL);

            m_materialSet.m_objectDescriptorLookup[apObject] = index;
        }
        cmdBindDescriptorSet(
            cmd,
            detail::resolveMaterialID(apMaterial->GetTextureAntistropy(), apMaterial->GetTextureWrap(), apMaterial->GetTextureFilter()),
            m_materialSet.m_materialConstSet.m_handle);
        cmdBindDescriptorSet(cmd, apMaterial->Index(), m_materialSet.m_perBatchSet[frame.m_frameIndex].m_handle);
        return index;
    }

void cRendererDeferred::RebuildGBuffer(ForgeRenderer& renderer, GBuffer& buffer, uint32_t width, uint32_t height) {
    ClearValue optimizedColorClearBlack = { { 0.0f, 0.0f, 0.0f, 0.0f } };
    auto deferredRenderTargetDesc = [&]() {
        RenderTargetDesc renderTarget = {};
        renderTarget.mArraySize = 1;
        renderTarget.mClearValue = optimizedColorClearBlack;
        renderTarget.mDepth = 1;
        renderTarget.mDescriptors = DESCRIPTOR_TYPE_TEXTURE;
        renderTarget.mWidth = width;
        renderTarget.mHeight = height;
        renderTarget.mSampleCount = SAMPLE_COUNT_1;
        renderTarget.mSampleQuality = 0;
        renderTarget.mStartState = RESOURCE_STATE_SHADER_RESOURCE;
        return renderTarget;
    };
    buffer.m_hizDepthBuffer.Load(renderer.Rend(), [&](RenderTarget** handle) {
        RenderTargetDesc renderTargetDesc = {};
        renderTargetDesc.mArraySize = 1;
        renderTargetDesc.mDepth = 1;
        renderTargetDesc.mMipLevels = std::min<uint8_t>(std::floor(std::log2(std::max(width, height))), MaxHiZMipLevels);
        renderTargetDesc.mFormat = TinyImageFormat_R32_SFLOAT;
        renderTargetDesc.mDescriptors = DESCRIPTOR_TYPE_TEXTURE | DESCRIPTOR_TYPE_RW_TEXTURE;
        renderTargetDesc.mWidth = width;
        renderTargetDesc.mHeight = height;
        renderTargetDesc.mSampleCount = SAMPLE_COUNT_1;
        renderTargetDesc.mSampleQuality = 0;
        renderTargetDesc.mStartState = RESOURCE_STATE_UNORDERED_ACCESS;
        renderTargetDesc.pName = "hi-z depth buffer";
        addRenderTarget(renderer.Rend(), &renderTargetDesc, handle);
        return true;
    });

    buffer.m_depthBuffer.Load(renderer.Rend(), [&](RenderTarget** handle) {
        auto targetDesc = deferredRenderTargetDesc();
        targetDesc.mFormat = DepthBufferFormat;
        targetDesc.mStartState = RESOURCE_STATE_DEPTH_WRITE;
        targetDesc.pName = "Depth RT";
        addRenderTarget(renderer.Rend(), &targetDesc, handle);
        return true;
    });
    buffer.m_normalBuffer.Load(renderer.Rend(), [&](RenderTarget** handle) {
        auto targetDesc = deferredRenderTargetDesc();
        targetDesc.mFormat = NormalBufferFormat;
        targetDesc.pName = "Normal RT";
        addRenderTarget(renderer.Rend(), &targetDesc, handle);
        return true;
    });
    buffer.m_positionBuffer.Load(renderer.Rend(), [&](RenderTarget** handle) {
        auto targetDesc = deferredRenderTargetDesc();
        targetDesc.mFormat = PositionBufferFormat;
        targetDesc.pName = "Position RT";
        addRenderTarget(renderer.Rend(), &targetDesc, handle);
        return true;
    });
    buffer.m_specularBuffer.Load(renderer.Rend(), [&](RenderTarget** handle) {
        auto targetDesc = deferredRenderTargetDesc();
        targetDesc.mFormat = SpecularBufferFormat;
        targetDesc.pName = "Specular RT";
        addRenderTarget(renderer.Rend(), &targetDesc, handle);
        return true;
    });
    buffer.m_colorBuffer.Load(renderer.Rend(), [&](RenderTarget** handle) {
        auto targetDesc = deferredRenderTargetDesc();
        targetDesc.mFormat = ColorBufferFormat;
        targetDesc.mDescriptors = DESCRIPTOR_TYPE_TEXTURE | DESCRIPTOR_TYPE_RW_TEXTURE;
        targetDesc.pName = "Color RT";
        addRenderTarget(renderer.Rend(), &targetDesc, handle);
        return true;
    });
    buffer.m_refractionImage.Load([&](Texture** texture) {
        TextureLoadDesc loadDesc = {};
        loadDesc.ppTexture = texture;
        TextureDesc refractionImageDesc = {};
        refractionImageDesc.mArraySize = 1;
        refractionImageDesc.mDepth = 1;
        refractionImageDesc.mMipLevels = 1;
        refractionImageDesc.mFormat = TinyImageFormat_R8G8B8A8_UNORM;
        refractionImageDesc.mDescriptors = DESCRIPTOR_TYPE_TEXTURE | DESCRIPTOR_TYPE_RW_TEXTURE;
        refractionImageDesc.mWidth = width;
        refractionImageDesc.mHeight = height;
        refractionImageDesc.mSampleCount = SAMPLE_COUNT_1;
        refractionImageDesc.mSampleQuality = 0;
        refractionImageDesc.mStartState = RESOURCE_STATE_SHADER_RESOURCE;
        refractionImageDesc.pName = "Refraction Image";
        loadDesc.pDesc = &refractionImageDesc;
        addResource(&loadDesc, nullptr);
        return true;
    });
    buffer.m_outputBuffer.Load(renderer.Rend(), [&](RenderTarget** handle) {
        auto targetDesc = deferredRenderTargetDesc();
        targetDesc.mFormat = ColorBufferFormat;
        targetDesc.mDescriptors = DESCRIPTOR_TYPE_RW_TEXTURE | DESCRIPTOR_TYPE_TEXTURE;
        addRenderTarget(renderer.Rend(), &targetDesc, handle);
        return true;
    });
}

void cRendererDeferred::Draw(
    const ForgeRenderer::Frame& frame,
    cViewport& viewport,
    float afFrameTime,
    cFrustum* apFrustum,
    cWorld* apWorld,
    cRenderSettings* apSettings,
    bool abSendFrameBufferToPostEffects) {
    iRenderer::Draw(frame, viewport, afFrameTime, apFrustum, apWorld, apSettings, abSendFrameBufferToPostEffects);
    // keep around for the moment ...
    BeginRendering(afFrameTime, apFrustum, apWorld, apSettings, abSendFrameBufferToPostEffects);

    if (frame.m_currentFrame != m_activeFrame) {
        m_materialSet.m_objectIndex = 0;
        m_materialSet.m_objectDescriptorLookup.clear();
        m_lightIndex = 0;
        m_lightDescriptorLookup.clear();
        m_activeFrame = frame.m_currentFrame;
    }

    const cMatrixf mainFrustumViewInv = cMath::MatrixInverse(apFrustum->GetViewMatrix());
    const cMatrixf mainFrustumView = apFrustum->GetViewMatrix();
    const cMatrixf mainFrustumProj = apFrustum->GetProjectionMatrix();

    const Matrix4 mainFrustumViewInvMat = inverse(apFrustum->GetViewMat());
    const Matrix4 mainFrustumViewMat = apFrustum->GetViewMat();
    const Matrix4 mainFrustumProjMat = apFrustum->GetProjectionMat();

    // auto& swapChainImage = frame.m_swapChain->ppRenderTargets[frame.m_swapChainIndex];
    auto common = m_boundViewportData.resolve(viewport);
    if (!common || common->m_size != viewport.GetSize()) {
        auto* forgeRenderer = Interface<ForgeRenderer>::Get();
        auto viewportData = std::make_unique<ViewportData>();
        viewportData->m_size = viewport.GetSize();
        for (auto& b : viewportData->m_gBuffer) {
            RebuildGBuffer(*forgeRenderer, b, viewportData->m_size.x, viewportData->m_size.y);
        }
        if(common) { // reusing state from gbuffer
            viewportData->m_reflectionBuffer = std::move(common->m_reflectionBuffer);
        }
        for(auto& reflection: viewportData->m_reflectionBuffer) {
            RebuildGBuffer(*forgeRenderer, reflection.m_buffer, viewportData->m_size.x / 2, viewportData->m_size.y / 2);
            reflection.m_target = nullptr;// we are clearing targets they are irrelevant when the gbuffer is discarded
            if(!reflection.m_pool.IsValid() ||
                !reflection.m_cmd.IsValid() ||
                !reflection.m_fence.IsValid()) {
                reflection.m_pool.Load(forgeRenderer->Rend(), [&](CmdPool** pool) {
                    CmdPoolDesc cmdPoolDesc = {};
                    cmdPoolDesc.pQueue = forgeRenderer->GetGraphicsQueue();
                    addCmdPool(forgeRenderer->Rend(), &cmdPoolDesc, pool);
                    return true;
                });
                reflection.m_cmd.Load(forgeRenderer->Rend(),  [&](Cmd** cmd) {
                    CmdDesc cmdDesc = {};
                    cmdDesc.pPool = reflection.m_pool.m_handle;
                    addCmd(forgeRenderer->Rend(), &cmdDesc, cmd);
                    return true;
                });
                reflection.m_fence.Load(forgeRenderer->Rend(), [&](Fence**  fence) {
                    addFence(forgeRenderer->Rend(), fence);
                    return true;
                });
            }
        }

        common = m_boundViewportData.update(viewport, std::move(viewportData));
    }

    auto& currentGBuffer = common->m_gBuffer[frame.m_frameIndex];
    uint32_t materialObjectIndex = getDescriptorIndexFromName(m_materialRootSignature.m_handle, "materialRootConstant");

    const uint32_t mainFrameIndex = updateFrameDescriptor(
        frame,
        frame.m_cmd,
        apWorld,
        { .m_size = float2(common->m_size.x, common->m_size.y), .m_viewMat = mainFrustumView, .m_projectionMat = mainFrustumProj });

    {

        std::array<Texture*, MaxReflectionBuffers> reflectionBuffers;
        for(size_t i = 0; i < common->m_reflectionBuffer.size(); i++) {
            reflectionBuffers[i] = common->m_reflectionBuffer[i].m_buffer.m_outputBuffer.m_handle->pTexture;
        }
        std::array<DescriptorData, 2> params = {};
        params[0].pName = "refractionMap";
        params[0].ppTextures = &currentGBuffer.m_refractionImage.m_handle;
        params[1].pName = "reflectionMap";
        params[1].ppTextures = reflectionBuffers.data();
        params[1].mCount = reflectionBuffers.size();
        updateDescriptorSet(
            frame.m_renderer->Rend(), mainFrameIndex, m_materialSet.m_frameSet[frame.m_frameIndex].m_handle, params.size(), params.data());
    }

    frame.m_resourcePool->Push(currentGBuffer.m_colorBuffer);
    frame.m_resourcePool->Push(currentGBuffer.m_normalBuffer);
    frame.m_resourcePool->Push(currentGBuffer.m_positionBuffer);
    frame.m_resourcePool->Push(currentGBuffer.m_specularBuffer);
    frame.m_resourcePool->Push(currentGBuffer.m_depthBuffer);
    frame.m_resourcePool->Push(currentGBuffer.m_outputBuffer);
    frame.m_resourcePool->Push(currentGBuffer.m_refractionImage);
    // Setup far plane coordinates

    ///////////////////////////
    // Occlusion testing
    m_rendererList.BeginAndReset(afFrameTime, apFrustum);
    cmdPreAndPostZ(
        frame.m_cmd,
        apWorld,
        currentGBuffer.m_preZPassRenderables,
        frame,
        m_rendererList,
        afFrameTime,
        currentGBuffer.m_depthBuffer.m_handle,
        currentGBuffer.m_hizDepthBuffer.m_handle,
        apFrustum,
        mainFrameIndex,
        mainFrustumView,
        mainFrustumProj, {});
    m_rendererList.End(
        eRenderListCompileFlag_Diffuse | eRenderListCompileFlag_Translucent | eRenderListCompileFlag_Decal |
        eRenderListCompileFlag_Illumination | eRenderListCompileFlag_FogArea);

    {
        cmdBindRenderTargets(frame.m_cmd, 0, NULL, NULL, NULL, NULL, NULL, -1, -1);
        std::array rtBarriers = {
            RenderTargetBarrier{ currentGBuffer.m_colorBuffer.m_handle, RESOURCE_STATE_SHADER_RESOURCE, RESOURCE_STATE_RENDER_TARGET },
            RenderTargetBarrier{ currentGBuffer.m_normalBuffer.m_handle, RESOURCE_STATE_SHADER_RESOURCE, RESOURCE_STATE_RENDER_TARGET },
            RenderTargetBarrier{ currentGBuffer.m_positionBuffer.m_handle, RESOURCE_STATE_SHADER_RESOURCE, RESOURCE_STATE_RENDER_TARGET },
            RenderTargetBarrier{ currentGBuffer.m_specularBuffer.m_handle, RESOURCE_STATE_SHADER_RESOURCE, RESOURCE_STATE_RENDER_TARGET },
        };
        cmdResourceBarrier(frame.m_cmd, 0, NULL, 0, NULL, rtBarriers.size(), rtBarriers.data());
    }
    cmdBuildPrimaryGBuffer(frame, frame.m_cmd, mainFrameIndex,
        m_rendererList,
        currentGBuffer.m_colorBuffer.m_handle,
        currentGBuffer.m_normalBuffer.m_handle,
        currentGBuffer.m_positionBuffer.m_handle,
        currentGBuffer.m_specularBuffer.m_handle,
        currentGBuffer.m_depthBuffer.m_handle, {
            .m_invert = false
        });

    {
        cmdBindRenderTargets(frame.m_cmd, 0, NULL, NULL, NULL, NULL, NULL, -1, -1);
        std::array rtBarriers = {
            RenderTargetBarrier{ currentGBuffer.m_colorBuffer.m_handle, RESOURCE_STATE_RENDER_TARGET, RESOURCE_STATE_SHADER_RESOURCE },
            RenderTargetBarrier{ currentGBuffer.m_normalBuffer.m_handle, RESOURCE_STATE_RENDER_TARGET, RESOURCE_STATE_SHADER_RESOURCE },
            RenderTargetBarrier{ currentGBuffer.m_positionBuffer.m_handle, RESOURCE_STATE_RENDER_TARGET, RESOURCE_STATE_SHADER_RESOURCE },
            RenderTargetBarrier{ currentGBuffer.m_specularBuffer.m_handle, RESOURCE_STATE_RENDER_TARGET, RESOURCE_STATE_SHADER_RESOURCE },
            RenderTargetBarrier{ currentGBuffer.m_outputBuffer.m_handle, RESOURCE_STATE_SHADER_RESOURCE, RESOURCE_STATE_RENDER_TARGET },

        };
        cmdResourceBarrier(frame.m_cmd, 0, NULL, 0, NULL, rtBarriers.size(), rtBarriers.data());
    }

    if (mpCurrentSettings->mbSSAOActive) {
        ASSERT(m_hbaoPlusPipeline && "Invalid pipeline");
        ASSERT(currentGBuffer.m_depthBuffer.IsValid() && "Invalid depth buffer");
        ASSERT(currentGBuffer.m_normalBuffer.IsValid() && "Invalid depth buffer");
        ASSERT(currentGBuffer.m_colorBuffer.IsValid() && "Invalid depth buffer");
        {
            std::array rtBarriers = {
                RenderTargetBarrier{ currentGBuffer.m_depthBuffer.m_handle, RESOURCE_STATE_DEPTH_WRITE, RESOURCE_STATE_SHADER_RESOURCE },
                RenderTargetBarrier{ currentGBuffer.m_colorBuffer.m_handle, RESOURCE_STATE_SHADER_RESOURCE, RESOURCE_STATE_UNORDERED_ACCESS },
            };
            cmdResourceBarrier(frame.m_cmd, 0, NULL, 0, NULL, rtBarriers.size(), rtBarriers.data());
        }
        m_hbaoPlusPipeline->cmdDraw(
            frame,
            apFrustum,
            &viewport,
            currentGBuffer.m_depthBuffer.m_handle->pTexture,
            currentGBuffer.m_normalBuffer.m_handle->pTexture,
            currentGBuffer.m_colorBuffer.m_handle->pTexture);
        {
            std::array rtBarriers = {
                RenderTargetBarrier{ currentGBuffer.m_depthBuffer.m_handle, RESOURCE_STATE_SHADER_RESOURCE, RESOURCE_STATE_DEPTH_WRITE },
                RenderTargetBarrier{ currentGBuffer.m_colorBuffer.m_handle, RESOURCE_STATE_UNORDERED_ACCESS, RESOURCE_STATE_SHADER_RESOURCE },
            };
            cmdResourceBarrier(frame.m_cmd, 0, NULL, 0, NULL, rtBarriers.size(), rtBarriers.data());
        }
    }

    cmdLightPass(
        frame.m_cmd,
        frame,
        apWorld,
        apFrustum,
        m_rendererList,
        mainFrameIndex,
        currentGBuffer.m_colorBuffer.m_handle,
        currentGBuffer.m_normalBuffer.m_handle,
        currentGBuffer.m_positionBuffer.m_handle,
        currentGBuffer.m_specularBuffer.m_handle,
        currentGBuffer.m_depthBuffer.m_handle,
        currentGBuffer.m_outputBuffer.m_handle,
        mainFrustumView,
        mainFrustumViewInv,
        mainFrustumProj, {});

    cmdIlluminationPass(
        frame.m_cmd,
        frame,
        m_rendererList,
        mainFrameIndex,
        currentGBuffer.m_depthBuffer.m_handle,
        currentGBuffer.m_outputBuffer.m_handle,
        {});
    // ------------------------------------------------------------------------
    // Render Illumination Pass --> renders to output target
    // ------------------------------------------------------------------------
    {
        LoadActionsDesc loadActions = {};
        loadActions.mLoadActionsColor[0] = LOAD_ACTION_LOAD;
        loadActions.mLoadActionDepth = LOAD_ACTION_LOAD;
        std::array targets = {
            currentGBuffer.m_outputBuffer.m_handle,
        };
        cmdBindRenderTargets(
            frame.m_cmd, targets.size(), targets.data(), currentGBuffer.m_depthBuffer.m_handle, &loadActions, nullptr, nullptr, -1, -1);
        cmdSetViewport(frame.m_cmd, 0.0f, 0.0f, (float)common->m_size.x, (float)common->m_size.y, 0.0f, 1.0f);
        cmdSetScissor(frame.m_cmd, 0, 0, common->m_size.x, common->m_size.y);
        cmdBindPipeline(frame.m_cmd, m_solidIlluminationPipelineCCW.m_handle);

        cmdBindDescriptorSet(frame.m_cmd, mainFrameIndex, m_materialSet.m_frameSet[frame.m_frameIndex].m_handle);

        for (auto& illuminationItem : m_rendererList.GetRenderableItems(eRenderListType_Illumination)) {
            cMaterial* pMaterial = illuminationItem->GetMaterial();
            iVertexBuffer* vertexBuffer = illuminationItem->GetVertexBuffer();
            if (pMaterial == nullptr || vertexBuffer == nullptr) {
                continue;
            }
            ASSERT(pMaterial->Descriptor().m_id == MaterialID::SolidDiffuse && "Invalid material type");
            MaterialRootConstant materialConst = {};
            uint32_t instance = cmdBindMaterialAndObject(frame.m_cmd, frame, pMaterial, illuminationItem);
            materialConst.objectId = instance;
            std::array targets = {
                eVertexBufferElement_Position,
                eVertexBufferElement_Texture0,
            };
            LegacyVertexBuffer::GeometryBinding binding;
            static_cast<LegacyVertexBuffer*>(vertexBuffer)->resolveGeometryBinding(frame.m_currentFrame, targets, &binding);
            detail::cmdDefaultLegacyGeomBinding(frame.m_cmd, frame, binding);
            cmdBindPushConstants(frame.m_cmd, m_materialRootSignature.m_handle, materialObjectIndex, &materialConst);
            cmdDrawIndexed(frame.m_cmd, binding.m_indexBuffer.numIndicies, 0, 0);
        }
    }

    // ------------------------------------------------------------------------
    // Render Fog Pass --> output target
    // ------------------------------------------------------------------------
    auto fogRenderData = detail::createFogRenderData(m_rendererList.GetFogAreas(), apFrustum);
    {
        cmdBeginDebugMarker(frame.m_cmd, 0, 1, 0, "Fog Box Pass ");

        auto createUniformFog = [&](bool isInsideNearPlane, cFogArea* fogArea) {
            UniformFogData fogUniformData = {};
            if (isInsideNearPlane) {
                fogUniformData.m_flags |= (fogArea->GetShowBacksideWhenInside() ? Fog::PipelineUseBackSide : Fog::PipelineVariantEmpty);
            } else {
                cMatrixf mtxInvModelView =
                    cMath::MatrixInverse(cMath::MatrixMul(apFrustum->GetViewMatrix(), *fogArea->GetModelMatrixPtr()));
                cVector3f vRayCastStart = cMath::MatrixMul(mtxInvModelView, cVector3f(0));

                cVector3f vNegPlaneDistNeg(
                    cMath::PlaneToPointDist(cPlanef(-1, 0, 0, 0.5f), vRayCastStart),
                    cMath::PlaneToPointDist(cPlanef(0, -1, 0, 0.5f), vRayCastStart),
                    cMath::PlaneToPointDist(cPlanef(0, 0, -1, 0.5f), vRayCastStart));
                cVector3f vNegPlaneDistPos(
                    cMath::PlaneToPointDist(cPlanef(1, 0, 0, 0.5f), vRayCastStart),
                    cMath::PlaneToPointDist(cPlanef(0, 1, 0, 0.5f), vRayCastStart),
                    cMath::PlaneToPointDist(cPlanef(0, 0, 1, 0.5f), vRayCastStart));
                fogUniformData.m_invModelRotation = cMath::ToForgeMat4(mtxInvModelView.GetRotation().GetTranspose());
                fogUniformData.m_rayCastStart = float4(vRayCastStart.x, vRayCastStart.y, vRayCastStart.z, 0.0f);
                fogUniformData.m_fogNegPlaneDistNeg =
                    float4(vNegPlaneDistNeg.x * -1.0f, vNegPlaneDistNeg.y * -1.0f, vNegPlaneDistNeg.z * -1.0f, 0.0f);
                fogUniformData.m_fogNegPlaneDistPos =
                    float4(vNegPlaneDistPos.x * -1.0f, vNegPlaneDistPos.y * -1.0f, vNegPlaneDistPos.z * -1.0f, 0.0f);
                fogUniformData.m_flags |= Fog::PipelineUseOutsideBox;
                fogUniformData.m_flags |= fogArea->GetShowBacksideWhenOutside() ? Fog::PipelineUseBackSide : Fog::PipelineVariantEmpty;
            }
            const auto fogColor = fogArea->GetColor();
            fogUniformData.m_color = float4(fogColor.r, fogColor.g, fogColor.b, fogColor.a);
            fogUniformData.m_start = fogArea->GetStart();
            fogUniformData.m_length = fogArea->GetEnd() - fogArea->GetStart();
            fogUniformData.m_falloffExp = fogArea->GetFalloffExp();

            const cMatrixf modelMat = fogArea->GetModelMatrixPtr() ? *fogArea->GetModelMatrixPtr() : cMatrixf::Identity;
            fogUniformData.m_mv = cMath::ToForgeMat4(cMath::MatrixMul(mainFrustumView, modelMat).GetTranspose());
            fogUniformData.m_mvp =
                cMath::ToForgeMat4(cMath::MatrixMul(cMath::MatrixMul(mainFrustumProj, mainFrustumView), modelMat).GetTranspose());
            return fogUniformData;
        };
        std::vector<cFogArea*> nearPlanFog;
        size_t fogIndex = 0;
        for (const auto& fogArea : fogRenderData) {
            cMatrixf mtxInvBoxSpace = cMath::MatrixInverse(*fogArea.m_fogArea->GetModelMatrixPtr());
            cVector3f boxSpaceFrustumOrigin = cMath::MatrixMul(mtxInvBoxSpace, apFrustum->GetOrigin());
            // test if inside near plane
            if (fogArea.m_insideNearFrustum) {
                nearPlanFog.emplace_back(fogArea.m_fogArea);
            } else {
                UniformFogData uniformData = createUniformFog(false, fogArea.m_fogArea);
                BufferUpdateDesc updateDesc = { m_fogPass.m_fogUniformBuffer[frame.m_frameIndex].m_handle,
                                                fogIndex * sizeof(UniformFogData) };
                beginUpdateResource(&updateDesc);
                (*reinterpret_cast<UniformFogData*>(updateDesc.pMappedData)) = uniformData;
                endUpdateResource(&updateDesc, NULL);
                fogIndex++;
            }
        }
        std::array targets = {
            currentGBuffer.m_outputBuffer.m_handle,
        };
        LoadActionsDesc loadActions = {};
        loadActions.mLoadActionsColor[0] = LOAD_ACTION_LOAD;
        loadActions.mLoadActionDepth = LOAD_ACTION_LOAD;
        cmdBindRenderTargets(
            frame.m_cmd, targets.size(), targets.data(), currentGBuffer.m_depthBuffer.m_handle, &loadActions, nullptr, nullptr, -1, -1);
        cmdSetViewport(frame.m_cmd, 0.0f, 0.0f, (float)common->m_size.x, (float)common->m_size.y, 0.0f, 1.0f);
        cmdSetScissor(frame.m_cmd, 0, 0, common->m_size.x, common->m_size.y);

        cmdBindPipeline(frame.m_cmd, m_fogPass.m_pipeline.m_handle);

        LegacyVertexBuffer::GeometryBinding binding{};
        std::array geometryStream = { eVertexBufferElement_Position };
        static_cast<LegacyVertexBuffer*>(m_box.get())->resolveGeometryBinding(frame.m_currentFrame, geometryStream, &binding);
        detail::cmdDefaultLegacyGeomBinding(frame.m_cmd, frame, binding);

        {
            std::array<DescriptorData, 1> params = {};
            params[0].pName = "positionMap";
            params[0].ppTextures = &currentGBuffer.m_positionBuffer.m_handle->pTexture;
            updateDescriptorSet(frame.m_renderer->Rend(), 0, m_fogPass.m_perFrameSet[frame.m_frameIndex], params.size(), params.data());
        }
        uint32_t rootConstantIndex = getDescriptorIndexFromName(m_fogPass.m_fogRootSignature.m_handle, "rootConstant");
        struct {
            float2 viewTexel;
            uint32_t m_instanceIndex;
            uint32_t pad0;
        } fogConstant = { float2(1.0f / static_cast<float>(common->m_size.x), 1.0f / static_cast<float>(common->m_size.y)) };

        cmdBindDescriptorSet(frame.m_cmd, 0, m_fogPass.m_perFrameSet[frame.m_frameIndex]);
        cmdBindPushConstants(frame.m_cmd, m_fogPass.m_fogRootSignature.m_handle, rootConstantIndex, &fogConstant);
        cmdDrawIndexedInstanced(frame.m_cmd, binding.m_indexBuffer.numIndicies, 0, fogIndex, 0, 0);

        size_t offsetIndex = fogIndex;
        for (auto& fogArea : nearPlanFog) {
            UniformFogData uniformData = createUniformFog(true, fogArea);
            BufferUpdateDesc updateDesc = { m_fogPass.m_fogUniformBuffer[frame.m_frameIndex].m_handle, fogIndex * sizeof(UniformFogData) };
            beginUpdateResource(&updateDesc);
            (*reinterpret_cast<UniformFogData*>(updateDesc.pMappedData)) = uniformData;
            endUpdateResource(&updateDesc, NULL);
            fogIndex++;
        }

        cmdBindPipeline(frame.m_cmd, m_fogPass.m_pipelineInsideNearFrustum.m_handle);
        fogConstant.m_instanceIndex = offsetIndex;
        cmdBindPushConstants(frame.m_cmd, m_fogPass.m_fogRootSignature.m_handle, rootConstantIndex, &fogConstant);
        cmdDrawIndexedInstanced(frame.m_cmd, binding.m_indexBuffer.numIndicies, 0, fogIndex - offsetIndex, 0, 0);

        cmdEndDebugMarker(frame.m_cmd);
        if (apWorld->GetFogActive()) {
            BufferUpdateDesc updateDesc = { m_fogPass.m_fogFullscreenUniformBuffer[frame.m_frameIndex].m_handle };
            beginUpdateResource(&updateDesc);
            auto* fogData = reinterpret_cast<UniformFullscreenFogData*>(updateDesc.pMappedData);
            auto fogColor = apWorld->GetFogColor();
            fogData->m_color = float4(fogColor.r, fogColor.g, fogColor.b, fogColor.a);
            fogData->m_fogStart = apWorld->GetFogStart();
            fogData->m_fogLength = apWorld->GetFogEnd() - apWorld->GetFogStart();
            fogData->m_fogFalloffExp = apWorld->GetFogFalloffExp();
            endUpdateResource(&updateDesc, NULL);

            cmdBindDescriptorSet(frame.m_cmd, 0, m_fogPass.m_perFrameSet[frame.m_frameIndex]);
            cmdBindPipeline(frame.m_cmd, m_fogPass.m_fullScreenPipeline.m_handle);
            cmdDraw(frame.m_cmd, 3, 0);
        }
    }
    cmdEndDebugMarker(frame.m_cmd);

    {
        cmdBindRenderTargets(frame.m_cmd, 0, NULL, NULL, NULL, NULL, NULL, -1, -1);

        {
            std::array textureBarriers = {
                TextureBarrier{
                    currentGBuffer.m_refractionImage.m_handle, RESOURCE_STATE_SHADER_RESOURCE, RESOURCE_STATE_UNORDERED_ACCESS },
            };
            std::array rtBarriers = {
                RenderTargetBarrier{ currentGBuffer.m_outputBuffer.m_handle, RESOURCE_STATE_RENDER_TARGET, RESOURCE_STATE_SHADER_RESOURCE },
            };
            cmdResourceBarrier(frame.m_cmd, 0, NULL, textureBarriers.size(), textureBarriers.data(), rtBarriers.size(), rtBarriers.data());
        }

        cmdBindPipeline(frame.m_cmd, m_materialTranslucencyPass.m_refractionCopyPipeline.m_handle);
        DescriptorData params[2] = {};
        params[0].pName = "sourceInput";
        params[0].ppTextures = &currentGBuffer.m_outputBuffer.m_handle->pTexture;
        params[1].pName = "destOutput";
        params[1].ppTextures = &currentGBuffer.m_refractionImage.m_handle;
        updateDescriptorSet(
            frame.m_renderer->Rend(), 0, m_materialTranslucencyPass.m_refractionPerFrameSet[frame.m_frameIndex].m_handle, 2, params);
        cmdBindDescriptorSet(frame.m_cmd, 0, m_materialTranslucencyPass.m_refractionPerFrameSet[frame.m_frameIndex].m_handle);
        cmdDispatch(
            frame.m_cmd,
            static_cast<uint32_t>(static_cast<float>(common->m_size.x) / 16.0f) + 1,
            static_cast<uint32_t>(static_cast<float>(common->m_size.y) / 16.0f) + 1,
            1);
        {
            std::array textureBarriers = {
                TextureBarrier{
                    currentGBuffer.m_refractionImage.m_handle, RESOURCE_STATE_UNORDERED_ACCESS, RESOURCE_STATE_SHADER_RESOURCE },
            };

            std::array rtBarriers = {
                RenderTargetBarrier{ currentGBuffer.m_outputBuffer.m_handle, RESOURCE_STATE_SHADER_RESOURCE, RESOURCE_STATE_RENDER_TARGET },
            };
            cmdResourceBarrier(frame.m_cmd, 0, NULL, textureBarriers.size(), textureBarriers.data(), rtBarriers.size(), rtBarriers.data());
        }
    }

    // notify post draw listeners
    // ImmediateDrawBatch postSolidBatch(context, sharedData->m_gBuffer.m_outputTarget, mainFrustumView, mainFrustumProj);
    cViewport::PostSolidDrawPacket postSolidEvent = cViewport::PostSolidDrawPacket();
    postSolidEvent.m_frustum = apFrustum;
    postSolidEvent.m_frame = &frame;
    postSolidEvent.m_outputTarget = &currentGBuffer.m_outputBuffer;
    postSolidEvent.m_viewport = &viewport;
    postSolidEvent.m_renderSettings = mpCurrentSettings;
    viewport.SignalDraw(postSolidEvent);

    // ------------------------------------------------------------------------
    // Translucency Pass --> output target
    // ------------------------------------------------------------------------
    {
        LoadActionsDesc loadActions = {};
        loadActions.mLoadActionsColor[0] = LOAD_ACTION_LOAD;
        loadActions.mLoadActionDepth = LOAD_ACTION_LOAD;
        std::array targets = {
            currentGBuffer.m_outputBuffer.m_handle,
        };
        cmdBindRenderTargets(
            frame.m_cmd, targets.size(), targets.data(), currentGBuffer.m_depthBuffer.m_handle, &loadActions, nullptr, nullptr, -1, -1);
        cmdSetViewport(frame.m_cmd, 0.0f, 0.0f, static_cast<float>(common->m_size.x), static_cast<float>(common->m_size.y), 0.0f, 1.0f);
        cmdSetScissor(frame.m_cmd, 0, 0, common->m_size.x, common->m_size.y);


        std::array<TranslucencyPipeline::TranslucencyBlend, eMaterialBlendMode_LastEnum> translucencyBlendTable;
        translucencyBlendTable[eMaterialBlendMode_None] = TranslucencyPipeline::TranslucencyBlend::BlendAdd;
        translucencyBlendTable[eMaterialBlendMode_Add] = TranslucencyPipeline::TranslucencyBlend::BlendAdd;
        translucencyBlendTable[eMaterialBlendMode_Mul] = TranslucencyPipeline::TranslucencyBlend::BlendMul;
        translucencyBlendTable[eMaterialBlendMode_MulX2] = TranslucencyPipeline::TranslucencyBlend::BlendMulX2;
        translucencyBlendTable[eMaterialBlendMode_Alpha] = TranslucencyPipeline::TranslucencyBlend::BlendAlpha;
        translucencyBlendTable[eMaterialBlendMode_PremulAlpha] = TranslucencyPipeline::TranslucencyBlend::BlendPremulAlpha;

        cmdBeginDebugMarker(frame.m_cmd, 0, 1, 0, "Translucency Pass");
        cmdBindDescriptorSet(frame.m_cmd, mainFrameIndex, m_materialSet.m_frameSet[frame.m_frameIndex].m_handle);
        for (auto& translucencyItem : m_rendererList.GetRenderableItems(eRenderListType_Translucent)) {
            cMaterial* pMaterial = translucencyItem->GetMaterial();
            iVertexBuffer* vertexBuffer = translucencyItem->GetVertexBuffer();
            if (pMaterial == nullptr || vertexBuffer == nullptr) {
                continue;
            }

            const bool isRefraction = iRenderer::GetRefractionEnabled() && pMaterial->HasRefraction();
            const bool isReflection = pMaterial->HasWorldReflection() && translucencyItem->GetRenderType() == eRenderableType_SubMesh &&
                mpCurrentSettings->mbRenderWorldReflection;
            const bool isFogActive = apWorld->GetFogActive();
            const bool isParticleEmitter = TypeInfo<iParticleEmitter>::IsSubtype(*translucencyItem);
            const auto cubeMap = pMaterial->GetImage(eMaterialTexture_CubeMap);
            uint32_t reflectionBufferIndex = 0;
            cMatrixf* pMatrix = translucencyItem->GetModelMatrix(apFrustum);

            if (translucencyItem->UpdateGraphicsForViewport(apFrustum, afFrameTime) == false) {
                continue;
            }

            if (isReflection) {
                cSubMeshEntity* pReflectionObject = static_cast<cSubMeshEntity*>(translucencyItem);

                bool bReflectionIsInRange = true;
                if (pMaterial->GetMaxReflectionDistance() > 0) {
                        cVector3f vPoint = apFrustum->GetOrigin() + apFrustum->GetForward() * -1 * pMaterial->GetMaxReflectionDistance();
                        cVector3f vNormal = apFrustum->GetForward();

                        cPlanef maxRelfctionDistPlane;
                        maxRelfctionDistPlane.FromNormalPoint(vNormal, vPoint);

                        if (cMath::CheckPlaneBVCollision(maxRelfctionDistPlane, *translucencyItem->GetBoundingVolume()) ==
                            eCollision_Outside) {
                            bReflectionIsInRange = false;
                        }
                }
                if (bReflectionIsInRange) {
                        ///////////////////////////
                        // Make a frustum, mirrored along the plane
                        cSubMesh* pSubMesh = pReflectionObject->GetSubMesh();
                        cVector3f vSurfaceNormal = cMath::Vector3Normalize(
                            cMath::MatrixMul3x3(pReflectionObject->GetWorldMatrix(), pSubMesh->GetOneSidedNormal()));
                        cVector3f vSurfacePos = cMath::MatrixMul(pReflectionObject->GetWorldMatrix(), pSubMesh->GetOneSidedPoint());

                        cPlanef reflectPlane;
                        reflectPlane.FromNormalPoint(vSurfaceNormal, vSurfacePos);

                        cMatrixf mtxReflection = cMath::MatrixPlaneMirror(reflectPlane);
                        cMatrixf mtxReflView = cMath::MatrixMul(apFrustum->GetViewMatrix(), mtxReflection);
                        cVector3f vReflOrigin = cMath::MatrixMul(mtxReflection, apFrustum->GetOrigin());

                        cMatrixf mtxProj = apFrustum->GetProjectionMatrix();

                        cPlanef cameraSpaceReflPlane = cMath::TransformPlane(mtxReflView, reflectPlane);
                        cMatrixf mtxReflProj = cMath::ProjectionMatrixObliqueNearClipPlane(mtxProj, cameraSpaceReflPlane);

                        cFrustum reflectFrustum;
                        reflectFrustum.SetupPerspectiveProj(
                            mtxReflProj,
                            mtxReflView,
                            apFrustum->GetFarPlane(),
                            apFrustum->GetNearPlane(),
                            apFrustum->GetFOV(),
                            apFrustum->GetAspect(),
                            vReflOrigin,
                            false,
                            &mtxProj,
                            true);
                        reflectFrustum.SetInvertsCullMode(true);

                        uint32_t maxFrameDistance = 0;
                        for (uint32_t i = 0; i < common->m_reflectionBuffer.size(); i++) {
                            if (common->m_reflectionBuffer[i].m_target == translucencyItem) {
                                reflectionBufferIndex = i;
                                break;
                            }

                            const uint32_t frameDist = frame.m_currentFrame - common->m_reflectionBuffer[i].m_frameCount;
                            if (frameDist > maxFrameDistance) {
                                maxFrameDistance = frameDist;
                                reflectionBufferIndex = i;
                            }
                        }
                        cRendererDeferred::ReflectionGBuffer& resolveReflectionBuffer = common->m_reflectionBuffer[reflectionBufferIndex];
                        const cMatrixf reflectionFrustumViewInv = cMath::MatrixInverse(reflectFrustum.GetViewMatrix());
                        const cMatrixf reflectionFrustumView = reflectFrustum.GetViewMatrix();
                        const cMatrixf reflectionFrustumProj = reflectFrustum.GetProjectionMatrix();
                        const uint32_t reflectionFrameDescIndex = updateFrameDescriptor(
                            frame,
                            frame.m_cmd,
                            apWorld,
                            { .m_size = float2(
                                  resolveReflectionBuffer.m_buffer.m_depthBuffer.m_handle->mWidth,
                                  resolveReflectionBuffer.m_buffer.m_depthBuffer.m_handle->mHeight),
                              .m_viewMat = reflectionFrustumView,
                              .m_projectionMat = reflectionFrustumProj });

                        FenceStatus fenceStatus;
                        getFenceStatus(frame.m_renderer->Rend(), resolveReflectionBuffer.m_fence.m_handle, &fenceStatus);
                        if (fenceStatus == FENCE_STATUS_INCOMPLETE) {
                            waitForFences(frame.m_renderer->Rend(), 1, &resolveReflectionBuffer.m_fence.m_handle);
                        }
                        resetCmdPool(frame.m_renderer->Rend(), resolveReflectionBuffer.m_pool.m_handle);

                        beginCmd(resolveReflectionBuffer.m_cmd.m_handle);
                        m_reflectionRendererList.BeginAndReset(afFrameTime, &reflectFrustum);
                        cmdPreAndPostZ(
                            resolveReflectionBuffer.m_cmd.m_handle,
                            apWorld,
                            resolveReflectionBuffer.m_buffer.m_preZPassRenderables,
                            frame,
                            m_reflectionRendererList,
                            afFrameTime,
                            resolveReflectionBuffer.m_buffer.m_depthBuffer.m_handle,
                            resolveReflectionBuffer.m_buffer.m_hizDepthBuffer.m_handle,
                            &reflectFrustum,
                            reflectionFrameDescIndex,
                            reflectionFrustumView,
                            reflectionFrustumProj,
                            {
                                .objectVisibilityFlags = eRenderableFlag_VisibleInReflection,
                                .clipPlanes = {},
                                .m_invert = true,
                                .m_disableOcclusionQueries = true,
                            });
                        m_reflectionRendererList.End(
                            eRenderListCompileFlag_Diffuse | eRenderListCompileFlag_Translucent | eRenderListCompileFlag_Decal |
                            eRenderListCompileFlag_Illumination );

                        {
                            cmdBindRenderTargets(resolveReflectionBuffer.m_cmd.m_handle, 0, NULL, NULL, NULL, NULL, NULL, -1, -1);
                            std::array rtBarriers = {
                                RenderTargetBarrier{ resolveReflectionBuffer.m_buffer.m_colorBuffer.m_handle,
                                                     RESOURCE_STATE_SHADER_RESOURCE,
                                                     RESOURCE_STATE_RENDER_TARGET },
                                RenderTargetBarrier{ resolveReflectionBuffer.m_buffer.m_normalBuffer.m_handle,
                                                     RESOURCE_STATE_SHADER_RESOURCE,
                                                     RESOURCE_STATE_RENDER_TARGET },
                                RenderTargetBarrier{ resolveReflectionBuffer.m_buffer.m_positionBuffer.m_handle,
                                                     RESOURCE_STATE_SHADER_RESOURCE,
                                                     RESOURCE_STATE_RENDER_TARGET },
                                RenderTargetBarrier{ resolveReflectionBuffer.m_buffer.m_specularBuffer.m_handle,
                                                     RESOURCE_STATE_SHADER_RESOURCE,
                                                     RESOURCE_STATE_RENDER_TARGET },
                            };
                            cmdResourceBarrier(
                                resolveReflectionBuffer.m_cmd.m_handle, 0, NULL, 0, NULL, rtBarriers.size(), rtBarriers.data());
                        }
                        // options.m_invert = true;
                        cmdBuildPrimaryGBuffer(
                            frame,
                            resolveReflectionBuffer.m_cmd.m_handle,
                            reflectionFrameDescIndex,
                            m_reflectionRendererList,
                            resolveReflectionBuffer.m_buffer.m_colorBuffer.m_handle,
                            resolveReflectionBuffer.m_buffer.m_normalBuffer.m_handle,
                            resolveReflectionBuffer.m_buffer.m_positionBuffer.m_handle,
                            resolveReflectionBuffer.m_buffer.m_specularBuffer.m_handle,
                            resolveReflectionBuffer.m_buffer.m_depthBuffer.m_handle,
                            { .m_invert = true });

                        {
                            cmdBindRenderTargets(resolveReflectionBuffer.m_cmd.m_handle, 0, NULL, NULL, NULL, NULL, NULL, -1, -1);
                            std::array rtBarriers = {
                                RenderTargetBarrier{ resolveReflectionBuffer.m_buffer.m_colorBuffer.m_handle,
                                                     RESOURCE_STATE_RENDER_TARGET,
                                                     RESOURCE_STATE_SHADER_RESOURCE },
                                RenderTargetBarrier{ resolveReflectionBuffer.m_buffer.m_normalBuffer.m_handle,
                                                     RESOURCE_STATE_RENDER_TARGET,
                                                     RESOURCE_STATE_SHADER_RESOURCE },
                                RenderTargetBarrier{ resolveReflectionBuffer.m_buffer.m_positionBuffer.m_handle,
                                                     RESOURCE_STATE_RENDER_TARGET,
                                                     RESOURCE_STATE_SHADER_RESOURCE },
                                RenderTargetBarrier{ resolveReflectionBuffer.m_buffer.m_specularBuffer.m_handle,
                                                     RESOURCE_STATE_RENDER_TARGET,
                                                     RESOURCE_STATE_SHADER_RESOURCE },
                                RenderTargetBarrier{ resolveReflectionBuffer.m_buffer.m_outputBuffer.m_handle,
                                                     RESOURCE_STATE_SHADER_RESOURCE,
                                                     RESOURCE_STATE_RENDER_TARGET },
                            };
                            cmdResourceBarrier(
                                resolveReflectionBuffer.m_cmd.m_handle, 0, NULL, 0, NULL, rtBarriers.size(), rtBarriers.data());
                        }
                        cmdLightPass(
                            resolveReflectionBuffer.m_cmd.m_handle,
                            frame,
                            apWorld,
                            apFrustum,
                            m_reflectionRendererList,
                            reflectionFrameDescIndex,
                            resolveReflectionBuffer.m_buffer.m_colorBuffer.m_handle,
                            resolveReflectionBuffer.m_buffer.m_normalBuffer.m_handle,
                            resolveReflectionBuffer.m_buffer.m_positionBuffer.m_handle,
                            resolveReflectionBuffer.m_buffer.m_specularBuffer.m_handle,
                            resolveReflectionBuffer.m_buffer.m_depthBuffer.m_handle,
                            resolveReflectionBuffer.m_buffer.m_outputBuffer.m_handle,
                            reflectionFrustumView,
                            reflectionFrustumViewInv,
                            reflectionFrustumProj,
                            { .m_invert = true });

                        cmdIlluminationPass(
                            resolveReflectionBuffer.m_cmd.m_handle,
                            frame,
                            m_reflectionRendererList,
                            reflectionFrameDescIndex,
                            resolveReflectionBuffer.m_buffer.m_depthBuffer.m_handle,
                            resolveReflectionBuffer.m_buffer.m_outputBuffer.m_handle,
                            { .m_invert = true });
                        {
                            cmdBindRenderTargets(resolveReflectionBuffer.m_cmd.m_handle, 0, NULL, NULL, NULL, NULL, NULL, -1, -1);
                            std::array rtBarriers = {
                                RenderTargetBarrier{ resolveReflectionBuffer.m_buffer.m_outputBuffer.m_handle,
                                                     RESOURCE_STATE_RENDER_TARGET,
                                                     RESOURCE_STATE_SHADER_RESOURCE },
                            };
                            cmdResourceBarrier(
                                resolveReflectionBuffer.m_cmd.m_handle, 0, NULL, 0, NULL, rtBarriers.size(), rtBarriers.data());
                        }

                        endCmd(resolveReflectionBuffer.m_cmd.m_handle);

                        QueueSubmitDesc submitDesc = {};
                        submitDesc.mCmdCount = 1;
                        submitDesc.ppCmds = &resolveReflectionBuffer.m_cmd.m_handle;
                        submitDesc.pSignalFence = resolveReflectionBuffer.m_fence.m_handle;
                        submitDesc.mSubmitDone = true;
                        queueSubmit(frame.m_renderer->GetGraphicsQueue(), &submitDesc);
                }
            }

            if (isParticleEmitter) {
                if (static_cast<LegacyVertexBuffer*>(vertexBuffer)->GetRequestNumberIndecies() == 0) {
                    continue;
                }
            }

            MaterialRootConstant materialConst = { 0 };
            float sceneAlpha = 1;
            for (auto& fogArea : fogRenderData) {
                sceneAlpha *= detail::GetFogAreaVisibilityForObject(fogArea, *apFrustum, translucencyItem);
            }
            materialConst.m_sceneAlpha = sceneAlpha;
            materialConst.m_lightLevel = 1.0f;

            if (pMaterial->IsAffectedByLightLevel()) {
                cVector3f vCenterPos = translucencyItem->GetBoundingVolume()->GetWorldCenter();
                float fLightAmount = 0.0f;

                ////////////////////////////////////////
                // Iterate lights and add light amount
                for (auto& light : m_rendererList.GetLights()) {
                    auto maxColorValue = [](const cColor& aCol) {
                        return cMath::Max(cMath::Max(aCol.r, aCol.g), aCol.b);
                    };
                    // Check if there is an intersection
                    if (light->CheckObjectIntersection(translucencyItem)) {
                        if (light->GetLightType() == eLightType_Box) {
                            fLightAmount += maxColorValue(light->GetDiffuseColor());
                        } else {
                            float fDist = cMath::Vector3Dist(light->GetWorldPosition(), vCenterPos);

                            fLightAmount += maxColorValue(light->GetDiffuseColor()) * cMath::Max(1.0f - (fDist / light->GetRadius()), 0.0f);
                        }

                        if (fLightAmount >= 1.0f) {
                            fLightAmount = 1.0f;
                            break;
                        }
                    }
                }
                materialConst.m_lightLevel = fLightAmount;
            }
            materialConst.m_afT = GetTimeCount();

            switch (pMaterial->Descriptor().m_id) {
            case MaterialID::Translucent:
                {
                    uint32_t instance = cmdBindMaterialAndObject(
                        frame.m_cmd, frame, pMaterial, translucencyItem, std::optional{ pMatrix ? *pMatrix : cMatrixf::Identity });
                    materialConst.objectId = instance;
                    LegacyVertexBuffer::GeometryBinding binding;

                    if (isParticleEmitter) {
                        std::array targets = { eVertexBufferElement_Position, eVertexBufferElement_Texture0, eVertexBufferElement_Color0 };
                        static_cast<LegacyVertexBuffer*>(vertexBuffer)->resolveGeometryBinding(frame.m_currentFrame, targets, &binding);
                    } else {
                        std::array targets = { eVertexBufferElement_Position,
                                               eVertexBufferElement_Texture0,
                                               eVertexBufferElement_Normal,
                                               eVertexBufferElement_Texture1Tangent,
                                               eVertexBufferElement_Color0 };
                        static_cast<LegacyVertexBuffer*>(vertexBuffer)->resolveGeometryBinding(frame.m_currentFrame, targets, &binding);
                    }

                    cRendererDeferred::TranslucencyPipeline::TranslucencyKey key = {};
                    key.m_field.m_hasDepthTest = pMaterial->GetDepthTest();

                    ASSERT(
                        pMaterial->GetBlendMode() < eMaterialBlendMode_LastEnum && pMaterial->GetBlendMode() != eMaterialBlendMode_None &&
                        "Invalid blend mode");
                    if (isParticleEmitter) {
                        cmdBindPipeline(
                            frame.m_cmd,
                            m_materialTranslucencyPass.m_particlePipelines[translucencyBlendTable[pMaterial->GetBlendMode()]][key.m_id]
                                .m_handle);
                    } else {
                        cmdBindPipeline(
                            frame.m_cmd,
                            (isRefraction
                                 ? m_materialTranslucencyPass.m_refractionPipeline[key.m_id].m_handle
                                 : m_materialTranslucencyPass.m_pipelines[translucencyBlendTable[pMaterial->GetBlendMode()]][key.m_id]
                                       .m_handle));
                    }

                    detail::cmdDefaultLegacyGeomBinding(frame.m_cmd, frame, binding);
                    materialConst.m_options = (isFogActive ? TranslucencyFlags::UseFog : 0) |
                        (isRefraction ? TranslucencyFlags::UseRefractionTrans : 0) | translucencyBlendTable[pMaterial->GetBlendMode()];

                    cmdBindPushConstants(frame.m_cmd, m_materialRootSignature.m_handle, materialObjectIndex, &materialConst);
                    cmdDrawIndexed(frame.m_cmd, binding.m_indexBuffer.numIndicies, 0, 0);

                    if (cubeMap && !isRefraction) {
                        materialConst.m_options = TranslucencyFlags::UseIlluminationTrans | (isFogActive ? TranslucencyFlags::UseFog : 0) |
                            TranslucencyPipeline::TranslucencyBlend::BlendAdd;

                        cmdBindPipeline(
                            frame.m_cmd,
                            m_materialTranslucencyPass.m_pipelines[TranslucencyPipeline::TranslucencyBlend::BlendAdd][key.m_id].m_handle);
                        cmdBindPushConstants(frame.m_cmd, m_materialRootSignature.m_handle, materialObjectIndex, &materialConst);
                        cmdDrawIndexed(frame.m_cmd, binding.m_indexBuffer.numIndicies, 0, 0);
                    }
                }
                break;
            case MaterialID::Water:
                {
                    LegacyVertexBuffer::GeometryBinding binding;
                    std::array targets = { eVertexBufferElement_Position,
                                           eVertexBufferElement_Texture0,
                                           eVertexBufferElement_Normal,
                                           eVertexBufferElement_Texture1Tangent,
                                           eVertexBufferElement_Color0 };
                    static_cast<LegacyVertexBuffer*>(vertexBuffer)->resolveGeometryBinding(frame.m_currentFrame, targets, &binding);

                    materialConst.m_options = (isFogActive ? TranslucencyFlags::UseFog : 0) |
                        (isRefraction ? TranslucencyFlags::UseRefractionTrans : 0) |
                        (isReflection ? TranslucencyFlags::UseReflectionTrans : 0) |
                        (isRefraction ? translucencyBlendTable[eMaterialBlendMode_None]: translucencyBlendTable[eMaterialBlendMode_Mul]) |
                        (((reflectionBufferIndex & TranslucencyReflectionBufferMask) << TranslucencyReflectionBufferOffset));


                    cRendererDeferred::TranslucencyPipeline::TranslucencyWaterKey key = {};
                    key.m_field.m_hasDepthTest = pMaterial->GetDepthTest();

                    detail::cmdDefaultLegacyGeomBinding(frame.m_cmd, frame, binding);

                    cmdBindPipeline(frame.m_cmd, m_materialTranslucencyPass.m_waterPipeline[key.m_id].m_handle);
                    uint32_t instance = cmdBindMaterialAndObject(
                        frame.m_cmd, frame, pMaterial, translucencyItem, std::optional{ pMatrix ? *pMatrix : cMatrixf::Identity });
                    materialConst.objectId = instance;
                    cmdBindPushConstants(frame.m_cmd, m_materialRootSignature.m_handle, materialObjectIndex, &materialConst);
                    cmdDrawIndexed(frame.m_cmd, binding.m_indexBuffer.numIndicies, 0, 0);

                }
                break;
            default:
                ASSERT(false && "Invalid material type");
                continue;
            }
        }
        cmdEndDebugMarker(frame.m_cmd);
    }

    // ImmediateDrawBatch postTransBatch(context, sharedData->m_gBuffer.m_outputTarget, mainFrustumView, mainFrustumProj);
    cViewport::PostTranslucenceDrawPacket translucenceEvent = cViewport::PostTranslucenceDrawPacket();
    translucenceEvent.m_frustum = apFrustum;
    translucenceEvent.m_frame = &frame;
    translucenceEvent.m_outputTarget = &currentGBuffer.m_outputBuffer;
    translucenceEvent.m_viewport = &viewport;
    translucenceEvent.m_renderSettings = mpCurrentSettings;
    viewport.SignalDraw(translucenceEvent);

    {
        cmdBindRenderTargets(frame.m_cmd, 0, NULL, NULL, NULL, NULL, NULL, -1, -1);
        std::array rtBarriers = {
            RenderTargetBarrier{ currentGBuffer.m_outputBuffer.m_handle, RESOURCE_STATE_RENDER_TARGET, RESOURCE_STATE_SHADER_RESOURCE },
        };
        cmdResourceBarrier(frame.m_cmd, 0, NULL, 0, NULL, rtBarriers.size(), rtBarriers.data());
    }
}

iVertexBuffer* cRendererDeferred::GetLightShape(iLight* apLight, eDeferredShapeQuality aQuality) const {
    switch (apLight->GetLightType()) {
    case eLightType_Point:
        return m_shapeSphere[aQuality].get();
    case eLightType_Spot:
        return m_shapePyramid.get();
    case eLightType_Box:
        return m_box.get();
    default:
        break;
    }
    return nullptr;
}
} // namespace hpl
