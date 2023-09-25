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
#include "dgContact.h"
#include "dgCollisionSphere.h"
#include "dgCollisionMesh.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////



#define DG_CONVEX_POLYGON_CRC 0x12341234

dgCollisionMesh::dgCollisionConvexPolygon::dgCollisionConvexPolygon (dgMemoryAllocator* const allocator)
	:dgCollisionConvex (allocator, DG_CONVEX_POLYGON_CRC, dgGetIdentityMatrix(), m_polygonCollision)
{
	m_count = 0;
	m_index = 0;
	m_vertex = NULL;
	m_stride = 0;
	m_paddedCount = 0;

	m_rtti |= dgCollisionConvexPolygon_RTTI;
	memset (m_localPoly, 0, sizeof (m_localPoly));
	m_normal = dgVector (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));
//	m_aabbP0 = dgVector (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));
//	m_aabbP1 = dgVector (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));
}

dgCollisionMesh::dgCollisionConvexPolygon::~dgCollisionConvexPolygon ()
{

}


dgInt32 dgCollisionMesh::dgCollisionConvexPolygon::CalculateSignature () const
{
	return DG_CONVEX_POLYGON_CRC;
}

void dgCollisionMesh::dgCollisionConvexPolygon::SetCollisionBBox (const dgVector& p0__, const dgVector& p1__)
{
	_DG_ASSERTE (0);
}

void dgCollisionMesh::dgCollisionConvexPolygon::Serialize(dgSerialize callback, void* const userData) const
{
	_DG_ASSERTE (0);
}

dgFloat32 dgCollisionMesh::dgCollisionConvexPolygon::RayCast (
	const dgVector& localP0, 
	const dgVector& localP1, 
	dgContactPoint& contactOut,
	OnRayPrecastAction preFilter, 
	const dgBody* const body,
	void* userData) const
{
	_DG_ASSERTE (0);
	return dgFloat32 (0.0f);
}

dgFloat32 dgCollisionMesh::dgCollisionConvexPolygon::GetVolume () const
{
	_DG_ASSERTE (0);
	return dgFloat32 (0.0f); 
}

dgFloat32 dgCollisionMesh::dgCollisionConvexPolygon::GetBoxMinRadius () const
{
	_DG_ASSERTE (0);
	return dgFloat32 (0.0f);  
}

dgFloat32 dgCollisionMesh::dgCollisionConvexPolygon::GetBoxMaxRadius () const
{
	_DG_ASSERTE (0);
	return dgFloat32 (0.0f);  
}

bool dgCollisionMesh::dgCollisionConvexPolygon::OOBBTest (const dgMatrix& matrix, const dgCollisionConvex* const shape, void* chache) const
{
	_DG_ASSERTE (0);
	return true;
}


void dgCollisionMesh::dgCollisionConvexPolygon::CalculateInertia (dgVector& inertia, dgVector& origin) const
{
	inertia.m_x = dgFloat32 (0.0f);
	inertia.m_y = dgFloat32 (0.0f);
	inertia.m_z = dgFloat32 (0.0f);

	origin.m_x = dgFloat32 (0.0f);
	origin.m_y = dgFloat32 (0.0f);
	origin.m_z = dgFloat32 (0.0f);
}

	
dgVector dgCollisionMesh::dgCollisionConvexPolygon::SupportVertex (const dgVector& dir) const
{
	dgInt32 i;
//	dgInt32 j;
	dgInt32 index;
	dgFloat32 val;
	dgFloat32 val1;

	_DG_ASSERTE (dgAbsf (dir % dir - 1.0f) < dgFloat32 (1.0e-2f));
	index = 0;
	val = m_localPoly[0] % dir;
	for (i = 1; i < m_count; i ++) {
		val1 = m_localPoly[i] % dir;
		if (val1 > val) {
			val = val1; 
			index = i;
		}
	}
	return m_localPoly[index];
}

dgVector dgCollisionMesh::dgCollisionConvexPolygon::SupportVertexSimd (const dgVector& dir) const
{
#ifdef DG_BUILD_SIMD_CODE	
//	dgInt32 i;
//	dgFloat32 fIndex;
//	simd_type dot;
//	simd_type dot1;
//	simd_type dirX;
//	simd_type dirY;
//	simd_type dirZ;
//	simd_type index;
//	simd_type indexAcc;
//	simd_type mask;
	

	_DG_ASSERTE (dgAbsf (dir % dir - 1.0f) < dgFloat32 (1.0e-3f));

	simd_type dirX = simd_permut_v (*(simd_type*)&dir, *(simd_type*)&dir, PURMUT_MASK(0, 0, 0, 0));
	simd_type dirY = simd_permut_v (*(simd_type*)&dir, *(simd_type*)&dir, PURMUT_MASK(1, 1, 1, 1));
	simd_type dirZ = simd_permut_v (*(simd_type*)&dir, *(simd_type*)&dir, PURMUT_MASK(2, 2, 2, 2));

	simd_type dot = simd_mul_add_v  (simd_mul_add_v (simd_mul_v (dirX, *(simd_type*)&m_localPolySimd[0]), 
													             dirY, *(simd_type*)&m_localPolySimd[1]),
													             dirZ, *(simd_type*)&m_localPolySimd[2]);
	simd_type index = *(simd_type*)&m_index_0123;
	simd_type indexAcc = index; 
	for (dgInt32 i = 3; i < m_paddedCount; i += 3) {
		indexAcc = simd_add_v (indexAcc, *(simd_type*)&m_indexStep);
		simd_type dot1 = simd_mul_add_v  (simd_mul_add_v (simd_mul_v (dirX, *(simd_type*)&m_localPolySimd[i + 0]), 
														              dirY, *(simd_type*)&m_localPolySimd[i + 1]),
														              dirZ, *(simd_type*)&m_localPolySimd[i + 2]);
		simd_type mask = simd_cmpgt_v(dot1, dot);
		dot = simd_max_v(dot1, dot);
		index = simd_or_v (simd_and_v(indexAcc, mask), simd_andnot_v (index, mask));
	}

	dirX = simd_permut_v (dot, dot, PURMUT_MASK(0, 0, 3, 2));
	simd_type mask = simd_cmpge_v(dot, dirX);
	dot = simd_max_v(dot, dirX);
	index = simd_or_v (simd_and_v(index, mask), simd_andnot_v (simd_permut_v (index, index, PURMUT_MASK(0, 0, 3, 2)), mask));

	mask = simd_cmpge_s(dot, simd_permut_v (dot, dot, PURMUT_MASK(0, 0, 0, 1)));

	dgInt32 i = simd_store_is (simd_or_v (simd_and_v(index, mask), simd_andnot_v (simd_permut_v (index, index, PURMUT_MASK(0, 0, 0, 1)), mask)));
//	dgFloat32 fIndex;
//	simd_store_s (simd_or_v (simd_and_v(index, mask), simd_andnot_v (simd_permut_v (index, index, PURMUT_MASK(0, 0, 0, 1)), mask)), &fIndex);
//	dgInt32 i = dgFastInt (fIndex);
	return m_localPoly[i]; 

#else
	return dgVector (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));
#endif
}


void dgCollisionMesh::dgCollisionConvexPolygon::CalculateNormalSimd()
{
	//	CalculateNormal();
#ifdef DG_BUILD_SIMD_CODE	
	if (m_normalIndex) {
		m_normal = dgVector (&m_vertex[m_normalIndex * m_stride]);
	} else {
		simd_type e10;
		simd_type e21;
		simd_type tmp0;
		simd_type mag2;
		simd_type normal;

		e10 = simd_sub_v (*(simd_type*)&m_localPoly[1], *(simd_type*)&m_localPoly[0]); 
		e21 = simd_sub_v (*(simd_type*)&m_localPoly[2], *(simd_type*)&m_localPoly[1]); 
		normal = simd_mul_sub_v (simd_mul_v (simd_permut_v(e10, e10, PURMUT_MASK(3, 0, 2, 1)), simd_permut_v(e21, e21, PURMUT_MASK(3, 1, 0, 2))), 
			simd_permut_v(e10, e10, PURMUT_MASK(3, 1, 0, 2)), simd_permut_v(e21, e21, PURMUT_MASK(3, 0, 2, 1)));

		_DG_ASSERTE (((dgFloat32*)&normal)[3] == dgFloat32 (0.0f));
		mag2 = simd_mul_v (normal, normal);
		mag2 = simd_add_v (mag2, simd_move_hl_v (mag2, mag2));
		mag2 = simd_sub_s (simd_add_s (mag2, simd_permut_v (mag2, mag2, PURMUT_MASK (3,3,3,1))), *(simd_type*)&m_negativeTiny);

		tmp0 = simd_rsqrt_s(mag2);
		mag2 = simd_mul_s (simd_mul_s(*(simd_type*)&m_nrh0p5, tmp0), simd_mul_sub_s (*(simd_type*)&m_nrh3p0, simd_mul_s (mag2, tmp0), tmp0));
		(*(simd_type*)&m_normal) = simd_mul_v (normal, simd_permut_v(mag2, mag2, PURMUT_MASK(3, 0, 0, 0)));
	}
#endif
}


void dgCollisionMesh::dgCollisionConvexPolygon::CalculateNormal()
{
	if (m_normalIndex) {
		m_normal = dgVector (&m_vertex[m_normalIndex * m_stride]);
	} else {
		//		dgInt32 i2;
		//		i2 = m_index[2] * m_stride;
		dgVector e10 (m_localPoly[1] - m_localPoly[0]);
		dgVector e21 (m_localPoly[2] - m_localPoly[1]);
		dgVector normal (e10 * e21);
		_DG_ASSERTE ((normal % normal) > dgFloat32 (0.0f));
		m_normal = normal.Scale (dgRsqrt (normal % normal + dgFloat32 (1.0e-24f)));
	}
}



dgInt32 dgCollisionMesh::dgCollisionConvexPolygon::QuickTestContinueSimd (
	const dgCollisionConvex* hull, 
	const dgMatrix& matrix)
{

#ifdef DG_BUILD_SIMD_CODE
	dgInt32 ret;
	dgFloat32 val1;
	m_localPoly[0] = dgVector (&m_vertex[m_index[0] * m_stride]);
	m_localPoly[1] = dgVector (&m_vertex[m_index[1] * m_stride]);
	m_localPoly[2] = dgVector (&m_vertex[m_index[2] * m_stride]);
	CalculateNormalSimd();

	dgVector p1 (matrix.UntransformVector (hull->SupportVertexSimd (matrix.RotateVectorSimd (m_normal))));

	val1 = (p1 - m_localPoly[0]) % m_normal;
	ret = (val1 >= dgFloat32 (0.0f));

	if (ret) {
		dgInt32 i1;
		dgInt32 i0; 
		for (i1 = 3; i1 < m_count; i1 ++) {
			m_localPoly[i1] = dgVector (&m_vertex[m_index[i1] * m_stride]);
		}

		i0 = (m_count + 3) & -4;
		for (; i1 < i0; i1 ++) {
			m_localPoly[i1] = m_localPoly[0];
		}

		i1 = 0;
		for (dgInt32 i = 0; i < i0; i += 4) {
			m_localPolySimd[i1 + 0] = dgVector (m_localPoly[i + 0].m_x, m_localPoly[i + 1].m_x, m_localPoly[i + 2].m_x, m_localPoly[i + 3].m_x); 
			m_localPolySimd[i1 + 1] = dgVector (m_localPoly[i + 0].m_y, m_localPoly[i + 1].m_y, m_localPoly[i + 2].m_y, m_localPoly[i + 3].m_y); 
			m_localPolySimd[i1 + 2] = dgVector (m_localPoly[i + 0].m_z, m_localPoly[i + 1].m_z, m_localPoly[i + 2].m_z, m_localPoly[i + 3].m_z); 
			i1 += 3;
		}
		m_paddedCount = i1;
	}

	return ret;
#else
	return 0;
#endif
}


dgInt32 dgCollisionMesh::dgCollisionConvexPolygon::QuickTestContinue (
	const dgCollisionConvex* hull, 
	const dgMatrix& matrix)
{
	dgInt32 ret;
	dgFloat32 val1;

	_DG_ASSERTE (m_count < (sizeof (m_localPoly) / sizeof (m_localPoly[0])));
	m_localPoly[0] = dgVector (&m_vertex[m_index[0] * m_stride]);
	m_localPoly[1] = dgVector (&m_vertex[m_index[1] * m_stride]);
	m_localPoly[2] = dgVector (&m_vertex[m_index[2] * m_stride]);
	CalculateNormal();

	dgVector p1 (matrix.UntransformVector (hull->SupportVertex (matrix.RotateVector (m_normal))));

	val1 = (p1 - m_localPoly[0]) % m_normal;
	ret = (val1 >= dgFloat32 (0.0f));
	if (ret) {
		for (dgInt32 i = 3; i < m_count; i ++) {
			m_localPoly[i] = dgVector (&m_vertex[m_index[i] * m_stride]);
		}
	}

	return ret;

}



dgInt32 dgCollisionMesh::dgCollisionConvexPolygon::QuickTestSimd (
	const dgCollisionConvex* hull, 
	const dgMatrix& matrix)
{
#ifdef DG_BUILD_SIMD_CODE	
	dgInt32 i; 
//	dgInt32 i0; 
//	dgInt32 i1; 
	dgFloat32 val0;
	dgFloat32 val1;
	simd_type normal;
	simd_type normal1;
	dgVector rotatedNormal;

	_DG_ASSERTE (m_count < (sizeof (m_localPoly) / sizeof (m_localPoly[0])));

	m_localPoly[0] = dgVector (&m_vertex[m_index[0] * m_stride]);
	m_localPoly[1] = dgVector (&m_vertex[m_index[1] * m_stride]);
	m_localPoly[2] = dgVector (&m_vertex[m_index[2] * m_stride]);
	CalculateNormalSimd();

	//	rotatedNormal = matrix.RotateVector (normal__);
	normal  = simd_mul_v (*(simd_type*)&m_normal, *(simd_type*)&m_negOne);
	normal1 = simd_mul_add_v (simd_mul_add_v (simd_mul_v (*(simd_type*)&matrix[0], simd_permut_v(normal, normal, PURMUT_MASK(3, 0, 0, 0))), 
		*(simd_type*)&matrix[1], simd_permut_v(normal, normal, PURMUT_MASK(3, 1, 1, 1))), 
		*(simd_type*)&matrix[2], simd_permut_v(normal, normal, PURMUT_MASK(3, 2, 2, 2)));
	(*(simd_type*)&rotatedNormal) = normal1;
	dgVector p0 (matrix.UntransformVector (hull->SupportVertexSimd(rotatedNormal)));

	(*(simd_type*)&rotatedNormal) = simd_mul_v (normal1, *(simd_type*)&m_negOne);
	dgVector p1 (matrix.UntransformVector (hull->SupportVertexSimd(rotatedNormal)));

	val0 = (m_localPoly[0] - p0) % m_normal + dgFloat32 (1.0e-1f);
	val1 = (m_localPoly[0] - p1) % m_normal - dgFloat32 (1.0e-1f);
	if (val0 * val1 >= dgFloat32 (0.0f)) {
		return 0;
	}

	for (i = 3; i < m_count; i ++) {
		m_localPoly[i] = dgVector (&m_vertex[m_index[i] * m_stride]);
	}

	dgInt32 i0 = (m_count + 3) & -4;
	for (; i < i0; i ++) {
		m_localPoly[i] = m_localPoly[0];
	}

	dgInt32 i1 = 0;
	for (dgInt32 i = 0; i < i0; i += 4) {
		m_localPolySimd[i1 + 0] = dgVector (m_localPoly[i + 0].m_x, m_localPoly[i + 1].m_x, m_localPoly[i + 2].m_x, m_localPoly[i + 3].m_x); 
		m_localPolySimd[i1 + 1] = dgVector (m_localPoly[i + 0].m_y, m_localPoly[i + 1].m_y, m_localPoly[i + 2].m_y, m_localPoly[i + 3].m_y); 
		m_localPolySimd[i1 + 2] = dgVector (m_localPoly[i + 0].m_z, m_localPoly[i + 1].m_z, m_localPoly[i + 2].m_z, m_localPoly[i + 3].m_z); 
		i1 += 3;
	}

	m_paddedCount = i1;
	return 1;

#else
	return 0;
#endif
}


dgInt32 dgCollisionMesh::dgCollisionConvexPolygon::QuickTest (const dgCollisionConvex* hull, const dgMatrix& matrix)
{
	dgFloat32 val0;
	dgFloat32 val1;

	_DG_ASSERTE (m_count < (sizeof (m_localPoly) / sizeof (m_localPoly[0])));

	m_localPoly[0] = dgVector (&m_vertex[m_index[0] * m_stride]);
	m_localPoly[1] = dgVector (&m_vertex[m_index[1] * m_stride]);
	m_localPoly[2] = dgVector (&m_vertex[m_index[2] * m_stride]);
	CalculateNormal();

	dgVector normal (m_normal.Scale (dgFloat32 (-1.0f)));
	dgVector rotatedNormal (matrix.RotateVector (m_normal));
	dgVector p0 (matrix.UntransformVector (hull->SupportVertex (rotatedNormal.Scale (dgFloat32 (-1.0f)))));
	dgVector p1 (matrix.UntransformVector (hull->SupportVertex (rotatedNormal)));

	val0 = (p0 - m_localPoly[0]) % normal + dgFloat32 (1.0e-1f);
	val1 = (p1 - m_localPoly[0]) % normal - dgFloat32 (1.0e-1f);
	if (val0 * val1 >= dgFloat32 (0.0f)) {
		return 0;
	}

	for (dgInt32 i = 3; i < m_count; i ++) {
		m_localPoly[i] = dgVector (&m_vertex[m_index[i] * m_stride]);
	}

	return 1;
}



dgInt32 dgCollisionMesh::dgCollisionConvexPolygon::ClipContacts (
	dgInt32 count, 
	dgContactPoint* const contactOut,
	const dgMatrix& globalMatrix) const
{
	dgVector normal (globalMatrix.RotateVector(m_normal));
	if (m_normalIndex) {
		for (dgInt32 i = 0; i < count; i ++) {
			dgFloat32 dist;
			dist = contactOut[i].m_normal % normal;
			contactOut[i].m_isEdgeContact = 0;
			if (dist <= dgFloat32 (0.9998f)) {
				dgInt32 j0;
				dgInt32 closestEdgeIndex;
				dgFloat32 closestEdgeDist;
				dgVector point (globalMatrix.UntransformVector(contactOut[i].m_point));

				j0 = m_count - 1;
				closestEdgeIndex = 0;
				contactOut[i].m_isEdgeContact = 1;
				closestEdgeDist = dgFloat32 (1.0e20f);
				for (dgInt32 j1 = 0; j1 < m_count; j1 ++) {
					dgFloat32 dist2;
					dgVector edge (m_localPoly[j1] - m_localPoly[j0]);
					dgVector dp (point - m_localPoly[j0]);
					dgVector p (dp - edge.Scale ((dp % edge) / (edge % edge)));
					dist2 = p % p;
					if (dist2 < closestEdgeDist) {
						closestEdgeDist = dist2;
						closestEdgeIndex = j0;
					}
					j0 = j1;
				}
				_DG_ASSERTE (m_adjacentNormalIndex);
				if ((closestEdgeDist > (dgFloat32 (0.25f) * dgFloat32 (0.25f))) || (m_adjacentNormalIndex[closestEdgeIndex] == -1)) {
					contactOut[i].m_normal = normal;
				} else {
					dgFloat32 dist;
					dgVector aNormal (globalMatrix.RotateVector(dgVector (&m_vertex[m_adjacentNormalIndex[closestEdgeIndex] * m_stride])));
					dgVector side (normal * aNormal);
					dist = side % side;
					if (dist < dgFloat32 (0.05f * 0.05f)) {
						normal += aNormal;
						contactOut[i].m_normal = normal.Scale (dgRsqrt (normal % normal));
					} else {
						dgVector plane (aNormal * side);
						dist = contactOut[i].m_normal % plane;
						if (dist < dgFloat32 (0.0f)) {
							contactOut[i].m_normal = aNormal;
						}
					}
				}
			}
		}
	} else {

		for (dgInt32 i = 0; i < count; i ++) {
			dgFloat32 dist;
			dist = contactOut[i].m_normal % normal;
			contactOut[i].m_isEdgeContact = (dist < dgFloat32 (0.999f));	
			if (dist < dgFloat32 (0.1f)) {
				contactOut[i] = contactOut[count - 1];
				count --;
				i --;
			}
		}

	}
	return count;
}

void dgCollisionMesh::dgCollisionConvexPolygon::BeamClippingSimd (const dgCollisionConvex* hull, const dgMatrix& matrix, dgFloat32 dist)
{
	BeamClipping (hull, matrix, dist);

	dgInt32 i0 = (m_count + 3) & -4;
	for (dgInt32 i = m_count; i < i0; i ++) {
		m_localPoly[i] = m_localPoly[0];
	}

	dgInt32 i1 = 0;
	for (dgInt32 i = 0; i < i0; i += 4) {
		m_localPolySimd[i1 + 0] = dgVector (m_localPoly[i + 0].m_x, m_localPoly[i + 1].m_x, m_localPoly[i + 2].m_x, m_localPoly[i + 3].m_x); 
		m_localPolySimd[i1 + 1] = dgVector (m_localPoly[i + 0].m_y, m_localPoly[i + 1].m_y, m_localPoly[i + 2].m_y, m_localPoly[i + 3].m_y); 
		m_localPolySimd[i1 + 2] = dgVector (m_localPoly[i + 0].m_z, m_localPoly[i + 1].m_z, m_localPoly[i + 2].m_z, m_localPoly[i + 3].m_z); 
		i1 += 3;
	}

	m_paddedCount = i1;
}


void dgCollisionMesh::dgCollisionConvexPolygon::BeamClipping (const dgCollisionConvex* hull, const dgMatrix& matrix, dgFloat32 dist)
{
	dgPlane planes[4];
	dgVector points[sizeof (m_localPoly) / sizeof (m_localPoly[0]) + 8];
	DG_CLIPPED_FACE_EDGE clippedFace [sizeof (m_localPoly) / sizeof (m_localPoly[0]) + 8];

	dgVector origin (matrix.UnrotateVector (matrix.m_posit.Scale (dgFloat32 (-1.0f))));	 
	dgVector dir (m_localPoly[1] - m_localPoly[0]);

	_DG_ASSERTE ((dir % dir) > dgFloat32 (1.0e-8f));
	dir = dir.Scale (dgRsqrt (dir % dir));

	dgFloat32 test (dir % origin);
	planes[0] = dgPlane (dir, dist - test);
	planes[2] = dgPlane (dir.Scale (dgFloat32 (-1.0f)), dist + test);

	dir = m_normal * dir;

	test = dir % origin;
	planes[1] = dgPlane (dir, dist - test);
	planes[3] = dgPlane (dir.Scale (dgFloat32 (-1.0f)), dist + test);

	for (dgInt32 i = 0; i < m_count; i ++) {
		dgInt32 j = i << 1;
		points[i] = m_localPoly[i];
		
		clippedFace[j + 0].m_twin = &clippedFace[j + 1];
		clippedFace[j + 0].m_next = &clippedFace[j + 2];
		clippedFace[j + 0].m_incidentVertex = i;
		clippedFace[j + 0].m_incidentNormal = m_adjacentNormalIndex ? m_adjacentNormalIndex[i] : -1;

		clippedFace[j + 1].m_twin = &clippedFace[j + 0];
		clippedFace[j + 1].m_next = &clippedFace[j - 2];
		clippedFace[j + 1].m_incidentVertex = i + 1;
		clippedFace[j + 0].m_incidentNormal = -1;
	}

	clippedFace[1].m_next = &clippedFace[m_count * 2 - 2 + 1];
	clippedFace[m_count * 2 - 2].m_next = &clippedFace[0];
	clippedFace[m_count * 2 - 2 + 1].m_incidentVertex = 0;

	dgInt32 edgeCount = m_count * 2;
	dgInt32 indexCount = m_count;
	DG_CLIPPED_FACE_EDGE* first = &clippedFace[0];
	for (dgInt32 i = 0; i < 4; i ++) {
		const dgPlane& plane = planes[i];
		_DG_ASSERTE (plane.Evalue(origin) > dgFloat32 (0.0f));


		dgInt32 conectCount = 0;
		DG_CLIPPED_FACE_EDGE* connect[2];
		DG_CLIPPED_FACE_EDGE* ptr = first;
		DG_CLIPPED_FACE_EDGE* newFirst = first;
		dgFloat32 test0 = plane.Evalue(points[ptr->m_incidentVertex]);
		do {
			dgFloat32 test1 = plane.Evalue(points[ptr->m_next->m_incidentVertex]);

			if (test0 > dgFloat32 (0.0f)) {
				if (test1 <= dgFloat32 (0.0f)) {
					const dgVector& p0 = points[ptr->m_incidentVertex];
					const dgVector& p1 = points[ptr->m_next->m_incidentVertex];
					dgVector dp (p1 - p0); 
					points[indexCount] = p0 - dp.Scale (test0  / (dp % plane));

					DG_CLIPPED_FACE_EDGE* const newEdge = &clippedFace[edgeCount];
					newEdge->m_twin = newEdge + 1;
					newEdge->m_twin->m_twin = newEdge;

					newEdge->m_twin->m_incidentNormal = -1;
					newEdge->m_incidentNormal = ptr->m_incidentNormal;

					newEdge->m_incidentVertex = indexCount;
					newEdge->m_twin->m_incidentVertex = ptr->m_next->m_incidentVertex;
					ptr->m_twin->m_incidentVertex = indexCount;

					newEdge->m_next = ptr->m_next;
					ptr->m_next->m_twin->m_next = newEdge->m_twin;
					newEdge->m_twin->m_next = ptr->m_twin;
					ptr->m_next = newEdge;

					connect[conectCount] = ptr;
					conectCount ++;
					indexCount ++;
					edgeCount += 2;
					ptr = newEdge;
				}
			} else {
				if (test1 > dgFloat32 (0.0f)) {
					newFirst = ptr->m_next;

					const dgVector& p0 = points[ptr->m_incidentVertex];
					const dgVector& p1 = points[ptr->m_next->m_incidentVertex];
					dgVector dp (p1 - p0); 
					points[indexCount] = p0 - dp.Scale (test0  / (dp % plane));

					DG_CLIPPED_FACE_EDGE* const newEdge = &clippedFace[edgeCount];
					newEdge->m_twin = newEdge + 1;
					newEdge->m_twin->m_twin = newEdge;

					newEdge->m_twin->m_incidentNormal = -1;
					newEdge->m_incidentNormal = ptr->m_incidentNormal;

					newEdge->m_incidentVertex = indexCount;
					newEdge->m_twin->m_incidentVertex = ptr->m_next->m_incidentVertex;
					ptr->m_twin->m_incidentVertex = indexCount;

					newEdge->m_next = ptr->m_next;
					ptr->m_next->m_twin->m_next = newEdge->m_twin;
					newEdge->m_twin->m_next = ptr->m_twin;
					ptr->m_next = newEdge;

					connect[conectCount] = ptr;
					conectCount ++;
					indexCount ++;
					edgeCount += 2;

					ptr = newEdge;
				}
			}


			test0 = test1;
			ptr = ptr->m_next;
		} while (ptr != first);

		if(conectCount) {
			first = newFirst;
			_DG_ASSERTE (conectCount == 2);

			DG_CLIPPED_FACE_EDGE* const newEdge = &clippedFace[edgeCount];
			newEdge->m_twin = newEdge + 1;
			newEdge->m_twin->m_twin = newEdge;

			newEdge->m_incidentNormal = -1;
			newEdge->m_twin->m_incidentNormal = -1;

			newEdge->m_incidentVertex = connect[0]->m_next->m_incidentVertex;
			newEdge->m_twin->m_next = connect[0]->m_next;
			connect[0]->m_next = newEdge;

			newEdge->m_twin->m_incidentVertex = connect[1]->m_next->m_incidentVertex;
			newEdge->m_next = connect[1]->m_next;
			connect[1]->m_next = newEdge->m_twin;

			edgeCount += 2;
		}
	}

	dgInt32 count = 0;
	DG_CLIPPED_FACE_EDGE* ptr = first;
	if (m_adjacentNormalIndex) {
		m_adjacentNormalIndex = &m_clippEdgeNormal[0];
		do {
	_DG_ASSERTE (ptr->m_incidentNormal == -1);
			m_clippEdgeNormal[count] = ptr->m_incidentNormal;
			m_localPoly[count] = points[ptr->m_incidentVertex];
			count ++;
			ptr = ptr->m_next;
		} while (ptr != first);

	} else { 
		do {
			m_localPoly[count] = points[ptr->m_incidentVertex];
			count ++;
			ptr = ptr->m_next;
		} while (ptr != first);
	}
	m_count = count;
}



dgVector dgCollisionMesh::dgCollisionConvexPolygon::ClosestDistanceToTriangle (
	const dgVector& point,
	const dgVector& p0, 
	const dgVector& p1, 
	const dgVector& p2) const
{
	dgFloat32 t;
	dgFloat32 s;
	dgFloat32 vc;
	dgFloat32 vb;
	dgFloat32 va;
	dgFloat32 den;
	dgFloat32 alpha1;
	dgFloat32 alpha2;
	dgFloat32 alpha3;
	dgFloat32 alpha4;
	dgFloat32 alpha5;
	dgFloat32 alpha6;

	//	const dgVector p (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));
	const dgVector p10 (p1 - p0);
	const dgVector p20 (p2 - p0);
	const dgVector p_p0 (point - p0);

	alpha1 = p10 % p_p0;
	alpha2 = p20 % p_p0;
	if ((alpha1 <= dgFloat32 (0.0f)) && (alpha2 <= dgFloat32 (0.0f))) {
		return p0;
	}

	dgVector p_p1 (point - p1);
	alpha3 = p10 % p_p1;
	alpha4 = p20 % p_p1;
	if ((alpha3 >= dgFloat32 (0.0f)) && (alpha4 <= alpha3)) {
		return p1;
	}

	vc = alpha1 * alpha4 - alpha3 * alpha2;
	if ((vc <= dgFloat32 (0.0f)) && (alpha1 >= dgFloat32 (0.0f)) && (alpha3 <= dgFloat32 (0.0f))) {
		t = alpha1 / (alpha1 - alpha3);
		_DG_ASSERTE (t >= dgFloat32 (0.0f));
		_DG_ASSERTE (t <= dgFloat32 (1.0f));
		return p0 + p10.Scale (t);
	}


	dgVector p_p2 (point - p2);
	alpha5 = p10 % p_p2;
	alpha6 = p20 % p_p2;
	if ((alpha6 >= dgFloat32 (0.0f)) && (alpha5 <= alpha6)) {
		return p2;
	}


	vb = alpha5 * alpha2 - alpha1 * alpha6;
	if ((vb <= dgFloat32 (0.0f)) && (alpha2 >= dgFloat32 (0.0f)) && (alpha6 <= dgFloat32 (0.0f))) {
		t = alpha2 / (alpha2 - alpha6);
		_DG_ASSERTE (t >= dgFloat32 (0.0f));
		_DG_ASSERTE (t <= dgFloat32 (1.0f));
		return p0 + p20.Scale (t);
	}


	va = alpha3 * alpha6 - alpha5 * alpha4;
	if ((va <= dgFloat32 (0.0f)) && ((alpha4 - alpha3) >= dgFloat32 (0.0f)) && ((alpha5 - alpha6) >= dgFloat32 (0.0f))) {
		t = (alpha4 - alpha3) / ((alpha4 - alpha3) + (alpha5 - alpha6));
		_DG_ASSERTE (t >= dgFloat32 (0.0f));
		_DG_ASSERTE (t <= dgFloat32 (1.0f));
		return p1 + (p2 - p1).Scale (t);
	}

	den = float(1.0f) / (va + vb + vc);
	t = vb * den;
	s = vc * den;
	_DG_ASSERTE (t >= dgFloat32 (0.0f));
	_DG_ASSERTE (s >= dgFloat32 (0.0f));
	_DG_ASSERTE (t <= dgFloat32 (1.0f));
	_DG_ASSERTE (s <= dgFloat32 (1.0f));
	return p0 + p10.Scale (t) + p20.Scale (s);
}

bool dgCollisionMesh::dgCollisionConvexPolygon::PointToPolygonDistance (
	const dgVector& p, 
	dgFloat32 radius,
	dgVector& out)
{
	dgFloat32 minDist;

	minDist = dgFloat32 (1.0e20f);
	m_localPoly[0] = dgVector (&m_vertex[m_index[0] * m_stride]);
	m_localPoly[1] = dgVector (&m_vertex[m_index[1] * m_stride]);
//	m_localPoly[2] = dgVector (&m_vertex[m_index[2] * m_stride]);

	dgVector closestPoint (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));
	for (dgInt32 i2 = 2; i2 < m_count; i2 ++) {
		dgFloat32 dist;
		m_localPoly[i2] = dgVector (&m_vertex[m_index[i2] * m_stride]);
		const dgVector q (ClosestDistanceToTriangle (p, m_localPoly[0], m_localPoly[i2 - 1], m_localPoly[i2]));
		const dgVector error (q - p);
		dist = error % error;
		if (dist < minDist) {
			minDist = dist;
			closestPoint = q;
		}
	}

	if (minDist > (radius * radius)) {
		return false;
	}

	CalculateNormal();
	out = closestPoint;
	return true;
}

bool dgCollisionMesh::dgCollisionConvexPolygon::DistanceToOrigen (const dgMatrix& matrix, const dgVector& scale, dgFloat32 radius, dgVector& out)
{

	dgFloat32 minDist;

	minDist = dgFloat32 (1.0e20f);
	m_localPoly[0] = scale.CompProduct (matrix.TransformVector(dgVector (&m_vertex[m_index[0] * m_stride])));
	m_localPoly[1] = scale.CompProduct (matrix.TransformVector(dgVector (&m_vertex[m_index[1] * m_stride])));

	dgVector origin (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));
	dgVector closestPoint (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));
	for (dgInt32 i2 = 2; i2 < m_count; i2 ++) {
		dgFloat32 dist;
		m_localPoly[i2] = scale.CompProduct (matrix.TransformVector(dgVector (&m_vertex[m_index[i2] * m_stride])));
		const dgVector q (ClosestDistanceToTriangle (origin, m_localPoly[0], m_localPoly[i2 - 1], m_localPoly[i2]));
		const dgVector error (q - origin);
		dist = error % error;
		if (dist < minDist) {
			minDist = dist;
			closestPoint = q;
		}
	}

	if (minDist > (radius * radius)) {
		return false;
	}

//	CalculateNormal();
	m_localPoly[0] = dgVector (&m_vertex[m_index[0] * m_stride]);
	m_localPoly[1] = dgVector (&m_vertex[m_index[1] * m_stride]);
	m_localPoly[2] = dgVector (&m_vertex[m_index[2] * m_stride]);

	dgVector e10 (m_localPoly[1] - m_localPoly[0]);
	dgVector e21 (m_localPoly[2] - m_localPoly[1]);
	dgVector normal (e10 * e21);
	_DG_ASSERTE ((normal % normal) > dgFloat32 (0.0f));
	m_normal = normal.Scale (dgRsqrt (normal % normal + dgFloat32 (1.0e-24f)));

	out = closestPoint;
	return true;
}

dgFloat32 dgCollisionMesh::dgCollisionConvexPolygon::MovingPointToPolygonContact (
	const dgVector& p, 
	const dgVector& veloc, 
	dgFloat32 radius,
	dgContactPoint& contact)
{
	dgFloat32 minDist;
	dgFloat32 timestep;
	dgFloat32 projVeloc;

	m_localPoly[0] = dgVector (&m_vertex[m_index[0] * m_stride]);
	m_localPoly[1] = dgVector (&m_vertex[m_index[1] * m_stride]);
	m_localPoly[2] = dgVector (&m_vertex[m_index[2] * m_stride]);
	CalculateNormal();

	timestep = dgFloat32 (-1.0f);
	dgVector closestPoint (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));
	minDist = dgFloat32 (1.0e20f);
	for (dgInt32 j = 2; j < m_count; j ++) {
		dgFloat32 dist;
		m_localPoly[j] = dgVector (&m_vertex[m_index[j] * m_stride]);
		const dgVector q (ClosestDistanceToTriangle (p, m_localPoly[0], m_localPoly[j - 1], m_localPoly[j]));
		const dgVector error (q - p);
		dist = error % error;
		if (dist < minDist) {
			minDist = dist;
			closestPoint = q;
		}
	}

	if (minDist <= (radius * radius)) {
		dgFloat32 dist2;
		dgVector dp (p - closestPoint);
		dist2 = dp % dp;
		if (dist2 > dgFloat32 (0.0f)) {
			dgFloat32 side;
			dgFloat32 dist2Inv;
			_DG_ASSERTE (dist2 > dgFloat32 (0.0f));
			dist2Inv = dgRsqrt (dist2);
			side =  dist2 * dist2Inv - radius;
			if (side < (-DG_RESTING_CONTACT_PENETRATION)) {
				dgVector normal (dp.Scale (dist2Inv));

				side = (dgAbsf (side) - DG_IMPULSIVE_CONTACT_PENETRATION);
				if (side < dgFloat32 (0.0f)) {
					side = dgFloat32 (0.0f);
				}

				timestep = dgFloat32 (0.0f);
				contact.m_point = p - normal.Scale (radius + side * dgFloat32 (0.5f));
				contact.m_normal = normal;
				contact.m_penetration = side;
			}
		}
	}

	if (timestep < 0.0f) {
		projVeloc = veloc % m_normal;
		if (projVeloc < dgFloat32 (-1.0e-1f)) {
			dgFloat32 minDist;
			dgFloat32 timeToImpact;
			dgVector supportPoint (p - m_normal.Scale (radius));

			timeToImpact = -(m_normal % (supportPoint - m_localPoly[0])) / (m_normal % veloc); 
			dgVector point (supportPoint + veloc.Scale (timeToImpact));
			dgVector closestPoint (point);
			minDist = dgFloat32 (1.0e20f);
			for (int i = 2; i < m_count; i ++) {
				dgInt32 i2;
				dgFloat32 dist;
				i2 = m_index[i] * m_stride;
				const dgVector q (ClosestDistanceToTriangle (point, m_localPoly[0], m_localPoly[i - 1], m_localPoly[i]));
				const dgVector error (q - point);
				dist = error % error;
				if (dist < minDist) {
					minDist = dist;
					closestPoint = q;
				}
			}

			if (minDist < dgFloat32 (1.0e-3f) ) {
				timestep = GetMax (timeToImpact, dgFloat32 (0.0f));
				contact.m_normal = m_normal;
				contact.m_penetration = dgFloat32 (0.0f);
				contact.m_point = (closestPoint + supportPoint).Scale (dgFloat32 (0.5f));
			} else {
				dgFloat32 a;
				dgFloat32 b;
				dgFloat32 c;
				dgFloat32 desc;
				dgVector dp (closestPoint - p);

				a = veloc % veloc;
				b = - dgFloat32 (2.0f) * (dp % veloc);
				c = dp % dp - radius * radius;

				desc = b * b - dgFloat32 (4.0f) * a * c;
				if (desc >= dgFloat32 (0.0f)) {
					dgFloat32 t;
					desc = dgSqrt (desc);
					t = dgFloat32 (0.5f) * GetMin ((b + desc), (b - desc)) / a;
					if (t >= 0.0f) {
						timestep = t;
						_DG_ASSERTE (timestep > dgFloat32 (0.0f));
						contact.m_penetration = dgFloat32 (0.0f);
						contact.m_point = closestPoint + veloc.Scale (timestep * dgFloat32 (0.5f));
						contact.m_normal = dp.Scale (dgRsqrt (dp % dp));
					}
				}
			}
		}
	}
	return timestep;
}



dgInt32 dgCollisionMesh::dgCollisionConvexPolygon::CalculatePlaneIntersection (
	const dgVector& normalIn, 
	const dgVector& origin, 
	dgVector contactsOut[]) const
{
	dgInt32 i;
	dgInt32 count;
	dgFloat32 t;
	dgFloat32 side0;
	dgFloat32 side1;
	dgFloat32 error;
	dgFloat32 maxDist;
	dgFloat32 projectFactor;
	dgVector normal(normalIn);

	count = 0;
	maxDist = dgFloat32 (1.0f);
	
	projectFactor = m_normal % normal;
	if (projectFactor < dgFloat32 (0.0f)) {
		projectFactor *= dgFloat32 (-1.0f);
		normal = normal.Scale (dgFloat32 (-1.0f));
	}

	if (projectFactor > dgFloat32 (0.9999f)) {
		for (i = 0; i < m_count; i ++) {
			contactsOut[count] = m_localPoly[i];
			count ++;
		}

		#ifdef _DG_DEBUG
		dgInt32 j;
		j = count - 1;
		for (i = 0; i < count; i ++) {
			dgVector error (contactsOut[i] - contactsOut[j]);
			_DG_ASSERTE ((error % error) > dgFloat32 (1.0e-20f));
			j = i;
		}
		#endif

	} else if (projectFactor > dgFloat32 (0.1736f)) {
		maxDist = dgFloat32 (0.0f);
		dgPlane plane (normal, - (normal % origin));
		
		dgVector p0 (m_localPoly[m_count - 1]);
		side0 = plane.Evalue (p0);
		for (i = 0; i < m_count; i ++) {
			dgVector p1 (m_localPoly[i]);
			side1 = plane.Evalue (p1);

			if (side0 > dgFloat32 (0.0f)) {
				maxDist = GetMax (maxDist, side0);
				contactsOut[count] = p0 - plane.Scale (side0);
				count ++;
				if (count > 1) {
					dgVector edgeSegment (contactsOut[count - 1] - contactsOut[count - 2]);
					error = edgeSegment % edgeSegment;
					if (error < dgFloat32 (1.0e-8f)) {
						count --;
					}
				}

				if (side1 <= dgFloat32 (0.0f)) {
					dgVector dp (p1 - p0);
					t = plane % dp;
					_DG_ASSERTE (dgAbsf (t) >= dgFloat32 (0.0f));
					if (dgAbsf (t) < dgFloat32 (1.0e-8f)) {
						t = GetSign(t) * dgFloat32 (1.0e-8f);	
					}
					contactsOut[count] = p0 - dp.Scale (side0 / t);
					count ++;
					if (count > 1) {
						dgVector edgeSegment (contactsOut[count - 1] - contactsOut[count - 2]);
						error = edgeSegment % edgeSegment;
						if (error < dgFloat32 (1.0e-8f)) {
							count --;
						}
					}
				} 
			} else if (side1 > dgFloat32 (0.0f)) {
				dgVector dp (p1 - p0);
				t = plane % dp;
				_DG_ASSERTE (dgAbsf (t) >= dgFloat32 (0.0f));
				if (dgAbsf (t) < dgFloat32 (1.0e-8f)) {
					t = GetSign(t) * dgFloat32 (1.0e-8f);	
				}
				contactsOut[count] = p0 - dp.Scale (side0 / t);
				count ++;
				if (count > 1) {
					dgVector edgeSegment (contactsOut[count - 1] - contactsOut[count - 2]);
					error = edgeSegment % edgeSegment;
					if (error < dgFloat32 (1.0e-8f)) {
						count --;
					}
				}
			}

			side0 = side1;
			p0 = p1;
		}
	}


	if (count > 1) {
		if (maxDist < dgFloat32 (1.0e-3f)) {
			dgFloat32 proj;
			dgFloat32 maxProjection;
			dgFloat32 minProjection;
			dgVector maxPoint (contactsOut[0]);
			dgVector minPoint (contactsOut[0]);
			dgVector lineDir (m_normal * normal);

			proj = contactsOut[0] % lineDir;
			maxProjection = proj;
			minProjection = proj;
			for (dgInt32 i = 1; i < count; i ++) {
				proj = contactsOut[i] % lineDir;
				if (proj > maxProjection) {
					maxProjection = proj;
					maxPoint = contactsOut[i];
				}
				if (proj < minProjection) {
					minProjection = proj;
					minPoint = contactsOut[i];
				}
			}	

			contactsOut[0] = maxPoint;
			contactsOut[1] = minPoint;
			count = 2;
		}


		dgVector error (contactsOut[count - 1] - contactsOut[0]);
		if ((error % error) < dgFloat32 (1.0e-8f)) {
			count --;
		}
	}

	#ifdef _DG_DEBUG
	if (count > 1) {
		dgInt32 j;
		j = count - 1;
		for (i = 0; i < count; i ++) {
			dgVector error (contactsOut[i] - contactsOut[j]);
			_DG_ASSERTE ((error % error) > dgFloat32 (1.0e-20f));
			j = i;
		}

		if (count >= 3) {
			dgVector n (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));
			dgVector e0 (contactsOut[1] - contactsOut[0]);
			for (i = 2; i < count; i ++) {
				dgVector e1 (contactsOut[i] - contactsOut[0]);
				n += e0 * e1;
				e0 = e1;
			} 
			n = n.Scale (dgFloat32 (1.0f) / dgSqrt(n % n));
			dgFloat32 val = n % normal;
			_DG_ASSERTE (val > dgFloat32 (0.9f));
		}
	}
	#endif

	return count;
}

dgInt32 dgCollisionMesh::dgCollisionConvexPolygon::CalculatePlaneIntersectionSimd (
	const dgVector& normal, 
	const dgVector& origin, 
	dgVector contactsOut[]) const
{
#ifdef DG_BUILD_SIMD_CODE
/*
	dgInt32 i;
	dgInt32 count;
	dgFloat32 t;
	dgFloat32 side0;
	dgFloat32 side1;
	dgFloat32 error;
	dgFloat32 projectFactor;

	count = 0;
	side0 = m_normal % normal;

	projectFactor = m_normal % normal;
	if (projectFactor < dgFloat32 (0.0f)) {
		projectFactor *= dgFloat32 (-1.0f);
		normal = normal.Scale (dgFloat32 (-1.0f));
	}

//	if (dgAbsf (projectFactor) > dgFloat32 (0.9999f)) {
	if (projectFactor > dgFloat32 (0.9999f)) {

		for (i = 0; i < m_count; i ++) {
			contactsOut[count] = m_localPoly[i];
			count ++;
		}

		#ifdef _DG_DEBUG
		dgInt32 j;
		j = count - 1;
		for (i = 0; i < count; i ++) {
			dgVector error (contactsOut[i] - contactsOut[j]);
			_DG_ASSERTE ((error % error) > dgFloat32 (1.0e-20f));
			j = i;
		}
		#endif


	} else if (projectFactor > dgFloat32 (0.1736f)) {
		dgPlane plane (normal, - (normal % origin));
		
		dgVector p0 (m_localPoly[m_count - 1]);
		side0 = plane.Evalue (p0);
		for (i = 0; i < m_count; i ++) {
			dgVector p1 (m_localPoly[i]);
			side1 = plane.Evalue (p1);

			if (side0 > dgFloat32 (0.0f)) {
				contactsOut[count] = p0 - plane.Scale (side0);
				count ++;
				if (count > 1) {
					dgVector edgeSegment (contactsOut[count - 1] - contactsOut[count - 2]);
					error = edgeSegment % edgeSegment;
					if (error < dgFloat32 (1.0e-8f)) {
						count --;
					}
				}

				if (side1 <= dgFloat32 (0.0f)) {
					dgVector dp (p1 - p0);
					t = plane % dp;
					_DG_ASSERTE (dgAbsf (t) >= dgFloat32 (0.0f));
					if (dgAbsf (t) < dgFloat32 (1.0e-8f)) {
						t = GetSign(t) * dgFloat32 (1.0e-8f);	
					}
					contactsOut[count] = p0 - dp.Scale (side0 / t);
					count ++;
					if (count > 1) {
						dgVector edgeSegment (contactsOut[count - 1] - contactsOut[count - 2]);
						error = edgeSegment % edgeSegment;
						if (error < dgFloat32 (1.0e-8f)) {
							count --;
						}
					}
				} 
			} else if (side1 > dgFloat32 (0.0f)) {
				dgVector dp (p1 - p0);
				t = plane % dp;
				_DG_ASSERTE (dgAbsf (t) >= dgFloat32 (0.0f));
				if (dgAbsf (t) < dgFloat32 (1.0e-8f)) {
					t = GetSign(t) * dgFloat32 (1.0e-8f);	
				}
				contactsOut[count] = p0 - dp.Scale (side0 / t);
				count ++;
				if (count > 1) {
					dgVector edgeSegment (contactsOut[count - 1] - contactsOut[count - 2]);
					error = edgeSegment % edgeSegment;
					if (error < dgFloat32 (1.0e-8f)) {
						count --;
					}
				}
			}

			side0 = side1;
			p0 = p1;
		}
	}

	if (count > 1) {
		dgVector error (contactsOut[count - 1] - contactsOut[0]);
		if ((error % error) < dgFloat32 (1.0e-8f)) {
			count --;
		}
	}

	#ifdef _DG_DEBUG
	if (count > 1) {
		dgInt32 j;
		j = count - 1;
		for (i = 0; i < count; i ++) {
			dgVector error (contactsOut[i] - contactsOut[j]);
			_DG_ASSERTE ((error % error) > dgFloat32 (1.0e-20f));
			j = i;
		}
		if (count >= 3) {
			dgVector n (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));
			dgVector e0 (contactsOut[1] - contactsOut[0]);
			for (i = 2; i < count; i ++) {
				dgVector e1 (contactsOut[i] - contactsOut[0]);
				n += e0 * e1;
				e0 = e1;
			} 
			n = n.Scale (dgFloat32 (1.0f) / dgSqrt(n % n));
			dgFloat32 test = n % normal;
			_DG_ASSERTE (test > dgFloat32 (0.9f));
		}
	}
	#endif

	return count;
*/
	return CalculatePlaneIntersection (normal, origin, contactsOut);
#else
	return 0;
#endif
}





dgCollisionMesh::dgCollisionMesh(dgMemoryAllocator* const allocator, dgCollisionID type)
	:dgCollision(allocator, 0, dgGetIdentityMatrix(), type)
{
	m_allocator = allocator;
	m_rtti |= dgCollisionMesh_RTTI;
	for (dgInt32 i = 0; i < DG_MAXIMUN_THREADS; i ++) {
		m_polygon[i] = new (allocator) dgCollisionConvexPolygon (allocator);			
	}

	m_debugCallback = NULL;
//	m_userRayCastCallback = NULL;
	SetCollisionBBox (dgVector (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f)),
					  dgVector (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f)));
}

//dgCollisionMesh::dgCollisionMesh (dgDeserialize deserialization, void* userData, const dgPolysoupCreation& data)
dgCollisionMesh::dgCollisionMesh (dgWorld* const world, dgDeserialize deserialization, void* const userData)
	:dgCollision(world, deserialization, userData)
{
	m_rtti |= dgCollisionMesh_RTTI;

	for (dgInt32 i = 0; i < DG_MAXIMUN_THREADS; i ++) {
		m_polygon[i] = new (world->GetAllocator()) dgCollisionConvexPolygon (world->GetAllocator());			
	}

	m_debugCallback = NULL;
//	m_userRayCastCallback = NULL;
	SetCollisionBBox (dgVector (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f)),
					  dgVector (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f)));

}

dgCollisionMesh::~dgCollisionMesh()
{
	for (dgInt32 i = 0; i < DG_MAXIMUN_THREADS; i ++) {
		m_polygon[i]->Release();
	}
}

void dgCollisionMesh::SetCollisionBBox (const dgVector& p0, const dgVector& p1)
{
	_DG_ASSERTE (p0.m_x <= p1.m_x);
	_DG_ASSERTE (p0.m_y <= p1.m_y);
	_DG_ASSERTE (p0.m_z <= p1.m_z);

	m_boxSize = (p1 - p0).Scale (dgFloat32 (0.5f)); 
	m_boxOrigin = (p1 + p0).Scale (dgFloat32 (0.5f)); 
}

dgInt32 dgCollisionMesh::CalculateSignature () const
{
	_DG_ASSERTE (0);
	return 0;
}


void* dgCollisionMesh::GetUserData () const
{
	return NULL;
}

/*
void dgCollisionMesh::SetCallBack___ (void *callBack)
{
	m_collideCallback = (OnPolygonSoupCollideCallback) callBack;
}


void *dgCollisionMesh::GetCallBack___ () const
{
	return (void*) m_collideCallback;
}
*/

void dgCollisionMesh::SetCollisionCallback (dgCollisionMeshCollisionCallback debugCallback)
{
	m_debugCallback = debugCallback;
}




#ifdef DG_DEBUG_AABB
dgVector dgCollisionMesh::BoxSupportMapping  (const dgVector& dir) const
{
	return dgVector (dir.m_x < dgFloat32 (0.0f) ? m_p0.m_x : m_p1.m_x, 
					 dir.m_y < dgFloat32 (0.0f) ? m_p0.m_y : m_p1.m_y, 
					 dir.m_z < dgFloat32 (0.0f) ? m_p0.m_z : m_p1.m_z, dgFloat32 (0.0f));
}
#endif


void dgCollisionMesh::CalcAABB(
	const dgMatrix &matrix,
	dgVector &p0, 
	dgVector &p1) const
{
	dgVector origin (matrix.TransformVector(m_boxOrigin));
	dgVector size (m_boxSize.m_x * dgAbsf(matrix[0][0]) + m_boxSize.m_y * dgAbsf(matrix[1][0]) + m_boxSize.m_z * dgAbsf(matrix[2][0]) + DG_MAX_COLLISION_PADDING,  
				   m_boxSize.m_x * dgAbsf(matrix[0][1]) + m_boxSize.m_y * dgAbsf(matrix[1][1]) + m_boxSize.m_z * dgAbsf(matrix[2][1]) + DG_MAX_COLLISION_PADDING,  
				   m_boxSize.m_x * dgAbsf(matrix[0][2]) + m_boxSize.m_y * dgAbsf(matrix[1][2]) + m_boxSize.m_z * dgAbsf(matrix[2][2]) + DG_MAX_COLLISION_PADDING,
				   dgFloat32 (0.0f));

	p0 = origin - size;
	p1 = origin + size;

#ifdef DG_DEBUG_AABB
	dgInt32 i;
	dgVector q0;
	dgVector q1;
	dgMatrix trans (matrix.Transpose());
	for (i = 0; i < 3; i ++) {
		q0[i] = matrix.m_posit[i] + matrix.RotateVector (BoxSupportMapping(trans[i].Scale (-1.0f)))[i];
		q1[i] = matrix.m_posit[i] + matrix.RotateVector (BoxSupportMapping(trans[i]))[i];
	}

	dgVector err0 (p0 - q0);
	dgVector err1 (p1 - q1);
	dgFloat32 err; 
	err = GetMax (size.m_x, size.m_y, size.m_z) * 0.5f; 
	_DG_ASSERTE ((err0 % err0) < err);
	_DG_ASSERTE ((err1 % err1) < err);
#endif
}


void dgCollisionMesh::CalcAABBSimd(const dgMatrix &matrix,	dgVector &p0, dgVector &p1) const
{
	CalcAABB(matrix, p0, p1);
}



dgInt32 dgCollisionMesh::CalculatePlaneIntersection (
	const dgFloat32* vertex, 
	const dgInt32* index, 
	dgInt32 indexCount, 
	dgInt32 stride, 
	const dgPlane& localPlane, 
	dgVector contactsOut[]) const
{
	dgInt32 i;
	dgInt32 j;
	dgInt32 count;
	dgFloat32 t;
	dgFloat32 side0;
	dgFloat32 side1;

	count = 0;
	j = index[indexCount - 1] * stride;
	dgVector p0 (&vertex[j]);
	side0 = localPlane.Evalue (p0);
	for (i = 0; i < indexCount; i ++) {
		j = index[i] * stride;
		dgVector p1 (&vertex[j]);
		side1 = localPlane.Evalue (p1);

		if (side0 < dgFloat32 (0.0f)) {
			if (side1 >= dgFloat32 (0.0f)) {
				dgVector dp (p1 - p0);
				t = localPlane % dp;
				_DG_ASSERTE (dgAbsf (t) >= dgFloat32 (0.0f));
				if (dgAbsf (t) < dgFloat32 (1.0e-8f)) {
					t = GetSign(t) * dgFloat32 (1.0e-8f);	
				}
_DG_ASSERTE (0);
				contactsOut[count] = p0 - dp.Scale (side0 / t);
				count ++;
			
			} 
		} else if (side1 <= dgFloat32 (0.0f)) {
			dgVector dp (p1 - p0);
			t = localPlane % dp;
			_DG_ASSERTE (dgAbsf (t) >= dgFloat32 (0.0f));
			if (dgAbsf (t) < dgFloat32 (1.0e-8f)) {
				t = GetSign(t) * dgFloat32 (1.0e-8f);	
			}
_DG_ASSERTE (0);
			contactsOut[count] = p0 - dp.Scale (side0 / t);
			count ++;
		}

		side0 = side1;
		p0 = p1;
	}

	return count;
}


dgVector dgCollisionMesh::CalculateVolumeIntegral (const dgMatrix& globalMatrix__, GetBuoyancyPlane buoyancuPlane__, void* context__) const
{
	return dgVector (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));
}


//void dgCollisionMesh::DebugCollision (const dgBody& myBody, DebugCollisionMeshCallback callback) const
void dgCollisionMesh::DebugCollision (const dgMatrix& matrixPtr, OnDebugCollisionMeshCallback callback, void* const userData) const
{
	_DG_ASSERTE (0);
}


dgFloat32 dgCollisionMesh::GetVolume () const
{
//	_DG_ASSERTE (0);
	return dgFloat32 (0.0f); 
}

dgFloat32 dgCollisionMesh::GetBoxMinRadius () const
{
	return dgFloat32 (0.0f);  
}

dgFloat32 dgCollisionMesh::GetBoxMaxRadius () const
{
	return dgFloat32 (0.0f);  
}



void dgCollisionMesh::CalculateInertia (dgVector& inertia, dgVector& origin) const
{
	inertia.m_x = dgFloat32 (0.0f);
	inertia.m_y = dgFloat32 (0.0f);
	inertia.m_z = dgFloat32 (0.0f);

	origin.m_x = dgFloat32 (0.0f);
	origin.m_y = dgFloat32 (0.0f);
	origin.m_z = dgFloat32 (0.0f);
}
	

void dgCollisionMesh::GetCollisionInfo(dgCollisionInfo* info) const
{
	_DG_ASSERTE (0);
//	dgCollision::GetCollisionInfo(info);
//	info->m_offsetMatrix = GetOffsetMatrix();
//	info->m_collisionType = m_collsionId;
}

void dgCollisionMesh::Serialize(dgSerialize callback, void* const userData) const
{
	_DG_ASSERTE (0);
}

dgVector dgCollisionMesh::SupportVertex (const dgVector& dir) const
{
	_DG_ASSERTE (0);
	return dgVector (0, 0, 0, 0);
}


bool dgCollisionMesh::OOBBTest (const dgMatrix& matrix, const dgCollisionConvex* const shape, void* const cacheOrder) const
{
	_DG_ASSERTE (0);
	return true;
}


