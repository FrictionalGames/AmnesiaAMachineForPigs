/* Copyright (c) <2003-2011> <Julio Jerez, Newton Game Dynamics>
* 
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
* 
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 
* 3. This notice may not be removed or altered from any source distribution.
*/

#include "dgPhysicsStdafx.h"
#include "dgBody.h"
#include "dgWorld.h"
#include "dgMeshEffect.h"
#include "dgCollisionConvexHull.h"


// create a convex hull
dgMeshEffect::dgMeshEffect (dgMemoryAllocator* const allocator, const dgFloat32* const vertexCloud, dgInt32 count, dgInt32 strideInByte, dgFloat32 distTol)
	:dgPolyhedra(allocator)
{
	Init(true);
	if (count >= 4) {
		dgConvexHull3d convexHull (allocator, vertexCloud, strideInByte, count, distTol);
		if (convexHull.GetCount()) {

			dgInt32 vertexCount = convexHull.GetVertexCount();
			dgStack<dgVector> pointsPool (convexHull.GetVertexCount());
			dgVector* const points = &pointsPool[0];
			for (dgInt32 i = 0; i < vertexCount; i ++) {
				points[i] = convexHull.GetVertex(i);
			}
			dgVector uv(dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));
			dgVector normal (dgFloat32 (0.0f), dgFloat32 (1.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));

			dgInt32 triangleCount = convexHull.GetCount();
			dgStack<dgInt32> faceCountPool (triangleCount);
			dgStack<dgInt32> materialsPool (triangleCount);
			dgStack<dgInt32> vertexIndexListPool (triangleCount * 3);
			dgStack<dgInt32> normalIndexListPool (triangleCount * 3);


			memset (&materialsPool[0], 0, triangleCount * sizeof (dgInt32));
			memset (&normalIndexListPool[0], 0, 3 * triangleCount * sizeof (dgInt32));

			dgInt32 index = 0;
			dgInt32* const faceCount = &faceCountPool[0];
			dgInt32* const vertexIndexList = &vertexIndexListPool[0];
			for (dgConvexHull3d::dgListNode* faceNode = convexHull.GetFirst(); faceNode; faceNode = faceNode->GetNext()) {
				dgConvexHull3DFace& face = faceNode->GetInfo();
				faceCount[index] = 3;
				vertexIndexList[index * 3 + 0] = face.m_index[0]; 
				vertexIndexList[index * 3 + 1] = face.m_index[1]; 
				vertexIndexList[index * 3 + 2] = face.m_index[2]; 
				index ++;
			}

			BuildFromVertexListIndexList(triangleCount, faceCount, &materialsPool[0], 
				&points[0].m_x, sizeof (dgVector), vertexIndexList,
				&normal.m_x, sizeof (dgVector), &normalIndexListPool[0],
				&uv.m_x, sizeof (dgVector), &normalIndexListPool[0],
				&uv.m_x, sizeof (dgVector), &normalIndexListPool[0]);
		}
	}
}


#if 0

class Tetrahedralization: public dgDelaunayTetrahedralization
{
	class dgIndexMapPair
	{
		public:
		dgInt32 m_meshIndex;
		dgInt32 m_convexIndex;
	};

	class dgMissingEdges: public dgList<dgPolyhedra::dgTreeNode*> 
	{	
		public:
		dgMissingEdges (dgMemoryAllocator* const allocator)
			:dgList<dgPolyhedra::dgTreeNode*> (allocator)
		{
		}
		~dgMissingEdges()
		{
		}
	};

	class dgEdgeSharedTetras: public dgList<dgListNode*>
	{
		public:
		dgEdgeSharedTetras(const dgEdgeSharedTetras& copy)
			:dgList(copy.GetAllocator())
		{
		}

		dgEdgeSharedTetras(dgMemoryAllocator* const allocator)
			:dgList(allocator)
		{
		}

		~dgEdgeSharedTetras ()
		{
		}
	};

	class dgEdgeMap: public dgTree<dgEdgeSharedTetras, dgUnsigned64>
	{
		public:
		dgEdgeMap(dgMemoryAllocator* const allocator)
			:dgTree<dgEdgeSharedTetras, dgUnsigned64>(allocator)
		{
		}

		~dgEdgeMap()
		{
			while(GetRoot()) {
				dgEdgeSharedTetras& header = GetRoot()->GetInfo();
				header.RemoveAll();
				Remove(GetRoot());
			}
		}
	};

	class dgVertexMap: public dgTree<dgEdgeSharedTetras, dgInt32>
	{
		public:
		dgVertexMap(dgMemoryAllocator* const allocator)
			:dgTree<dgEdgeSharedTetras, dgInt32>(allocator)
		{
		}

		~dgVertexMap()
		{
			while(GetRoot()) {
				dgEdgeSharedTetras& header = GetRoot()->GetInfo();
				header.RemoveAll();
				Remove(GetRoot());
			}
		}
	};


/*
#ifdef _DG_DEBUG
	class dgEdgeFaceKey
	{
	public:
		dgEdgeFaceKey ()
		{}

		dgEdgeFaceKey (dgInt32 i0, dgInt32 i1, dgInt32 i2)
		{
			m_index[0] = i0;
			m_index[1] = i1;
			m_index[2] = i2;
			while ((m_index[0] > m_index[1]) || (m_index[0] > m_index[2])) {
				i0 = m_index[0];
				m_index[0] = m_index[1];
				m_index[1] = m_index[2];
				m_index[2] = i0;
			}
		}

		dgInt32 Compared (const dgEdgeFaceKey& key) const 
		{
			for (dgInt32 i = 0; i < 3; i ++) {
				if (m_index[i] < key.m_index[i]) {
					return -1;
				} else if (m_index[i] > key.m_index[i]) {
					return 1;
				}
			}
			return 0;
		}


		bool operator < (const dgEdgeFaceKey& key) const 
		{
			return (Compared (key) < 0);
		}

		bool operator > (const dgEdgeFaceKey& key) const 
		{
			return (Compared (key) > 0);
		}


		dgInt32 m_index[3];

	};


	class dgFaceKeyMap: public dgTree<dgListNode*, dgEdgeFaceKey>
	{
	public:
		dgFaceKeyMap(dgMemoryAllocator* const allocator)
			:dgTree<dgListNode*, dgEdgeFaceKey>(allocator)
		{

		}
	};
#endif

*/
public:
	Tetrahedralization (dgMeshEffect& mesh)
		:dgDelaunayTetrahedralization (mesh.GetAllocator(), mesh.GetVertexPool(), mesh.GetVertexCount(), sizeof (dgVector), 0.0f),
		m_mesh (&mesh),
		m_edgeMap (mesh.GetAllocator()), 
		m_vertexMap (mesh.GetAllocator()), 
		m_missinEdges(mesh.GetAllocator()), 
		m_indexMap (mesh.GetVertexCount() * 8 + 2048, mesh.GetAllocator())
	{
		if (GetCount()) {

			#if (defined (WIN32) || defined(_WIN32))
			dgUnsigned32 controlWorld = dgControlFP (0xffffffff, 0);
			dgControlFP (_PC_53, _MCW_PC);
			#endif

			// add every edge of each tetrahedral to a edge list
			BuildTetrahedraEdgeListAndVertexList ();

			// make a index map to quickly find vertex mapping form the mesh to the delaunay tetrahedron
			CreateIndexMap ();

			// Insert all missing edge in mesh as a new into the tetrahedral list
			RecoverEdges ();

			// Recover the solid mesh from the delaunay tetrahedron	
			RecoverFaces ();

			#if (defined (WIN32) || defined(_WIN32))
			dgControlFP (controlWorld, _MCW_PC);
			#endif
		}
	}

	~Tetrahedralization()
	{
	}

	dgUnsigned64 GetKey (dgInt32 i0, dgInt32 i1) const
	{
		return (i1 > i0) ?  (dgUnsigned64 (i1) << 32) + i0 : (dgUnsigned64 (i0) << 32) + i1;
	}

	void InsertNewEdgeNode (dgInt32 i0, dgInt32 i1, dgListNode* const node)
	{
		dgUnsigned64 key = GetKey (i1, i0);

		dgEdgeMap::dgTreeNode* edgeNode = m_edgeMap.Find(key);
		if (!edgeNode) {
			dgEdgeSharedTetras tmp (GetAllocator());
			edgeNode = m_edgeMap.Insert(tmp, key);
		}
		dgEdgeSharedTetras& header = edgeNode->GetInfo();

		#ifdef _DG_DEBUG
			for (dgEdgeSharedTetras::dgListNode* ptr = header.GetFirst(); ptr; ptr = ptr->GetNext()) {
				_DG_ASSERTE (ptr->GetInfo() != node);
			}
		#endif

		header.Append(node);
	}

	void RemoveEdgeNode(dgInt32 i0, dgInt32 i1, dgListNode* const node)
	{
		dgUnsigned64 key = GetKey (i0, i1);

		dgEdgeMap::dgTreeNode* const edgeNode = m_edgeMap.Find(key);
		if (edgeNode) {
			dgEdgeSharedTetras& header = edgeNode->GetInfo();
			for (dgEdgeSharedTetras::dgListNode* ptr = header.GetFirst(); ptr; ptr = ptr->GetNext()) {
				dgListNode* const me = ptr->GetInfo();
				if (me == node)  {
					header.Remove(ptr);
					if (!header.GetCount()) {
						m_edgeMap.Remove(edgeNode);
					}

//					dgInt32 index0 = GetVertexIndex(i0);
//					dgInt32 index1 = GetVertexIndex(i1);
//					dgPolyhedra::dgTreeNode* const edgeNode = m_mesh->FindEdgeNode(index0, index1);
//					if(edgeNode) {
//						m_missinEdges.Append(edgeNode);
//					}

					break;
				}
			}
		}
	}


	void InsertNewVertexNode (dgInt32 index, dgListNode* const node)
	{
		dgVertexMap::dgTreeNode* vertexNode = m_vertexMap.Find(index);
		if (!vertexNode) {
			dgEdgeSharedTetras tmp (GetAllocator());
			vertexNode = m_vertexMap.Insert(tmp, index);
		}
		dgEdgeSharedTetras& header = vertexNode->GetInfo();

		#ifdef _DG_DEBUG
			for (dgEdgeSharedTetras::dgListNode* ptr = header.GetFirst(); ptr; ptr = ptr->GetNext()) {
				_DG_ASSERTE (ptr->GetInfo() != node);
			}
		#endif

		header.Append(node);
	}

	void RemoveNewVertexNode (dgInt32 index, dgListNode* const node)
	{
		dgVertexMap::dgTreeNode* vertexNode = m_vertexMap.Find(index);
		_DG_ASSERTE (vertexNode);
		dgEdgeSharedTetras& header = vertexNode->GetInfo();

		for (dgEdgeSharedTetras::dgListNode* ptr = header.GetFirst(); ptr; ptr = ptr->GetNext()) {
			if (ptr->GetInfo() == node) {
				header.Remove(node);
				break;
			}
		}
		_DG_ASSERTE (header.GetCount());

	}
	

	void AddEdgesAndFaces(dgListNode* const node)
	{
		dgConvexHull4dTetraherum* const tetra = &node->GetInfo();
		if (GetTetraVolume (tetra) < 0.0f) {
			const dgConvexHull4dTetraherum::dgTetrahedrumFace& face = tetra->m_faces[0];
			for (dgInt32 i = 0; i < 3; i ++) {
				dgInt32 i0 = face.m_otherVertex;
				dgInt32 i1 = face.m_index[i];
				InsertNewEdgeNode (i0, i1, node);
			}

			dgInt32 i0 = face.m_index[2];
			for (dgInt32 i = 0; i < 3; i ++) {
				dgInt32 i1 = face.m_index[i];
				InsertNewEdgeNode (i0, i1, node);
				InsertNewVertexNode (i0, node);
				i0 = i1;
			}
			InsertNewVertexNode (face.m_otherVertex, node);
		}
	}

	void RemoveEdgesAndFaces (dgListNode* const node)
	{
		dgConvexHull4dTetraherum* const tetra = &node->GetInfo();
		const dgConvexHull4dTetraherum::dgTetrahedrumFace& face = tetra->m_faces[0];
		for (dgInt32 i = 0; i < 3; i ++) {
			dgInt32 i0 = face.m_otherVertex;
			dgInt32 i1 = face.m_index[i];
			RemoveEdgeNode(i0, i1, node);
		}

		dgInt32 i0 = face.m_index[2];
		for (dgInt32 i = 0; i < 3; i ++) {
			dgInt32 i1 = face.m_index[i];
			RemoveEdgeNode(i0, i1, node);
			RemoveNewVertexNode(i0, node);
			i0 = i1;
		}

		RemoveNewVertexNode(face.m_otherVertex, node);
	}


	dgListNode* AddFace (dgInt32 i0, dgInt32 i1, dgInt32 i2, dgInt32 i3)
	{
		dgListNode* const faceNode = dgDelaunayTetrahedralization::AddFace(i0, i1, i2, i3);
		AddEdgesAndFaces (faceNode);
		return faceNode;
	}


	void DeleteFace (dgListNode* const node) 
	{
		RemoveEdgesAndFaces (node);

		dgConvexHull4dTetraherum* const tetra = &node->GetInfo();
		for (dgInt32 i= 0; i < 4; i ++) {
			const dgConvexHull4dTetraherum::dgTetrahedrumFace& face = tetra->m_faces[i];
			dgListNode* const twinNode = face.m_twin;
			if (twinNode) {
				dgConvexHull4dTetraherum* const twinTetra = &twinNode->GetInfo();
				for (dgInt32 i = 0; i < 4; i ++) {
					if (twinTetra->m_faces[i].m_twin == node) {
						twinTetra->m_faces[i].m_twin = NULL;
					}
				}
			}
		}

		dgDelaunayTetrahedralization::DeleteFace(node);
	}



	void BuildTetrahedraEdgeListAndVertexList () 
	{
		for (dgListNode* node = GetFirst(); node; node = node->GetNext()) {
			AddEdgesAndFaces (node);
		}
	}


	static dgInt32 ConvexCompareIndex(const dgIndexMapPair* const  A, const dgIndexMapPair* const B, void* const context)
	{
		if (A->m_meshIndex > B->m_meshIndex) {
			return 1;
		} else if (A->m_meshIndex < B->m_meshIndex) {
			return -1;
		}
		return 0;
	}

	void CreateIndexMap ()
	{
		// make a index map to quickly find vertex mapping form the mesh to the delaunay tetrahedron
		m_indexMap[GetVertexCount()].m_meshIndex = 0;
		dgIndexMapPair* const indexMap = &m_indexMap[0];
		for (dgInt32 i = 0; i < GetVertexCount(); i ++) {
			indexMap[i].m_convexIndex = i;
			indexMap[i].m_meshIndex = GetVertexIndex(i);
		}
		dgSort(indexMap, GetVertexCount(), ConvexCompareIndex);
	}


	bool SanityPointInTetra (dgConvexHull4dTetraherum* const tetra, const dgBigVector& vertex) const 
	{

		for (dgInt32 i = 0; i < 4; i ++) {
			const dgBigVector& p0 = m_points[tetra->m_faces[i].m_index[0]];
			const dgBigVector& p1 = m_points[tetra->m_faces[i].m_index[1]];
			const dgBigVector& p2 = m_points[tetra->m_faces[i].m_index[2]];
			dgBigPlane plane (p0, p1, p2);
			dgFloat64 dist = plane.Evalue(vertex);
			if (dist > dgFloat64 (1.0e-12f)) {
				return false;
			}
		}
		return true;
	}

	dgInt32 ReplaceFaceNodes (dgListNode* const faceNode, dgInt32 faceIndex, const dgBigVector& vertex)
	{
		dgConvexHull4dTetraherum* const tetra = &faceNode->GetInfo();
		dgListNode* const neighborghNode = tetra->m_faces[faceIndex].m_twin;
		_DG_ASSERTE (neighborghNode);

		dgConvexHull4dTetraherum* const neighborghTetra = &neighborghNode->GetInfo();

		dgInt32 vertexIndex = m_count;
		m_points[vertexIndex] = vertex;
		m_points[vertexIndex].m_index = vertexIndex;
		m_count ++;

		_DG_ASSERTE (SanityPointInTetra (tetra, vertex));
		_DG_ASSERTE (SanityPointInTetra (neighborghTetra, vertex));

		dgInt32 mark = IncMark();
		tetra->SetMark(mark);
		neighborghTetra->SetMark(mark);

		dgInt32 deletedCount = 2;
		dgListNode* deletedNodes[2];
		deletedNodes[0] = faceNode;
		deletedNodes[1] = neighborghNode;

		dgInt32 perimeterCount = 0;
		dgListNode* perimeter[16];
		for (dgInt32 i = 0; i < deletedCount; i ++) {
			dgListNode* const deleteTetraNode = deletedNodes[i];

			dgConvexHull4dTetraherum* const deletedTetra = &deleteTetraNode->GetInfo();
			_DG_ASSERTE (deletedTetra->GetMark() == mark);

			for (dgInt32 i = 0; i < 4; i ++) {
				dgListNode* const twinNode = deletedTetra->m_faces[i].m_twin;
				dgConvexHull4dTetraherum* const twinTetra = &twinNode->GetInfo();
				_DG_ASSERTE (twinTetra);

				if (twinTetra->GetMark() != mark) {
					dgInt32 index = 0;
					for (index = 0; index < perimeterCount; index ++) {
						if (perimeter[index] == twinNode) {
							break;
						}
					}
					if (index == perimeterCount) {
						perimeter[perimeterCount] = twinNode;
						perimeterCount ++;
					}
				}
				deletedTetra->m_faces[i].m_twin = NULL;
			}
		}

		dgInt32 coneListCount = 0;
		dgListNode* coneList[32];
		for (dgInt32 i = 0; i < perimeterCount; i ++) {
			dgListNode* const perimeterNode = perimeter[i];
			dgConvexHull4dTetraherum* const perimeterTetra = &perimeterNode->GetInfo();

			for (dgInt32 i = 0; i < 4; i ++) {
				dgConvexHull4dTetraherum::dgTetrahedrumFace* const perimeterFace = &perimeterTetra->m_faces[i];
				if (perimeterFace->m_twin->GetInfo().GetMark() == mark) {

					dgListNode* const newNode = AddFace (vertexIndex, perimeterFace->m_index[0], perimeterFace->m_index[1], perimeterFace->m_index[2]);

					dgConvexHull4dTetraherum* const newTetra = &newNode->GetInfo();
					newTetra->m_faces[2].m_twin = perimeterNode;
					perimeterFace->m_twin = newNode;
					coneList[coneListCount] = newNode;
					coneListCount ++;
				}
			}
		}


		for (int i = 0; i < (coneListCount - 1); i ++) {
			dgListNode* const coneNodeA = coneList[i];
			for (dgInt32 j = i + 1; j < coneListCount; j ++) {
				dgListNode* const coneNodeB = coneList[j];
				LinkSibling (coneNodeA, coneNodeB);
			}
		}

		for (dgInt32 i = 0; i < deletedCount; i ++) {
			//dgListNode* const node = deleteNode->GetInfo();
			dgListNode* const deleteTetraNode = deletedNodes[i];
			DeleteFace (deleteTetraNode); 
		}

		return vertexIndex;
	}



	void RecoverEdges ()
	{
		// split every missing edge at the center and add the two half to the triangulation
		// keep doing it until all edge are present in the triangulation.

		dgInt32 mark = m_mesh->IncLRU();

		// create a list all all the edge that are in the mesh but that do not appear in the delaunay tetrahedron 
		const dgIndexMapPair* const indexMap = &m_indexMap[0];
		dgPolyhedra::Iterator iter (*m_mesh);

		for (iter.Begin(); iter; iter ++) {
			dgEdge* const edge = &iter.GetNode()->GetInfo();
			if (edge->m_mark != mark) {
				edge->m_mark = mark;
				edge->m_twin->m_mark = mark;

				dgInt32 i0 = indexMap[edge->m_incidentVertex].m_convexIndex;
				dgInt32 i1 = indexMap[edge->m_twin->m_incidentVertex].m_convexIndex;
				dgUnsigned64 key = GetKey (i0, i1);
				dgEdgeMap::dgTreeNode* const edgeNode = m_edgeMap.Find(key);
				if (!edgeNode) {
					m_missinEdges.Append(iter.GetNode());
				}
			}
		}

//m_missinEdges.Remove(m_missinEdges.GetLast());

		while (m_missinEdges.GetCount()){
			dgIndexMapPair* const indexMap = &m_indexMap[0];

			dgMissingEdges::dgListNode* missingEdgeNode = m_missinEdges.GetFirst();
			dgEdge* missingEdge = &missingEdgeNode->GetInfo()->GetInfo();

			dgInt32 k0 = missingEdge->m_incidentVertex;
			dgInt32 k1 = missingEdge->m_twin->m_incidentVertex;
			dgInt32 i0 = indexMap[k0].m_convexIndex;
			dgInt32 i1 = indexMap[k1].m_convexIndex;

			m_missinEdges.Remove(missingEdgeNode);
			dgUnsigned64 key = GetKey (i0, i1);
			if (!m_edgeMap.Find(key)) {
				dgVertexMap::dgTreeNode* const vertexNode = m_vertexMap.Find(i0);
				_DG_ASSERTE (vertexNode);
				const dgEdgeSharedTetras& tetraMap = vertexNode->GetInfo();

				const dgBigVector& p0 = GetVertex(i0);
				const dgBigVector& p1 = GetVertex(i1);
				bool edgeFound = false;
				for (dgEdgeSharedTetras::dgListNode* node = tetraMap.GetFirst(); node; node = node->GetNext()) {
					dgListNode* const tetraNode = node->GetInfo();
					dgConvexHull4dTetraherum* const tetra = &tetraNode->GetInfo();
					dgInt32 faceIndex = -1;
					for (dgInt32 i = 0; i < 4; i ++) {
						if (tetra->m_faces[i].m_otherVertex == i0) {
							faceIndex = i;	
						}
					}
					_DG_ASSERTE (faceIndex != -1);

					const dgBigVector& A = GetVertex(tetra->m_faces[faceIndex].m_index[0]);
					const dgBigVector& B = GetVertex(tetra->m_faces[faceIndex].m_index[1]);
					const dgBigVector& C = GetVertex(tetra->m_faces[faceIndex].m_index[2]);
					dgBigVector baricentric (LineTriangleIntersection (p0, p1, A, B, C));
					if (baricentric.m_w == dgFloat64 (0.0f)) {
						_DG_ASSERTE ((baricentric.m_x > dgFloat64 (0.0f)) && (baricentric.m_y > dgFloat64 (0.0f)) && (baricentric.m_z > dgFloat64 (0.0f)));
						dgBigVector point (A.Scale4 (baricentric.m_x) + B.Scale4 (baricentric.m_y) + C.Scale4 (baricentric.m_z));
						dgInt32 index = ReplaceFaceNodes(tetraNode, faceIndex, point);
						

						dgBigVector pp0 (point - p0);
						dgBigVector p1p0 (p1 - p0);
						dgFloat64 spliteParam = (pp0 % p1p0) / (p1p0 % p1p0);
						dgEdge* const newEdge = m_mesh->InsertEdgeVertex (missingEdge, dgFloat32 (spliteParam));

						indexMap[newEdge->m_twin->m_incidentVertex].m_convexIndex = index;

						i0 = indexMap[newEdge->m_next->m_incidentVertex].m_convexIndex;
						i1 = indexMap[newEdge->m_next->m_twin->m_incidentVertex].m_convexIndex;
						key = GetKey (i0, i1);
						if (!m_edgeMap.Find(key)) {
							dgInt32 index0 = GetVertexIndex(i0);
							dgInt32 index1 = GetVertexIndex(i1);
							dgPolyhedra::dgTreeNode* const edgeNode = m_mesh->FindEdgeNode(index0, index1);
							_DG_ASSERTE (edgeNode);
							m_missinEdges.Addtop(edgeNode);
						}


						edgeFound = true;
						break;
					}
				}
				_DG_ASSERTE (edgeFound);
			} 
		}
	}

/*
	void RemoveDegeneratedTetras ()
	{
		dgInt32 mark = m_mesh->IncLRU(); 
		dgPolyhedra::Iterator iter (*m_mesh);
		for (iter.Begin(); iter; iter ++) {
			dgEdge* const faceEdge = &iter.GetNode()->GetInfo();
			dgInt32 count = 0; 
			dgEdge* ptr = faceEdge;
			do {
				count ++;
				ptr->m_mark = mark;
				ptr = ptr->m_next;
			} while (ptr != faceEdge);

			if (count > 3) {
				dgEdge* ptr = faceEdge;
				do {
					dgInt32 k0 = m_indexMap[ptr->m_incidentVertex].m_convexIndex;
					dgInt32 k1 = m_indexMap[ptr->m_next->m_incidentVertex].m_convexIndex;
					dgUnsigned64 key = GetKey (k0, k1);
					dgEdgeMap::dgTreeNode* const edgeNode = m_edgeMap.Find(key);
					if (edgeNode) {
						dgEdgeSharedTetras& header = edgeNode->GetInfo();
						for (dgEdgeSharedTetras::dgListNode* ptr1 = header.GetFirst(); ptr1; ptr1 = ptr1->GetNext()) {
							dgListNode* const tetraNode = ptr1->GetInfo();
							dgConvexHull4dTetraherum* const tetra = &tetraNode->GetInfo();
							const dgConvexHull4dTetraherum::dgTetrahedrumFace& face = tetra->m_faces[0];
							dgInt32 index[4];
							index[0] = GetVertexIndex(face.m_index[0]);
							index[1] = GetVertexIndex(face.m_index[1]);
							index[2] = GetVertexIndex(face.m_index[2]);
							index[3] = GetVertexIndex(face.m_otherVertex);

							dgInt32 duplicates = 0;
							dgEdge* ptr3 = faceEdge;
							do {
								for (dgInt32 i = 0; i < 4; i ++) {
									duplicates += (ptr3->m_incidentVertex == index[i]) ? 1 : 0;
								}
								ptr3 = ptr3->m_next;
							} while (ptr3 != faceEdge);
							if (duplicates > 3) {
								DeleteFace(tetraNode);
								break;
							}
						}
					}

					ptr = ptr->m_next;
				} while (ptr != faceEdge);
			}
		}
	}
*/

	bool MatchFace (dgMeshEffect& mesh, dgEdge* const faceEdge, dgInt32 tetraMark) const
	{
		dgInt32 k0 = m_indexMap[faceEdge->m_incidentVertex].m_convexIndex;
		dgInt32 k1 = m_indexMap[faceEdge->m_next->m_incidentVertex].m_convexIndex;
		dgUnsigned64 key = GetKey (k0, k1);
		dgEdgeMap::dgTreeNode* const edgeNode = m_edgeMap.Find(key);

		_DG_ASSERTE (edgeNode);
		dgEdgeSharedTetras& header = edgeNode->GetInfo();
		for (dgEdgeSharedTetras::dgListNode* ptr = header.GetFirst(); ptr; ptr = ptr->GetNext()) {
			dgListNode* const tetraNode = ptr->GetInfo();
			dgConvexHull4dTetraherum* const tetra = &tetraNode->GetInfo();
			for (dgInt32 i = 0; i < 4; i ++) {
				dgConvexHull4dTetraherum::dgTetrahedrumFace& face = tetra->m_faces[i];
				dgInt32 i0 = face.m_index[0];
				dgInt32 i1 = face.m_index[1];
				dgInt32 i2 = face.m_index[2];

				if (((i0 == k0) && (i1 == k1)) || ((i1 == k0) && (i2 == k1)) || ((i2 == k0) && (i0 == k1))) {
					dgInt32 index[3];

					index[0] = GetVertexIndex (i0);
					index[1] = GetVertexIndex (i1);
					index[2] = GetVertexIndex (i2);
					while (index[0] != faceEdge->m_incidentVertex) {
						dgInt32 tmp = index[0];
						index[0] = index[1];
						index[1] = index[2];
						index[2] = tmp;
					}
					_DG_ASSERTE (index[0] == faceEdge->m_incidentVertex);
					_DG_ASSERTE (index[1] == faceEdge->m_next->m_incidentVertex);

					dgEdge* nextEdge = faceEdge->m_next->m_next;
					do {
						if (nextEdge->m_incidentVertex == index[2]) {
							break;
						}
						nextEdge = nextEdge->m_next;
					} while (nextEdge != faceEdge);

					if (nextEdge != faceEdge) {
						if (nextEdge->m_prev != faceEdge->m_next) {
							dgEdge* const edge = mesh.ConectVertex(faceEdge->m_next, nextEdge);
							_DG_ASSERTE (edge);
							_DG_ASSERTE (edge->m_next);
							_DG_ASSERTE (edge->m_prev);
							_DG_ASSERTE (edge->m_twin->m_next);
							_DG_ASSERTE (edge->m_twin->m_prev);
							_DG_ASSERTE (faceEdge->m_next == edge->m_twin);
						}
						if (nextEdge->m_next != faceEdge) {
#ifdef	_DG_DEBUG
							dgEdge* const edge = mesh.ConectVertex(faceEdge, nextEdge);
							_DG_ASSERTE (edge);
							_DG_ASSERTE (edge->m_next);
							_DG_ASSERTE (edge->m_prev);
							_DG_ASSERTE (edge->m_twin->m_next);
							_DG_ASSERTE (edge->m_twin->m_prev);
							_DG_ASSERTE (faceEdge->m_prev == edge);
#else
							mesh.ConectVertex(faceEdge, nextEdge);
#endif

						}

						if (tetraMark != -1) {
							tetra->SetMark (tetraMark);
							face.m_twin = NULL;
						}
						return true;
					}
				}
			}
		}
		return false;
	}


	void RecoverFace (dgMeshEffect& mesh, dgEdge* const face, dgInt32 faceMark, dgInt32 tetraMark, dgTree<dgEdge*, dgEdge*>& edgeInconflict) const
	{
		dgInt32 count = 0;
		dgInt32 perimeterCount = 0;
		dgEdge* edgeArray[1024];
		dgEdge* perimterEdges[1024 + 1];

		dgEdge* ptr = face;
		do {
			edgeArray[count] = ptr;
			perimterEdges[count] = ptr;
			count ++;
			_DG_ASSERTE (count < sizeof (edgeArray) / sizeof (edgeArray[0]));
			ptr = ptr->m_next;
		} while (ptr != face);
		perimeterCount = count;
		perimterEdges[count] = face;


		while (count) {
			count --;
			dgEdge* const triangleFace = edgeArray[count];
			bool state = MatchFace (mesh, triangleFace, tetraMark);
			if (state) {
				_DG_ASSERTE (triangleFace == triangleFace->m_next->m_next->m_next);
				triangleFace->m_mark = faceMark;
				dgEdge* ptr = triangleFace->m_next; 
				do {
					ptr->m_mark = faceMark;
					for (dgInt32 i = 0; i < count; i ++) {
						if (ptr == edgeArray[i]) {
							edgeArray[i] = edgeArray[count - 1];
							i --;
							count --;
							break;
						}
					}
					ptr = ptr->m_next;
				} while (ptr != triangleFace);
			}
		}

		_DG_ASSERTE (count == 0);
		for (dgInt32 i = 1; i <= perimeterCount; i ++) {
			dgEdge* const last = perimterEdges[i - 1];
			for (dgEdge* edge = perimterEdges[i]->m_prev; edge != last; edge = edge->m_twin->m_prev) {
				if (edge->m_mark != faceMark) {
					dgInt32 index = 0;
					for (index = 0; index < count; index ++) {
						if ((edgeArray[index] == edge) || (edgeArray[index] == edge->m_twin)) {
							break;
						}
					}
					if (index == count) {
						edgeArray[count] = edge;
						count ++;
					}
				}
			}
		}

		if (count) {
			while (count) {
				count --;
				dgEdge* const triangleFace = edgeArray[count];
				bool state = MatchFace (mesh, triangleFace, tetraMark);
				if (state) {
					_DG_ASSERTE (triangleFace == triangleFace->m_next->m_next->m_next);
					triangleFace->m_mark = faceMark;
					dgEdge* ptr = triangleFace->m_next; 
					do {
						ptr->m_mark = faceMark;
						for (dgInt32 i = 0; i < count; i ++) {
							if (ptr == edgeArray[i]) {
								edgeArray[i] = edgeArray[count - 1];
								i --;
								count --;
								break;
							}
						}
						ptr = ptr->m_next;
					} while (ptr != triangleFace);
				}
			}

			_DG_ASSERTE (count == 0);
			for (dgInt32 i = 0; i < perimeterCount; i ++) {
				dgEdge* const edge = perimterEdges[i];
				if (edge->m_mark != faceMark) {
					dgEdge* const borderEdge = m_mesh->FindEdge(edge->m_incidentVertex, edge->m_twin->m_incidentVertex);
					_DG_ASSERTE (borderEdge);
					if (!(edgeInconflict.Find(borderEdge) || edgeInconflict.Find(borderEdge->m_twin))) {
						edgeInconflict.Insert(borderEdge, borderEdge);
					}
				}
				edge->m_mark = faceMark;
			}

			for (dgInt32 i = 1; i <= perimeterCount; i ++) {
				const dgEdge* const last = perimterEdges[i - 1];
				for (dgEdge* edge = perimterEdges[i]->m_prev; edge != last; edge = edge->m_twin->m_prev) {
					if (edge->m_mark != faceMark) {
						edge->m_mark = faceMark;
						edge->m_twin->m_mark = faceMark;

						dgEdge* begin = NULL;
						for (dgEdge* ptr = edge; !begin; ptr = ptr->m_next->m_twin) {
							begin = m_mesh->FindEdge(ptr->m_next->m_incidentVertex, ptr->m_next->m_twin->m_incidentVertex);
						}
						_DG_ASSERTE (begin);

						dgEdge* end = NULL;
						for (dgEdge* ptr = edge->m_twin; !end; ptr = ptr->m_next->m_twin) {
							end = m_mesh->FindEdge(ptr->m_next->m_incidentVertex, ptr->m_next->m_twin->m_incidentVertex);
						}
						_DG_ASSERTE (end);
						dgEdge* const newEdge = m_mesh->ConectVertex(end, begin);
						_DG_ASSERTE (!edgeInconflict.Find(newEdge));
						edgeInconflict.Insert(newEdge, newEdge);
					}
				}
			}
		}
	}

	void RecoverFaces ()
	{
		// recover all sub faces into a temporary mesh		
		bool allFaceFound = true;

//		dgIndexMapPair* const indexMap = &m_indexMap[0];
		do {
			dgMeshEffect tmpMesh (*m_mesh);
			dgInt32 mark = m_mesh->IncLRU(); 

			dgTree<dgEdge*, dgEdge*> edgeInconflict(GetAllocator());
			dgMeshEffect::Iterator iter(tmpMesh);
			for (iter.Begin(); iter; iter ++) {
				dgEdge* const face = &iter.GetNode()->GetInfo();
				if (face->m_mark != mark){
					RecoverFace (tmpMesh, face, mark, -1, edgeInconflict);
				}
			}

			// if there are missing sub faces then we must recover those by insertion point on the sub edges of the missing faces
			allFaceFound = true;
			if (edgeInconflict.GetCount()) {
				_DG_ASSERTE (0);
/*
				allFaceFound = false;

				dgTree<dgEdge*, dgEdge*>::Iterator iter (edgeInconflict);
				for (iter.Begin(); iter; iter ++) {
					dgEdge* const missingEdge = iter.GetNode()->GetInfo();

					dgInt32 k0 = missingEdge->m_incidentVertex;
					dgInt32 k1 = missingEdge->m_twin->m_incidentVertex;
					dgInt32 i0 = indexMap[k0].m_convexIndex;
					dgInt32 i1 = indexMap[k1].m_convexIndex;

					const dgBigVector& p0 = GetVertex(i0);
					const dgBigVector& p1 = GetVertex(i1);
					dgFloat32 spliteParam = dgFloat32 (0.5f);

					dgEdge* const newEdge = m_mesh->InsertEdgeVertex (missingEdge, spliteParam);
					dgBigVector p (p1.Add4(p0).Scale4 (spliteParam));
					dgInt32 index = AddVertex(p);
					_DG_ASSERTE (index != -1);
					indexMap[newEdge->m_twin->m_incidentVertex].m_convexIndex = index;
				}
				RecoverEdges ();
*/
			}
#ifdef _DG_DEBUG
			if (allFaceFound) {
//				_DG_ASSERTE (0);
/*
				dgFaceKeyMap faceMap (GetAllocator());
				for (dgListNode* node = GetFirst(); node; node = node->GetNext()) {
					dgConvexHull4dTetraherum* const tetra = &node->GetInfo();
					for (dgInt32 i = 0; i < 4; i ++) {
						dgConvexHull4dTetraherum::dgTetrahedrumFace& face = tetra->m_faces[i];
						dgEdgeFaceKey key (face.m_index[0], face.m_index[1], face.m_index[2]);
						_DG_ASSERTE (!faceMap.Find(key));
						faceMap.Insert (node, key);
					}
				}

				dgInt32 mark = tmpMesh.IncLRU(); 
				for (iter.Begin(); iter; iter ++) {
					dgEdge* const face = &iter.GetNode()->GetInfo();
					if (face->m_mark != mark){
						dgEdge* ptr = face;
						do {
							ptr->m_mark = mark;
							ptr = ptr->m_next;
						} while (ptr != face);
						dgEdgeFaceKey key (face->m_incidentVertex, face->m_next->m_incidentVertex, face->m_next->m_next->m_incidentVertex);
						_DG_ASSERTE (faceMap.Find(key));
					}
				}
*/
			}
#endif


		} while (!allFaceFound);


		// all faces are present in the mesh now we can recover the mesh
		// remove all tetrahedral with negative volume
		RemoveUpperHull ();

		// remove any tetrahedron that by round off error might have more that three point on on a face
//		RemoveDegeneratedTetras ();


		//dgInt32 tetraMark = 1;
		dgInt32 tetraMark = IncMark();
		dgInt32 mark = m_mesh->IncLRU(); 

		dgTree<dgEdge*, dgEdge*> edgeInconflict(GetAllocator());
		dgMeshEffect::Iterator iter(*m_mesh);
		for (iter.Begin(); iter; iter ++) {
			dgEdge* const face = &iter.GetNode()->GetInfo();
			if (face->m_mark != mark){
				dgEdge* ptr = face;
				//dgTrace (("%d:", ptr->m_incidentFace))
				do {
					ptr->m_mark = mark;
					//dgTrace ((" %d", ptr->m_incidentVertex))
					ptr = ptr->m_next;
				} while (ptr != face);
				//dgTrace (("\n"));
				RecoverFace (*m_mesh, face, mark, tetraMark, edgeInconflict);
			}
		}

		// color codes all tetrahedron inside the mesh volume
		for (dgListNode* node = GetFirst(); node; node = node->GetNext()) {
			dgConvexHull4dTetraherum* const tetra = &node->GetInfo();
			if (tetra->GetMark() == tetraMark) {
				dgInt32 stack = 0;
				dgConvexHull4dTetraherum* stackPool[1024 * 4];
				for (dgInt32 i = 0; i < 4; i ++) {
					dgConvexHull4dTetraherum::dgTetrahedrumFace& face = tetra->m_faces[i];
					if (face.m_twin && (face.m_twin->GetInfo().GetMark() != tetraMark)) {
						stackPool[stack] = &face.m_twin->GetInfo();
						stack ++;
					}
				}

				while (stack) {
					stack --;
					dgConvexHull4dTetraherum* const skinTetra = stackPool[stack];
					skinTetra->SetMark (tetraMark);
					for (dgInt32 i = 0; i < 4; i ++) {
						dgConvexHull4dTetraherum::dgTetrahedrumFace& face = skinTetra->m_faces[i];
						if (face.m_twin && (face.m_twin->GetInfo().GetMark() != tetraMark)) {
							stackPool[stack] = &face.m_twin->GetInfo();
							stack ++;
							_DG_ASSERTE (stack < sizeof (stackPool) / sizeof (stackPool[0]));
						}
					}
				}
			}
		}

		// remove all tetrahedron outsize the mesh volume (those who are not painted)
		dgListNode* nextNode = NULL;
		for (dgListNode* node = GetFirst(); node; node = nextNode) {
			nextNode = node->GetNext();
			dgConvexHull4dTetraherum* const tetra = &node->GetInfo();
			if (tetra->GetMark() != tetraMark) {
				DeleteFace (node);
			}
		}
	}


	dgMeshEffect* m_mesh;
	dgEdgeMap m_edgeMap;
	dgVertexMap m_vertexMap;
	dgMissingEdges m_missinEdges;
	dgArray<dgIndexMapPair> m_indexMap;
};


#else

class Tetrahedralization: public dgDelaunayTetrahedralization
{
	class dgIndexMapPair
	{
		public:
		dgInt32 m_meshIndex;
		dgInt32 m_convexIndex;
	};

	class dgMissingEdges: public dgList<dgPolyhedra::dgTreeNode*> 
	{	
		public:
		dgMissingEdges (dgMemoryAllocator* const allocator)
			:dgList<dgPolyhedra::dgTreeNode*> (allocator)
		{
		}
		~dgMissingEdges()
		{
		}
	};

	class dgEdgeSharedTetras: public dgList<dgDelaunayTetrahedralization::dgListNode*>
	{
		public:
		dgEdgeSharedTetras(const dgEdgeSharedTetras& copy)
			:dgList<dgDelaunayTetrahedralization::dgListNode*>(copy.GetAllocator())
		{
		}

		dgEdgeSharedTetras(dgMemoryAllocator* const allocator)
			:dgList<dgDelaunayTetrahedralization::dgListNode*>(allocator)
		{
		}

		~dgEdgeSharedTetras ()
		{
		}
	};

	class dgEdgeMap: public dgTree<dgEdgeSharedTetras, dgUnsigned64>
	{
		public:
		dgEdgeMap(dgMemoryAllocator* const allocator)
			:dgTree<dgEdgeSharedTetras, dgUnsigned64>(allocator)
		{
		}

		~dgEdgeMap()
		{
			while(GetRoot()) {
				dgEdgeSharedTetras& header = GetRoot()->GetInfo();
				header.RemoveAll();
				Remove(GetRoot());
			}
		}
	};

#ifdef _DG_DEBUG
	class dgEdgeFaceKey
	{
		public:
		dgEdgeFaceKey ()
		{}

		dgEdgeFaceKey (dgInt32 i0, dgInt32 i1, dgInt32 i2)
		{
			m_index[0] = i0;
			m_index[1] = i1;
			m_index[2] = i2;
			while ((m_index[0] > m_index[1]) || (m_index[0] > m_index[2])) {
				i0 = m_index[0];
				m_index[0] = m_index[1];
				m_index[1] = m_index[2];
				m_index[2] = i0;
			}
		}

		dgInt32 Compared (const dgEdgeFaceKey& key) const 
		{
			for (dgInt32 i = 0; i < 3; i ++) {
				if (m_index[i] < key.m_index[i]) {
					return -1;
				} else if (m_index[i] > key.m_index[i]) {
					return 1;
				}
			}
			return 0;
		}


		bool operator < (const dgEdgeFaceKey& key) const 
		{
			return (Compared (key) < 0);
		}

		bool operator > (const dgEdgeFaceKey& key) const 
		{
			return (Compared (key) > 0);
		}


		dgInt32 m_index[3];

	};


	class dgFaceKeyMap: public dgTree<dgListNode*, dgEdgeFaceKey>
	{
		public:
		dgFaceKeyMap(dgMemoryAllocator* const allocator)
			:dgTree<dgListNode*, dgEdgeFaceKey>(allocator)
		{

		}
	};
#endif

	public:
	Tetrahedralization (dgMeshEffect& mesh)
		:dgDelaunayTetrahedralization (mesh.GetAllocator(), mesh.GetVertexPool(), mesh.GetVertexCount(), sizeof (dgVector), 0.0f),
		m_mesh (&mesh),
		m_edgeMap (mesh.GetAllocator()), 
		m_missinEdges(mesh.GetAllocator()), 
		m_indexMap (mesh.GetVertexCount() * 8 + 2048, mesh.GetAllocator())
	{
		if (GetCount()) {
			// add every edge of each tetrahedral to a edge list
			BuildTetrahedraEdgeList ();

			// make a index map to quickly find vertex mapping form the mesh to the delaunay tetrahedron
			CreateIndexMap ();

			// Insert all missing edge in mesh as a new into the tetrahedral list
			RecoverEdges ();

			// Recover the solid mesh from the delaunay tetrahedron	
			RecoverFaces ();
		}
	}


	~Tetrahedralization()
	{
	}

	void BuildTetrahedraEdgeList () 
	{
		for (dgListNode* node = GetFirst(); node; node = node->GetNext()) {
			AddEdges (node); 
		}
	}

	void RecoverEdges ()
	{
		// split every missing edge at the center and add the two half to the triangulation
		// keep doing it until all edge are present in the triangulation.

		dgInt32 mark = m_mesh->IncLRU();

		// create a list all all the edge that are in the mesh but that do not appear in the delaunay tetrahedron 
		const dgIndexMapPair* const indexMap = &m_indexMap[0];
		dgPolyhedra::Iterator iter (*m_mesh);
		for (iter.Begin(); iter; iter ++) {
			dgEdge* const edge = &iter.GetNode()->GetInfo();
			if (edge->m_mark != mark) {
				edge->m_mark = mark;
				edge->m_twin->m_mark = mark;

				dgInt32 i0 = indexMap[edge->m_incidentVertex].m_convexIndex;
				dgInt32 i1 = indexMap[edge->m_twin->m_incidentVertex].m_convexIndex;
				dgUnsigned64 key = GetKey (i0, i1);
				dgEdgeMap::dgTreeNode* const edgeNode = m_edgeMap.Find(key);
				if (!edgeNode) {
					m_missinEdges.Append(iter.GetNode());
				}
			}
		}

		while (m_missinEdges.GetCount()){
			dgIndexMapPair* const indexMap = &m_indexMap[0];

			dgMissingEdges::dgListNode* missingEdgeNode = m_missinEdges.GetFirst();
			dgEdge* missingEdge = &missingEdgeNode->GetInfo()->GetInfo();

			dgInt32 k0 = missingEdge->m_incidentVertex;
			dgInt32 k1 = missingEdge->m_twin->m_incidentVertex;
			dgInt32 i0 = indexMap[k0].m_convexIndex;
			dgInt32 i1 = indexMap[k1].m_convexIndex;

			m_missinEdges.Remove(missingEdgeNode);
			dgUnsigned64 key = GetKey (i0, i1);
			if (!m_edgeMap.Find(key)) {
				const dgBigVector& p0 = GetVertex(i0);
				const dgBigVector& p1 = GetVertex(i1);
				dgFloat64 spliteParam = dgFloat64 (0.5f);
				dgEdge* const newEdge = m_mesh->InsertEdgeVertex (missingEdge, dgFloat32 (spliteParam));
				newEdge->m_mark = mark;
				newEdge->m_next->m_mark = mark;
				newEdge->m_twin->m_mark = mark;
				newEdge->m_twin->m_prev->m_mark = mark;

				dgBigVector p (p1.Add4(p0).Scale4 (spliteParam));
				dgInt32 index = AddVertex(p);
				_DG_ASSERTE (index != -1);
				indexMap[newEdge->m_twin->m_incidentVertex].m_convexIndex = index;

				i0 = indexMap[newEdge->m_incidentVertex].m_convexIndex;
				i1 = indexMap[newEdge->m_twin->m_incidentVertex].m_convexIndex;
				key = GetKey (i0, i1);
				if (!m_edgeMap.Find(key)) {
					dgInt32 index0 = GetVertexIndex(i0);
					dgInt32 index1 = GetVertexIndex(i1);
					dgPolyhedra::dgTreeNode* const edgeNode = m_mesh->FindEdgeNode(index0, index1);
					_DG_ASSERTE (edgeNode);
					m_missinEdges.Append(edgeNode);
				}

				i0 = indexMap[newEdge->m_next->m_incidentVertex].m_convexIndex;
				i1 = indexMap[newEdge->m_next->m_twin->m_incidentVertex].m_convexIndex;
				key = GetKey (i0, i1);
				if (!m_edgeMap.Find(key)) {
					dgInt32 index0 = GetVertexIndex(i0);
					dgInt32 index1 = GetVertexIndex(i1);
					dgPolyhedra::dgTreeNode* const edgeNode = m_mesh->FindEdgeNode(index0, index1);
					_DG_ASSERTE (edgeNode);
					m_missinEdges.Append(edgeNode);
				}
			} 
		}
	}

	void RemoveDegeneratedTetras ()
	{
		dgInt32 mark = m_mesh->IncLRU(); 
		dgPolyhedra::Iterator iter (*m_mesh);
		for (iter.Begin(); iter; iter ++) {
			dgEdge* const faceEdge = &iter.GetNode()->GetInfo();
			dgInt32 count = 0; 
			dgEdge* ptr = faceEdge;
			do {
				count ++;
				ptr->m_mark = mark;
				ptr = ptr->m_next;
			} while (ptr != faceEdge);

			if (count > 3) {
				dgEdge* ptr = faceEdge;
				do {
					dgInt32 k0 = m_indexMap[ptr->m_incidentVertex].m_convexIndex;
					dgInt32 k1 = m_indexMap[ptr->m_next->m_incidentVertex].m_convexIndex;
					dgUnsigned64 key = GetKey (k0, k1);
					dgEdgeMap::dgTreeNode* const edgeNode = m_edgeMap.Find(key);
					if (edgeNode) {
						dgEdgeSharedTetras& header = edgeNode->GetInfo();
						for (dgEdgeSharedTetras::dgListNode* ptr1 = header.GetFirst(); ptr1; ptr1 = ptr1->GetNext()) {
							dgListNode* const tetraNode = ptr1->GetInfo();
							dgConvexHull4dTetraherum* const tetra = &tetraNode->GetInfo();
							const dgConvexHull4dTetraherum::dgTetrahedrumFace& face = tetra->m_faces[0];
							dgInt32 index[4];
							index[0] = GetVertexIndex(face.m_index[0]);
							index[1] = GetVertexIndex(face.m_index[1]);
							index[2] = GetVertexIndex(face.m_index[2]);
							index[3] = GetVertexIndex(face.m_otherVertex);

							dgInt32 duplicates = 0;
							dgEdge* ptr3 = faceEdge;
							do {
								for (dgInt32 i = 0; i < 4; i ++) {
									duplicates += (ptr3->m_incidentVertex == index[i]) ? 1 : 0;
								}
								ptr3 = ptr3->m_next;
							} while (ptr3 != faceEdge);
							if (duplicates > 3) {
								DeleteFace(tetraNode);
								break;
							}
						}
					}

					ptr = ptr->m_next;
				} while (ptr != faceEdge);
			}
		}
	}


	bool MatchFace (dgMeshEffect& mesh, dgEdge* const faceEdge, dgInt32 tetraMark) const
	{
		dgInt32 k0 = m_indexMap[faceEdge->m_incidentVertex].m_convexIndex;
		dgInt32 k1 = m_indexMap[faceEdge->m_next->m_incidentVertex].m_convexIndex;
		dgUnsigned64 key = GetKey (k0, k1);
		dgEdgeMap::dgTreeNode* const edgeNode = m_edgeMap.Find(key);

		_DG_ASSERTE (edgeNode);
		dgEdgeSharedTetras& header = edgeNode->GetInfo();
		for (dgEdgeSharedTetras::dgListNode* ptr = header.GetFirst(); ptr; ptr = ptr->GetNext()) {
			dgListNode* const tetraNode = ptr->GetInfo();
			dgConvexHull4dTetraherum* const tetra = &tetraNode->GetInfo();
			for (dgInt32 i = 0; i < 4; i ++) {
				dgConvexHull4dTetraherum::dgTetrahedrumFace& face = tetra->m_faces[i];
				dgInt32 i0 = face.m_index[0];
				dgInt32 i1 = face.m_index[1];
				dgInt32 i2 = face.m_index[2];

				if (((i0 == k0) && (i1 == k1)) || ((i1 == k0) && (i2 == k1)) || ((i2 == k0) && (i0 == k1))) {
					dgInt32 index[3];

					index[0] = GetVertexIndex (i0);
					index[1] = GetVertexIndex (i1);
					index[2] = GetVertexIndex (i2);
					while (index[0] != faceEdge->m_incidentVertex) {
						dgInt32 tmp = index[0];
						index[0] = index[1];
						index[1] = index[2];
						index[2] = tmp;
					}
					_DG_ASSERTE (index[0] == faceEdge->m_incidentVertex);
					_DG_ASSERTE (index[1] == faceEdge->m_next->m_incidentVertex);

					dgEdge* nextEdge = faceEdge->m_next->m_next;
					do {
						if (nextEdge->m_incidentVertex == index[2]) {
							break;
						}
						nextEdge = nextEdge->m_next;
					} while (nextEdge != faceEdge);

					if (nextEdge != faceEdge) {
						#ifdef _MSC_VER
							#ifdef _DG_DEBUG
								if (nextEdge->m_prev != faceEdge->m_next) {
									dgEdge* const edge = mesh.ConectVertex(faceEdge->m_next, nextEdge);
									_DG_ASSERTE (edge);
									_DG_ASSERTE (edge->m_next);
									_DG_ASSERTE (edge->m_prev);
									_DG_ASSERTE (edge->m_twin->m_next);
									_DG_ASSERTE (edge->m_twin->m_prev);
									_DG_ASSERTE (faceEdge->m_next == edge->m_twin);
								}
							#endif
						#endif
						if (nextEdge->m_next != faceEdge) {
							#ifdef	_DG_DEBUG
								dgEdge* const edge = mesh.ConectVertex(faceEdge, nextEdge);
								_DG_ASSERTE (edge);
								_DG_ASSERTE (edge->m_next);
								_DG_ASSERTE (edge->m_prev);
								_DG_ASSERTE (edge->m_twin->m_next);
								_DG_ASSERTE (edge->m_twin->m_prev);
								_DG_ASSERTE (faceEdge->m_prev == edge);
							#else
								mesh.ConectVertex(faceEdge, nextEdge);
							#endif

						}

						if (tetraMark != -1) {
							tetra->SetMark (tetraMark);
							face.m_twin = NULL;
						}
						return true;
					}
				}
			}
		}
		return false;
	}

	void RecoverFace (dgMeshEffect& mesh, dgEdge* const face, dgInt32 faceMark, dgInt32 tetraMark, dgTree<dgEdge*, dgEdge*>& edgeInconflict) const
	{
		dgInt32 count = 0;
		dgInt32 perimeterCount = 0;
		dgEdge* edgeArray[1024];
		dgEdge* perimterEdges[1024 + 1];

		dgEdge* ptr = face;
		do {
			edgeArray[count] = ptr;
			perimterEdges[count] = ptr;
			count ++;
			_DG_ASSERTE (count < sizeof (edgeArray) / sizeof (edgeArray[0]));
			ptr = ptr->m_next;
		} while (ptr != face);
		perimeterCount = count;
		perimterEdges[count] = face;


		while (count) {
			count --;
			dgEdge* const triangleFace = edgeArray[count];
			bool state = MatchFace (mesh, triangleFace, tetraMark);
			if (state) {
				_DG_ASSERTE (triangleFace == triangleFace->m_next->m_next->m_next);
				triangleFace->m_mark = faceMark;
				dgEdge* ptr = triangleFace->m_next; 
				do {
					ptr->m_mark = faceMark;
					for (dgInt32 i = 0; i < count; i ++) {
						if (ptr == edgeArray[i]) {
							edgeArray[i] = edgeArray[count - 1];
							i --;
							count --;
							break;
						}
					}
					ptr = ptr->m_next;
				} while (ptr != triangleFace);
			}
		}
		
		_DG_ASSERTE (count == 0);
		for (dgInt32 i = 1; i <= perimeterCount; i ++) {
			dgEdge* const last = perimterEdges[i - 1];
			for (dgEdge* edge = perimterEdges[i]->m_prev; edge != last; edge = edge->m_twin->m_prev) {
				if (edge->m_mark != faceMark) {
					dgInt32 index = 0;
					for (index = 0; index < count; index ++) {
						if ((edgeArray[index] == edge) || (edgeArray[index] == edge->m_twin)) {
							break;
						}
					}
					if (index == count) {
						edgeArray[count] = edge;
						count ++;
					}
				}
			}
		}

		if (count) {
			while (count) {
				count --;
				dgEdge* const triangleFace = edgeArray[count];
				bool state = MatchFace (mesh, triangleFace, tetraMark);
				if (state) {
					_DG_ASSERTE (triangleFace == triangleFace->m_next->m_next->m_next);
					triangleFace->m_mark = faceMark;
					dgEdge* ptr = triangleFace->m_next; 
					do {
						ptr->m_mark = faceMark;
						for (dgInt32 i = 0; i < count; i ++) {
							if (ptr == edgeArray[i]) {
								edgeArray[i] = edgeArray[count - 1];
								i --;
								count --;
								break;
							}
						}
						ptr = ptr->m_next;
					} while (ptr != triangleFace);
				}
			}

			_DG_ASSERTE (count == 0);
			for (dgInt32 i = 0; i < perimeterCount; i ++) {
				dgEdge* const edge = perimterEdges[i];
				if (edge->m_mark != faceMark) {
					dgEdge* const borderEdge = m_mesh->FindEdge(edge->m_incidentVertex, edge->m_twin->m_incidentVertex);
					_DG_ASSERTE (borderEdge);
					if (!(edgeInconflict.Find(borderEdge) || edgeInconflict.Find(borderEdge->m_twin))) {
						edgeInconflict.Insert(borderEdge, borderEdge);
					}
				}
				edge->m_mark = faceMark;
			}
			
			for (dgInt32 i = 1; i <= perimeterCount; i ++) {
				const dgEdge* const last = perimterEdges[i - 1];
				for (dgEdge* edge = perimterEdges[i]->m_prev; edge != last; edge = edge->m_twin->m_prev) {
					if (edge->m_mark != faceMark) {
						edge->m_mark = faceMark;
						edge->m_twin->m_mark = faceMark;

						dgEdge* begin = NULL;
						for (dgEdge* ptr = edge; !begin; ptr = ptr->m_next->m_twin) {
							begin = m_mesh->FindEdge(ptr->m_next->m_incidentVertex, ptr->m_next->m_twin->m_incidentVertex);
						}
						_DG_ASSERTE (begin);

						dgEdge* end = NULL;
						for (dgEdge* ptr = edge->m_twin; !end; ptr = ptr->m_next->m_twin) {
							end = m_mesh->FindEdge(ptr->m_next->m_incidentVertex, ptr->m_next->m_twin->m_incidentVertex);
						}
						_DG_ASSERTE (end);
						dgEdge* const newEdge = m_mesh->ConectVertex(end, begin);
						_DG_ASSERTE (!edgeInconflict.Find(newEdge));
						edgeInconflict.Insert(newEdge, newEdge);
					}
				}
			}
		}
	}


	static dgInt32 ConvexCompareIndex(const dgIndexMapPair* const  A, const dgIndexMapPair* const B, void* const context)
	{
		if (A->m_meshIndex > B->m_meshIndex) {
			return 1;
		} else if (A->m_meshIndex < B->m_meshIndex) {
			return -1;
		}
		return 0;
	}

	void CreateIndexMap ()
	{
		// make a index map to quickly find vertex mapping form the mesh to the delaunay tetrahedron
		m_indexMap[GetVertexCount()].m_meshIndex = 0;
		dgIndexMapPair* const indexMap = &m_indexMap[0];
		for (dgInt32 i = 0; i < GetVertexCount(); i ++) {
			indexMap[i].m_convexIndex = i;
			indexMap[i].m_meshIndex = GetVertexIndex(i);
		}
		dgSort(indexMap, GetVertexCount(), ConvexCompareIndex);
	}

	dgUnsigned64 GetKey (dgInt32 i0, dgInt32 i1) const
	{
		return (i1 > i0) ?  (dgUnsigned64 (i1) << 32) + i0 : (dgUnsigned64 (i0) << 32) + i1;
	}

	void InsertNewNode (dgInt32 i0, dgInt32 i1, dgListNode* const node)
	{
		dgUnsigned64 key = GetKey (i1, i0);

		dgEdgeMap::dgTreeNode* edgeNode = m_edgeMap.Find(key);
		if (!edgeNode) {
			dgEdgeSharedTetras tmp (GetAllocator());
			edgeNode = m_edgeMap.Insert(tmp, key);
		}
		dgEdgeSharedTetras& header = edgeNode->GetInfo();

		#ifdef _DG_DEBUG
		for (dgEdgeSharedTetras::dgListNode* ptr = header.GetFirst(); ptr; ptr = ptr->GetNext()) {
			_DG_ASSERTE (ptr->GetInfo() != node);
		}
		#endif
		header.Append(node);
	}

	void AddEdges (dgListNode* const node) 
	{
		dgConvexHull4dTetraherum* const tetra = &node->GetInfo();
		dgFloat64 volume = GetTetraVolume (tetra);
		if (volume < dgFloat64 (0.0f)) {
			const dgConvexHull4dTetraherum::dgTetrahedrumFace& face = tetra->m_faces[0];
			for (dgInt32 i = 0; i < 3; i ++) {
				dgInt32 i0 = face.m_otherVertex;
				dgInt32 i1 = face.m_index[i];
				InsertNewNode (i0, i1, node);
			}

			dgInt32 i0 = face.m_index[2];
			for (dgInt32 i = 0; i < 3; i ++) {
				dgInt32 i1 = face.m_index[i];
				InsertNewNode (i0, i1, node);
				i0 = i1;
			}
		}
	}

	void RemoveNode(dgInt32 i0, dgInt32 i1, dgListNode* const node)
	{
		dgUnsigned64 key = GetKey (i0, i1);

		dgEdgeMap::dgTreeNode* const edgeNode = m_edgeMap.Find(key);
		if (edgeNode) {
			dgEdgeSharedTetras& header = edgeNode->GetInfo();
			for (dgEdgeSharedTetras::dgListNode* ptr = header.GetFirst(); ptr; ptr = ptr->GetNext()) {
				dgListNode* const me = ptr->GetInfo();
				if (me == node)  {
					header.Remove(ptr);
					if (!header.GetCount()) {
						m_edgeMap.Remove(edgeNode);
					}

					dgInt32 index0 = GetVertexIndex(i0);
					dgInt32 index1 = GetVertexIndex(i1);
					dgPolyhedra::dgTreeNode* const edgeNode = m_mesh->FindEdgeNode(index0, index1);
					if(edgeNode) {
						m_missinEdges.Append(edgeNode);
					}
					break;
				}
			}
		}
	}

	void RemoveEdges (dgListNode* const node)
	{
		dgConvexHull4dTetraherum* const tetra = &node->GetInfo();
		const dgConvexHull4dTetraherum::dgTetrahedrumFace& face = tetra->m_faces[0];
		for (dgInt32 i = 0; i < 3; i ++) {
			dgInt32 i0 = face.m_otherVertex;
			dgInt32 i1 = face.m_index[i];
			RemoveNode(i0, i1, node);
		}

		dgInt32 i0 = face.m_index[2];
		for (dgInt32 i = 0; i < 3; i ++) {
			dgInt32 i1 = face.m_index[i];
			RemoveNode(i0, i1, node);
			i0 = i1;
		}
	}

	dgListNode* AddFace (dgInt32 i0, dgInt32 i1, dgInt32 i2, dgInt32 i3)
	{
		dgListNode* const face = dgDelaunayTetrahedralization::AddFace(i0, i1, i2, i3);
		AddEdges(face);
		return face;
	}

	void DeleteFace (dgListNode* const node) 
	{
		RemoveEdges (node);

		dgConvexHull4dTetraherum* const tetra = &node->GetInfo();
		for (dgInt32 i= 0; i < 4; i ++) {
			const dgConvexHull4dTetraherum::dgTetrahedrumFace& face = tetra->m_faces[i];
			dgListNode* const twinNode = face.m_twin;
			if (twinNode) {
				dgConvexHull4dTetraherum* const twinTetra = &twinNode->GetInfo();
				for (dgInt32 i = 0; i < 4; i ++) {
					if (twinTetra->m_faces[i].m_twin == node) {
						twinTetra->m_faces[i].m_twin = NULL;
					}
				}
			}
		}
		
		dgDelaunayTetrahedralization::DeleteFace(node);
	}




	void RecoverFaces ()
	{
		// recover all sub faces into a temporary mesh		
		bool allFaceFound = true;

		dgIndexMapPair* const indexMap = &m_indexMap[0];
		do {
			dgMeshEffect tmpMesh (*m_mesh);
			dgInt32 mark = m_mesh->IncLRU(); 

			dgTree<dgEdge*, dgEdge*> edgeInconflict(GetAllocator());
			dgMeshEffect::Iterator iter(tmpMesh);
			for (iter.Begin(); iter; iter ++) {
				dgEdge* const face = &iter.GetNode()->GetInfo();
				if (face->m_mark != mark){
					RecoverFace (tmpMesh, face, mark, -1, edgeInconflict);
				}
			}

			// if there are missing sub faces then we must recover those by insertion point on the sub edges of the missing faces
			allFaceFound = true;
			if (edgeInconflict.GetCount()) {
				allFaceFound = false;

				dgTree<dgEdge*, dgEdge*>::Iterator iter (edgeInconflict);
				for (iter.Begin(); iter; iter ++) {
					dgEdge* const missingEdge = iter.GetNode()->GetInfo();

					dgInt32 k0 = missingEdge->m_incidentVertex;
					dgInt32 k1 = missingEdge->m_twin->m_incidentVertex;
					dgInt32 i0 = indexMap[k0].m_convexIndex;
					dgInt32 i1 = indexMap[k1].m_convexIndex;

					const dgBigVector& p0 = GetVertex(i0);
					const dgBigVector& p1 = GetVertex(i1);
					dgFloat32 spliteParam = dgFloat32 (0.5f);

					dgEdge* const newEdge = m_mesh->InsertEdgeVertex (missingEdge, spliteParam);
					dgBigVector p (p1.Add4(p0).Scale4 (spliteParam));
					dgInt32 index = AddVertex(p);
					_DG_ASSERTE (index != -1);
					indexMap[newEdge->m_twin->m_incidentVertex].m_convexIndex = index;
				}
				RecoverEdges ();
			}

			#ifdef _DG_DEBUG
			if (allFaceFound) {
				dgFaceKeyMap faceMap (GetAllocator());
				for (dgListNode* node = GetFirst(); node; node = node->GetNext()) {
					dgConvexHull4dTetraherum* const tetra = &node->GetInfo();
					for (dgInt32 i = 0; i < 4; i ++) {
						dgConvexHull4dTetraherum::dgTetrahedrumFace& face = tetra->m_faces[i];
						dgEdgeFaceKey key (face.m_index[0], face.m_index[1], face.m_index[2]);
						_DG_ASSERTE (!faceMap.Find(key));
						faceMap.Insert (node, key);
					}
				}

				dgInt32 mark = tmpMesh.IncLRU(); 
				for (iter.Begin(); iter; iter ++) {
					dgEdge* const face = &iter.GetNode()->GetInfo();
					if (face->m_mark != mark){
						dgEdge* ptr = face;
						do {
							ptr->m_mark = mark;
							ptr = ptr->m_next;
						} while (ptr != face);
						dgEdgeFaceKey key (face->m_incidentVertex, face->m_next->m_incidentVertex, face->m_next->m_next->m_incidentVertex);
						_DG_ASSERTE (faceMap.Find(key));
					}
				}
			}
			#endif


		} while (!allFaceFound);


		// all faces are present in the mesh now we can recover the mesh
		// remove all tetrahedral with negative volume
		RemoveUpperHull ();

		// remove any tetrahedron that by round off error might have more that three point on on a face
		RemoveDegeneratedTetras ();


		//dgInt32 tetraMark = 1;
		dgInt32 tetraMark = IncMark();
		dgInt32 mark = m_mesh->IncLRU(); 

		dgTree<dgEdge*, dgEdge*> edgeInconflict(GetAllocator());
		dgMeshEffect::Iterator iter(*m_mesh);
		for (iter.Begin(); iter; iter ++) {
			dgEdge* const face = &iter.GetNode()->GetInfo();
			if (face->m_mark != mark){
				dgEdge* ptr = face;
				//dgTrace (("%d:", ptr->m_incidentFace))
				do {
					ptr->m_mark = mark;
					//dgTrace ((" %d", ptr->m_incidentVertex))
					ptr = ptr->m_next;
				} while (ptr != face);
				//dgTrace (("\n"));
				RecoverFace (*m_mesh, face, mark, tetraMark, edgeInconflict);
			}
		}

		// color codes all tetrahedron inside the mesh volume
		for (dgListNode* node = GetFirst(); node; node = node->GetNext()) {
			dgConvexHull4dTetraherum* const tetra = &node->GetInfo();
			if (tetra->GetMark() == tetraMark) {
				dgInt32 stack = 0;
				dgConvexHull4dTetraherum* stackPool[1024 * 4];
				for (dgInt32 i = 0; i < 4; i ++) {
					dgConvexHull4dTetraherum::dgTetrahedrumFace& face = tetra->m_faces[i];
					if (face.m_twin && (face.m_twin->GetInfo().GetMark() != tetraMark)) {
						stackPool[stack] = &face.m_twin->GetInfo();
						stack ++;
					}
				}

				while (stack) {
					stack --;
					dgConvexHull4dTetraherum* const skinTetra = stackPool[stack];
					skinTetra->SetMark (tetraMark);
					for (dgInt32 i = 0; i < 4; i ++) {
						dgConvexHull4dTetraherum::dgTetrahedrumFace& face = skinTetra->m_faces[i];
						if (face.m_twin && (face.m_twin->GetInfo().GetMark() != tetraMark)) {
							stackPool[stack] = &face.m_twin->GetInfo();
							stack ++;
							_DG_ASSERTE (stack < sizeof (stackPool) / sizeof (stackPool[0]));
						}
					}
				}
			}
		}

		// remove all tetrahedron outsize the mesh volume (those who are not painted)
		dgListNode* nextNode = NULL;
		for (dgListNode* node = GetFirst(); node; node = nextNode) {
			nextNode = node->GetNext();
			dgConvexHull4dTetraherum* const tetra = &node->GetInfo();
			if (tetra->GetMark() != tetraMark) {
				DeleteFace (node);
			}
		}
	}


	dgMeshEffect* m_mesh;
	dgEdgeMap m_edgeMap;
	dgMissingEdges m_missinEdges;
	dgArray<dgIndexMapPair> m_indexMap;
};

#endif



dgCollision* dgMeshEffect::CreateConvexApproximationCollision(dgWorld* const world, dgInt32 maxCount, dgInt32 shapeId, dgInt32 childrenID) const
{
	dgMeshEffect tmpMesh (*this);
	tmpMesh.WeldTJoints();

_DG_ASSERTE (!tmpMesh.HasOpenEdges ());
tmpMesh.ConvertToPolygons();
_DG_ASSERTE (!tmpMesh.HasOpenEdges ());
//tmpMesh.Triangulate();
	dgStack<dgInt32> vertexMap(tmpMesh.GetVertexCount());
	tmpMesh.RemoveUnusedVertices(&vertexMap[0]);

Iterator iter(tmpMesh);
for (iter.Begin(); iter; iter ++)
{
//  dgEdge* const xxxx = &iter.GetNode()->GetInfo();
//  _DG_ASSERTE (xxxx->m_incidentFace > 0);
}


	dgCollision* compound = NULL;
	Tetrahedralization convexHull (tmpMesh);
	if (convexHull.GetCount()) {

		dgInt32 count = 0;
		dgStack<dgCollision*> collisionArray(convexHull.GetCount());

		for (dgConvexHull4d::dgListNode* node = convexHull.GetFirst(); node; node = node->GetNext()) {
			dgVector vertexPool[4];
			dgConvexHull4dTetraherum* const tetra = &node->GetInfo();
			const dgConvexHull4dTetraherum::dgTetrahedrumFace& face0 = tetra->m_faces[0];

			const dgBigVector& p0 (convexHull.GetVertex(face0.m_index[0]));
			const dgBigVector& p1 (convexHull.GetVertex(face0.m_index[1]));
			const dgBigVector& p2 (convexHull.GetVertex(face0.m_index[2]));
			const dgBigVector& p3 (convexHull.GetVertex(face0.m_otherVertex));

			vertexPool[0] = dgVector (dgFloat32 (p0.m_x), dgFloat32 (p0.m_y), dgFloat32 (p0.m_z), dgFloat32 (0.0f));
			vertexPool[1] = dgVector (dgFloat32 (p1.m_x), dgFloat32 (p1.m_y), dgFloat32 (p1.m_z), dgFloat32 (0.0f));
			vertexPool[2] = dgVector (dgFloat32 (p2.m_x), dgFloat32 (p2.m_y), dgFloat32 (p2.m_z), dgFloat32 (0.0f));
			vertexPool[3] = dgVector (dgFloat32 (p3.m_x), dgFloat32 (p3.m_y), dgFloat32 (p3.m_z), dgFloat32 (0.0f));

			dgVector origin (vertexPool[0] + vertexPool[1] + vertexPool[2] + vertexPool[3]);
			origin = origin.Scale (0.25f);
			dgFloat32 xxx = 0.9f;
			vertexPool[0] = (vertexPool[0] - origin).Scale (xxx) + origin;
			vertexPool[1] = (vertexPool[1] - origin).Scale (xxx) + origin;
			vertexPool[2] = (vertexPool[2] - origin).Scale (xxx) + origin;
			vertexPool[3] = (vertexPool[3] - origin).Scale (xxx) + origin;

			dgCollision* collision = world->CreateConvexHull(4, &vertexPool[0].m_x, sizeof (dgVector), dgFloat32 (0.0f), childrenID);
			if (collision) {
				collisionArray[count] = collision;
				count ++;
			}
		}

		compound = world->CreateCollisionCompound(count, &collisionArray[0]);
		for (dgInt32 i = 0; i < count; i ++) {
			world->ReleaseCollision(collisionArray[i]);
		}
	}

	return compound;
}


