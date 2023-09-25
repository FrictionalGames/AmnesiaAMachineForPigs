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

#ifndef __dgPolyhedra__
#define __dgPolyhedra__

#include "dgStdafx.h"
#include "dgList.h"
#include "dgTree.h"
#include "dgDebug.h"



class dgEdge;
class dgPlane;
class dgSphere;
class dgMatrix;
class dgPolyhedra;


typedef dgInt64 dgEdgeKey;


class dgPolyhedraDescriptor
{
	public:

	 dgPolyhedraDescriptor (const dgPolyhedra& polyhedra);
	 ~dgPolyhedraDescriptor ();
	 void Update (const dgPolyhedra& polyhedra);

	dgInt32 m_faceCount;							 // total face count including openfaces
	dgInt32 m_edgeCount;							 // total edge count count including openfaces
	dgInt32 m_vertexCount;						 // total vertex count including openfaces
	dgInt32 m_maxVertexIndex;
	dgList<dgEdge*> m_unboundedLoops;
};


DG_MSC_VECTOR_ALIGMENT
class dgEdge
{
	public:
	dgEdge ()
	{
	}

	dgEdge (dgInt32 vertex, dgInt32 face, dgUnsigned64 userdata = 0)
	{
		m_mark = 0;
		m_twin = NULL;
		m_next = NULL;
		m_prev = NULL;
		m_userData = userdata;
		m_incidentVertex = vertex;
		m_incidentFace = face;
	}

	dgInt32 m_incidentVertex;
	dgInt32 m_incidentFace;
	dgUnsigned64 m_userData;
	dgEdge* m_next;
	dgEdge* m_prev;
	dgEdge* m_twin;
	dgInt32 m_mark;
}DG_GCC_VECTOR_ALIGMENT;


class dgPolyhedra: public dgTree <dgEdge, dgEdgeKey>
{
	public:

		struct dgPairKey
		{
			dgPairKey ()
			{
			}

			dgPairKey (dgInt64 val)
			{
	//			i0 = (dgInt32) (val & 0xffffffff);
	//			i1 = (dgInt32) ((val >> 32) & 0xffffffff);
				m_key = dgUnsigned64 (val);
			}

			dgPairKey (dgInt32 v0, dgInt32 v1)
			{
	//			i0 = v0;
	//			i1 = v1;
				m_key = dgUnsigned64 ((dgInt64 (v0) << 32) | v1);
			}

			dgInt64 GetVal () const 
			{
	//			return (((dgInt64)i1) << 32) + i0;
				return dgInt64 (m_key);
			}

			dgInt32 GetLowKey () const 
			{
	//			return i0 & 0x0xffffffff;
				return dgInt32 (m_key>>32);
			}

			dgInt32 GetHighKey () const 
			{
	//			return i1;
				return dgInt32 (m_key & 0xffffffff);
			}

		private:
	//		dgInt32 i0;
	//		dgInt32 i1;
			dgUnsigned64 m_key;

		};



//	dgPolyhedra ();
	dgPolyhedra (dgMemoryAllocator* const allocator);
	dgPolyhedra (const dgPolyhedra &polyhedra);
	virtual ~dgPolyhedra();

	bool SanityCheck() const;

	void SetLRU(dgInt32 lru)	const
	{
		if (lru > m_edgeMark) {
			m_edgeMark = lru;
		}
	}

	dgInt32 IncLRU() const
	{	
		m_edgeMark ++;
		_DG_ASSERTE (m_edgeMark < 0x7fffffff);
		return m_edgeMark;
	}

	void BeginFace();
	dgEdge* AddFace (dgInt32 v0, dgInt32 v1, dgInt32 v2);
	dgEdge* AddFace (dgInt32 count, const dgInt32* const index) {return AddFace (count, index, NULL);}
	dgEdge* AddFace (dgInt32 count, const dgInt32* const index, const dgInt64* const userdata);

	void EndFace ();

	// create an edge and add it to the tree. 
	// the edge is not linked to the existing edge list
	dgEdge* AddHalfEdge (dgInt32 v0, dgInt32 v1);

	// create a complete edge and add it to the tree
	//	the new edge is linked to the existing edge list
//	dgEdge* AddEdge (dgInt32 v0, dgInt32 v1);

	void DeleteEdge (dgInt32 v0, dgInt32 v1);
	void DeleteEdge (dgEdge* const edge);

	void DeleteAllFace();
	void DeleteFace(dgEdge* const edge);
	void DeleteDegenerateFaces (const dgFloat32* const pool, dgInt32 dstStrideInBytes, dgFloat32 minArea);

  	dgInt32 GetMaxIndex() const;
  	dgInt32 GetFaceCount() const;
	dgInt32 GetUnboundedFaceCount() const;
	dgVector FaceNormal (dgEdge* const face, const dgFloat32* const vertex, dgInt32 strideInBytes) const;
	dgBigVector BigFaceNormal (dgEdge* const face, const dgFloat32* const pool, dgInt32 strideInBytes) const;


	bool FlipEdge (dgEdge* const edge);
	dgEdge* SpliteEdge (dgInt32 newIndex, dgEdge* const edge);
	dgEdge* SpliteEdgeAndTriangulate (dgInt32 newIndex, dgEdge* const edge);

	dgEdge* FindVertexNode (dgInt32 v0) const;
	dgEdge* FindEdge (dgInt32 v0, dgInt32 v1) const;
	dgTreeNode* FindEdgeNode (dgInt32 v0, dgInt32 v1) const;
	
//	bool TriangulateFace (dgEdge* const face, const dgFloat32* const vertex, dgInt32 strideInBytes, dgVector& normalOut);
//	void OptimizeTriangulation (const dgFloat32* const vertex, dgInt32 strideInBytes);
	
	//void Quadrangulate (const dgFloat32* const vertex, dgInt32 strideInBytes);
	
	void Triangulate (const dgFloat32* const vertex, dgInt32 strideInBytes, dgPolyhedra* const leftOversOut);
	void ConvexPartition (const dgFloat32* const vertex, dgInt32 strideInBytes, dgPolyhedra* const leftOversOut);
	dgSphere CalculateSphere (const dgFloat32* const vertex, dgInt32 strideInBytes, const dgMatrix* const basis = NULL) const;

	dgInt32 PackVertex (dgFloat32* const destArray, const dgFloat32* const unpackArray, dgInt32 strideInBytes);

//	void Merge (dgPolyhedraDescriptor &desc, dgFloat32 myPool[], dgInt32 myStrideInBytes, const dgPolyhedra& he, const dgFloat32 hisPool[], dgInt32 hisStrideInBytes);
//	void CombineOpenFaces (const dgFloat32* const pool, dgInt32 strideInBytes, dgFloat32 distTol);

	void DeleteOverlapingFaces (const dgFloat32* const pool, dgInt32 strideInBytes, dgFloat32 distTol);


	void BeginConectedSurface() const
	{
	   m_baseMark = IncLRU();
	}
	bool GetConectedSurface (dgPolyhedra &polyhedra) const;
	void EndConectedSurface() const
	{
	}

	void InvertWinding ();

//	dgEdge* GetBadEdge (dgList<dgEdge*>& faceList  const dgFloat32* const pool, dgInt32 strideInBytes) const; 

	// find edges edge shared by two or more non adjacent faces
	// this make impossible to triangulate the polyhedra
	void GetBadEdges (dgList<dgEdge*>& faceList, const dgFloat32* const pool, dgInt32 strideInBytes) const; 

	void ChangeEdgeIncidentVertex (dgEdge* const edge, dgInt32 newIndex);
	void GetCoplanarFaces (dgList<dgEdge*>& faceList, dgEdge* startFace, 
						   const dgFloat32* const pool, dgInt32 hisStrideInBytes,
						   dgFloat32 normalDeviation) const;


	void GetOpenFaces (dgList<dgEdge*>& faceList) const;



	// reduce number of unnecessary edges in a polyhedra
	// note: the polyhedra must be a triangular polyhedra
	void Optimize (const dgFloat32* const pool, dgInt32 strideInBytes, dgFloat32 tol);
	

	void CollapseDegenerateFaces (dgPolyhedraDescriptor &desc, const dgFloat32* const pool, dgInt32 strideInBytes, dgFloat32 area);
	dgEdge* CollapseEdge(dgEdge* const edge);


	// this function ayend to create a better triangulation of a mesh
	// by first calling the calling quadrangular and then triangulate 
	// all quad strips.
	//void OptimalTriangulation (const dgFloat32* const vertex, dgInt32 strideInBytes);


	// this function assume the mesh is a legal mesh;
	// note: recommend a call to Triangulate, or OptimalTriangulation 
	//			before using this function.
	// return index count
	dgInt32 TriangleList (dgUnsigned32 outputBuffer[], dgInt32 maxBufferSize, dgInt32 vertexCacheSize = 12) const;

	// this function assume the mesh is a legal mesh;
	// note1: recommend a call to Triangulate or OptimalTriangulation 
	//			 before using this function
	// note2: a index set to 0xffffffff indicate a run start
	// return index count
	//dgInt32 TriangleStrips (dgUnsigned32 outputBuffer[], dgInt32 maxBufferSize, dgInt32 vertexCacheSize = 12) const;

	void SwapInfo (dgPolyhedra& source);

	private:
	mutable dgInt32 m_baseMark;
	mutable dgInt32 m_edgeMark;
	mutable dgInt32 m_faceSecuence;

	friend class dgPolyhedraDescriptor;
	
};

#endif

