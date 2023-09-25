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
#include "dgContact.h"
#include "dgMeshEffect.h"
#include "dgCollisionConvexHull.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////




struct dgPlaneLocation: public dgPlane 
{
	int m_index;
	const dgConvexSimplexEdge* m_face;
};


dgCollisionConvexHull::dgCollisionConvexHull(
	dgMemoryAllocator* allocator, 
	dgUnsigned32 signature, 
	dgInt32 count, 
	dgInt32 strideInBytes, 
	dgFloat32 tolerance,
	const dgFloat32* vertexArray, 
	const dgMatrix& matrix)
	:dgCollisionConvex(allocator, signature, matrix, m_convexHullCollision)
{
	m_faceCount = 0;
	m_edgeCount = 0;
	m_vertexCount = 0;
	m_vertex = NULL;
	m_simplex = NULL;
	m_faceArray = NULL;
	m_boundPlanesCount = 0;

	m_rtti |= dgCollisionConvexHull_RTTI;
	Create (count, strideInBytes, vertexArray, tolerance);

	dgInt32 planeCount = 0;
	dgPlaneLocation planesArray[1024];
	const dgConvexSimplexEdge* const* faceArray = m_faceArray;
	for (dgInt32 i = 0; i < m_faceCount; i ++) {
		dgInt32 i0;
		dgInt32 i1;
		dgInt32 i2;
		dgInt32 add;


		const dgConvexSimplexEdge* const face = faceArray[i];

		i0 = face->m_prev->m_vertex;
		i1 = face->m_vertex;
		i2 = face->m_next->m_vertex;
		const dgVector& p0 = m_vertex[i0];

		dgVector normal ((m_vertex[i1] - p0) * (m_vertex[i2] - p0));
		normal = normal.Scale (dgFloat32 (1.0f) / dgSqrt (normal % normal));
		add = 1;
		for (dgInt32 j = 0; j < 3; j ++) {
			if (dgAbsf (normal[j]) > dgFloat32 (0.98f)) {
				add = 0;
			}
		}

		if (add) {
			for (dgInt32 j = 0; j < planeCount; j ++) {
				dgFloat32 coplanar;
				coplanar = normal % planesArray[j];
				if (coplanar > 0.98f) {
					add = 0;
					break;
				}
			}

			if (add) {
				dgPlane plane (normal, dgFloat32 (0.0f));
				dgVector planeSupport (SupportVertex (plane));
				plane.m_w = - (plane % planeSupport);
//				_DG_ASSERTE (plane.Evalue(m_boxOrigin) < 0.0f);
				dgPlane& tmpPlane = planesArray[planeCount];
				tmpPlane = plane;
				planesArray[planeCount].m_index = i;
				planesArray[planeCount].m_face = face;
				planeCount ++;
				_DG_ASSERTE (planeCount < (sizeof (planesArray) / sizeof (planesArray[0])));
			}
		}
	}

	m_boundPlanesCount = 0;
	for (dgInt32 i = 0; i < planeCount; i ++) {
		dgPlaneLocation& plane = planesArray[i];
		if (plane.m_face == m_faceArray[plane.m_index]) {
			Swap (m_faceArray[plane.m_index], m_faceArray[m_boundPlanesCount]);
		} else {
			dgInt32 j;
			for (j = m_boundPlanesCount; j < m_faceCount; j ++) {
				if (plane.m_face == m_faceArray[j]) {
					Swap (m_faceArray[j], m_faceArray[m_boundPlanesCount]);
					break;
				}
			}
			_DG_ASSERTE (j < m_faceCount);
		}
		m_boundPlanesCount ++;
	}

	m_destructionImpulse = dgFloat32 (1.0e20f);
}

dgCollisionConvexHull::dgCollisionConvexHull(dgWorld* const world, dgDeserialize deserialization, void* const userData)
	:dgCollisionConvex (world, deserialization, userData)
{
	m_rtti |= dgCollisionConvexHull_RTTI;
	deserialization (userData, &m_vertexCount, sizeof (dgInt32));
	deserialization (userData, &m_vertexCount, sizeof (dgInt32));
	deserialization (userData, &m_faceCount, sizeof (dgInt32));
	deserialization (userData, &m_edgeCount, sizeof (dgInt32));
	deserialization (userData, &m_boundPlanesCount, sizeof (dgInt32));
	deserialization (userData, &m_destructionImpulse, sizeof (dgFloat32));
	
	m_vertex = (dgVector*) m_allocator->Malloc (dgInt32 (m_vertexCount * sizeof (dgVector)));
	m_simplex = (dgConvexSimplexEdge*) m_allocator->Malloc (dgInt32 (m_edgeCount * sizeof (dgConvexSimplexEdge)));
	m_faceArray = (dgConvexSimplexEdge **) m_allocator->Malloc(dgInt32 (m_faceCount * sizeof(dgConvexSimplexEdge *)));

	deserialization (userData, m_vertex, m_vertexCount * sizeof (dgVector));

	for (dgInt32 i = 0; i < m_edgeCount; i ++) {
		dgInt32 serialization[4];
		deserialization (userData, serialization, sizeof (serialization));

		m_simplex[i].m_vertex = serialization[0];
		m_simplex[i].m_twin = m_simplex + serialization[1];
		m_simplex[i].m_next = m_simplex + serialization[2];
		m_simplex[i].m_prev = m_simplex + serialization[3];
	}

	for (dgInt32 i = 0; i < m_faceCount; i ++) {
		dgInt32 faceOffset;
		deserialization (userData, &faceOffset, sizeof (dgInt32));
		m_faceArray[i] = m_simplex + faceOffset; 
	}

//	if (m_boundPlanesCount) {
//		deserialization (userData, m_boundPlanes, m_boundPlanesCount * sizeof (dgPlane));
//	}


	SetVolumeAndCG ();
}

dgCollisionConvexHull::~dgCollisionConvexHull()
{
	if (m_faceArray) {
		m_allocator->Free(m_faceArray);
	}
}


dgInt32 dgCollisionConvexHull::GetFaceIndices (dgInt32 index, dgInt32* indices) const
{
	dgInt32 count;

	count = 0;
	const dgConvexSimplexEdge* face = m_faceArray[index];
	do {
		indices [count] = face->m_vertex;
		count ++;
		face = face->m_next;
	} while (face != m_faceArray[index]);

	return count;
}

dgBigVector dgCollisionConvexHull::FaceNormal (const dgEdge *face, const dgVector* const pool) const
{
	const dgEdge *edge = face;
	dgBigVector p0 (&pool[edge->m_incidentVertex].m_x);
	edge = edge->m_next;

	dgBigVector p1 (&pool[edge->m_incidentVertex].m_x);
	dgBigVector e1 (p1 - p0);

	dgBigVector normal (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));
	for (edge = edge->m_next; edge != face; edge = edge->m_next) {
		dgBigVector p2 (&pool[edge->m_incidentVertex].m_x);
		dgBigVector e2 (p2 - p0);
		dgBigVector n1 (e1 * e2);
#ifdef _DG_DEBUG
		dgFloat64 mag = normal % n1;
		_DG_ASSERTE ( mag >= -dgFloat32 (0.1f));
#endif
		normal += n1;
		e1 = e2;
	} 
	dgFloat64 den = sqrt (normal % normal) + dgFloat64 (1.0e-24f);
	normal = normal.Scale (dgFloat64 (1.0f)/ den);

#ifdef _DG_DEBUG
	edge = face;
	dgBigVector e0 (pool[edge->m_incidentVertex] - pool[edge->m_prev->m_incidentVertex]);	
	do {
		dgBigVector e1 (pool[edge->m_next->m_incidentVertex] - pool[edge->m_incidentVertex]);	
		dgBigVector n1 (e0 * e1);
		dgFloat64 x = normal % n1;
		_DG_ASSERTE (x > -dgFloat64 (0.01f));
		e0 = e1;
		edge = edge->m_next;
	} while (edge != face);
#endif

	return normal;
}


bool dgCollisionConvexHull::RemoveCoplanarEdge (dgPolyhedra& polyhedra, dgVector* const hullVertexArray) const
{
	bool removeEdge;
	dgInt32 mark;

	removeEdge = false;
	// remove coplanar edges
	mark = polyhedra.IncLRU();
	dgPolyhedra::Iterator iter (polyhedra);
	for (iter.Begin(); iter; ) {
		dgEdge* edge0 = &(*iter);
		iter ++;

		if (edge0->m_incidentFace != -1) {

			if (edge0->m_mark < mark) {
				edge0->m_mark = mark;
				edge0->m_twin->m_mark = mark;
				dgBigVector normal0 (FaceNormal (edge0, &hullVertexArray[0]));
				dgBigVector normal1 (FaceNormal (edge0->m_twin, &hullVertexArray[0]));

				dgFloat64 test = normal0 % normal1;
				if (test > dgFloat64 (0.99995f)) {

					if ((edge0->m_twin->m_next->m_twin->m_next != edge0) && (edge0->m_next->m_twin->m_next != edge0->m_twin)) {
						#define DG_MAX_EDGE_ANGLE dgFloat32 (1.0e-3f)

						if (edge0->m_twin == &(*iter)) {
							if (iter) {
								iter ++;
							}
						}

						dgBigVector e1 (hullVertexArray[edge0->m_twin->m_next->m_next->m_incidentVertex] - hullVertexArray[edge0->m_incidentVertex]);
						dgBigVector e0 (hullVertexArray[edge0->m_incidentVertex] - hullVertexArray[edge0->m_prev->m_incidentVertex]);

						_DG_ASSERTE ((e0 % e0) >= dgFloat64 (0.0f));
						_DG_ASSERTE ((e1 % e1) >= dgFloat64 (0.0f));

						e0 = e0.Scale (dgFloat64 (1.0f) / sqrt (e0 % e0));
						e1 = e1.Scale (dgFloat64 (1.0f) / sqrt (e1 % e1));
						dgBigVector n1 (e0 * e1);

						dgFloat64 projection = n1 % normal0;
						if (projection >= DG_MAX_EDGE_ANGLE) {

							dgBigVector e1 (hullVertexArray[edge0->m_next->m_next->m_incidentVertex] - hullVertexArray[edge0->m_twin->m_incidentVertex]);
							dgBigVector e0 (hullVertexArray[edge0->m_twin->m_incidentVertex] - hullVertexArray[edge0->m_twin->m_prev->m_incidentVertex]);
							_DG_ASSERTE ((e0 % e0) >= dgFloat64 (0.0f));
							_DG_ASSERTE ((e1 % e1) >= dgFloat64 (0.0f));
							//e0 = e0.Scale (dgRsqrt (e0 % e0));
							//e1 = e1.Scale (dgRsqrt (e1 % e1));
							e0 = e0.Scale (dgFloat64 (1.0f) / sqrt (e0 % e0));
							e1 = e1.Scale (dgFloat64 (1.0f) / sqrt (e1 % e1));

							dgBigVector n1 (e0 * e1);
							projection = n1 % normal0;
							if (projection >= DG_MAX_EDGE_ANGLE) {
								_DG_ASSERTE (&(*iter) != edge0);
								_DG_ASSERTE (&(*iter) != edge0->m_twin);
								polyhedra.DeleteEdge(edge0);
								removeEdge = true;
							}
						}

					} else {
						dgEdge *next; 
						dgEdge *prev; 

						next = edge0->m_next;
						prev = edge0->m_prev;
						polyhedra.DeleteEdge(edge0);
						for (edge0 = next; edge0->m_prev->m_twin == edge0; edge0 = next) {
							next = edge0->m_next;
							polyhedra.DeleteEdge(edge0);
						}

						for (edge0 = prev; edge0->m_next->m_twin == edge0; edge0 = prev) {
							prev = edge0->m_prev;
							polyhedra.DeleteEdge(edge0);
						}
						iter.Begin(); 
						removeEdge = true;
					}
				}
			}
		}
	}

	return removeEdge;
}


bool dgCollisionConvexHull::CheckConvex (dgPolyhedra& polyhedra1, const dgVector* hullVertexArray) const
{
	dgPolyhedra polyhedra(polyhedra1);

	dgPolyhedra::Iterator iter (polyhedra);
	dgVector center (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));

	dgInt32 count = 0;
	dgInt32 mark = polyhedra.IncLRU();
	for (iter.Begin(); iter; iter ++) {
		dgEdge* const edge = &(*iter);
		if (edge->m_mark < mark) {
			count ++;
			center += hullVertexArray[edge->m_incidentVertex];
			dgEdge* ptr = edge;
			do {
				ptr->m_mark = mark;
				ptr = ptr->m_twin->m_next;
			} while (ptr != edge);
		}
	}
	center = center.Scale (dgFloat32 (1.0f) / dgFloat32 (count));

	for (iter.Begin(); iter; iter ++) {
		dgEdge* const edge = &(*iter);
		dgBigVector normal0 (FaceNormal (edge, &hullVertexArray[0]));
		dgBigVector normal1 (FaceNormal (edge->m_twin, &hullVertexArray[0]));

		dgBigPlane plane0 (normal0, - (normal0 % dgBigVector(hullVertexArray[edge->m_incidentVertex])));
		dgBigPlane plane1 (normal1, - (normal1 % dgBigVector(hullVertexArray[edge->m_twin->m_incidentVertex])));
		dgFloat64 test0 = plane0.Evalue(center);
		if (test0 > dgFloat64 (1.0e-3f)) {
			return false;
		}
		dgFloat64 test1 = plane1.Evalue(center);
//		if (test1 > dgFloat64 (0.0f)) {
		if (test1 > dgFloat64 (1.0e-3f)) {
			return false;
		}
	}

	return true;
}


#ifdef DG_USE_OLD_CONVEXHULL 

#define DG_EDGE_MARKED		0x040000
#define DG_EDGE_TO_DESTROY  0x080000

dgInt32 dgCollisionConvexHull::BruteForceSupportVertex (int count, dgVector* array, const dgBigVector& dir) const
{
	dgFloat64 dist = dgFloat32 (-1.0e10f);
	dgInt32 index = -1;
	for (dgInt32 i = 0; i < count; i ++) {

		dgFloat64 dist1 = dgBigVector(array[i]) % dir;
		if (dist1 > dist) {
			dist = dist1;
			index = i;
		}
	}
	return index;
}


bool dgCollisionConvexHull::Create (dgInt32 count, dgInt32 strideInBytes, const dgFloat32* const vertexArray, dgFloat32 tolerance)
{
	dgStack<dgEdge*> stack(4096 + count);
	dgStack<dgEdge*> deleted(4096 + count);
	dgStack<dgVector> array (count + 16);
	dgStack<dgVector> hullVertex (count + 16);
	dgVector* const tmpArray = &array[0];
	dgVector* const hullVertexArray = &hullVertex[0];;

	#define MAX_CONVEX_HULL_SCALE	dgFloat32 (128.0f)
	#define CONVEX_HULL_GRANULARITY	dgFloat32 (2048.0f)
	#define CONVEX_HULL_INV_GRANULARITY	(dgFloat32 (1.0f) / CONVEX_HULL_GRANULARITY)

	m_faceCount = 0;
	m_faceArray = NULL;

	dgVector minValue;
	dgVector maxValue;
	GetMinMax (minValue, maxValue, vertexArray, count, strideInBytes);
	dgVector scale (maxValue - minValue);
	dgVector center (maxValue + minValue);

	scale = scale.Scale (dgFloat32 (0.5f));
	center = center.Scale (dgFloat32 (0.5f));

	if (scale.m_x < dgFloat32(1.0f)) {
		scale.m_x = dgFloat32(1.0f)/ (scale.m_x + dgFloat32(1.0e-6f));
	} else if (scale.m_x > MAX_CONVEX_HULL_SCALE) {
		scale.m_x = MAX_CONVEX_HULL_SCALE / scale.m_x;
	}

	if (scale.m_y < dgFloat32(1.0f)) {
		scale.m_y = dgFloat32(1.0f)/ (scale.m_y + dgFloat32(1.0e-6f));
	} else if (scale.m_y > MAX_CONVEX_HULL_SCALE) {
		scale.m_y = MAX_CONVEX_HULL_SCALE/ scale.m_y;
	}

	if (scale.m_z < dgFloat32(1.0f)) {
		scale.m_z = dgFloat32(1.0f)/ (scale.m_z + dgFloat32(1.0e-6f));
	} else if (scale.m_y > MAX_CONVEX_HULL_SCALE) {
		scale.m_z = MAX_CONVEX_HULL_SCALE/ scale.m_z;
	}

	
	dgInt32 stride = strideInBytes / sizeof (dgFloat32);
	for (dgInt32 i = 0; i < count; i ++) {
		dgVector p (vertexArray[i * stride + 0], vertexArray[i * stride + 1], vertexArray[i * stride + 2], dgFloat32 (0.0f));
		p -= center;
		tmpArray[i].m_x = dgFloor (scale.m_x * p.m_x * CONVEX_HULL_GRANULARITY) * CONVEX_HULL_INV_GRANULARITY;
		tmpArray[i].m_y = dgFloor (scale.m_y * p.m_y * CONVEX_HULL_GRANULARITY) * CONVEX_HULL_INV_GRANULARITY;
		tmpArray[i].m_z = dgFloor (scale.m_z * p.m_z * CONVEX_HULL_GRANULARITY) * CONVEX_HULL_INV_GRANULARITY;
		tmpArray[i].m_w = dgFloat32 (0.0f);
	}

	count = dgVertexListToIndexList (&tmpArray[0].m_x, sizeof (dgVector), 3 * sizeof (dgFloat32), 0, count, (dgInt32*) &stack[0], dgFloat32 (0.01f)); 
	if (count < 4) {
		return false;
	}

	dgInt32 index = BruteForceSupportVertex (count, &tmpArray[0], dgBigVector (&m_hullDirs[0].m_x));
	_DG_ASSERTE (index >= 0);
	_DG_ASSERTE (index < count);
	hullVertexArray[0] = tmpArray[index];
	--count;
	_DG_ASSERTE (count >= 0);
	tmpArray[index] = tmpArray[count]; 

	dgInt32 i;
	dgInt32 normalsCount;
	dgVector e1;
	normalsCount = sizeof (m_hullDirs) / sizeof (dgTriplex);
	for (i = 1; i < normalsCount; i ++) {
		index = BruteForceSupportVertex (count, &tmpArray[0], dgBigVector (&m_hullDirs[i].m_x));
		_DG_ASSERTE (index >= 0);
		_DG_ASSERTE (index < count);

		hullVertexArray[1] = tmpArray[index];
		e1 = hullVertexArray[1] - hullVertexArray[0];
		dgFloat32 error2 = e1 % e1;
		if (error2 > dgFloat32 (1.0e-2f)) {
			--count;
			_DG_ASSERTE (count >= 0);
			tmpArray[index] = tmpArray[count]; 
			break;
		}
		_DG_ASSERTE (i < normalsCount);
	}

	dgVector e2;
	dgVector normal (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));
	for (i ++; i < normalsCount; i ++) {
		index = BruteForceSupportVertex (count, &tmpArray[0], dgBigVector (&m_hullDirs[i].m_x));
		_DG_ASSERTE (index >= 0);
		_DG_ASSERTE (index < count);

		hullVertexArray[2] = tmpArray[index];
		e2 = hullVertexArray[2] - hullVertexArray[0];
		normal = e1 * e2;
		dgFloat32 error2 = normal % normal;
		if (error2 > dgFloat32 (1.0e-2f)) {
			--count;
			_DG_ASSERTE (count >= 0);
			tmpArray[index] = tmpArray[count]; 
			break;
		}
	}
	_DG_ASSERTE (i < normalsCount);

	dgInt32 bestIndex = -1;
	dgFloat32 maxError = dgFloat32 (0.0f);
	dgFloat32 error2 = dgFloat32(0.0f);
	dgVector e3;
	for (i ++; i < normalsCount; i ++) {
		index = BruteForceSupportVertex (count, &tmpArray[0], dgBigVector (&m_hullDirs[i].m_x));
		_DG_ASSERTE (index >= 0);
		_DG_ASSERTE (index < count);

		hullVertexArray[3] = tmpArray[index];
		e3 = hullVertexArray[3] - hullVertexArray[0];
		error2 = normal % e3;
		if (dgAbsf (error2) > dgFloat32 (1.0e-2f)) {
			--count;
			_DG_ASSERTE (count >= 0);
			tmpArray[index] = tmpArray[count]; 
			break;
		} else if (error2 > maxError) {
			maxError = error2 ;
			bestIndex = index;
		}

	}

	if (i >= normalsCount) {
		if (maxError < dgFloat32 (1.0e-3f)) {
			return false;
		}
		hullVertexArray[3] = tmpArray[bestIndex];
		e3 = hullVertexArray[3] - hullVertexArray[0];
		error2 = normal % e3;
		--count;
		_DG_ASSERTE (count >= 0);
		tmpArray[bestIndex] = tmpArray[count]; ; 

	}

	if (error2 > dgFloat32(0.0f)) {
		Swap (hullVertexArray[0], hullVertexArray[1]);
	}

	dgPolyhedra polyhedra(GetAllocator());
	polyhedra.BeginFace();
	dgEdge *edge0 = polyhedra.AddFace (0, 1, 2);
	dgEdge *edge1 = polyhedra.AddFace (0, 2, 3);
	dgEdge *edge2 = polyhedra.AddFace (2, 1, 3);
	dgEdge *edge3 = polyhedra.AddFace (1, 0, 3);
	polyhedra.EndFace();

	dgBigPlane plane0 (FaceNormal(edge0, &hullVertexArray[0]), dgFloat32(0.0f));
	dgBigPlane plane1 (FaceNormal(edge1, &hullVertexArray[0]), dgFloat32(0.0f));
	dgBigPlane plane2 (FaceNormal(edge2, &hullVertexArray[0]), dgFloat32(0.0f));
	dgBigPlane plane3 (FaceNormal(edge3, &hullVertexArray[0]), dgFloat32(0.0f));

	plane0.m_w = - (plane0 % dgBigVector (hullVertexArray[edge0->m_incidentVertex]));
	plane1.m_w = - (plane1 % dgBigVector (hullVertexArray[edge1->m_incidentVertex]));
	plane2.m_w = - (plane2 % dgBigVector (hullVertexArray[edge2->m_incidentVertex]));
	plane3.m_w = - (plane3 % dgBigVector (hullVertexArray[edge3->m_incidentVertex]));

	for (i = 0; i < count; i ++) {
		dgVector p (tmpArray[i]);
		if (plane0.Evalue (p) < dgFloat32 (-1.0e-6f)) {
			if (plane1.Evalue (p) < dgFloat32 (-1.0e-6f)) {
				if (plane2.Evalue (p) < dgFloat32 (-1.0e-6f)) {
					if (plane3.Evalue (p) < dgFloat32 (-1.0e-6f)) {
						--count;
						_DG_ASSERTE (count >= 0);
						tmpArray[i] = tmpArray[count]; 
						i --;
					}
				}
			}
		}
	}


	dgTree<dgEdge*, dgInt64> faceList(GetAllocator());
	faceList.Insert (edge0, dgPolyhedra::dgPairKey(edge0->m_incidentVertex, edge0->m_twin->m_incidentVertex).GetVal());
	faceList.Insert (edge1, dgPolyhedra::dgPairKey(edge1->m_incidentVertex, edge1->m_twin->m_incidentVertex).GetVal());
	faceList.Insert (edge2, dgPolyhedra::dgPairKey(edge2->m_incidentVertex, edge2->m_twin->m_incidentVertex).GetVal());
	faceList.Insert (edge3, dgPolyhedra::dgPairKey(edge3->m_incidentVertex, edge3->m_twin->m_incidentVertex).GetVal());

	dgInt32 faceId = 10;
	dgInt32 vertexIndex = 4;

	tolerance = dgAbsf(tolerance) + dgFloat32 (1.0e-4f);
	while (faceList.GetCount() && count) {

		edge0 = faceList.GetRoot()->GetInfo();
		faceList.Remove (faceList.GetRoot());

		dgBigVector normal (FaceNormal (edge0, &hullVertexArray[0]));

		index = BruteForceSupportVertex (count, &tmpArray[0], normal);

		if (index != -1) {
			dgBigVector p0 (hullVertexArray[edge0->m_incidentVertex]);
			dgBigVector p (tmpArray[index]);

			dgFloat64 dist = (p - p0) % normal;

			if (dist > tolerance) {

				dgEdge *silhouette = edge0;
				stack[0] = edge0;

				dgInt32 stackIndex = 1;
				dgInt32 deletedCount = 0;

				while (stackIndex) {

					stackIndex --;
					dgEdge* edge0 = stack[stackIndex];

					if (!(edge0->m_userData & DG_EDGE_MARKED)) {
						dgInt32 i0 = edge0->m_incidentVertex;
						dgInt32 i1 = edge0->m_next->m_incidentVertex;
						dgInt32 i2 = edge0->m_prev->m_incidentVertex;

						dgBigVector r0 (hullVertexArray[i0]);
						dgBigVector r1 (hullVertexArray[i1]);
						dgBigVector r2 (hullVertexArray[i2]);

						dgBigVector e0 (r1 - r0);
						dgBigVector e1 (r2 - r0);

						dgBigVector n (e0 * e1);
						//dgFloat64 test = n % (p - hullVertexArray[i0]);
						dgFloat64 test = n % (p - r0);
						if (test > dgFloat64(0.0f)) { 
							dgEdge* ptr = edge0;

							do {
								ptr->m_userData |= DG_EDGE_MARKED;
								dgEdge* twin = ptr->m_twin;
								if (!(twin->m_userData & DG_EDGE_MARKED)) {
									stack[stackIndex] = twin;
									stackIndex ++;
									silhouette = ptr;
	//								_DG_ASSERTE (stackIndex < DG_HULL_MAX_DEPTH);
								} else if ((twin->m_userData & (DG_EDGE_TO_DESTROY | DG_EDGE_MARKED)) == DG_EDGE_MARKED) {
									_DG_ASSERTE ((twin->m_userData & DG_EDGE_TO_DESTROY) == 0);
									_DG_ASSERTE (twin->m_userData & DG_EDGE_MARKED);
									ptr->m_userData = DG_EDGE_TO_DESTROY | DG_EDGE_MARKED;
									twin->m_userData = DG_EDGE_TO_DESTROY | DG_EDGE_MARKED;
									deleted[deletedCount] = ptr;
									deletedCount ++;
	//								_DG_ASSERTE (deletedCount < DG_HULL_MAX_DEPTH);
								}
								ptr = ptr->m_next;
							} while (ptr != edge0);
						}
					}
				}

				for (dgInt32 i = 0; i < deletedCount; i ++) { 
					dgEdge* ptr = deleted[i];
					faceList.Remove (dgPolyhedra::dgPairKey (ptr->m_incidentVertex, ptr->m_twin->m_incidentVertex).GetVal());
					faceList.Remove (dgPolyhedra::dgPairKey (ptr->m_twin->m_incidentVertex, ptr->m_incidentVertex).GetVal());
					polyhedra.DeleteEdge(ptr);
				}

				dgEdge* ptr = silhouette;
				do {
					//faceList.Remove (ptr);
					faceList.Remove (dgPolyhedra::dgPairKey (ptr->m_incidentVertex, ptr->m_twin->m_incidentVertex).GetVal());
					ptr->m_userData &= ~(DG_EDGE_MARKED);
					ptr = ptr->m_next;
				} while (ptr != silhouette);

				dgEdge* prev = polyhedra.AddHalfEdge(vertexIndex, silhouette->m_incidentVertex);
				dgEdge* edge1 = polyhedra.AddHalfEdge(silhouette->m_incidentVertex, vertexIndex);

				_DG_ASSERTE (prev);
				_DG_ASSERTE (edge1);

				edge1->m_twin = prev;
				prev->m_twin = edge1;

				edge1->m_next = prev;
				prev->m_prev = edge1;

				dgEdge* last = silhouette->m_prev;

				prev->m_incidentVertex = vertexIndex;
				prev->m_twin->m_incidentVertex = silhouette->m_incidentVertex;

				silhouette->m_prev->m_next = prev->m_twin;
				prev->m_twin->m_prev = silhouette->m_prev;

				silhouette->m_prev = prev;
				prev->m_next = silhouette;
				do {
					silhouette = silhouette->m_next;

					dgEdge* next = polyhedra.AddHalfEdge(vertexIndex, silhouette->m_incidentVertex);
					edge1 = polyhedra.AddHalfEdge(silhouette->m_incidentVertex, vertexIndex);

					edge1->m_next = next;
					next->m_prev = edge1;

					_DG_ASSERTE (next);
					_DG_ASSERTE (edge1);

					edge1->m_twin = next;
					next->m_twin = edge1;

					next->m_incidentVertex = vertexIndex;
					next->m_twin->m_incidentVertex = silhouette->m_incidentVertex;

					silhouette->m_prev->m_next = next->m_twin;
					next->m_twin->m_prev = silhouette->m_prev;

					silhouette->m_prev = next;
					next->m_next = silhouette;

					next->m_prev = prev->m_prev;
					prev->m_prev->m_next = next;

					next->m_twin->m_next = prev;
					prev->m_prev = next->m_twin;

					prev = next;

				} while (silhouette != last);

				
				_DG_ASSERTE (vertexIndex < dgInt32 (hullVertex.GetElementsCount()));
				hullVertexArray[vertexIndex] = dgVector (dgFloat32 (p.m_x), dgFloat32 (p.m_y), dgFloat32 (p.m_z), dgFloat32 (0.0f));;
				vertexIndex++;

				--count;
				_DG_ASSERTE (count >= 0);
				tmpArray[index] = tmpArray[count]; 

				ptr = prev;
				do {
					//faceList.Insert (ptr, ptr);
					faceList.Insert(ptr, dgPolyhedra::dgPairKey (ptr->m_incidentVertex, ptr->m_twin->m_incidentVertex).GetVal());
					ptr->m_incidentFace = faceId;
					ptr->m_next->m_incidentFace = faceId;
					ptr->m_prev->m_incidentFace = faceId;
					faceId ++;

					ptr = ptr->m_twin->m_next;
				} while (ptr != prev);
			}

//			_DG_ASSERTE (CheckConvex (polyhedra, hullVertexArray));
		}
	}


	_DG_ASSERTE (CheckConvex (polyhedra, hullVertexArray));
	bool edgeRemoved = false;
	while (RemoveCoplanarEdge (polyhedra, hullVertexArray)) {
		edgeRemoved = true;
	}
	if (edgeRemoved) {
		if (!CheckConvex (polyhedra, hullVertexArray)) {
			return false;
		}
	}
//	_DG_ASSERTE (CheckConvex (polyhedra, hullVertexArray));

	dgEdge *firstFace = &polyhedra.GetRoot()->GetInfo();

	_DG_ASSERTE (firstFace->m_twin->m_next != firstFace);

	dgInt32 stackIndex = 1; 
	stack[0] = firstFace;

	dgStack<dgInt32> vertexMap(vertexIndex);
	memset (&vertexMap[0], -1, vertexIndex * sizeof (dgInt32));

	m_edgeCount = 0;
	dgInt32 i1 = polyhedra.IncLRU();
	m_vertexCount = 0;
	while (stackIndex) {
		stackIndex --;
		dgEdge *edge0 = stack[stackIndex];

		if (edge0->m_mark != i1) {
			if (vertexMap[edge0->m_incidentVertex] == -1) {
				vertexMap[edge0->m_incidentVertex] = m_vertexCount;
				m_vertexCount ++;
			}
			dgEdge *ptr = edge0;
			do {
				stack[stackIndex] = ptr->m_twin;
				stackIndex++;
				ptr->m_mark = i1;
				ptr->m_userData = m_edgeCount;
				m_edgeCount ++;
				ptr = ptr->m_twin->m_next;
			} while (ptr != edge0) ;
		}
	} 

	m_vertex = (dgVector*) m_allocator->Malloc (m_vertexCount * sizeof (dgVector));
	m_simplex = (dgConvexSimplexEdge*) m_allocator->Malloc (m_edgeCount * sizeof (dgConvexSimplexEdge));

	scale.m_x = dgFloat32(1.0f) / scale.m_x;
	scale.m_y = dgFloat32(1.0f) / scale.m_y;
	scale.m_z = dgFloat32(1.0f) / scale.m_z;
	for (i = 0; i < vertexIndex; i ++) {
		if (vertexMap[i] != -1) {
			m_vertex[vertexMap[i]] = scale.CompProduct (hullVertexArray[i]) + center;
			m_vertex[vertexMap[i]].m_w = dgFloat32 (1.0f);
		}
	}


	i1 = polyhedra.IncLRU();
	stackIndex = 1; 
	stack[0] = firstFace;
	while (stackIndex) {
		stackIndex --;
		dgEdge *edge0 = stack[stackIndex];

		if (edge0->m_mark != i1) {

			dgEdge *ptr = edge0;
			do {
				ptr->m_mark = i1;
				stack[stackIndex] = ptr->m_twin;
				stackIndex++;

				dgConvexSimplexEdge *simplexPtr = &m_simplex[ptr->m_userData];
				simplexPtr->m_vertex = vertexMap[ptr->m_incidentVertex];
				simplexPtr->m_next = &m_simplex[ptr->m_next->m_userData];
				simplexPtr->m_prev = &m_simplex[ptr->m_prev->m_userData];
				simplexPtr->m_twin = &m_simplex[ptr->m_twin->m_userData];

				ptr = ptr->m_twin->m_next;
			} while (ptr != edge0) ;
		}
	} 

	SetVolumeAndCG ();
	m_faceCount = 0;
	dgStack<char>  mark (m_edgeCount);
	memset (&mark[0], 0, m_edgeCount * sizeof (dgInt8));

	dgStack<dgConvexSimplexEdge*> faceArray (m_edgeCount);
	for (i = 0; i < m_edgeCount; i ++) {
		dgConvexSimplexEdge *ptr;
		dgConvexSimplexEdge *face;
		face = &m_simplex[i];
		if (!mark[i]) {
			ptr = face;
			do {
				_DG_ASSERTE ((ptr - m_simplex) >= 0);
				mark[dgInt32 (ptr - m_simplex)] = '1';
				ptr = ptr->m_next;
			} while (ptr != face);

			faceArray[m_faceCount] = face;
			m_faceCount ++;
		}
	}
	m_faceArray = (dgConvexSimplexEdge **) m_allocator->Malloc(m_faceCount * sizeof(dgConvexSimplexEdge *));
	memcpy (m_faceArray, &faceArray[0], m_faceCount * sizeof(dgConvexSimplexEdge *));
	return true;
}



#else

bool dgCollisionConvexHull::Create (dgInt32 count, dgInt32 strideInBytes, const dgFloat32* const vertexArray, dgFloat32 tolerance)
{
//	dgMeshEffect convexHull (GetAllocator(), vertexArray, count, strideInBytes, tolerance);
	dgConvexHull3d convexHull (GetAllocator(), vertexArray, strideInBytes, count, tolerance);
	if (!convexHull.GetCount()) {
		return false;
	}
	dgInt32 vertexCount = convexHull.GetVertexCount();
//	dgStack<dgVector> vertexPool(vertexCount);
//	dgVector* const hullVertexArray = &vertexPool[0];
//	for (dgInt32 i = 0; i < vertexCount; i ++) {
//		hullVertexArray[i] = convexHull.GetVertex(i);
//	}
	dgVector* const hullVertexArray = (dgVector*) convexHull.GetVertexPool();

	dgPolyhedra polyhedra (GetAllocator());
	polyhedra.BeginFace();
	for (dgConvexHull3d::dgListNode* node = convexHull.GetFirst(); node; node = node->GetNext()) {
		dgConvexHull3DFace& face = node->GetInfo();
		polyhedra.AddFace (face.m_index[0], face.m_index[1], face.m_index[2]);
	}

	polyhedra.EndFace();
	

	if (vertexCount > 4) {
		bool edgeRemoved = false;
		while (RemoveCoplanarEdge (polyhedra, hullVertexArray)) {
			edgeRemoved = true;
		}
		if (edgeRemoved) {
			if (!CheckConvex (polyhedra, hullVertexArray)) {
				return false;
			}
		}
	}

	dgInt32 maxEdgeCount = polyhedra.GetCount();

	dgStack<dgEdge*> stack(1024 + maxEdgeCount);
	dgEdge* firstFace = &polyhedra.GetRoot()->GetInfo();

	_DG_ASSERTE (firstFace->m_twin->m_next != firstFace);

	dgInt32 stackIndex = 1; 
	stack[0] = firstFace;

//	dgInt32 vertexCount = polyhedra.GetVertexCount();
	dgStack<dgInt32> vertexMap(vertexCount);
	memset (&vertexMap[0], -1, vertexCount * sizeof (dgInt32));

//	m_edgeCount = 0;
//	m_vertexCount = 0;

	dgInt32 i1 = polyhedra.IncLRU();
	while (stackIndex) {
		stackIndex --;
		dgEdge* const edge0 = stack[stackIndex];

		if (edge0->m_mark != i1) {
			if (vertexMap[edge0->m_incidentVertex] == -1) {
				vertexMap[edge0->m_incidentVertex] = m_vertexCount;
				m_vertexCount ++;
			}
			dgEdge* ptr = edge0;
			do {
				stack[stackIndex] = ptr->m_twin;
				stackIndex++;
				ptr->m_mark = i1;
				ptr->m_userData = m_edgeCount;
				m_edgeCount ++;
				ptr = ptr->m_twin->m_next;
			} while (ptr != edge0) ;
		}
	} 

	m_vertex = (dgVector*) m_allocator->Malloc (dgInt32 (m_vertexCount * sizeof (dgVector)));
	m_simplex = (dgConvexSimplexEdge*) m_allocator->Malloc (dgInt32 (m_edgeCount * sizeof (dgConvexSimplexEdge)));

	for (dgInt32 i = 0; i < vertexCount; i ++) {
		if (vertexMap[i] != -1) {
			m_vertex[vertexMap[i]] = hullVertexArray[i];
			m_vertex[vertexMap[i]].m_w = dgFloat32 (1.0f);
		}
	}

	i1 = polyhedra.IncLRU();
	stackIndex = 1; 
	stack[0] = firstFace;
	while (stackIndex) {

		stackIndex --;
		dgEdge* const edge0 = stack[stackIndex];

		if (edge0->m_mark != i1) {

			dgEdge *ptr = edge0;
			do {
				ptr->m_mark = i1;
				stack[stackIndex] = ptr->m_twin;
				stackIndex++;

				dgConvexSimplexEdge* const simplexPtr = &m_simplex[ptr->m_userData];
				simplexPtr->m_vertex = vertexMap[ptr->m_incidentVertex];
				simplexPtr->m_next = &m_simplex[ptr->m_next->m_userData];
				simplexPtr->m_prev = &m_simplex[ptr->m_prev->m_userData];
				simplexPtr->m_twin = &m_simplex[ptr->m_twin->m_userData];

				ptr = ptr->m_twin->m_next;
			} while (ptr != edge0) ;
		}
	} 

	SetVolumeAndCG ();
	m_faceCount = 0;
	dgStack<char> mark (m_edgeCount);
	memset (&mark[0], 0, m_edgeCount * sizeof (dgInt8));

	dgStack<dgConvexSimplexEdge*> faceArray (m_edgeCount);
	for (dgInt32 i = 0; i < m_edgeCount; i ++) {
		dgConvexSimplexEdge* const face = &m_simplex[i];
		if (!mark[i]) {
			dgConvexSimplexEdge* ptr = face;
			do {
				_DG_ASSERTE ((ptr - m_simplex) >= 0);
				mark[dgInt32 (ptr - m_simplex)] = '1';
				ptr = ptr->m_next;
			} while (ptr != face);

			faceArray[m_faceCount] = face;
			m_faceCount ++;
		}
	}
	m_faceArray = (dgConvexSimplexEdge **) m_allocator->Malloc(dgInt32 (m_faceCount * sizeof(dgConvexSimplexEdge *)));
	memcpy (m_faceArray, &faceArray[0], m_faceCount * sizeof(dgConvexSimplexEdge *));

	return true;
}


#endif

dgInt32 dgCollisionConvexHull::CalculateSignature () const
{
	_DG_ASSERTE (0);
	return dgInt32 (GetSignature());
}

void dgCollisionConvexHull::SetBreakImpulse(dgFloat32 force)
{
	m_destructionImpulse = force;
}

dgFloat32 dgCollisionConvexHull::GetBreakImpulse() const
{
	return m_destructionImpulse;
}



void dgCollisionConvexHull::SetCollisionBBox (const dgVector& p0__, const dgVector& p1__)
{
	_DG_ASSERTE (0);
}



void dgCollisionConvexHull::DebugCollision (const dgMatrix& matrixPtr, OnDebugCollisionMeshCallback callback, void* const userData) const
{
	dgInt32 i;
	dgInt32 count;
	dgConvexSimplexEdge *ptr;
	dgConvexSimplexEdge *face;
	
	dgStack<dgTriplex> tmp (m_vertexCount);
	dgMatrix matrix (GetOffsetMatrix() * matrixPtr);
	matrix.TransformTriplex (&tmp[0], sizeof (dgTriplex), m_vertex, sizeof (dgVector), m_vertexCount);

	for (i = 0; i < m_faceCount; i ++) {
		face = m_faceArray[i];
		ptr = face;
		count = 0;
		dgTriplex vertex[256];
		do {
			vertex[count] = tmp[ptr->m_vertex];
			count ++;
			ptr = ptr->m_next;
		} while (ptr != face);
		callback (userData, count, &vertex[0].m_x, 0);
	}
}


void dgCollisionConvexHull::GetCollisionInfo(dgCollisionInfo* info) const
{
	dgCollisionConvex::GetCollisionInfo(info);

	info->m_offsetMatrix = GetOffsetMatrix();
//	strcpy (info->m_collisionType, "convexHull");
	info->m_collisionType = m_collsionId;

	info->m_convexHull.m_vertexCount = m_vertexCount;
	info->m_convexHull.m_strideInBytes = sizeof (dgVector);
	info->m_convexHull.m_faceCount = m_faceCount;
	info->m_convexHull.m_vertex = &m_vertex[0];

}

void dgCollisionConvexHull::Serialize(dgSerialize callback, void* const userData) const
{
	SerializeLow(callback, userData);

	callback (userData, &m_vertexCount, sizeof (dgInt32));
	callback (userData, &m_vertexCount, sizeof (dgInt32));
	callback (userData, &m_faceCount, sizeof (dgInt32));
	callback (userData, &m_edgeCount, sizeof (dgInt32));
	callback (userData, &m_boundPlanesCount, sizeof (dgInt32));
	callback (userData, &m_destructionImpulse, sizeof (dgFloat32));


	callback (userData, m_vertex, m_vertexCount * sizeof (dgVector));

	for (dgInt32 i = 0; i < m_edgeCount; i ++) {
		dgInt32 serialization[4];
		serialization[0] = m_simplex[i].m_vertex;
		serialization[1] = dgInt32 (m_simplex[i].m_twin - m_simplex);
		serialization[2] = dgInt32 (m_simplex[i].m_next - m_simplex);
		serialization[3] = dgInt32 (m_simplex[i].m_prev - m_simplex);
		callback (userData, serialization, sizeof (serialization));
	}

	for (dgInt32 i = 0; i < m_faceCount; i ++) {
		dgInt32 faceOffset;
		faceOffset = dgInt32 (m_faceArray[i] - m_simplex); 
		callback (userData, &faceOffset, sizeof (dgInt32));
	}
}



bool dgCollisionConvexHull::OOBBTest (const dgMatrix& matrix, const dgCollisionConvex* const shape, void* const cacheOrder) const
{
	bool ret;
	_DG_ASSERTE (cacheOrder);

	ret = dgCollisionConvex::OOBBTest (matrix, shape, cacheOrder);
	if (ret) {
		const dgConvexSimplexEdge* const* faceArray = m_faceArray;
		dgCollisionBoundPlaneCache* const cache = (dgCollisionBoundPlaneCache*)cacheOrder;

		for (dgInt32 i = 0; i < dgInt32 (sizeof (cache->m_planes) / sizeof (dgPlane)); i ++) {
			dgFloat32 dist;
			const dgPlane& plane = cache->m_planes[i];
			if ((plane % plane) > dgFloat32 (0.0f)) {
				dgVector dir (matrix.UnrotateVector(plane.Scale (-1.0f)));
				dir.m_w = dgFloat32 (0.0f);
				dgVector p (matrix.TransformVector (shape->SupportVertex(dir)));
				dist = plane.Evalue (p);
				if (dist > dgFloat32 (0.1f)){
					return false;
				} 
			}
		}

		for (dgInt32 i = 0; i < m_boundPlanesCount; i ++) {
			dgInt32 i0;
			dgInt32 i1;
			dgInt32 i2;
			dgFloat32 dist;

			const dgConvexSimplexEdge* const face = faceArray[i];
			i0 = face->m_prev->m_vertex;
			i1 = face->m_vertex;
			i2 = face->m_next->m_vertex;
			const dgVector& p0 = m_vertex[i0];

			dgVector normal ((m_vertex[i1] - p0) * (m_vertex[i2] - p0));
			normal = normal.Scale (dgFloat32 (1.0f) / dgSqrt (normal % normal));

			dgVector dir (matrix.UnrotateVector(normal.Scale (-1.0f)));
			dir.m_w = dgFloat32 (0.0f);
			dgVector p (matrix.TransformVector (shape->SupportVertex(dir)));

			//_DG_ASSERTE ((normal % (m_boxOrigin - p0)) < 0.0f);
			dist = normal % (p - p0);
			if (dist > dgFloat32 (0.1f)){
				for (dgInt32 j = 0; j < (dgInt32 (sizeof (cache->m_planes) / sizeof (dgPlane)) - 1); j ++) {
					cache->m_planes[j + 1] = cache->m_planes[j];
				}
				cache->m_planes[1] = dgPlane (normal, - (normal % p0));
				return false;
			} 
		}
	}
	return ret;
}

