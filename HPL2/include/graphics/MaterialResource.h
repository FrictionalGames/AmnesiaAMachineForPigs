#pragma once

#include "graphics/Material.h"
#include <cstdint>

namespace hpl {
    class cMaterial;

    namespace material {
        enum TextureConfigFlags {
            EnableDiffuse = 1 << 0,
            EnableNormal = 1 << 1,
            EnableSpecular = 1 << 2,
            EnableAlpha = 1 << 3,
            EnableHeight = 1 << 4,
            EnableIllumination = 1 << 5,
            EnableCubeMap = 1 << 6,
            EnableDissolveAlpha = 1 << 7,
            EnableCubeMapAlpha = 1 << 8,

            // additional flags
            IsHeightMapSingleChannel = 1 << 9,
            IsAlphaSingleChannel = 1 << 10,

            // Solid Diffuse
            UseDissolveFilter = 1 << 14,

            // Translucent
            UseRefractionNormals = 1 << 14,
            UseRefractionEdgeCheck = 1 << 15,
        };

        struct UniformMaterialBlock {
            union {
                struct {
                    uint32_t m_materialConfig;
                } m_common;
                struct {
                    uint32_t m_materialConfig;
                    float m_heightMapScale;
                    float m_heightMapBias;
                    float m_frenselBias;

                    float m_frenselPow;
                    uint32_t m_pad0;
                    uint32_t m_pad1;
                    uint32_t m_pad2;
                } m_solid;

                struct {
                    uint32_t m_materialConfig;
                    float m_refractionScale;
                    float m_frenselBias;
                    float m_frenselPow;

                    float mfRimLightMul;
                    float mfRimLightPow;
                    uint32_t m_pad0;
                    uint32_t m_pad1;
                } m_translucent;

                struct {
                    uint32_t m_materialConfig;
                    float mfRefractionScale;
                    float mfFrenselBias;
                    float mfFrenselPow;

                    float m_reflectionFadeStart;
                    float m_reflectionFadeEnd;
                    float m_waveSpeed;
                    float mfWaveAmplitude;

                    float mfWaveFreq;
                    uint32_t m_pad0;
                    uint32_t m_pad1;
                    uint32_t m_pad2;
                } m_water;
            };
            static UniformMaterialBlock CreateFromMaterial(cMaterial& material);
            static uint32_t CreateMaterailConfigFlags(cMaterial& material);
        };
    } // namespace material
} // namespace hpl
