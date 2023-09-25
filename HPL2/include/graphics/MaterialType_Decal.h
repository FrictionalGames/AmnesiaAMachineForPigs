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

#include "graphics/MaterialType.h"
#include "graphics/Material.h"

namespace hpl {

    // DEPRECATED
	class cMaterialType_Decal : public iMaterialType
	{
		HPL_RTTI_IMPL_CLASS(iMaterialType, cMaterialType_Decal, "{c0c72fe2-2cb7-4b77-a49b-65660b4dfe45}")

	public:
		cMaterialType_Decal(cGraphics *apGraphics, cResources *apResources);
		~cMaterialType_Decal();

		iMaterialVars* CreateSpecificVariables() override;
		void LoadVariables(cMaterial *apMaterial, cResourceVarsObject *apVars) override;
		void GetVariableValues(cMaterial* apMaterial, cResourceVarsObject* apVars) override;

		void CompileMaterialSpecifics(cMaterial *apMaterial) override;
	private:
		void LoadData();
		void DestroyData();
	};

	//---------------------------------------------------

};
