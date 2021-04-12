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

#ifndef HPL_MESH_LOADER_FBX_H
#define HPL_MESH_LOADER_FBX_H

#define K_PLUGIN
#define K_FBXSDK
#define K_NODLL


#include <fbxsdk.h>
#include "math/MathTypes.h"

#include "resources/MeshLoader.h"

using namespace FBXSDK_NAMESPACE;

namespace hpl {

	class cMesh;
	class cNode3D;
	class iVertexBuffer;
	class cBone;
	class cSkeleton;
	class cAnimation;
	class cAnimationTrack;
	class cMeshLoaderMSH;

	//------------------------------------------------------------
	
	class cSubMeshData
	{
	public:
		iVertexBuffer* mpVtxBuffer;
        
		cMatrixf m_mtxGlobal;
		cMatrixf m_mtxLocal;
		
		tString msName;
		tString msMaterial;

		tVertexBonePairVec mvVtxBonePairs;
		
		bool mbVisible;
	};

	typedef std::list<cSubMeshData> tSubMeshDataList;
	typedef tSubMeshDataList::iterator tSubMeshDataListIt;
	
	//------------------------------------------------------------
	
	class cTakeKeyData
	{
	public: 
		float mfTime;
		float mfValue;
	};

	typedef std::vector<cTakeKeyData> tTakeKeyDataVec;
	typedef tTakeKeyDataVec::iterator tTakeKeyDataVecIt;

	//------------------------------------------------------------
	
	class cTempKeyFrameData
	{
	public: 
		float mfTime;
		cVector3f vTrans;
		cVector3f vScale;
		cVector3f vRot;
		cQuaternion qFinalRot;
	};
	
	typedef std::vector<cTempKeyFrameData> tTempKeyFrameDataVec;
	
	//------------------------------------------------------------

	class cExtraVertrices
	{
	public:
		tIntVec mvNewPoints;
	};

	typedef std::vector<cExtraVertrices> tExtraVertricesVec;
	typedef tExtraVertricesVec::iterator tExtraVertricesVecIt;

	//------------------------------------------------------------

	class cExtraVtxValue
	{
	public:
		cExtraVtxValue(int idx, const cVector3f& avVal) : mlIndexNum(idx), mvVal(avVal){}

		int mlIndexNum;
		cVector3f mvVal;
	};

	typedef std::list<cExtraVtxValue> tExtraVtxValueList;
	typedef tExtraVtxValueList::iterator tExtraVtxValueListIt;
	
	//------------------------------------------------------------
	
	typedef std::set<float> tAnimTimeSet;
	typedef tAnimTimeSet::iterator tAnimTimeSetIt;

	//------------------------------------------------------------

	class cMeshLoaderFBX : public iMeshLoader
	{
	public:
		cMeshLoaderFBX(iLowLevelGraphics *apLowLevelGraphics, cMeshLoaderMSH *apMeshLoaderMSH, bool abLoadAndSaveMSHFormat);
		~cMeshLoaderFBX();

		virtual cMesh* LoadMesh(const tWString& asFile, tMeshLoadFlag aFlags);
		virtual bool SaveMesh(cMesh* apMesh,const tWString& asFile) { return false; };

		virtual cAnimation* LoadAnimation(const tWString& asFile);
		virtual bool SaveAnimation(cAnimation* apAnimation, const tWString& asFile) { return false; };

		//bool IsSupported(const tWString asFileType);
	private:
		cAnimation* LoadAnimations(fbxsdk::FbxScene *apScene,FbxImporter * apImporter, const tWString& asFile, cSkeleton * apSkeleton);
		void LoadAnimationRec(fbxsdk::FbxScene *apScene,FbxNode * apNode,cAnimation* apAnimation, const tString &asAnimStackName, 
		int alDepth,cVector3f vParentT, cVector3f vParentS, cVector3f vParentR, cSkeleton * apSkeleton);

		void MakeFinalBonesRec(cBone* apBone, cMatrixf a_mtxParentGlobal, cMatrixf a_mtxParentGlobalUnscaled);

		void LoadSkeletonRec(cBone* apBone,FbxNode *apNode, int alDepth);
		void LoadSceneRec(tSubMeshDataList* apSubMeshList,cSkeleton* apSkeleton, cNode3D* apHplNode,FbxNode *apNode, int alDepth, bool animationOnly);
		void LoadMeshData(tSubMeshDataList* apSubMeshList,cSkeleton* apSkeleton, cNode3D* apHplNode, FbxNode *apNode, int alDepth, bool animationOnly);

		cBone* LoadSkeletonData(cBone* apBone,FbxNode *apNode, int alDepth);

		const char* GetTabs(int alDepth);
		const char* GetAttrName(FbxNodeAttribute::EType alNum);
		const char* GetSkelTypeName(FbxSkeleton::EType alNum);
		
		const char* GetLinkModeName(fbxsdk::FbxCluster::ELinkMode alNum);

		bool LoadScene(FbxManager* pSdkManager, FbxDocument* pScene, const char* pFilename);

		FbxManager* mpSdkManager;

		tString msTemp;
		bool mbLog;
		bool mbLowLog;
		bool mbLoadAndSaveMSHFormat;

		cMeshLoaderMSH * mpMeshLoaderMSH;
	};

};
#endif // HPL_MESH_LOADER_FBX_H
