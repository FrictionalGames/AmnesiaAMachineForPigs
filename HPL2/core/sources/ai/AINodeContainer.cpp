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

#include "ai/AINodeContainer.h"

#include "scene/World.h"
#include "physics/PhysicsBody.h"
#include "system/String.h"
#include "system/LowLevelSystem.h"
#include "system/Platform.h"

#include "math/Math.h"


#include "impl/tinyXML/tinyxml.h"

#include <algorithm>

namespace hpl {

	//////////////////////////////////////////////////////////////////////////
	// AI NODE
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------
	
	cAINode::cAINode()
	{
		mlListID = -1;
	}

	//-----------------------------------------------------------------------

	cAINode::~cAINode()
	{
	}

	//-----------------------------------------------------------------------

	void cAINode::AddEdge(cAINode *pNode)
	{
		cAINodeEdge Edge;
		
		Edge.mpNode = pNode;
		Edge.mfDistance = cMath::Vector3Dist(mvPosition, pNode->mvPosition);
		
		mvEdges.push_back(Edge);
	}
	
	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// RAY INTERSECT
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	void cAINodeRayCallback::Reset()
	{
		mbIntersected = false;
		mpCallback = NULL;
	}
	
	//-----------------------------------------------------------------------

	bool cAINodeRayCallback::Intersected()
	{
		return mbIntersected;
	}

	//-----------------------------------------------------------------------
	
	bool cAINodeRayCallback::BeforeIntersect(iPhysicsBody *pBody)
	{
		if(pBody->GetCollideCharacter()==false) return false;

		if( (mFlags & eAIFreePathFlag_SkipStatic) && pBody->GetMass() == 0) return false;

		if( (mFlags & eAIFreePathFlag_SkipDynamic) && 
			(pBody->GetMass() > 0 || pBody->IsCharacter()) ) return false;

		if( (mFlags & eAIFreePathFlag_SkipVolatile) && pBody->IsVolatile()) return false;

		return true;
	}

	//-----------------------------------------------------------------------

	bool cAINodeRayCallback::OnIntersect(iPhysicsBody *pBody,cPhysicsRayParams *apParams)
	{
		if(mpCallback)
		{
			if(mpCallback->Intersects(pBody,apParams))
			{
				mbIntersected = true;
				return false;
			}
			else
			{
				return true;
			}
		}
		else
		{
			mbIntersected = true;
			return false;
		}
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	cAINodeIterator::cAINodeIterator(cAINodeContainer *apContainer, const cVector3f &avPos, float afRadius)
	{
		mpContainer = apContainer;
		mvPosition = avPos;
		mfRadius = afRadius;
		
		//Calculate local position
		cVector2f vLocalPos(mvPosition.x, mvPosition.z);
		vLocalPos -= mpContainer->mvMinGridPos;
		
		cVector2f vLocalStart = vLocalPos - cVector2f(afRadius);
		cVector2f vLocalEnd = vLocalPos + cVector2f(afRadius);

		mvStartGridPos = mpContainer->GetGridPosFromLocal(vLocalStart);
		mvEndGridPos =  mpContainer->GetGridPosFromLocal(vLocalEnd);
		mvGridPos = mvStartGridPos;

		mpNodeList = &mpContainer->GetGrid(mvGridPos)->mlstNodes;
		while(mpNodeList->empty())
		{
			if(IncGridPos())
			{
				mpNodeList = &mpContainer->GetGrid(mvGridPos)->mlstNodes;
			}
			else
			{
				mpNodeList = NULL;
				break;
			}
		}

		if(mpNodeList){
			mNodeIt = mpNodeList->begin();
		}
		
		//Log("--------------------------------------\n");
		//Log("Iterating (%d %d) -> (%d %d)\n",	mvStartGridPos.x,mvStartGridPos.y,
		//										mvEndGridPos.x,mvEndGridPos.y);
	}

	//-----------------------------------------------------------------------

	bool cAINodeIterator::HasNext()
	{
		if(mpNodeList==NULL || mpNodeList->empty()) return false;

		return true;
	}

	//-----------------------------------------------------------------------

	cAINode *cAINodeIterator::Next()
	{
		cAINode* pNode = *mNodeIt;

		++mNodeIt;
		if(mNodeIt == mpNodeList->end())
		{
			if(IncGridPos())
			{
				mpNodeList = &mpContainer->GetGrid(mvGridPos)->mlstNodes;
				while(mpNodeList->empty())
				{
					if(IncGridPos()){
						mpNodeList = &mpContainer->GetGrid(mvGridPos)->mlstNodes;
					}
					else{
                        mpNodeList = NULL;
						break;
					}
				}
			}
			else
			{
				mpNodeList = NULL;
			}
			
			if(mpNodeList) mNodeIt = mpNodeList->begin();
		}
		
		return pNode;
	}

	//-----------------------------------------------------------------------

	bool cAINodeIterator::IncGridPos()
	{
		mvGridPos.x++;
		if(mvGridPos.x > mvEndGridPos.x)
		{
			mvGridPos.x = mvStartGridPos.x;
			mvGridPos.y++;
			if(mvGridPos.y > mvEndGridPos.y){
				return false;
			}
		}

		return true;
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	cAINodeContainer::cAINodeContainer(	const tString& asName, const tString &asNodeName, 
										cWorld *apWorld, const cVector3f &avCollideSize)
	{
		mpWorld = apWorld;
		mvSize = avCollideSize;
		msName = asName;
		msNodeName = asNodeName;

		mpRayCallback = hplNew( cAINodeRayCallback, () );

		mlMaxNodeEnds = 5;
		mlMinNodeEnds = 2;
		mfMaxEndDistance = 3.0f;
		mfMaxHeight = 0.1f;

		mlNodesPerGrid = 6;
		mlListNum = 0;

		mbNodeIsAtCenter = true;
	}

	//-----------------------------------------------------------------------

	cAINodeContainer::~cAINodeContainer()
	{
		hplDelete(mpRayCallback);
		mvNodes.clear();
	}	

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PUBLIC METHODS
	//////////////////////////////////////////////////////////////////////////
	
	//-----------------------------------------------------------------------

	void cAINodeContainer::ReserveSpace(size_t alReserveSpace)
	{
		mvNodes.reserve(alReserveSpace);
	}

	//-----------------------------------------------------------------------
	
	void cAINodeContainer::AddNode(const tString &asName, int alID, const cVector3f &avPosition, void *apUserData)
	{
		// Only add if there are reserved
		if(mvNodes.size() < mvNodes.capacity())
		{
			cAINode pNode = cAINode();
			pNode.msName = asName;
			pNode.mlID = alID;
			pNode.mvPosition = avPosition;
			pNode.mpUserData = apUserData;

			mvNodes.push_back(pNode);
			m_mapNodesByName.insert(tAINodeNameMap::value_type(asName,&mvNodes.back()));
			m_mapNodesByID.insert(tAINodeIDMap::value_type(alID,&mvNodes.back()));
		}
	}

	//-----------------------------------------------------------------------

	int cAINodeContainer::GetNodeNum() const
	{
		return (int)mvNodes.size();
	}

	//-----------------------------------------------------------------------

	cAINode* cAINodeContainer::GetNodeFromName(const tString &asName)
	{
		tAINodeNameMapIt it = m_mapNodesByName.find(asName);
		if(it == m_mapNodesByName.end())
		{
			return NULL;
		}
		return it->second;
	}
	
	//-----------------------------------------------------------------------

	cAINode* cAINodeContainer::GetNodeFromID(int alID)
	{
		tAINodeIDMapIt it = m_mapNodesByID.find(alID);
		if(it == m_mapNodesByID.end())
		{
			return NULL;
		}
		return it->second;
	}

	//-----------------------------------------------------------------------

	class cSortEndNodes
	{
	public:
		bool operator()(const cAINodeEdge &aEndA, const cAINodeEdge &aEndB)
		{
			return aEndA.mfDistance < aEndB.mfDistance;
		}
	};

	void cAINodeContainer::Compile()
	{
		BuildNodeGridMap();

		tAINodeVecIt CurrentNodeIt = mvNodes.begin();
		for(; CurrentNodeIt != mvNodes.end(); ++CurrentNodeIt)
		{
			cAINode *pNode = &*CurrentNodeIt;

			cAINodeIterator nodeIt = GetNodeIterator(pNode->mvPosition,mfMaxEndDistance*1.5f);
			while(nodeIt.HasNext())
			{
				cAINode *pEndNode = nodeIt.Next();

                if(pEndNode == pNode) continue;				
				float fDist = cMath::Vector3Dist(pNode->mvPosition, pEndNode->mvPosition);
				if(fDist > mfMaxEndDistance*2) continue;
                				
				float fHeight = fabs(pNode->mvPosition.y - pEndNode->mvPosition.y);
				
				tAIFreePathFlag flag = eAIFreePathFlag_SkipDynamic | eAIFreePathFlag_SkipVolatile;
				if(	fHeight <= mfMaxHeight &&
					FreePath(pNode->mvPosition, pEndNode->mvPosition,-1,flag))
				{
					pNode->AddEdge(pEndNode);	
				}
			}

			
			///////////////////////////////////////
			//Sort nodes and remove unwanted ones.
			std::sort(pNode->mvEdges.begin(), pNode->mvEdges.end(), cSortEndNodes());

			if(mlMaxNodeEnds > 0 && (int)pNode->mvEdges.size() > mlMaxNodeEnds) 
			{
				pNode->mvEdges.resize(mlMaxNodeEnds);
			}

			//Remove ends to far, but skip if min nodes is not met
			for(size_t i=0; i< pNode->mvEdges.size(); ++i)
			{
				if( pNode->mvEdges[i].mfDistance > mfMaxEndDistance && (int)i >= mlMinNodeEnds)
				{
					pNode->mvEdges.resize(i);
					break;
				}
			}
		}	

		SetupListID();
	}

	//-----------------------------------------------------------------------

	void cAINodeContainer::BuildNodeGridMap()
	{
		bool bLog=false;

		if(bLog)Log("Nodes: %d\n",mvNodes.size());

		////////////////////////////////////
		// Calculate min and max
		cVector2f vMin(mvNodes[0].GetPosition().x,mvNodes[0].GetPosition().z);
		cVector2f vMax(mvNodes[0].GetPosition().x,mvNodes[0].GetPosition().z);

		for(size_t i=1; i< mvNodes.size(); ++i)
		{
			cAINode *pNode = &mvNodes[i];

			if(vMin.x > pNode->GetPosition().x) vMin.x = pNode->GetPosition().x;
			if(vMin.y > pNode->GetPosition().z) vMin.y = pNode->GetPosition().z;

			if(vMax.x < pNode->GetPosition().x) vMax.x = pNode->GetPosition().x;
			if(vMax.y < pNode->GetPosition().z) vMax.y = pNode->GetPosition().z;
		}

		mvMinGridPos = vMin;
		mvMaxGridPos = vMax;

		////////////////////////////////////
		// Determine size of grids 
		int lGridNum = (int)(sqrt((float)mvNodes.size() / (float)mlNodesPerGrid)+0.5f)+1;

		if(bLog)Log("Grid Num: %d\n",lGridNum);

		mvGridMapSize.x = lGridNum;
		mvGridMapSize.y = lGridNum;
		
		//+1 to fix so that nodes on the border has a grid)
		mvGrids.resize((lGridNum+1) * (lGridNum+1));

		mvGridSize = (mvMaxGridPos - mvMinGridPos);
		mvGridSize.x /= (float)mvGridMapSize.x;
		mvGridSize.y /= (float)mvGridMapSize.y;

		if(bLog)Log("GridSize: %f : %f\n",mvGridSize.x,mvGridSize.y);
		if(bLog)Log("MinPos: %s\n",mvMinGridPos.ToString().c_str());
		if(bLog)Log("MaxPos: %s\n",mvMaxGridPos.ToString().c_str());
		
		////////////////////////////////////
		// Add nodes to grid
		for(size_t i=0; i< mvNodes.size(); ++i)
		{
			cAINode *pNode = &mvNodes[i];

			cVector2f vLocalPos(pNode->GetPosition().x, pNode->GetPosition().z);
			vLocalPos -= mvMinGridPos;
            
            cVector2l vGridPos(0);
			//Have checks so we are sure there is no division by zero.
			if(mvGridSize.x >0)
				vGridPos.x = (int)(vLocalPos.x / mvGridSize.x);
			if(mvGridSize.y >0)
				vGridPos.y = (int)(vLocalPos.y / mvGridSize.y);

			if(false)Log("Adding node %d, world: (%s) local (%s), at %d : %d\n",i,
												pNode->GetPosition().ToString().c_str(),
												vLocalPos.ToString().c_str(),
												vGridPos.x,vGridPos.y);

			mvGrids[vGridPos.y * (mvGridMapSize.x+1) + vGridPos.x].mlstNodes.push_back(pNode);
		}
	}

	//-----------------------------------------------------------------------

	void cAINodeContainer::SetupListID()
	{
		bool bLog=false;

		if(bLog)Log("Nodes: %d\n",mvNodes.size());

		////////////////////////////////////
		// Reset variables
		for(size_t i=0; i< mvNodes.size(); ++i)
		{
			mvNodes[i].mlListID = -1;
		}

		//////////////
		// Search for unintialized node and create list from it and all nodes connected to it in any way
		mlListNum = 0;

		for(size_t i=0; i< mvNodes.size(); ++i)
		{
			if(mvNodes[i].mlListID == -1)
			{
				SetupListIDIterative(&mvNodes[i], mlListNum++);
			}
		}
	}

	void cAINodeContainer::SetupListIDIterative(cAINode* apNode, int alID)
	{
		/////////////
		// Set id
		apNode->mlListID = alID;

		/////////////
		// Set id of all children that have not been set yet
		for(size_t i = 0; i < apNode->mvEdges.size(); ++i)
		{
			if(apNode->mvEdges[i].mpNode->mlListID == -1)
			{
				SetupListIDIterative(apNode->mvEdges[i].mpNode, alID);
			}
		}
	}

	//-----------------------------------------------------------------------

	cAINodeIterator cAINodeContainer::GetNodeIterator(const cVector3f &avPosition, float afRadius)
	{
		return cAINodeIterator(this,avPosition, afRadius);
	}

	//-----------------------------------------------------------------------

	static const cVector2f gvPosAdds[] = {cVector2f(0,0),
										cVector2f(1,0),
										cVector2f(-1,0),
										cVector2f(0,1),
										cVector2f(0,-1),
										cVector2f(0.5,0.5),
										cVector2f(0.5,-0.5),
										cVector2f(-0.5,0.5),
										cVector2f(-0.5,-0.5)
	};	
	
	bool cAINodeContainer::FreePath(const cVector3f &avStart, const cVector3f &avEnd, int alRayNum, 
									tAIFreePathFlag aFlags,iAIFreePathCallback *apCallback)
	{
		iPhysicsWorld *pPhysicsWorld = mpWorld->GetPhysicsWorld();
		if(pPhysicsWorld==NULL) return true;

		
		if(alRayNum<0 || alRayNum>9) alRayNum =5;

		/////////////////////////////
		//Calculate the right vector
		const cVector3f vForward = cMath::Vector3Normalize(avEnd - avStart);
		const cVector3f vUp = cVector3f(0,1.0f,0);
		const cVector3f vRight = cMath::Vector3Cross(vForward, vUp);
		
		//Get the center
		const cVector3f vStartCenter = mbNodeIsAtCenter ? avStart : avStart + cVector3f(0,mvSize.y/2,0);
		const cVector3f vEndCenter  = mbNodeIsAtCenter ? avEnd : avEnd + cVector3f(0,mvSize.y/2,0);
		
		//Get the half with and height. Make them a little smaller so that player can slide over funk on floor.
		const float fHalfWidth = mvSize.x * 0.55f;
		const float fHalfHeight = mvSize.y * 0.4f;
		
		//Setup ray callback
		mpRayCallback->SetFlags(aFlags);

		//Iterate through all the rays.
		for(int i=0; i< alRayNum; ++i)
		{
			cVector3f vAdd = vRight * (gvPosAdds[i].x*fHalfWidth) + vUp * (gvPosAdds[i].y*fHalfHeight);
			cVector3f vStart = vStartCenter + vAdd;
			cVector3f vEnd = vEndCenter + vAdd;

			mpRayCallback->Reset(); 
			mpRayCallback->mpCallback = apCallback;

			pPhysicsWorld->CastRay(mpRayCallback,vStart,vEnd,false,false,false,true);
			
			if(mpRayCallback->Intersected()) return false;
		}

		return true;
	}

	//-----------------------------------------------------------------------
	
	void cAINodeContainer::SaveToFile(const tWString &asFile)
	{
		TiXmlDocument* pXmlDoc = hplNew( TiXmlDocument,() );
		
		TiXmlElement *pRootElem = static_cast<TiXmlElement*>(pXmlDoc->InsertEndChild(TiXmlElement("AINodes")));
		pRootElem->SetAttribute("ListNum", cString::ToString(mlListNum).c_str());

		for(size_t i=0; i< mvNodes.size(); ++i)
		{
			cAINode * pNode = &mvNodes[i];
			TiXmlElement *pNodeElem = static_cast<TiXmlElement*>(pRootElem->InsertEndChild(TiXmlElement("Node")));

			pNodeElem->SetAttribute("Name", pNode->GetName().c_str());
			pNodeElem->SetAttribute("ID", cString::ToString(pNode->GetID()).c_str());
			pNodeElem->SetAttribute("ListID", cString::ToString(pNode->GetListID()).c_str());

			for(int edge =0; edge < pNode->GetEdgeNum(); ++edge)
			{
				cAINodeEdge *pEdge = pNode->GetEdge(edge);
				TiXmlElement *pEdgeElem = static_cast<TiXmlElement*>(pNodeElem->InsertEndChild(TiXmlElement("Edge")));

				pEdgeElem->SetAttribute("Node", pEdge->mpNode->GetName().c_str());
				tString sDistance = cString::ToString(pEdge->mfDistance);
				pEdgeElem->SetAttribute("Distance", sDistance.c_str());
			}
		}

		FILE *pFile = cPlatform::OpenFile(asFile, _W("w+"));
		if(pFile==NULL || pXmlDoc->SaveFile(pFile)==false)
		{
			Error("Couldn't save XML file '%s'\n",cString::To8Char(asFile).c_str());
			hplDelete(pXmlDoc);
			return;
		}
		fclose(pFile);
		hplDelete(pXmlDoc);
	}
	
	//-----------------------------------------------------------------------
	
	void cAINodeContainer::LoadFromFile(const tWString &asFile)
	{
		BuildNodeGridMap();

		FILE *pFile = cPlatform::OpenFile(asFile, _W("rb"));
		if(pFile==NULL) return;
		
		TiXmlDocument* pXmlDoc = hplNew( TiXmlDocument, () );
		if(pXmlDoc->LoadFile(pFile)==false)
		{
			Warning("Couldn't open XML file %s\n",cString::To8Char(asFile).c_str());
			fclose(pFile);
			hplDelete(pXmlDoc);
			return;
		}
		fclose(pFile);

		TiXmlElement *pRootElem = pXmlDoc->RootElement();
		mlListNum = cString::ToInt(pRootElem->Attribute("ListNum"), 1);

		TiXmlElement *pNodeElem = pRootElem->FirstChildElement("Node");
		for(; pNodeElem != NULL; pNodeElem = pNodeElem->NextSiblingElement("Node"))
		{
			tString sName = cString::ToString(pNodeElem->Attribute("Name"),"");
			int lID = cString::ToInt(pNodeElem->Attribute("ID"),-1);
			int lListID = cString::ToInt(pNodeElem->Attribute("ListID"), 0);


			cAINode *pNode = GetNodeFromID(lID);
			if(pNode==NULL)
			{
				Error("Could not find node with id %d in node container cache '%s'\n", lID, cString::To8Char(asFile).c_str());
				continue;
			}

			pNode->mlListID = lListID;
            
			TiXmlElement *pEdgeElem = pNodeElem->FirstChildElement("Edge");
			for(; pEdgeElem != NULL; pEdgeElem = pEdgeElem->NextSiblingElement("Edge"))
			{
				tString sNodeName = cString::ToString(pEdgeElem->Attribute("Node"),"");			
				cAINode *pEdgeNode = GetNodeFromName(sNodeName);
				if(pEdgeNode == NULL)
				{
					Error("Could not find edge node in node %d with name '%s' in node container cache '%s'\n", lID, sNodeName.c_str(), cString::To8Char(asFile).c_str());
					continue;
				}

				cAINodeEdge Edge;
				Edge.mpNode = pEdgeNode;
				Edge.mfDistance = cString::ToFloat(pEdgeElem->Attribute("Distance"),0);

				pNode->mvEdges.push_back(Edge);
			}
		}

		hplDelete(pXmlDoc);
	}
	//-----------------------------------------------------------------------
	
	//////////////////////////////////////////////////////////////////////////
	// PRIVATE METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------
	
	cVector2l cAINodeContainer::GetGridPosFromLocal(const cVector2f &avLocalPos)
	{
		cVector2l vGridPos;
		vGridPos.x = (int)(avLocalPos.x / mvGridSize.x);
		vGridPos.y = (int)(avLocalPos.y / mvGridSize.y);

		if(vGridPos.x <0)vGridPos.x =0;
		if(vGridPos.y <0)vGridPos.y =0;
		if(vGridPos.x > mvGridMapSize.x) vGridPos.x = mvGridMapSize.x;
		if(vGridPos.y > mvGridMapSize.y) vGridPos.y = mvGridMapSize.y;

		return vGridPos;
	}

	//-----------------------------------------------------------------------
	
	cAIGridNode* cAINodeContainer::GetGrid(const cVector2l& avPos)
	{
		int lIndex = avPos.y * (mvGridMapSize.x+1) + avPos.x;
		//Log(" index: %d Max: %d\n",lIndex,(int)mvGrids.size());

		return &mvGrids[lIndex];
	}

	//-----------------------------------------------------------------------
}
