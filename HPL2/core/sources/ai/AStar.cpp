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

#include "ai/AStar.h"

#include "ai/AINodeContainer.h"

#include "math/Math.h"

#include "system/LowLevelSystem.h"

#include <algorithm>


namespace hpl {

	//////////////////////////////////////////////////////////////////////////
	// NODE
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	cAStarNode::cAStarNode(cAINode *apAINode)
	{
		mpParent = NULL;
		mpAINode = apAINode;
	}

	//-----------------------------------------------------------------------

	bool cAStarNodeCompare::operator()(cAStarNode* apNodeA,cAStarNode* apNodeB) const
	{
		///////////////
		// Sort after cost and unqiue pointer id
		if(apNodeA->mfCost == apNodeB->mfCost)
		{
			return apNodeA->mpAINode < apNodeB->mpAINode;
		}
		return apNodeA->mfCost < apNodeB->mfCost;
	}

	bool cAStarNodePresort::operator()(cAStarNode* apNodeA,cAStarNode* apNodeB) const
	{
		if((apNodeA->mFlags & 0xF) == (apNodeB->mFlags & 0xF))
		{
			if(apNodeA->mfDistance == apNodeB->mfDistance)
			{
				return apNodeA->mpAINode < apNodeB->mpAINode;
			}

			return apNodeA->mfDistance < apNodeB->mfDistance;
		}

		return (apNodeA->mFlags & 0xF) < (apNodeB->mFlags & 0xF);
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	cAStarHandler::cAStarHandler(cAINodeContainer *apContainer)
	{
		mlMaxIterations = -1;
		mlAStarCount = 0;

		mpContainer = apContainer;
		
		mvOpenListCount.resize(mpContainer->GetListNum(), 0);
		mvGoalListCount.resize(mpContainer->GetListNum(), 0);

		mpCallback = NULL;

		//////////////////
		// Create a copy of the nodes
		mvNodes.reserve(apContainer->GetNodeNum());

		for(size_t i = 0; i < mvNodes.capacity(); i++)
		{
			cAStarNode node = cAStarNode(mpContainer->GetNode(i));
			node.mlAStarCount = -1;
			
			mvNodes.push_back(node);
		}
	}

	//-----------------------------------------------------------------------

	cAStarHandler::~cAStarHandler()
	{
		m_setOpenList.clear();
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PUBLIC METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	bool cAStarHandler::GetPath(const cVector3f& avStart, const cVector3f& avGoal,tAINodeList *apNodeList)
	{
		float fMaxHeight = mpContainer->GetMaxHeight()*1.5f;

		/////////////////////////////////////////////////
		// check if there is free path from start to goal
		float fHeight = fabs(avStart.y - avGoal.y);
		if(fHeight <= fMaxHeight && mpContainer->FreePath(avStart,avGoal,-1,eAIFreePathFlag_SkipDynamic|eAIFreePathFlag_SkipVolatile))
		{
			mpGoalNode = NULL;
			return true;
		}

		////////////////////////////////////////////////
		//Reset all variables
		m_setGoalNodes.clear();
		m_setOpenList.clear();
		m_setPresortNodes.clear();
		mpGoalNode=NULL;
		mlAStarCount = mlAStarCount++;

		for(size_t i = 0; i < mvOpenListCount.size(); ++i)
		{
			mvOpenListCount[i] = 0;
			mvGoalListCount[i] = 0;
		}

		mvGoal = avGoal;
		mvStart = avStart;

		////////////////////////////////////////////////
		//Find nodes reachable from the start and goal position (use double 2*2 distance)
		float fMaxDist = mpContainer->GetMaxEdgeDistance() * 2;

		////////////////////////////////
		//Check with Goal
		cAINodeIterator goalNodeIt =  mpContainer->GetNodeIterator(avGoal,fMaxDist);
		while(goalNodeIt.HasNext())
		{
			cAINode *pAINode = goalNodeIt.Next();
			
			float fHeight = fabs(avGoal.y - pAINode->GetPosition().y);
			float fDist = cMath::Vector3Dist(avGoal,pAINode->GetPosition());
			if(fDist < fMaxDist && fHeight <= fMaxHeight)
			{
				/////////////
				// Potential goal, insert and set so that this list can reach the goal
				// A ray cast to this node is cast when it is reached, so that only needed ray casts are performed
				std::pair<tAINodeSetIt, bool> pair = m_setGoalNodes.insert(pAINode);
				mvGoalListCount[pAINode->GetListID()] += pair.second ? 1 : 0;
			}
		}
		
		if(m_setGoalNodes.empty()) return false;

		/////////////////////
		//Check with Start
		cAINodeIterator startNodeIt =  mpContainer->GetNodeIterator(avStart,fMaxDist);
		while(startNodeIt.HasNext())
		{
			cAINode *pAINode = startNodeIt.Next();

			///////////////////
			// Check if this node can reach a goal
			if(mvGoalListCount[pAINode->GetListID()] == 0) 
				continue;

			float fHeight = fabs(avStart.y - pAINode->GetPosition().y);
			float fDist = cMath::Vector3Dist(avStart,pAINode->GetPosition());
			if(fDist < fMaxDist && fHeight <= fMaxHeight)
			{
				////////////////
				// Get XY angle from the start
				cVector3f vDir = avStart - pAINode->GetPosition();
				float fAngle = atan2(vDir.y, vDir.x);
				int lAngleID = int(7.95 * (kPif + fAngle) / kPi2f); // Limit the angle to 8

				///////////////////
				// Add this node to a temporary list used for presorting
				cAStarNode* pNode = GetNode(pAINode);
				pNode->mFlags = lAngleID;
				pNode->mfDistance = fDist;
				m_setPresortNodes.insert(pNode);
			}
		}

		if(m_setPresortNodes.empty()) return false;

		/////////////
		// Get the first 8 nodes, one in each direction from the start
		int lNeeded = 8;
		while(m_setPresortNodes.empty() == false && m_setOpenList.size() < lNeeded)
		{
			int lAngle = -1;
			tAStarNodePresortSetIt it = m_setPresortNodes.begin();

			while(it != m_setPresortNodes.end())
			{
				/////////////
				// Add the closest node in each direction to the open pool
				cAStarNode* pNode = *it;
				int lNodeAngle = (pNode->mFlags & 0xF);

				if(lAngle < lNodeAngle)
				{
					lAngle = lNodeAngle;
					AddOpenNode(pNode->mpAINode, NULL, pNode->mfDistance);

					m_setPresortNodes.erase(it++);
				}
				else
				{
					// Go forward until next unique direction
					++it;
				}
			}

			///////////
			// Reduce the number of needed nodes if not all directions have a node
			lNeeded-=2;
		}

		if(m_setOpenList.empty()) return false;

		//////////////
		// Remove all nodes that can not reach
		for(size_t i = 0; i < mvGoalListCount.size(); ++i)
		{
			if(mvGoalListCount[i] != 0 && mvOpenListCount[i] == 0)
			{
				//////////
				// There are goal nodes that can not be reached, clear list
				ClearUnlistedNodes(true, true);
				break;
			}
		}


		////////////////////////////////////////////////
		//Iterate the algorithm
		IterateAlgorithm();

		////////////////////////////////////////////////
		//Check if goal was found, if so build path.
		if(mpGoalNode)
		{
			if(apNodeList)
			{
				cAStarNode *pParentNode = mpGoalNode;
				while(pParentNode != NULL)
				{
					apNodeList->push_back(pParentNode->mpAINode);
					pParentNode = pParentNode->mpParent;
				}
			}

			return true;
		}
		else
		{
			 return false;
		}
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PRIVATE METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	void cAStarHandler::IterateAlgorithm()
	{
		int lIterationCount=0;
		while(m_setOpenList.empty()==false && m_setGoalNodes.empty() == false && (mlMaxIterations <0 || lIterationCount < mlMaxIterations))
		{
			///////////////
			// Get the node with the smallest cost
			cAStarNode *pNode = GetBestNode();

			if(pNode == NULL)
			{
				////////////
				// No more open nodes
				break;
			}

			cAINode *pAINode = pNode->mpAINode;

			//////////////////////
			// Check if current node can reach goal
			if(IsGoalNode(pAINode))
			{
				mpGoalNode = pNode;
				break;
			}

			/////////////////////
			//Add nodes connected to current node
			int lEdgeCount = pAINode->GetEdgeNum();
			for(int i=0; i< lEdgeCount; ++i)
			{
				cAINodeEdge *pEdge = pAINode->GetEdge(i);

				if(mpCallback == NULL || mpCallback->CanAddNode(pAINode, pEdge->mpNode))
				{
					AddOpenNode(pEdge->mpNode, pNode, pNode->mfDistance + pEdge->mfDistance);
				}
			}

			++lIterationCount;
		}
	}

	//-----------------------------------------------------------------------

	cAStarNode* cAStarHandler::GetNode(cAINode *apAINode)
	{
		/////////////
		// Get unique id for this node
		size_t lId = mpContainer->GetUniqueID(apAINode);

		if(lId < mvNodes.size()) return &mvNodes[lId];

		return NULL;
	}

	//---------------------------------------------------------------------------

	void cAStarHandler::AddOpenNode(cAINode *apAINode, cAStarNode *apParent, float afDistance)
	{
		cAStarNode *pNode = GetNode(apAINode);

		if(pNode == NULL) return;

		if(pNode->mlAStarCount == mlAStarCount)
		{
			///////////////////////
			// This node has been added before, check if there is a better fit
			if((pNode->mFlags & eAStarNodeFlag_Open) == false) return; //Best match already found
			
			if(afDistance > pNode->mfDistance)
			{
				if((pNode->mFlags & eAStarNodeFlag_CastRay) && apParent != NULL)
				{
					/////////////
					// Requires ray trace to the start to calculate if this is a better match
					pNode->mFlags &= ~eAStarNodeFlag_CastRay;

					if(mpContainer->FreePath(mvStart, apAINode->GetPosition(), -1, eAIFreePathFlag_SkipDynamic|eAIFreePathFlag_SkipVolatile))
					{
						// There is a free path and the original node has a better path
						return;
					}
				}
				else
				{
					// Further away than the previous, dont switch
					return;
				}
			}

			///////////////
			// Remove from open list, todo: any way to resort without removing?
			tAStarNodeSetIt it = m_setOpenList.find(pNode);
			if (it != m_setOpenList.end()) m_setOpenList.erase(it);
			mvOpenListCount[apAINode->GetListID()]--;
		}

		//////////////
		// Setup node and add to open list
		pNode->mfDistance = afDistance;
		pNode->mfCost = Cost(afDistance, apAINode, apParent) + Heuristic(pNode->mpAINode->GetPosition(), mvGoal);
		pNode->mpParent = apParent;
		pNode->mFlags = eAStarNodeFlag_Open;
		if(apParent == NULL) pNode->mFlags |= eAStarNodeFlag_CastRay; //Requires ray cast to start if it doesnt have a parent
		pNode->mlAStarCount = mlAStarCount;
	
		m_setOpenList.insert(pNode);
		mvOpenListCount[apAINode->GetListID()]++;
	}

	//-----------------------------------------------------------------------

	cAStarNode* cAStarHandler::GetBestNode()
	{
		tAStarNodeSetIt it = m_setOpenList.begin();

		//////////////
		// Get the best node from the node list, the set is sorted by best cost
		while(it != m_setOpenList.end())
		{
			cAStarNode* pNode = *it;

			if(pNode->mFlags & eAStarNodeFlag_CastRay)
			{
				//////////////
				// Need to cast a ray to see if there is a free path, if there is not erase it
				if(mpContainer->FreePath(mvStart, pNode->mpAINode->GetPosition(), -1, eAIFreePathFlag_SkipDynamic|eAIFreePathFlag_SkipVolatile))
				{
					// Free path, use this one
					pNode->mFlags &= ~eAStarNodeFlag_CastRay;
				}
				else
				{
					// Remove from open node list and get the next
					pNode->mlAStarCount--;
					m_setOpenList.erase(it++);
					continue;
				}
			}

			////////////
			// Remove from open
			m_setOpenList.erase(it);
			pNode->mFlags &= ~eAStarNodeFlag_Open;

			if(--mvOpenListCount[pNode->mpAINode->GetListID()] == 0)
			{
				///////////
				// No more open nodes of this id exists, remove all goal nodes with this list id
				ClearUnlistedNodes(true, false);
			}

			return pNode;
		}

		return NULL;
	}

	//-----------------------------------------------------------------------

	float cAStarHandler::Heuristic(const cVector3f& avStart, const cVector3f& avGoal)
	{
		//return cMath::Vector3DistSqr(avStart, avGoal);
		return cMath::Vector3Dist(avStart, avGoal);
	}

	float cAStarHandler::Cost(float afDistance, cAINode *apAINode, cAStarNode *apParent)
	{
		if(apParent)
		{
			float fHeight = (1+fabs(apAINode->GetPosition().y - apParent->mpAINode->GetPosition().y));
			return afDistance * fHeight;
		}
		else
			return afDistance;
	}

	//-----------------------------------------------------------------------

	bool cAStarHandler::IsGoalNode(cAINode *apAINode)
	{
		tAINodeSetIt it = m_setGoalNodes.find(apAINode);
		if(it == m_setGoalNodes.end()) return false;

		//////////////
		// Check if there is a clear path to goal
		cAINode *pAINode = *it;

		if(mpContainer->FreePath(pAINode->GetPosition(), mvGoal,-1, eAIFreePathFlag_SkipDynamic|eAIFreePathFlag_SkipVolatile))
		{
			//////////
			// Free path found, use this
			return true;
		}
		else
		{
			/////////
			// Not a clear path, remove it
			if(--mvGoalListCount[pAINode->GetListID()] == 0)
			{
				///////////
				// No more goals of this id exists, remove all open nodes with this list id
				ClearUnlistedNodes(false, true);
			}

			m_setGoalNodes.erase(it);
			return false;
		}
	}

	//-----------------------------------------------------------------------
	
	void cAStarHandler::ClearUnlistedNodes(bool abGoals, bool abOpen)
	{
		if(m_setGoalNodes.empty() || m_setOpenList.empty()) return;

		if(abGoals)
		{
			///////////////
			// Remove goal nodes that no longer have open nodes available
			tAINodeSetIt it = m_setGoalNodes.begin();

			while(it != m_setGoalNodes.end())
			{
				cAINode *pAINode = *it;

				if(mvOpenListCount[pAINode->GetListID()] == 0)
				{
					//////////////
					// Remove it
					m_setGoalNodes.erase(it++);
					mvGoalListCount[pAINode->GetListID()]--;
				}
				else
				{
					++it;
				}
			}
		}

		if(abOpen)
		{
			///////////////
			// Remove open nodes that no longer have goal nodes available
			tAStarNodeSetIt it = m_setOpenList.begin();

			while(it != m_setOpenList.end())
			{
				cAStarNode *pNode = *it;

				if(mvGoalListCount[pNode->mpAINode->GetListID()] == 0)
				{
					//////////////
					// Remove it
					m_setOpenList.erase(it++);
					pNode->mFlags &= ~eAStarNodeFlag_Open;
					mvOpenListCount[pNode->mpAINode->GetListID()]--;
				}
				else
				{
					++it;
				}
			}
		}
	}

	//-----------------------------------------------------------------------
}
