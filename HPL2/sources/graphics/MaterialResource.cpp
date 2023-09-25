#include "tinyimageformat_query.h"
#include <graphics/MaterialResource.h>
#include <graphics/Material.h>

namespace hpl::material {

    UniformMaterialBlock UniformMaterialBlock::CreateFromMaterial(cMaterial &material) {
        UniformMaterialBlock  block = {};
        auto& descriptor = material.Descriptor();
		const auto alphaMapImage = material.GetImage(eMaterialTexture_Alpha);
		const auto heightMapImage = material.GetImage(eMaterialTexture_Height);
        block.m_common.m_materialConfig = UniformMaterialBlock::CreateMaterailConfigFlags(material);
        switch(descriptor.m_id) {
            case hpl::MaterialID::SolidDiffuse:
                block.m_solid.m_heightMapScale = descriptor.m_solid.m_heightMapScale;
                block.m_solid.m_heightMapBias = descriptor.m_solid.m_heightMapBias;
                block.m_solid.m_frenselBias = descriptor.m_solid.m_frenselBias;
                block.m_solid.m_frenselPow = descriptor.m_solid.m_frenselPow;
                break;
            case hpl::MaterialID::Decal:
                break;
            case hpl::MaterialID::Translucent:
                block.m_translucent.m_refractionScale = descriptor.m_translucent.m_refractionScale;
                block.m_translucent.m_frenselBias = descriptor.m_translucent.m_frenselBias;
                block.m_translucent.m_frenselPow = descriptor.m_translucent.m_frenselPow;
                block.m_translucent.mfRimLightMul = descriptor.m_translucent.m_rimLightMul;
                block.m_translucent.mfRimLightPow = descriptor.m_translucent.m_rimLightPow;
                break;
            case hpl::MaterialID::Water:
                block.m_water.mfRefractionScale = descriptor.m_water.m_refractionScale;
 		        block.m_water.mfFrenselBias = descriptor.m_water.m_frenselBias;
		        block.m_water.mfFrenselPow = descriptor.m_water.m_frenselPow;

		        block.m_water.m_reflectionFadeStart = descriptor.m_water.m_reflectionFadeStart;
		        block.m_water.m_reflectionFadeEnd = descriptor.m_water.m_reflectionFadeEnd;
		        block.m_water.m_waveSpeed = descriptor.m_water.m_waveSpeed;
		        block.m_water.mfWaveAmplitude = descriptor.m_water.m_waveAmplitude;

		        block.m_water.mfWaveFreq = descriptor.m_water.m_waveFreq;

                break;
            default:
                ASSERT(false && "unhandeled material config");
                break;
        }
        return block;
    }

    uint32_t UniformMaterialBlock::CreateMaterailConfigFlags(cMaterial& material) {
	    const auto alphaMapImage = material.GetImage(eMaterialTexture_Alpha);
		const auto heightMapImage = material.GetImage(eMaterialTexture_Height);
        uint32_t flags =
			(material.GetImage(eMaterialTexture_Diffuse) ? EnableDiffuse: 0) |
			(material.GetImage(eMaterialTexture_NMap) ? EnableNormal: 0) |
 			(material.GetImage(eMaterialTexture_Specular) ? EnableSpecular: 0) |
			(alphaMapImage ? EnableAlpha: 0) |
			(heightMapImage ? EnableHeight: 0) |
			(material.GetImage(eMaterialTexture_Illumination) ? EnableIllumination: 0) |
			(material.GetImage(eMaterialTexture_CubeMap) ? EnableCubeMap: 0) |
			(material.GetImage(eMaterialTexture_DissolveAlpha) ? EnableDissolveAlpha: 0) |
			(material.GetImage(eMaterialTexture_CubeMapAlpha) ? EnableCubeMapAlpha: 0) |
			((alphaMapImage && TinyImageFormat_ChannelCount(static_cast<TinyImageFormat>(alphaMapImage->GetTexture().m_handle->mFormat)) == 1) ? IsAlphaSingleChannel: 0) |
			((heightMapImage && TinyImageFormat_ChannelCount(static_cast<TinyImageFormat>(heightMapImage->GetTexture().m_handle->mFormat)) == 1) ? IsHeightMapSingleChannel: 0);
        auto& descriptor = material.Descriptor();
        switch(descriptor.m_id) {
            case hpl::MaterialID::SolidDiffuse:
                flags |=
                    (descriptor.m_solid.m_alphaDissolveFilter ? UseDissolveFilter : 0);
                break;
            case hpl::MaterialID::Translucent:
                flags |=
                    (descriptor.m_translucent.m_hasRefraction ? UseRefractionNormals : 0) |
                    (descriptor.m_translucent.m_hasRefraction && descriptor.m_translucent.m_refractionEdgeCheck ? UseRefractionEdgeCheck : 0);
                break;
            default:
                break;
        }
        return flags;
    }
};
