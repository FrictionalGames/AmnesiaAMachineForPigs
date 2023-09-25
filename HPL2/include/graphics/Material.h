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

#include "engine/Event.h"
#include "graphics/AnimatedImage.h"
#include "graphics/IndexPool.h"
#include "resources/Resources.h"
#include <graphics/Image.h>
#include <graphics/ImageResourceWrapper.h>
#include <system/SystemTypes.h>
#include <math/MathTypes.h>
#include <graphics/GraphicsTypes.h>
#include <resources/ResourceBase.h>

namespace hpl {

	//---------------------------------------------------

	class cGraphics;
	class cResources;
	class iTexture;
	class iMaterialType;
	class cResourceVarsObject;

    struct MaterialDecal final {
        eMaterialBlendMode m_blend;
    };

    struct MaterialDiffuseSolid final {
        float m_heightMapScale;
        float m_heightMapBias;
        float m_frenselBias;
        float m_frenselPow;
        bool m_alphaDissolveFilter;
    };

    struct MaterialTranslucent final {
        eMaterialBlendMode m_blend;

        bool m_isAffectedByLightLevel;
        bool m_hasRefraction;
        bool m_refractionEdgeCheck;
        bool m_refractionNormals;
        bool m_affectedByLightLevel;

        float m_refractionScale;
        float m_frenselBias;
        float m_frenselPow;
        float m_rimLightMul;
        float m_rimLightPow;
    };

    // always has refraction if enabled
    struct MaterialWater final {
        bool m_hasReflection;
        bool m_isLargeSurface;
        bool m_worldReflectionOcclusionTest; //DEPRECATED

        float m_refractionScale;
        float m_frenselBias;
        float m_frenselPow;
        float m_reflectionFadeStart;
        float m_reflectionFadeEnd;
        float m_waveSpeed;
        float m_waveAmplitude;
        float m_waveFreq;
    };

    enum class MaterialID : uint8_t {
        Unknown = 0,
        SolidDiffuse,
        Translucent,
        Water,
        Decal,
        MaterialIDCount
    };

    struct MaterialDescriptor final {
        MaterialID m_id = MaterialID::Unknown;
        union {
            MaterialDecal m_decal;
            MaterialDiffuseSolid m_solid;
            MaterialTranslucent m_translucent;
            MaterialWater m_water;
        };
    };

	class cMaterialUvAnimation final
	{
	public:
		cMaterialUvAnimation(eMaterialUvAnimation aType, float afSpeed, float afAmp, eMaterialAnimationAxis aAxis) :
							  mType(aType), mfSpeed(afSpeed), mfAmp(afAmp), mAxis(aAxis) {}

	    eMaterialUvAnimation mType;

		float mfSpeed;
		float mfAmp;

		eMaterialAnimationAxis mAxis;
	};


	class cMaterial : public iResourceBase {
	friend class iMaterialType;
	public:
		static constexpr uint32_t MaxMaterialID = 2048;
        static constexpr bool IsTranslucent(const MaterialID id) {
            return id == MaterialID::Water ||
                id == MaterialID::Translucent ||
                id == MaterialID::Decal;
        }

        enum TextureAntistropy {
            Antistropy_None = 0,
            Antistropy_8 = 1,
            Antistropy_16 = 2,
            Antistropy_Count = 3
        };

		struct MaterialUserVariable {
			const char* m_name;
			const char* m_type;
			const char* m_defaultValue;
		};

		struct MaterialMeta {
			MaterialID m_id;
			const char* m_name;
			std::span<const eMaterialTexture> m_usedTextures;
			std::span<const MaterialUserVariable> m_userVariables;
		};

		static constexpr eMaterialTexture SolidMaterialUsedTextures[] = {
			eMaterialTexture_Diffuse ,eMaterialTexture_NMap ,eMaterialTexture_Alpha ,eMaterialTexture_Specular ,eMaterialTexture_Height ,eMaterialTexture_Illumination ,eMaterialTexture_DissolveAlpha ,eMaterialTexture_CubeMap ,eMaterialTexture_CubeMapAlpha
		};
		static constexpr eMaterialTexture TranslucentMaterialUsedTextures[] = {
			eMaterialTexture_Diffuse, eMaterialTexture_NMap, eMaterialTexture_CubeMap, eMaterialTexture_CubeMapAlpha
		};
		static constexpr eMaterialTexture WaterMaterialUsedTextures[] = {
			eMaterialTexture_Diffuse, eMaterialTexture_NMap, eMaterialTexture_CubeMap
		};
		static constexpr eMaterialTexture DecalMaterialUsedTextures[] = {
			eMaterialTexture_Diffuse
		};

		static constexpr const std::array MaterialMetaTable {
			MaterialMeta { MaterialID::SolidDiffuse,"soliddiffuse", std::span(SolidMaterialUsedTextures)},
			MaterialMeta { MaterialID::Translucent,"translucent", std::span(TranslucentMaterialUsedTextures)},
			MaterialMeta { MaterialID::Water,"water", std::span(WaterMaterialUsedTextures)},
			MaterialMeta { MaterialID::Decal,"decal", std::span(DecalMaterialUsedTextures)}
		};

		cMaterial(const tString& asName, const tWString& asFullPath, cResources *apResources);
		virtual ~cMaterial();

		void SetImage(eMaterialTexture aType, iResourceBase *apTexture);
		Image* GetImage(eMaterialTexture aType);
		const Image* GetImage(eMaterialTexture aType) const;

		void SetAutoDestroyTextures(bool abX);
		void SetTextureAnisotropy(float afx);

	    void SetDepthTest(bool abDepthTest);
		inline bool GetDepthTest() const { return mbDepthTest; }

		void SetPhysicsMaterial(const tString & asPhysicsMaterial){ msPhysicsMaterial = asPhysicsMaterial;}
		const tString& GetPhysicsMaterial(){ return msPhysicsMaterial;}

		void UpdateBeforeRendering(float afTimeStep);

        inline eTextureFilter GetTextureFilter() const { return m_textureFilter; }
        inline TextureAntistropy GetTextureAntistropy() const { return m_antistropy; }
        inline eTextureWrap GetTextureWrap() const { return m_textureWrap; }

        void setTextureFilter(eTextureFilter filter);
        void setTextureWrap(eTextureWrap wrap);
		inline int GetRenderFrameCount() const { return mlRenderFrameCount;}
		inline void SetRenderFrameCount(const int alCount) { mlRenderFrameCount = alCount;}

		//Animation
		void AddUvAnimation(eMaterialUvAnimation aType, float afSpeed, float afAmp, eMaterialAnimationAxis aAxis);
		int GetUvAnimationNum(){ return (int)mvUvAnimations.size();}
		cMaterialUvAnimation *GetUvAnimation(int alIdx){ return &mvUvAnimations[alIdx]; }
		inline const cMatrixf& GetUvMatrix() const { return m_mtxUV;}
		void ClearUvAnimations();

        // we want to build a derived state from the matera information
        // decouples the state managment to work on forward++ model
        eMaterialBlendMode GetBlendMode() const;
        eMaterialAlphaMode GetAlphaMode() const;
        bool HasRefraction() const;
        bool HasReflection() const;
        bool HasWorldReflection() const;
        bool IsAffectedByLightLevel() const;
        bool GetLargeTransperantSurface() const;
        float GetMaxReflectionDistance() const;

		bool Reload(){ return false;}
		void Unload(){}
		void Destroy(){}

        inline void SetDescriptor(const MaterialDescriptor& desc) {
            m_descriptor = desc;
            IncreaseGeneration();
        }
        inline const MaterialDescriptor& Descriptor() const {
            return m_descriptor;
        }

		inline uint32_t Index() { return m_handle.get(); }
	    inline void SetHandle(IndexPoolHandle&& handle) { m_handle = std::move(handle); }
		inline uint32_t Generation() { return m_generation; }
	    inline void IncreaseGeneration() { m_generation++; }
    private:
        TextureAntistropy m_antistropy = Antistropy_None;
        eTextureWrap m_textureWrap = eTextureWrap::eTextureWrap_Clamp;
        eTextureFilter m_textureFilter = eTextureFilter::eTextureFilter_Nearest;

		cResources *mpResources;
	    MaterialDescriptor m_descriptor;
		IndexPoolHandle m_handle;

		std::array<ImageResourceWrapper, eMaterialTexture_LastEnum> m_image = {ImageResourceWrapper()};
		std::vector<cMaterialUvAnimation> mvUvAnimations;
		cMatrixf m_mtxUV = cMatrixf::Identity;
		tString msPhysicsMaterial;

		uint32_t m_generation = 0; // used to check if the material has changed since last frame
		float mfAnimTime = 0;
		int mlRenderFrameCount = -1;

		bool mbAutoDestroyTextures = true;
		bool mbDepthTest = true;

	};

};
