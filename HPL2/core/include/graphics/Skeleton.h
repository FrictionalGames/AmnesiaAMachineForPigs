/*
 * Copyright © 2011-2020 Frictional Games
 * 
 * This file is part of Amnesia: A Machine For Pigs.
 * 
 * Amnesia: A Machine For Pigs is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version. 

 * Amnesia: A Machine For Pigs is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Amnesia: A Machine For Pigs.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef HPL_SKELETON_H
#define HPL_SKELETON_H

#include <map>

#include "math/MathTypes.h"
#include "graphics/GraphicsTypes.h"
#include "system/SystemTypes.h"

namespace hpl {

	class cBone;

	typedef std::vector<cBone*> tBoneVec;
	typedef tBoneVec::iterator tBoneVecIt;
	
	typedef std::map<tString,int> tBoneIdxNameMap;
	typedef tBoneIdxNameMap::iterator tBoneIdxNameMapIt;

	class cSkeleton
	{
	public:
		cSkeleton();
		~cSkeleton();

		void AddBone(cBone* apBone);
		void RemoveBone(cBone* apBone);

		cBone* GetRootBone();
		
        cBone* GetBoneByIndex(int alIndex);
		cBone* GetBoneByName(const tString &asName);
		cBone* GetBoneBySid(const tString &asSid);
		int GetBoneIndexBySid(const tString &asName);
		int GetBoneIndexByName(const tString &asName);
		int GetBoneNum();
	
	private:
		cBone* mpRootBone;

		tBoneVec mvBones;
		tBoneIdxNameMap m_mapBonesIdxByName;
	};

};
#endif // HPL_SKELETON_H
