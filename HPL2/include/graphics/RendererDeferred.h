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

#include "scene/Viewport.h"
#include "scene/World.h"
#include "windowing/NativeWindow.h"

#include <graphics/ForgeHandles.h>
#include <graphics/ForgeRenderer.h>
#include <graphics/Image.h>
#include <graphics/Material.h>
#include <graphics/PassHBAOPlus.h>
#include <graphics/RenderList.h>
#include <graphics/RenderTarget.h>
#include <graphics/Renderable.h>
#include <graphics/Renderer.h>
#include <math/MathTypes.h>

#include <Common_3/Graphics/Interfaces/IGraphics.h>
#include <Common_3/Utilities/RingBuffer.h>
#include <FixPreprocessor.h>

#include <folly/small_vector.h>

#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

namespace hpl {

    class iFrameBuffer;
    class iDepthStencilBuffer;
    class iTexture;
    class iLight;
    class cSubMeshEntity;

    enum eDeferredShapeQuality {
        eDeferredShapeQuality_Low,
        eDeferredShapeQuality_Medium,
        eDeferredShapeQuality_High,
        eDeferredShapeQuality_LastEnum,
    };

    enum eDeferredSSAO {
        eDeferredSSAO_InBoxLight,
        eDeferredSSAO_OnColorBuffer,
        eDeferredSSAO_LastEnum,
    };

    class cRendererDeferred : public iRenderer {
        HPL_RTTI_IMPL_CLASS(iRenderer, cRendererDeferred, "{A3E5E5A1-1F9C-4F5C-9B9B-5B9B9B5B9B9B}")
    public:
        static constexpr TinyImageFormat DepthBufferFormat = TinyImageFormat_D32_SFLOAT_S8_UINT;
        static constexpr TinyImageFormat NormalBufferFormat = TinyImageFormat_R16G16B16A16_SFLOAT;
        static constexpr TinyImageFormat PositionBufferFormat = TinyImageFormat_R32G32B32A32_SFLOAT;
        static constexpr TinyImageFormat SpecularBufferFormat = TinyImageFormat_R8G8_UNORM;
        static constexpr TinyImageFormat ColorBufferFormat = TinyImageFormat_R8G8B8A8_UNORM;
        static constexpr TinyImageFormat ShadowDepthBufferFormat = TinyImageFormat_D32_SFLOAT;
        static constexpr uint32_t MaxReflectionBuffers = 4;
        static constexpr uint32_t MaxObjectUniforms = 4096;
        static constexpr uint32_t MaxLightUniforms = 1024;
        static constexpr uint32_t MaxHiZMipLevels = 10;
        static constexpr uint32_t MaxViewportFrameDescriptors = 256;
        static constexpr uint32_t MaxMaterialSamplers = static_cast<uint32_t>(eTextureWrap_LastEnum) * static_cast<uint32_t>(eTextureFilter_LastEnum) * static_cast<uint32_t>(cMaterial::TextureAntistropy::Antistropy_Count);
        static constexpr uint32_t MaxObjectTest = 32768;
        static constexpr uint32_t MaxOcclusionDescSize = 4096;
        static constexpr uint32_t MaxQueryPoolSize = MaxOcclusionDescSize * 2;

        static constexpr uint32_t TranslucencyBlendModeMask = 0xf;

        static constexpr uint32_t TranslucencyReflectionBufferMask = 0x7;
        static constexpr uint32_t TranslucencyReflectionBufferOffset = 4;

        static constexpr float ShadowDistanceMedium = 10;
        static constexpr float ShadowDistanceLow = 20;
        static constexpr float ShadowDistanceNone = 40;
        enum LightConfiguration { HasGoboMap = 0x1, HasShadowMap = 0x2 };

        enum LightPipelineVariants {
            LightPipelineVariant_CW = 0x0,
            LightPipelineVariant_CCW = 0x1,
            LightPipelineVariant_StencilTest = 0x2,
            LightPipelineVariant_Size = 4,
        };

        enum TranslucencyFlags {
            UseIlluminationTrans = (1 << 7),
            UseReflectionTrans = (1 << 8),
            UseRefractionTrans = (1 << 9),
            UseFog = (1 << 10),
        };
        struct MaterialRootConstant {
            uint32_t objectId;
            uint32_t m_options;
            float m_afT;
            float m_sceneAlpha;
            float m_lightLevel;
        };
        union UniformLightData {
            struct LightUniformCommon {
                mat4 m_mvp;

                uint32_t m_config;
            } m_common;
            struct {
                mat4 m_mvp;

                uint32_t m_config;
                uint32_t m_pad[3];

                mat4 m_invViewRotation;

                float3 m_lightPos;
                float m_radius;

                float4 m_lightColor;

            } m_pointLight;
            struct {
                mat4 m_mvp;

                uint32_t m_config;
                uint32_t m_pad[3];

                float3 m_forward;
                float m_oneMinusCosHalfSpotFOV;

                mat4 m_spotViewProj;

                float4 m_color;

                float3 m_pos;
                float m_radius;
            } m_spotLight;
            struct {
                mat4 m_mvp;

                uint32_t m_config;
                uint32_t m_pad[3];

                float4 m_lightColor;
            } m_boxLight;
        };

        struct UniformObject {
            float m_dissolveAmount;
            uint m_materialIndex;
            uint pad[2];
            mat4 m_modelMat;
            mat4 m_invModelMat;
            mat4 m_uvMat;
        };

        struct UniformPerFrameData {
            mat4 m_invViewRotation;
            mat4 m_viewMatrix;
            mat4 m_invViewMatrix;
            mat4 m_projectionMatrix;
            mat4 m_viewProjectionMatrix;

            float worldFogStart;
            float worldFogLength;
            float oneMinusFogAlpha;
            float fogFalloffExp;
            float4 fogColor;

            float2 viewTexel;
            float2 viewportSize;
        };
        struct UniformFogData {
            mat4 m_mvp;
            mat4 m_mv;
            mat4 m_invModelRotation;
            float4 m_color;
            float4 m_rayCastStart;
            float4 m_fogNegPlaneDistNeg;
            float4 m_fogNegPlaneDistPos;
            float m_start;
            float m_length;
            float m_falloffExp;
            uint32_t m_flags;
        };

        struct UniformFullscreenFogData {
            float4 m_color;
            float m_fogStart;
            float m_fogLength;
            float m_fogFalloffExp;
        };

        class ShadowMapData {
        public:
            SharedRenderTarget m_target;
            uint32_t m_transformCount = 0;
            uint32_t m_frameCount = 0;
            float m_radius = 0.0f;
            float m_fov = 0.0f;
            float m_aspect = 0.0f;

            iLight* m_light = nullptr;
            SharedCmdPool m_pool;
            SharedCmd m_cmd;
            SharedFence m_shadowFence;
        };

        struct FogRendererData {
            cFogArea* m_fogArea;
            bool m_insideNearFrustum;
            cVector3f m_boxSpaceFrustumOrigin;
            cMatrixf m_mtxInvBoxSpace;
        };

        struct GBuffer {
        public:
            GBuffer() = default;
            GBuffer(const GBuffer&) = delete;
            GBuffer(GBuffer&& buffer)
                : m_colorBuffer(std::move(buffer.m_colorBuffer))
                , m_normalBuffer(std::move(buffer.m_normalBuffer))
                , m_positionBuffer(std::move(buffer.m_positionBuffer))
                , m_specularBuffer(std::move(buffer.m_specularBuffer))
                , m_depthBuffer(std::move(buffer.m_depthBuffer))
                , m_outputBuffer(std::move(buffer.m_outputBuffer))
                , m_refractionImage(std::move(buffer.m_refractionImage))
                , m_hizDepthBuffer(std::move(buffer.m_hizDepthBuffer))
                , m_preZPassRenderables(std::move(buffer.m_preZPassRenderables))
                 {
            }
            void operator=(GBuffer&& buffer) {
                m_colorBuffer = std::move(buffer.m_colorBuffer);
                m_normalBuffer = std::move(buffer.m_normalBuffer);
                m_positionBuffer = std::move(buffer.m_positionBuffer);
                m_specularBuffer = std::move(buffer.m_specularBuffer);
                m_depthBuffer = std::move(buffer.m_depthBuffer);
                m_outputBuffer = std::move(buffer.m_outputBuffer);
                m_refractionImage = std::move(buffer.m_refractionImage);
                m_hizDepthBuffer = std::move(buffer.m_hizDepthBuffer);
                m_preZPassRenderables = std::move(buffer.m_preZPassRenderables);
            }
            std::set<iRenderable*> m_preZPassRenderables;

            SharedTexture m_refractionImage;
            SharedRenderTarget m_hizDepthBuffer;

            SharedRenderTarget m_colorBuffer;
            SharedRenderTarget m_normalBuffer;
            SharedRenderTarget m_positionBuffer;
            SharedRenderTarget m_specularBuffer;
            SharedRenderTarget m_depthBuffer;
            SharedRenderTarget m_outputBuffer;

            bool isValid() {
                return m_refractionImage.IsValid()
                    && m_hizDepthBuffer.IsValid()
                    && m_colorBuffer.IsValid()
                    && m_normalBuffer.IsValid()
                    && m_positionBuffer.IsValid()
                    && m_specularBuffer.IsValid()
                    && m_depthBuffer.IsValid()
                    && m_outputBuffer.IsValid();
            }
        };

        struct ReflectionGBuffer {
            iRenderable* m_target = nullptr;

            uint32_t m_frameCount = 0;
            GBuffer m_buffer;
            SharedCmdPool m_pool;
            SharedCmd m_cmd;
            SharedFence m_fence;

            ReflectionGBuffer() = default;
            ReflectionGBuffer(ReflectionGBuffer&& buffer):
                m_pool(std::move(buffer.m_pool)),
                m_cmd(std::move(buffer.m_cmd)),
                m_fence(std::move(buffer.m_fence)),
                m_buffer(std::move(buffer.m_buffer)),
                m_frameCount(buffer.m_frameCount){
            }
            void operator=(ReflectionGBuffer&& buffer) {
                m_pool = std::move(buffer.m_pool);
                m_cmd = std::move(buffer.m_cmd);
                m_fence = std::move(buffer.m_fence);
                m_buffer = std::move(buffer.m_buffer);
                m_frameCount= buffer.m_frameCount;
            }
        };

        struct ViewportData {
        public:
            ViewportData() = default;
            ViewportData(const ViewportData&) = delete;
            ViewportData(ViewportData&& buffer)
                : m_size(buffer.m_size)
                , m_refractionImage(std::move(buffer.m_refractionImage))
                , m_gBuffer(std::move(buffer.m_gBuffer))
                , m_reflectionBuffer(std::move(buffer.m_reflectionBuffer)) {
            }

            ViewportData& operator=(const ViewportData&) = delete;

            void operator=(ViewportData&& buffer) {
                m_size = buffer.m_size;
                m_refractionImage = std::move(buffer.m_refractionImage);
                m_gBuffer = std::move(buffer.m_gBuffer);
                m_reflectionBuffer = std::move(buffer.m_reflectionBuffer);
            }

            cVector2l m_size = cVector2l(0, 0);
            std::array<GBuffer, ForgeRenderer::SwapChainLength> m_gBuffer;
            std::array<ReflectionGBuffer, MaxReflectionBuffers> m_reflectionBuffer;
            std::shared_ptr<Image> m_refractionImage;
        };

        cRendererDeferred(cGraphics* apGraphics, cResources* apResources);
        ~cRendererDeferred();

        inline ViewportData* GetSharedData(cViewport& viewport) {
            return m_boundViewportData.resolve(viewport);
        }
        virtual SharedRenderTarget GetOutputImage(uint32_t frameIndex, cViewport& viewport) override {
            auto sharedData = m_boundViewportData.resolve(viewport);
            if (!sharedData) {
                return SharedRenderTarget();
            }
            return sharedData->m_gBuffer[frameIndex].m_outputBuffer;
        }

        virtual bool LoadData() override;
        virtual void DestroyData() override;

        virtual void Draw(
            const ForgeRenderer::Frame& frame,
            cViewport& viewport,
            float afFrameTime,
            cFrustum* apFrustum,
            cWorld* apWorld,
            cRenderSettings* apSettings,
            bool abSendFrameBufferToPostEffects) override;

    private:
        iVertexBuffer* GetLightShape(iLight* apLight, eDeferredShapeQuality aQuality) const;
        struct PerObjectOption {
            std::optional<cMatrixf> m_modelMatrix = std::nullopt;
        };
        struct PerFrameOption {
            float2 m_size;
            cMatrixf m_viewMat;
            cMatrixf m_projectionMat;
        };
        void RebuildGBuffer(ForgeRenderer& renderer,GBuffer& buffer, uint32_t width, uint32_t height);
        struct AdditionalGbufferPassOptions {
            bool m_invert = false;
        };

        void cmdBuildPrimaryGBuffer(const ForgeRenderer::Frame& frame, Cmd* cmd,
            uint32_t frameDescriptorIndex,
            cRenderList& renderList,
            RenderTarget* colorBuffer,
            RenderTarget* normalBuffer,
            RenderTarget* positionBuffer,
            RenderTarget* specularBuffer,
            RenderTarget* depthBuffer,
            AdditionalGbufferPassOptions options
        );

        struct AdditionalLightPassOptions {
            bool m_invert = false;
        };
        void cmdLightPass(Cmd* cmd,
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
            AdditionalLightPassOptions options);

        struct AdditionalIlluminationPassOptions {
            bool m_invert = false;
        };
        void cmdIlluminationPass(Cmd* cmd,
            const ForgeRenderer::Frame& frame,
            cRenderList& renderList,
            uint32_t frameDescriptorIndex,
            RenderTarget* depthBuffer,
            RenderTarget* outputBuffer,
            AdditionalIlluminationPassOptions options
        );

        struct AdditionalZPassOptions {
            tRenderableFlag objectVisibilityFlags = eRenderableFlag_VisibleInNonReflection;
            std::span<cPlanef> clipPlanes = {};
            bool m_invert = false;
            bool m_disableOcclusionQueries = false;
        };
        void cmdPreAndPostZ(
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
            AdditionalZPassOptions flags);

        uint32_t updateFrameDescriptor(const ForgeRenderer::Frame& frame,Cmd* cmd, cWorld* apWorld,const PerFrameOption& options);
        uint32_t cmdBindMaterialAndObject(Cmd* cmd,
            const ForgeRenderer::Frame& frame,
            cMaterial* apMaterial,
            iRenderable* apObject,
            std::optional<cMatrixf> modelMatrix = std::nullopt);

        std::array<std::unique_ptr<iVertexBuffer>, eDeferredShapeQuality_LastEnum> m_shapeSphere;
        std::unique_ptr<iVertexBuffer> m_shapePyramid;
        std::unique_ptr<iVertexBuffer> m_box;
        std::array<folly::small_vector<ShadowMapData, 32>, eShadowMapResolution_LastEnum> m_shadowMapData;

        UniqueViewportData<ViewportData> m_boundViewportData;

        SharedTexture m_shadowJitterTexture;
        SharedTexture m_ssaoScatterDiskTexture;

        Image* m_dissolveImage;
        std::array<SharedBuffer, MaxViewportFrameDescriptors> m_perFrameBuffer;

        // decal pass
        std::array<SharedPipeline, eMaterialBlendMode_LastEnum> m_decalPipeline;
        std::array<SharedPipeline, eMaterialBlendMode_LastEnum> m_decalPipelineCW;
        SharedShader m_decalShader;

        struct Fog {
            static constexpr uint32_t MaxFogCount = 128;

            enum FogVariant {
                EmptyVariant = 0x0,
                UseBackSide = 0x1,
                UseOutsideBox = 0x2,
            };

            enum PipelineVariant {
                PipelineVariantEmpty = 0x0,
                PipelineUseBackSide = 0x1,
                PipelineUseOutsideBox = 0x2,
            };

            std::array<DescriptorSet*, ForgeRenderer::SwapChainLength> m_perFrameSet{};

            std::array<SharedBuffer, ForgeRenderer::SwapChainLength> m_fogUniformBuffer;
            std::array<SharedBuffer, ForgeRenderer::SwapChainLength> m_fogFullscreenUniformBuffer;
            uint32_t m_fogIndex = 0;
            SharedRootSignature m_fogRootSignature;
            SharedShader m_shader;
            SharedPipeline m_pipeline;
            SharedPipeline m_pipelineInsideNearFrustum;

            SharedShader m_fullScreenShader;
            SharedPipeline m_fullScreenPipeline;
        } m_fogPass;

        SharedRootSignature m_materialRootSignature;
        // diffuse solid
        struct MaterialSolid {
            SharedShader m_solidDiffuseShader;
            SharedShader m_solidDiffuseParallaxShader;
            SharedPipeline m_solidDiffusePipeline;
            SharedPipeline m_solidDiffuseParallaxPipeline;

            SharedPipeline m_solidDiffusePipelineCW;
            SharedPipeline m_solidDiffuseParallaxPipelineCW;
        } m_materialSolidPass;

        // illumination pass
        SharedShader m_solidIlluminationShader;
        SharedPipeline m_solidIlluminationPipelineCCW;
        SharedPipeline m_solidIlluminationPipelineCW;

        // translucency pass
        struct TranslucencyPipeline {
            enum TranslucencyShaderVariant {
                TranslucencyShaderVariantEmpty = 0x0,
                TranslucencyShaderVariantFog = 0x1,
                TranslucencyRefraction = 0x2,
                TranslucencyVariantCount = 4
            };

            enum TranslucencyWaterShaderVariant {
                TranslucencyWaterShaderVariantEmpty = 0x0,
                TranslucencyWaterShaderVariantFog = 0x1,
                TranslucencyWaterRefraction = 0x2,
                TranslucencyWaterReflection = 0x4,
                TranslucencyWaterVariantCount = 8
            };

            enum TranslucencyParticleShaderVariant {
                TranslucencyParticleShaderVariantEmpty = 0x0,
                TranslucencyParticleShaderVariantFog = 0x1,
                TranslucencyParticleVariantCount = 2
            };
            enum TranslucencyBlend : uint8_t {
                BlendAdd,
                BlendMul,
                BlendMulX2,
                BlendAlpha,
                BlendPremulAlpha,
                BlendModeCount };

            // 3 bit key for pipeline variant
            union TranslucencyKey {
                uint8_t m_id;
                struct {
                    uint8_t m_hasDepthTest : 1;
                } m_field;
                static constexpr size_t NumOfVariants = 2;
            };
            union TranslucencyWaterKey {
                uint8_t m_id;
                struct {
                    uint8_t m_hasDepthTest : 1;
                } m_field;
                static constexpr size_t NumOfVariants = 2;
            };
            SharedRootSignature m_refractionCopyRootSignature;
            std::array<SharedDescriptorSet, ForgeRenderer::SwapChainLength> m_refractionPerFrameSet;
            SharedPipeline m_refractionCopyPipeline;
            SharedShader m_copyRefraction;

            SharedShader m_shader{};
            SharedShader m_particleShader{};
            SharedShader m_waterShader{};

            std::array<std::array<SharedPipeline, TranslucencyKey::NumOfVariants>, TranslucencyBlend::BlendModeCount> m_pipelines;
            std::array<SharedPipeline, TranslucencyWaterKey::NumOfVariants> m_waterPipeline;
            std::array<SharedPipeline, TranslucencyKey::NumOfVariants> m_refractionPipeline;
            std::array<std::array<SharedPipeline, TranslucencyKey::NumOfVariants>, TranslucencyBlend::BlendModeCount> m_particlePipelines;

        } m_materialTranslucencyPass;

        // post processing
        struct ObjectSamplerKey {
            union {
                uint8_t m_id;
                struct {
                    AddressMode m_addressMode : 2;
                } m_field;
            };
            static constexpr size_t NumOfVariants = 4;
        };
        struct LightResourceEntry {
            SharedTexture m_goboCubeMap;
            SharedTexture m_goboMap;
            SharedTexture m_falloffMap;
            SharedTexture m_attenuationLightMap;
        };
        folly::F14ValueMap<iRenderable*, uint32_t> m_lightDescriptorLookup;
        std::array<std::array<LightResourceEntry, MaxLightUniforms>, ForgeRenderer::SwapChainLength> m_lightResources{};
        // z pass
        SharedShader m_zPassShader;
        SharedPipeline m_zPassPipelineCCW;
        SharedPipeline m_zPassPipelineCW;

        SharedPipeline m_zPassShadowPipelineCW;
        SharedPipeline m_zPassShadowPipelineCCW;

        std::array<SharedBuffer, ForgeRenderer::SwapChainLength> m_objectUniformBuffer;
        struct MaterialPassDescriptorSet {
            std::array<SharedDescriptorSet, ForgeRenderer::SwapChainLength> m_frameSet;
            std::array<SharedDescriptorSet, ForgeRenderer::SwapChainLength> m_perBatchSet;
            std::array<SharedDescriptorSet, ForgeRenderer::SwapChainLength> m_perObjectSet;
            folly::F14ValueMap<iRenderable*, uint32_t> m_objectDescriptorLookup;
            uint32_t m_frameIndex = 0;
            uint32_t m_objectIndex = 0;

            // Material
            struct MaterialInfo {
                struct MaterialDescInfo {
                    void* m_material = nullptr; // void* to avoid accessing the material
                    uint32_t m_version = 0; // version of the material
                    std::array<SharedTexture, eMaterialTexture_LastEnum> m_textureHandles{}; // handles to keep textures alive for the descriptor
                } m_materialDescInfo[ForgeRenderer::SwapChainLength];
            };

            std::array<MaterialInfo, cMaterial::MaxMaterialID> m_materialInfo;
            std::array<SharedSampler, MaxMaterialSamplers> m_samplers;
            SharedDescriptorSet m_materialConstSet;
            SharedBuffer m_materialUniformBuffer;
        } m_materialSet;

        uint32_t m_activeFrame = 0; // tracks the active frame if differnt then we need to reset some state
        SharedCmdPool m_prePassPool;
        SharedCmd m_prePassCmd;
        SharedFence m_prePassFence;

        SharedRootSignature m_rootSignatureHIZOcclusion;
        SharedShader m_ShaderHIZGenerate;
        SharedShader m_shaderTestOcclusion;
        SharedDescriptorSet m_descriptorSetHIZGenerate;
        SharedPipeline m_pipelineHIZGenerate;

        SharedRootSignature m_rootSignatureCopyDepth;
        SharedDescriptorSet m_descriptorCopyDepth;
        SharedDescriptorSet m_descriptorAABBOcclusionTest;
        SharedPipeline m_pipelineCopyDepth;
        SharedPipeline m_pipelineAABBOcclusionTest;
        SharedShader m_copyDepthShader;
        SharedBuffer m_hiZOcclusionUniformBuffer;
        SharedBuffer m_hiZBoundBoxBuffer;
        SharedBuffer m_occlusionTestBuffer;

        struct OcclusionQueryAlpha {
            iRenderable* m_renderable = nullptr;
            uint32_t m_maxQueryIndex = 0;
            uint32_t m_queryIndex = 0;
        };

        SharedQueryPool m_occlusionQuery;
        uint32_t m_occlusionIndex = 0;
        SharedBuffer m_occlusionUniformBuffer;
        SharedRootSignature m_rootSignatureOcclusuion;
        SharedDescriptorSet m_descriptorOcclusionConstSet;
        SharedBuffer m_occlusionReadBackBuffer;
        SharedShader m_shaderOcclusionQuery;
        SharedPipeline m_pipelineMaxOcclusionQuery;
        SharedPipeline m_pipelineOcclusionQuery;

        struct UniformTest {
            bool m_preZPass = false;
            iRenderable* m_renderable = nullptr;
        };

        struct UniformPropBlock {
            uint2 depthDim;
            uint32_t numObjects;
            uint32_t maxMipLevel;
        };

        // light pass
        struct UniformLightPerFrameSet {
            float gamma;
            float3 pad;
        };
        std::array<SharedBuffer, ForgeRenderer::SwapChainLength> m_lightPassBuffer;

        SharedRootSignature m_lightPassRootSignature;
        SharedPipeline m_lightStencilPipelineCCW;
        SharedPipeline m_lightStencilPipelineCW;
        std::array<SharedPipeline, LightPipelineVariant_Size> m_pointLightPipeline;
        std::array<SharedPipeline, LightPipelineVariant_Size> m_boxLightPipeline;
        std::array<SharedPipeline, LightPipelineVariant_Size> m_spotLightPipeline;
        SharedShader m_pointLightShader;
        SharedShader m_spotLightShader;
        SharedShader m_stencilLightShader;
        SharedShader m_boxLightShader;
        std::array<SharedDescriptorSet, ForgeRenderer::SwapChainLength> m_lightPerLightSet;
        std::array<SharedDescriptorSet, ForgeRenderer::SwapChainLength> m_lightPerFrameSet;
        uint32_t m_lightIndex = 0;

        SharedSampler m_shadowCmpSampler;
        SharedSampler m_samplerPointClampToBorder;
        SharedSampler m_samplerPointClampToEdge;
        SharedSampler m_samplerPointWrap;
        SharedSampler m_goboSampler;
        SharedSampler m_bilinearSampler;

        cRenderList m_rendererList;
        cRenderList m_reflectionRendererList;
        std::unique_ptr<renderer::PassHBAOPlus> m_hbaoPlusPipeline;
    };
}; // namespace hpl
