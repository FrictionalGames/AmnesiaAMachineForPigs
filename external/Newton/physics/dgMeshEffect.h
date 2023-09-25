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

#ifndef __dgMeshEffect_H__
#define __dgMeshEffect_H__

#include <dgRefCounter.h>

class dgCollision;
class dgMeshTreeCSGFace;
class dgMeshEffectSolidTree;
class dgMeshTreeCSGEdgePool;
class dgMeshTreeCSGPointsPool;


#define DG_MESH_EFFECT_INITIAL_VERTEX_SIZE	8
#define DG_MESH_EFFECT_BOLLEAN_STACK		128
#define DG_MESH_EFFECT_POINT_SPLITED		512
#define DG_MESH_EFFECT_POLYGON_SPLITED		256
#define DG_MESH_EFFECT_TRIANGLE_MIN_AREA	(dgFloat32 (1.0e-9f))
#define DG_MESH_EFFECT_PLANE_TOLERANCE		(dgFloat64 (1.0e-5f))
#define DG_MESH_EFFECT_FLAT_CUT_BORDER_EDGE	0x01
#define DG_MESH_EFFECT_QUANTIZE_FLOAT(x)	(x)
#define DG_VERTEXLIST_INDEXLIST_TOL			(dgFloat32 (1.0e-6f))


class dgMeshEffect: public dgPolyhedra, public dgRefCounter
{
	public:

	struct dgVertexAtribute 
	{
		dgVector m_vertex;
		dgTriplex m_normal;
		dgFloat32 m_u0;
		dgFloat32 m_v0;
		dgFloat32 m_u1;
		dgFloat32 m_v1;
		dgInt32 m_material;
	};

	struct dgIndexArray 
	{
		dgInt32 m_materialCount;
		dgInt32 m_indexCount;
		dgInt32 m_materials[256];
		dgInt32 m_materialsIndexCount[256];
		dgInt32* m_indexList;
	};


	dgMeshEffect(dgMemoryAllocator* const allocator, bool preAllocaBuffers);
	dgMeshEffect(dgCollision* const collision);
	dgMeshEffect(const dgMeshEffect& source);
	dgMeshEffect(dgPolyhedra& mesh, const dgMeshEffect& source);
	
	// Create a convex hull Mesh form point cloud
	dgMeshEffect (dgMemoryAllocator* const allocator, const dgFloat32* const vertexCloud, dgInt32 count, dgInt32 strideInByte, dgFloat32 distTol);

	// create a planar Mesh
	dgMeshEffect(dgMemoryAllocator* const allocator, const dgMatrix& planeMatrix, dgFloat32 witdth, dgFloat32 breadth, dgInt32 material, const dgMatrix& textureMatrix0, const dgMatrix& textureMatrix1);
	virtual ~dgMeshEffect(void);

	dgMatrix CalculateOOBB (dgVector& size) const;
	void CalculateAABB (dgVector& min, dgVector& max) const;

	void CalculateNormals (dgFloat32 angleInRadians);
	void SphericalMapping (dgInt32 material);
	void BoxMapping (dgInt32 front, dgInt32 side, dgInt32 top);
	void CylindricalMapping (dgInt32 cylinderMaterial, dgInt32 capMaterial);

	dgEdge* InsertEdgeVertex (dgEdge* const edge, dgFloat32 param);

	dgMeshEffect* Union (const dgMatrix& matrix, const dgMeshEffect* clip) const;
	dgMeshEffect* Difference (const dgMatrix& matrix, const dgMeshEffect* clip) const;
	dgMeshEffect* Intersection (const dgMatrix& matrix, const dgMeshEffect* clip) const;
	void ClipMesh (const dgMatrix& matrix, const dgMeshEffect* clip, dgMeshEffect** top, dgMeshEffect** bottom) const;

	bool CheckIntersection (const dgMeshEffectSolidTree* const solidTree, dgFloat32 scale) const;
	dgMeshEffectSolidTree* CreateSolidTree() const;
	static void DestroySolidTree (dgMeshEffectSolidTree* tree);
	static bool CheckIntersection (const dgMeshEffect* const meshA, const dgMeshEffectSolidTree* const solidTreeA,
								   const dgMeshEffect* const meshB, const dgMeshEffectSolidTree* const solidTreeB, dgFloat32 scale);

	void Triangulate ();
	void ConvertToPolygons ();

	void RemoveUnusedVertices(dgInt32* const vertexRemapTable);
	
	void BeginPolygon ();
	void AddPolygon (dgInt32 count, const dgFloat32* vertexList, dgInt32 stride, dgInt32 material);
	void EndPolygon ();

	void PackVertexArrays ();

	void BuildFromVertexListIndexList(dgInt32 faceCount, const dgInt32 * const faceIndexCount, const dgInt32 * const faceMaterialIndex, 
		const dgFloat32* const vertex, dgInt32  vertexStrideInBytes, const dgInt32 * const vertexIndex,
		const dgFloat32* const normal, dgInt32  normalStrideInBytes, const dgInt32 * const normalIndex,
		const dgFloat32* const uv0, dgInt32  uv0StrideInBytes, const dgInt32 * const uv0Index,
		const dgFloat32* const uv1, dgInt32  uv1StrideInBytes, const dgInt32 * const uv1Index);


	dgInt32 GetVertexCount() const;
	dgInt32 GetVertexStrideInByte() const {return sizeof (dgVector);}
	dgFloat32* GetVertexPool () const {return &m_points[0].m_x;}

	dgInt32 GetPropertiesCount() const;
	dgInt32 GetPropertiesStrideInByte() const {return sizeof (dgVertexAtribute);}
	dgFloat32* GetAttributePool() const {return &m_attib->m_vertex.m_x;}
	dgFloat32* GetNormalPool() const {return &m_attib->m_normal.m_x;}
	dgFloat32* GetUV0Pool() const {return &m_attib->m_u0;}
	dgFloat32* GetUV1Pool() const {return &m_attib->m_u1;}

	dgEdge* ConectVertex (dgEdge* const e0, dgEdge* const e1);

	dgInt32 GetTotalFaceCount() const;
	dgInt32 GetTotalIndexCount() const;
	void GetFaces (dgInt32* const faceCount, dgInt32* const materials, void** const faceNodeList) const;

	void WeldTJoints ();
	bool SeparateDuplicateLoops (dgEdge* const edge);
	bool HasOpenEdges () const;
	dgFloat32 CalculateVolume () const;

	void GetVertexStreams (dgInt32 vetexStrideInByte, dgFloat32* vertex, dgInt32 normalStrideInByte, dgFloat32* normal, dgInt32 uvStrideInByte0, dgFloat32* uv0, dgInt32 uvStrideInByte1, dgFloat32* uv1);
	void GetIndirectVertexStreams(dgInt32 vetexStrideInByte, dgFloat32* vertex, dgInt32* vertexIndices, dgInt32* vertexCount,
								  dgInt32 normalStrideInByte, dgFloat32* normal, dgInt32* normalIndices, dgInt32* normalCount,
								  dgInt32 uvStrideInByte0, dgFloat32* uv0, dgInt32* uvIndices0, dgInt32* uvCount0,
								  dgInt32 uvStrideInByte1, dgFloat32* uv1, dgInt32* uvIndices1, dgInt32* uvCount1);

	

	dgIndexArray* MaterialGeomteryBegin();
	void MaterialGeomteryEnd(dgIndexArray* handle);
	dgInt32 GetFirstMaterial (dgIndexArray* Handle);
	dgInt32 GetNextMaterial (dgIndexArray* handle, dgInt32 materialHandle);
	dgInt32 GetMaterialID (dgIndexArray* handle, dgInt32 materialHandle);
	dgInt32 GetMaterialIndexCount (dgIndexArray* handle, dgInt32 materialHandle);
	void GetMaterialGetIndexStream (dgIndexArray* handle, dgInt32 materialHandle, dgInt32* index);
	void GetMaterialGetIndexStreamShort (dgIndexArray* handle, dgInt32 materialHandle, dgInt16* index);

	dgCollision* CreateConvexApproximationCollision(dgWorld* const world, dgInt32 maxCount, dgInt32 shapeId, dgInt32 childrenID) const;
	dgCollision* CreateConvexCollision(dgFloat32 tolerance, dgInt32 shapeID, const dgMatrix& matrix = dgGetIdentityMatrix()) const;
	
	

	void PlaneClipMesh (const dgPlane& plane, dgMeshEffect** leftMeshSource, dgMeshEffect** rightMeshSource);

	dgVertexAtribute& GetAttribute (dgInt32 index) const;
	void TransformMesh (const dgMatrix& matrix);


	void* GetFirstVertex ();
	void* GetNextVertex (const void* vertex);
	int GetVertexIndex (const void* vertex) const;

	void* GetFirstPoint ();
	void* GetNextPoint (const void* point);
	int GetPointIndex (const void* point) const;
	int GetVertexIndexFromPoint (const void* point) const;


	void* GetFirstEdge ();
	void* GetNextEdge (const void* edge);
	void GetEdgeIndex (const void* edge, dgInt32& v0, dgInt32& v1) const;
//	void GetEdgeAttributeIndex (const void* edge, dgInt32& v0, dgInt32& v1) const;

	void* GetFirstFace ();
	void* GetNextFace (const void* face);
	int IsFaceOpen (const void* face) const;
	int GetFaceMaterial (const void* face) const;
	int GetFaceIndexCount (const void* face) const;
	void GetFaceIndex (const void* face, int* indices) const;
	void GetFaceAttributeIndex (const void* face, int* indices) const;

	bool Sanity () const;
	private:

	void Init (bool preAllocaBuffers);
	dgVector GetOrigin ()const;
	dgInt32 CalculateMaxAttributes () const;
	void EnumerateAttributeArray (dgVertexAtribute* attib);
	void ApplyAttributeArray (dgVertexAtribute* attib);
	void AddVertex(const dgVector& vertex);
	void AddAtribute (const dgVertexAtribute& attib);
	void AddPoint(const dgFloat32* vertexList, dgInt32 material);
	void FixCylindricalMapping (dgVertexAtribute* attib) const;

	void MergeFaces (dgMeshEffect* source);
	void ReverseMergeFaces (dgMeshEffect* source);
	dgVertexAtribute InterpolateEdge (dgEdge* const edge, dgFloat32 param) const;
	dgVertexAtribute InterpolateVertex (const dgVector& point, dgEdge* const face) const;
	
	void ClipMesh (const dgMeshEffect* clipMesh, dgMeshEffect** leftMeshSource, dgMeshEffect** rightMeshSource) const;
	dgInt32 PlaneApplyCap (const dgMeshEffect* planeMesh, const dgPlane& normal);
	void PlaneClipMesh (const dgMeshEffect* planeMesh, dgMeshEffect** leftMeshSource, dgMeshEffect** rightMeshSource) const;

	void CopyCGSFace (const dgMeshEffect& reference, dgEdge* const face);
	void AddCGSFace (const dgMeshEffect& reference, dgEdge* const refFace, dgInt32 count, dgMeshTreeCSGFace** const faces, const dgMeshTreeCSGPointsPool& points);
	void ClipFace (const dgBigPlane& plane, dgMeshTreeCSGFace* src, dgMeshTreeCSGFace** left, dgMeshTreeCSGFace** right,	dgMeshTreeCSGPointsPool& pointPool) const;

	
	

	bool CheckSingleMesh() const;


	dgInt32 m_isFlagFace;
	dgInt32 m_pointCount;
	dgInt32 m_maxPointCount;

	dgInt32 m_atribCount;
	dgInt32 m_maxAtribCount;

	dgVector* m_points;
	dgVertexAtribute* m_attib;

	
	friend class dgConvexHull3d;
	friend class dgConvexHull4d;
	friend class dgMeshEffectSolidTree;
};

inline dgInt32 dgMeshEffect::GetVertexCount() const
{
	return m_pointCount;
}

inline dgInt32 dgMeshEffect::GetPropertiesCount() const
{
	return m_atribCount;
}

inline dgInt32 dgMeshEffect::GetMaterialID (dgIndexArray* handle, dgInt32 materialHandle)
{
	return handle->m_materials[materialHandle];
}

inline dgInt32 dgMeshEffect::GetMaterialIndexCount (dgIndexArray* handle, dgInt32 materialHandle)
{
	return handle->m_materialsIndexCount[materialHandle];
}

inline dgMeshEffect::dgVertexAtribute& dgMeshEffect::GetAttribute (dgInt32 index) const 
{
	return m_attib[index];
}

#endif
