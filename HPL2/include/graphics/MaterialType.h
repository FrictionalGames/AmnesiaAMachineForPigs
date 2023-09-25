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

#ifndef HPL_MATERIAL_TYPE_H
#define HPL_MATERIAL_TYPE_H

#include "scene/Viewport.h"
#include "system/SystemTypes.h"
#include "engine/EngineTypes.h"
#include "math/MathTypes.h"
#include "graphics/GraphicsTypes.h"
#include <functional>
#include <span>

namespace hpl {

	//---------------------------------------------------

	class cGraphics;
	class cResources;
	class iTexture;
	class cMaterial;
	class iRenderable;
	class cParserVarContainer;
	class iRenderer;
	class cResourceVarsObject;
	class iMaterialVars;


	class cMaterialUsedTexture
	{
	public:
		eMaterialTexture mType;
	};

	typedef std::vector<cMaterialUsedTexture> tMaterialUsedTextureVec;
	typedef tMaterialUsedTextureVec::iterator tMaterialUsedTextureVecIt;

	//---------------------------------------------------

	class cMaterialUserVariable
	{
	public:
		tString msName;
		eVariableType mType;
        tString msValue;
		tString msDescription;
		tStringVec mvEnumValues;
	};

	typedef std::vector<cMaterialUserVariable> tMaterialUserVariableVec;
	typedef tMaterialUserVariableVec::iterator tMaterialUserVariableVecIt;

	class iMaterialType
	{
		HPL_RTTI_CLASS(iMaterialType, "{f76039f6-2f46-4135-a7d7-85788ef21cce}")
	public:
		static constexpr uint32_t MaterialIDBufferSize = 60000;
		static uint32_t GetMaterialID();

		iMaterialType(cGraphics *apGraphics, cResources *apResources);
		virtual ~iMaterialType();

		void SetName(const tString& asName);
		const tString& GetName(){ return msName;}

		bool IsTranslucent(){ return mbIsTranslucent; }
		bool IsDecal(){ return mbIsDecal;}

		[[deprecated("use span GetUsedTextures")]]
		int GetUsedTextureNum(){ return (int)m_usedTextures.size(); }
		[[deprecated("use span GetUsedTextures")]]
		cMaterialUsedTexture* GetUsedTexture(int alIdx){ return &m_usedTextures[alIdx]; }

		[[deprecated("use span GetUserVariables")]]
		int GetUserVariableNum() { return (int)m_userVariables.size(); }
		[[deprecated("use span GetUserVariables")]]
		cMaterialUserVariable* GetUserVariable(int alIdx);
		[[deprecated("use GetUserVariables")]]
		cMaterialUserVariable* GetUserVariable(const tString& asName);

		std::span<cMaterialUserVariable> GetUserVariables();

		void Reload();

		virtual void LoadData()=0;
		virtual void DestroyData()=0;

		virtual iMaterialVars* CreateSpecificVariables()=0;
		virtual void LoadVariables(cMaterial *apMaterial, cResourceVarsObject *apVars)=0;
		virtual void GetVariableValues(cMaterial* apMaterial, cResourceVarsObject* apVars)=0;

		virtual void CompileMaterialSpecifics(cMaterial *apMaterial)=0;

		inline bool HasTypeSpecifics(eMaterialRenderMode aMode) const { return mbHasTypeSpecifics[aMode];}
	protected:
		void AddUsedTexture(eMaterialTexture aType);

		void AddVar(const tString& asName, eVariableType aType, const tString& asDefaultValue, const tString& asDesc="", const tStringVec& avEnumValues = tStringVec());

		void AddVarBool(const tString& asName, bool abDefaultValue, const tString& asDesc="");
		void AddVarInt(const tString& asName, int alDefaultValue, const tString& asDesc="");
		void AddVarFloat(const tString& asName, float afDefaultValue, const tString& asDesc="");
		void AddVarVec2(const tString& asName, const cVector2f& avDefaultValue, const tString& asDesc="");
		void AddVarVec3(const tString& asName, const cVector3f& avDefaultValue, const tString& asDesc="");
		void AddVarString(const tString& asName, const tString& asDefaultValue, const tString& asDesc="");
		void AddVarColor(const tString& asName, const cColor& aDefaultValue, const tString& asDesc="");
		void AddVarEnum(const tString& asName, const tString& asDefaultValue, const tStringVec& avEnumValues, const tString& asDesc="");

		cGraphics *mpGraphics;
		cResources *mpResources;

		tString msName;

		bool mbIsTranslucent;
		bool mbIsDecal;

		bool mbHasTypeSpecifics[eMaterialRenderMode_LastEnum];

		std::vector<cMaterialUsedTexture> m_usedTextures;
		std::vector<cMaterialUserVariable> m_userVariables;
	};

};
#endif // HPL_MATERIAL_H
