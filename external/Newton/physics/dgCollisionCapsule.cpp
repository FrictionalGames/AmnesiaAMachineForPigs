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
#include "dgCollisionCapsule.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////



dgInt32 dgCollisionCapsule::m_shapeRefCount = 0;
dgConvexSimplexEdge dgCollisionCapsule::m_edgeArray[DG_CAPSULE_SEGMENTS * (6 + 8 * (DG_CAP_SEGMENTS - 1))];

dgCollisionCapsule::dgCollisionCapsule(
	dgMemoryAllocator* allocator, 
	dgUnsigned32 signature, 
	dgFloat32 radius, 
	dgFloat32 height, 
	const dgMatrix& matrix)
	:dgCollisionConvex(allocator, signature, matrix, m_capsuleCollision)
{
	Init (radius, height);
}

dgCollisionCapsule::dgCollisionCapsule(dgWorld* const world, dgDeserialize deserialization, void* const userData)
	:dgCollisionConvex (world, deserialization, userData)
{
	dgVector size;
	deserialization (userData, &size, sizeof (dgVector));
	Init (size.m_x, size.m_y);
}


dgCollisionCapsule::~dgCollisionCapsule()
{
	m_shapeRefCount --;
	_DG_ASSERTE (m_shapeRefCount >= 0);

	dgCollisionConvex::m_simplex = NULL;
	dgCollisionConvex::m_vertex = NULL;
}

void dgCollisionCapsule::Init (dgFloat32 radius, dgFloat32 height)
{
//	dgInt32 i;
//	dgInt32 j;
//	dgInt32 i0;
//	dgInt32 i1;
//	dgFloat32 x;
//	dgFloat32 y;
//	dgFloat32 z;
//	dgFloat32 r;
//	dgFloat32 angle;
//	dgEdge *edge;

	m_rtti |= dgCollisionCapsule_RTTI;

	dgInt32 i0 = 0;
	dgInt32 i1 = DG_CAPSULE_SEGMENTS * DG_CAP_SEGMENTS * 2;

	m_radius = dgAbsf (radius);
	m_height[0] = GetMax (dgFloat32(0.01f), dgAbsf (height * dgFloat32 (0.5f)) - m_radius);
	m_height[1] = - m_height[0];

	m_silhuette[0] = dgVector ( m_height[0], -m_radius, dgFloat32 (0.0f), dgFloat32 (0.0f)); 
	m_silhuette[1] = dgVector (-m_height[0], -m_radius, dgFloat32 (0.0f), dgFloat32 (0.0f)); 
	m_silhuette[2] = dgVector (-m_height[0],  m_radius, dgFloat32 (0.0f), dgFloat32 (0.0f)); 
	m_silhuette[3] = dgVector ( m_height[0],  m_radius, dgFloat32 (0.0f), dgFloat32 (0.0f)); 

	m_tethaStep = GetDiscretedAngleStep (m_radius);
	m_tethaStepInv = dgFloat32 (1.0f) / m_tethaStep;
	m_delCosTetha = dgCos (m_tethaStep);
	m_delSinTetha = dgSin (m_tethaStep);

//	dgFloat32 x = dgFloat32 (0.5f) * m_radius / DG_CAP_SEGMENTS;
	for (dgInt32 j = 0; j < DG_CAP_SEGMENTS; j ++) {
		dgFloat32 angle = dgFloat32 (0.0f);
		dgFloat32 x = (DG_CAP_SEGMENTS - j - 1) * m_radius / DG_CAP_SEGMENTS;
		dgFloat32 r = dgSqrt (m_radius * m_radius - x * x);
		i1 -= DG_CAPSULE_SEGMENTS;
		for (dgInt32 i = 0; i < DG_CAPSULE_SEGMENTS; i ++) {
			dgFloat32 z = dgSin (angle) * r;
			dgFloat32 y = dgCos (angle) * r;
			m_vertex[i0] = dgVector (- (m_height[0] + x), y, z, dgFloat32 (1.0f));
			m_vertex[i1] = dgVector (  (m_height[0] + x), y, z, dgFloat32 (1.0f));
			i0 ++;
			i1 ++;
			angle += dgPI2 / DG_CAPSULE_SEGMENTS;
		}
		i1 -= DG_CAPSULE_SEGMENTS;
	}

	m_vertexCount = DG_CAPSULE_SEGMENTS * DG_CAP_SEGMENTS * 2;
	m_edgeCount = DG_CAPSULE_SEGMENTS * (6 + 8 * (DG_CAP_SEGMENTS - 1));
	dgCollisionConvex::m_vertex = m_vertex;


	if (!m_shapeRefCount) {
		dgPolyhedra polyhedra(m_allocator);
		dgInt32 wireframe[DG_CAPSULE_SEGMENTS + 10];

		i1 = 0;
		i0 = DG_CAPSULE_SEGMENTS - 1;
		polyhedra.BeginFace ();
		for (dgInt32 j = 0; j < DG_CAP_SEGMENTS * 2 - 1; j ++) {
			for (dgInt32 i = 0; i < DG_CAPSULE_SEGMENTS; i ++) { 
				wireframe[0] = i0;
				wireframe[1] = i1;
				wireframe[2] = i1 + DG_CAPSULE_SEGMENTS;
				wireframe[3] = i0 + DG_CAPSULE_SEGMENTS;
				i0 = i1;
				i1 ++;
				polyhedra.AddFace (4, wireframe);
			}
			i0 = i1 + DG_CAPSULE_SEGMENTS - 1;
		}

		for (dgInt32 i = 0; i < DG_CAPSULE_SEGMENTS; i ++) { 
			wireframe[i] = DG_CAPSULE_SEGMENTS - 1 - i;
		}
		polyhedra.AddFace (DG_CAPSULE_SEGMENTS, wireframe);

		for (dgInt32 i = 0; i < DG_CAPSULE_SEGMENTS; i ++) { 
			wireframe[i] = i + DG_CAPSULE_SEGMENTS * (DG_CAP_SEGMENTS * 2 - 1);
		}
		polyhedra.AddFace (DG_CAPSULE_SEGMENTS, wireframe);
		polyhedra.EndFace ();

		_DG_ASSERTE (SanityCheck (polyhedra));

		dgUnsigned64 i = 0;
		dgPolyhedra::Iterator iter (polyhedra);
		for (iter.Begin(); iter; iter ++) {
			dgEdge* const edge = &(*iter);
			edge->m_userData = i;
			i ++;
		}

		for (iter.Begin(); iter; iter ++) {
			dgEdge* const edge = &(*iter);

			dgConvexSimplexEdge* const ptr = &m_edgeArray[edge->m_userData];

			ptr->m_vertex = edge->m_incidentVertex;
			ptr->m_next = &m_edgeArray[edge->m_next->m_userData];
			ptr->m_prev = &m_edgeArray[edge->m_prev->m_userData];
			ptr->m_twin = &m_edgeArray[edge->m_twin->m_userData];
		}

	}

	m_shapeRefCount ++;
	dgCollisionConvex::m_simplex = m_edgeArray;

	SetVolumeAndCG ();

	dgVector inertia;
	dgVector centerOfMass;
	dgVector crossInertia;
	m_volume.m_w = CalculateMassProperties (inertia, crossInertia, centerOfMass);
}

dgInt32 dgCollisionCapsule::CalculateSignature () const
{
	dgUnsigned32 buffer[2 * sizeof (dgMatrix) / sizeof(dgInt32)];

	memset (buffer, 0, sizeof (buffer));
	buffer[0] = m_capsuleCollision;
	buffer[1] = dgCollision::Quantize (m_radius);
	buffer[2] = dgCollision::Quantize (m_height[0]);
	memcpy (&buffer[3], &m_offset, sizeof (dgMatrix));
	return dgInt32 (dgCollision::MakeCRC(buffer, sizeof (buffer)));
}

void dgCollisionCapsule::TesselateTriangle (dgInt32 level, dgFloat32 side, const dgVector& p0, const dgVector& p1, const dgVector& p2, dgInt32& count, dgVector* ouput) const
{
	if (level) {
		_DG_ASSERTE (dgAbsf (p0 % p0 - dgFloat32 (1.0f)) < dgFloat32 (1.0e-4f));
		_DG_ASSERTE (dgAbsf (p1 % p1 - dgFloat32 (1.0f)) < dgFloat32 (1.0e-4f));
		_DG_ASSERTE (dgAbsf (p2 % p2 - dgFloat32 (1.0f)) < dgFloat32 (1.0e-4f));
		dgVector p01 (p0 + p1);
		dgVector p12 (p1 + p2);
		dgVector p20 (p2 + p0);

		p01 = p01.Scale (dgFloat32 (1.0f) / dgSqrt(p01 % p01));
		p12 = p12.Scale (dgFloat32 (1.0f) / dgSqrt(p12 % p12));
		p20 = p20.Scale (dgFloat32 (1.0f) / dgSqrt(p20 % p20));

		_DG_ASSERTE (dgAbsf (p01 % p01 - dgFloat32 (1.0f)) < dgFloat32 (1.0e-4f));
		_DG_ASSERTE (dgAbsf (p12 % p12 - dgFloat32 (1.0f)) < dgFloat32 (1.0e-4f));
		_DG_ASSERTE (dgAbsf (p20 % p20 - dgFloat32 (1.0f)) < dgFloat32 (1.0e-4f));

		TesselateTriangle (level - 1, side, p0,  p01, p20, count, ouput);
		TesselateTriangle (level - 1, side, p1,  p12, p01, count, ouput);
		TesselateTriangle (level - 1, side, p2,  p20, p12, count, ouput);
		TesselateTriangle (level - 1, side, p01, p12, p20, count, ouput);

	} else {
		ouput[count + 0] = p0.Scale (m_radius);
		ouput[count + 1] = p1.Scale (m_radius);
		ouput[count + 2] = p2.Scale (m_radius);
		ouput[count + 0].m_x += side;
		ouput[count + 1].m_x += side;
		ouput[count + 2].m_x += side;
		count += 3;
	}
}


void dgCollisionCapsule::DebugCollision (const dgMatrix& matrixPtr, OnDebugCollisionMeshCallback callback, void* const userData) const
{
	dgInt32 i0;
	dgInt32 i1;
	dgInt32 j0;
	dgInt32 j1;

	dgInt32 count;
	dgFloat32 y;
	dgFloat32 z;
	dgFloat32 angle;
	#define POWER 2
	#define STEPS (4 * (1 << POWER))

	dgTriplex face[32];
	dgTriplex pool[1024];
	dgVector tmpVectex[1024];

	angle = dgFloat32 (0.0f);
	for (i0 = 0; i0 < STEPS; i0 ++) {
		z = dgSin (angle) * m_radius;
		y = dgCos (angle) * m_radius;
		tmpVectex[i0].m_x = - m_height[0];
		tmpVectex[i0].m_y = y;
		tmpVectex[i0].m_z = z;
		tmpVectex[i0 + STEPS].m_x = m_height[0];
		tmpVectex[i0 + STEPS].m_y = y;
		tmpVectex[i0 + STEPS].m_z = z;
		angle += dgPI2 / dgFloat32 (STEPS);
	}

	
	dgVector p0 ( dgFloat32 (1.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f)); 
	dgVector p1 (-dgFloat32 (1.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f)); 
	dgVector p2 ( dgFloat32 (0.0f), dgFloat32 (1.0f), dgFloat32 (0.0f), dgFloat32 (0.0f)); 
	dgVector p3 ( dgFloat32 (0.0f),-dgFloat32 (1.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));
	dgVector p4 ( dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (1.0f), dgFloat32 (0.0f));
	dgVector p5 ( dgFloat32 (0.0f), dgFloat32 (0.0f),-dgFloat32 (1.0f), dgFloat32 (0.0f));

	count = STEPS * 2;
	TesselateTriangle (POWER, m_height[0], p0, p2, p4, count, tmpVectex);
	TesselateTriangle (POWER, m_height[0], p0, p4, p3, count, tmpVectex);
	TesselateTriangle (POWER, m_height[0], p0, p3, p5, count, tmpVectex);
	TesselateTriangle (POWER, m_height[0], p0, p5, p2, count, tmpVectex);
	
	TesselateTriangle (POWER, -m_height[0], p1, p4, p2, count, tmpVectex);
	TesselateTriangle (POWER, -m_height[0], p1, p3, p4, count, tmpVectex);
	TesselateTriangle (POWER, -m_height[0], p1, p5, p3, count, tmpVectex);
	TesselateTriangle (POWER, -m_height[0], p1, p2, p5, count, tmpVectex);

	dgMatrix matrix (GetOffsetMatrix() * matrixPtr);
	matrix.TransformTriplex (pool, sizeof (dgTriplex), tmpVectex, sizeof (dgVector), count);

	
	i0 = STEPS - 1;
	j1 = STEPS;
	j0 = STEPS + STEPS - 1;
	for (i1 = 0; i1 < STEPS; i1 ++) {
		face[0] = pool[i0];
		face[1] = pool[i1];
		face[2] = pool[j1];
		face[3] = pool[j0];
		callback (userData, 4, &face[0].m_x, 0);
		i0 = i1;
		j0 = j1;
		j1 ++;
	}

	for (i1 = STEPS * 2; i1 < count; i1 += 3) {
		callback (userData, 3, &pool[i1].m_x, 0);
	}
}


void dgCollisionCapsule::SetCollisionBBox (const dgVector& p0__, const dgVector& p1__)
{
	_DG_ASSERTE (0);
}



dgVector dgCollisionCapsule::SupportVertexSimd (const dgVector& dir) const
{
#ifdef DG_BUILD_SIMD_CODE

	dgInt32 index;
	dgFloatSign *ptr; 

	_DG_ASSERTE (dgAbsf(dir % dir - dgFloat32 (1.0f)) < dgFloat32 (1.0e-3f));

	ptr =  (dgFloatSign*) &dir; 
	index = -(ptr[0].m_integer.m_iVal >> 31);
	dgVector p (dir.Scale (m_radius));
	p.m_x += m_height[index];
	return p;

#else
	return dgVector (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));
#endif
}


dgVector dgCollisionCapsule::SupportVertex (const dgVector& dir) const
{
/*
	dgInt32 index;
	dgFloat32 x0;
	dgFloat32 z0;
	dgFloat32 x1;
	dgFloat32 z1;
	dgFloat32 height;
	dgFloat32 dist0;
	dgFloat32 dist1;
	dgFloat32 tetha;
	dgFloat32 alpha;
	dgFloat32 sinAlpha;
	dgFloat32 cosAlpha;
	dgFloat32 sinTetha;
	dgFloat32 cosTetha;
	dgFloatSign *ptr; 

	_DG_ASSERTE (0);

	//	dgFloat32 sign;
	_DG_ASSERTE (dgAbsf(dir % dir - dgFloat32 (1.0f)) < dgFloat32 (1.0e-3f));


	ptr =  (dgFloatSign*) &dir; 
	index = -(ptr[0].m_integer.m_iVal >> 31);
	height = m_height[index];

	if (dgAbsf (dir.m_x) > dgFloat32 (0.9998f)) {
		if (dir.m_x > dgFloat32 (0.9998f)) {
			return dgVector (height + m_radius, dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f)); 
		}
		return dgVector (height - m_radius, dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f)); 
	}

	tetha = m_tethaStep * dgFloor (dgAtan2 (dir.m_y, dir.m_z) * m_tethaStepInv);
	alpha = m_tethaStep * dgFloor (dgAsin (dir.m_x) * m_tethaStepInv);;

	dgSinCos (tetha, sinTetha, cosTetha);
	dgSinCos (alpha, sinAlpha, cosAlpha);

	x0 = m_radius * sinAlpha;
	z0 = m_radius * cosAlpha;

	dgVector p0 (x0, z0 * sinTetha, z0 * cosTetha, dgFloat32 (0.0f));

	x1 = x0 * m_delCosTetha + z0 * m_delSinTetha;
	z1 = z0 * m_delCosTetha - x0 * m_delSinTetha;

	dgVector p1 (x1, z1 * sinTetha, z1 * cosTetha, dgFloat32 (0.0f));
	dgVector p2 (x0, z0 * sinTetha * m_delCosTetha + z0 * cosTetha * m_delSinTetha,
					 z0 * cosTetha * m_delCosTetha - z0 * sinTetha * m_delSinTetha, dgFloat32 (0.0f));

	dgVector p3 (x1, z1 * sinTetha * m_delCosTetha + z1 * cosTetha * m_delSinTetha,
					 z1 * cosTetha * m_delCosTetha - z1 * sinTetha * m_delSinTetha, dgFloat32 (0.0f));

	dist0 = p0 % dir;
	dist1 = p1 % dir;
	if (dist1 > dist0) {
		p0 = p1;
		dist0 = dist1;
	}

	dist1 = p2 % dir;
	if (dist1 > dist0) {
		p0 = p2;
		dist0 = dist1;
	}

	dist1 = p3 % dir;
	if (dist1 > dist0) {
		p0 = p3;
		dist0 = dist1;
	}

	p0.m_x += height; 
	return p0;       
*/

	dgInt32 index;
	dgFloatSign *ptr; 

	_DG_ASSERTE (dgAbsf(dir % dir - dgFloat32 (1.0f)) < dgFloat32 (1.0e-3f));

	ptr =  (dgFloatSign*) &dir; 
	index = -(ptr[0].m_integer.m_iVal >> 31);
	dgVector p (dir.Scale (m_radius));
	p.m_x += m_height[index];
	return p;

}


dgFloat32 dgCollisionCapsule::RayCast (const dgVector& q0, const dgVector& q1, dgContactPoint& contactOut, OnRayPrecastAction preFilter, const dgBody* const body, void* const userData) const
{
	dgFloat32 t;
	dgFloat32 a;
	dgFloat32 b;
	dgFloat32 c;
	dgFloat32 x;
	dgFloat32 t1;
	dgFloat32 desc;
	dgFloat32 radius;

	if (PREFILTER_RAYCAST (preFilter, body, this, userData)) {
		return dgFloat32 (1.2f);
	}


	t = dgFloat32 (1.2f);

	dgVector p0 (q0);
	p0.m_x = dgFloat32 (0.0f);
	radius = m_radius;

	c = (p0 % p0) - radius * radius;
	if (c > dgFloat32 (0.0f)) {
		dgVector p1 (q1);
		p1.m_x = dgFloat32 (0.0f);

//		dgVector dp (p1 - q0);
		dgVector dp (p1 - p0);

		a = dp % dp;
		b = dgFloat32 (2.0f) * (p0 % dp);
		desc = b * b - dgFloat32 (4.0f) * a * c;

		if (desc > dgFloat32 (1.0e-8f)) {
			desc = dgSqrt (desc);
			a = dgFloat32 (1.0f) / (dgFloat32 (2.0f) * a);
			t1 = GetMin ((- b + desc) * a, (- b - desc) * a);
			if ((t1 >= dgFloat32 (0.0f)) && t1 < dgFloat32 (1.0f)) {
				x = q0.m_x + (q1.m_x - q0.m_x) * t1;
				if (x > m_height[0]) {
					dgVector h (m_height[0], dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));
					dgVector dq (q1 - q0);
					dgVector dqp0 (q0 - h);
					a = dq % dq;
					b = dgFloat32 (2.0f) * (dqp0 % dq);
					c = dqp0 % dqp0 - radius * radius;
					desc = b * b - dgFloat32 (4.0f) * a * c;
					if (desc > dgFloat32 (1.0e-8f)) {
						desc = dgSqrt (desc);
						a = dgFloat32 (1.0f) / (dgFloat32 (2.0f) * a);
						t = GetMin ((- b + desc) * a, (- b - desc) * a);
						dgVector n (q0 + dq.Scale (t) - h); 
						contactOut.m_normal = n.Scale (dgRsqrt (n % n));
						contactOut.m_userId = SetUserDataID();
					}

				} else if (x < -m_height[0]) {
					dgVector h (-m_height[0], dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));
					dgVector dq (q1 - q0);
					dgVector dqp0 (q0 - h);
					a = dq % dq;
					b = dgFloat32 (2.0f) * (dqp0 % dq);
					c = dqp0 % dqp0 - radius * radius;
					desc = b * b - dgFloat32 (4.0f) * a * c;
					if (desc > dgFloat32 (1.0e-8f)) {
						desc = dgSqrt (desc);
						a = dgFloat32 (1.0f) / (dgFloat32 (2.0f) * a);
						t = GetMin ((- b + desc) * a, (- b - desc) * a);
						dgVector n (q0 + dq.Scale (t) - h); 
						contactOut.m_normal = n.Scale (dgRsqrt (n % n));
						contactOut.m_userId = SetUserDataID();
					}
				} else { 
					t = t1;
					dgVector n (p0 + dp.Scale (t));
					contactOut.m_normal = n.Scale (dgRsqrt (n % n));
					contactOut.m_userId = SetUserDataID();
				}
			}
		}
	} else {
		if (q0.m_x > m_height[0]) {
			dgVector h (m_height[0], dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));
			dgVector dq (q1 - q0);
			dgVector dqp0 (q0 - h);
			a = dq % dq;
			b = dgFloat32 (2.0f) * (dqp0 % dq);
			c = dqp0 % dqp0 - radius * radius;
			if (c > dgFloat32 (0.0f)) {
				desc = b * b - dgFloat32 (4.0f) * a * c;
				if (desc > dgFloat32 (1.0e-8f)) {
					dgFloat32 t1;
					desc = dgSqrt (desc);
					a = dgFloat32 (1.0f) / (dgFloat32 (2.0f) * a);
					t1 = GetMin ((- b + desc) * a, (- b - desc) * a);
					if (t1 >= dgFloat32 (0.0f)) {
						t = t1;
						dgVector n (q0 + dq.Scale (t) - h); 
						contactOut.m_normal = n.Scale (dgRsqrt (n % n));
						contactOut.m_userId = SetUserDataID();
					}
				}
			}

		} else if (q0.m_x < -m_height[0]) {
			dgVector h (-m_height[0], dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));
			dgVector dq (q1 - q0);
			dgVector dqp0 (q0 - h);
			a = dq % dq;
			b = dgFloat32 (2.0f) * (dqp0 % dq);
			c = dqp0 % dqp0 - radius * radius;
			if (c > dgFloat32 (0.0f)) {
				desc = b * b - dgFloat32 (4.0f) * a * c;
				if (desc > dgFloat32 (1.0e-8f)) {
					dgFloat32 t1;
					desc = dgSqrt (desc);
					a = dgFloat32 (1.0f) / (dgFloat32 (2.0f) * a);
					t1 = GetMin ((- b + desc) * a, (- b - desc) * a);
					if (t1 >= dgFloat32 (0.0f)) {
						t = t1;
						dgVector n (q0 + dq.Scale (t) - h); 
						contactOut.m_normal = n.Scale (dgRsqrt (n % n));
						contactOut.m_userId = SetUserDataID();
					}
				}
			}
		}
	}

	return t;
}

dgFloat32 dgCollisionCapsule::RayCastSimd (const dgVector& q0, const dgVector& q1, dgContactPoint& contactOut, OnRayPrecastAction preFilter, const dgBody* const body, void* const userData) const
{
	return RayCast (q0, q1, contactOut, preFilter, body, userData);
}


dgFloat32 dgCollisionCapsule::CalculateMassProperties (dgVector& inertia, dgVector& crossInertia, dgVector& centerOfMass) const
{
	dgFloat32 volume;
	dgFloat32 inertiaxx;
	dgFloat32 inertiayyzz;
	dgFloat32 cylVolume;
	dgFloat32 sphVolume;
	dgFloat32 cylInertiaxx;
	dgFloat32 sphInertiaxx;
	dgFloat32 cylInertiayyzz;
	dgFloat32 sphInertiayyzz;

	centerOfMass = GetOffsetMatrix().m_posit;
	cylVolume = dgFloat32 (3.1616f * 2.0f) * m_radius * m_radius * m_height[0];
	sphVolume = dgFloat32 (3.1616f * 4.0f / 3.0f) * m_radius * m_radius * m_radius; 

	cylInertiaxx = (dgFloat32 (0.5f) * m_radius * m_radius) * cylVolume;
	sphInertiaxx = (dgFloat32 (2.0f / 5.0f) * m_radius * m_radius) * sphVolume;

	cylInertiayyzz = (dgFloat32 (0.25f) * m_radius *  m_radius + dgFloat32 (1.0f / 3.0f) * m_height[0] * m_height[0]) * cylVolume;
	sphInertiayyzz = sphInertiaxx + m_height[0] * m_height[0] * sphVolume;

	volume = cylVolume + sphVolume;
	inertiaxx = cylInertiaxx + sphInertiaxx;
	inertiayyzz = cylInertiayyzz + sphInertiayyzz;


	dgMatrix inertiaTensor (dgGetIdentityMatrix());

	inertiaTensor[0][0] = inertiaxx;
	inertiaTensor[1][1] = inertiayyzz;
	inertiaTensor[2][2] = inertiayyzz;

	inertiaTensor = GetOffsetMatrix().Inverse() * inertiaTensor * GetOffsetMatrix();

	crossInertia.m_x = inertiaTensor[1][2] - volume * centerOfMass.m_y * centerOfMass.m_z;
	crossInertia.m_y = inertiaTensor[0][2] - volume * centerOfMass.m_z * centerOfMass.m_x;
	crossInertia.m_z = inertiaTensor[0][1] - volume * centerOfMass.m_x * centerOfMass.m_y;

	dgVector central (centerOfMass.CompProduct(centerOfMass));
	inertia.m_x = inertiaTensor[0][0] + volume * (central.m_y + central.m_z);
	inertia.m_y = inertiaTensor[1][1] + volume * (central.m_z + central.m_x);
	inertia.m_z = inertiaTensor[2][2] + volume * (central.m_x + central.m_y);

	centerOfMass = centerOfMass.Scale (volume);
	return volume;
}



dgInt32 dgCollisionCapsule::CalculatePlaneIntersectionSimd (
	const dgVector& normal, 
	const dgVector& origin, 
	dgVector contactsOut[]) const
{
#ifdef DG_BUILD_SIMD_CODE
	return dgCollisionCapsule::CalculatePlaneIntersection (normal, origin, contactsOut);
#else
	return 0;
#endif
}



dgInt32 dgCollisionCapsule::CalculatePlaneIntersection (
	const dgVector& normal, 
	const dgVector& origin, 
	dgVector contactsOut[]) const
{
	dgInt32 i;
	dgInt32 count;
	dgFloat32 a;
	dgFloat32 b;
	dgFloat32 c;
	dgFloat32 d;
	dgFloat32 r;
	dgFloat32 x;
	dgFloat32 y;
	dgFloat32 z;
	dgFloat32 x0;
	dgFloat32 x1;
	dgFloat32 den;
	dgFloat32 desc;
	dgFloat32 test0;
	dgFloat32 test1;
	dgFloat32 cosAng;
	dgFloat32 sinAng;
	dgFloat32 magInv;

	count = 0;
	if (dgAbsf (normal.m_x) > dgFloat32 (0.999f)) { 
		x = (normal.m_x > dgFloat32 (0.0f)) ? dgFloat32 (1.0f) : dgFloat32 (-1.0f);
		contactsOut[count] = dgVector ( x, dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));
		count = 1;

	} else {
		magInv = dgRsqrt (normal.m_y * normal.m_y + normal.m_z * normal.m_z);
		cosAng = normal.m_y * magInv;
		sinAng = normal.m_z * magInv;
		_DG_ASSERTE (dgAbsf (normal.m_z * cosAng - normal.m_y * sinAng) < dgFloat32 (1.0e-4f));
		dgVector normal1 (normal.m_x, normal.m_y * cosAng + normal.m_z * sinAng, dgFloat32 (0.0f), dgFloat32 (0.0f));
		dgVector origin1 (origin.m_x, origin.m_y * cosAng + origin.m_z * sinAng, 
						  origin.m_z * cosAng - origin.m_y * sinAng, dgFloat32 (0.0f));
		dgPlane plane (normal1, - (normal1 % origin1));
		dgVector maxDir ((normal1.m_x > dgFloat32 (0.0f)) ? m_silhuette[3].m_x : -m_silhuette[3].m_x,
						 (normal1.m_y > dgFloat32 (0.0f)) ? m_silhuette[3].m_y : -m_silhuette[3].m_y, dgFloat32 (0.0f), dgFloat32 (0.0f));  

		test0 = plane.Evalue (maxDir);
		test1 = plane.Evalue (maxDir.Scale (dgFloat32 (-1.0f)));

		if ((test0 * test1) > dgFloat32 (0.0f)) {
			test0 = plane.m_w + plane.m_x * m_height[0];
			if (dgAbsf (test0) < m_radius) {
				contactsOut[count] = normal1.Scale (-test0);
				contactsOut[count].m_x += m_height[0];
				count = 1;
			} else {
				test0 = plane.m_w - plane.m_x * m_height[0];
				if (dgAbsf (test0) < m_radius) {
					contactsOut[count] = normal1.Scale (-test0);
					contactsOut[count].m_x -= m_height[0];
					count = 1;
				}
			}
		} else {
			dgVector dp (m_silhuette[1] - m_silhuette[0]);
			den = normal1 % dp;
			if (dgAbsf (den) > dgFloat32 (0.0f)) {
				test0 = -plane.Evalue (m_silhuette[0]) / den;
				if ((test0 <= dgFloat32 (1.0)) && (test0 >= dgFloat32 (0.0f))) {
					contactsOut[count] = m_silhuette[0] + dp.Scale (test0);
					count ++;
				}
			}

			if (count < 2) {
				test0 = plane.m_w - plane.m_x * m_height[0];
				if (dgAbsf (test0) < m_radius) {
					r = -m_height[0];
					d = plane.m_w + r * plane.m_x;

					a = plane.m_x * plane.m_x + plane.m_y * plane.m_y;
					b = dgFloat32 (2.0f) * plane.m_x * d;
					c = d * d - m_radius * m_radius * plane.m_y * plane.m_y;
					desc = b * b - dgFloat32 (4.0f) * a * c;
					if (desc > dgFloat32 (0.0f)) {
						_DG_ASSERTE (dgAbsf (a) > dgFloat32 (0.0f));
						desc = dgSqrt (desc);
						a = - dgFloat32 (0.5f) * b / a; 
						x0 = a + desc;
						x1 = a - desc;
						if (x0 > dgFloat32 (0.0f)) {
							x0 = x1;
						}
						if (x0 < 0.0f) {
							_DG_ASSERTE (x0 <= dgFloat32 (0.0f));
							_DG_ASSERTE (dgAbsf (plane.m_y) > dgFloat32 (0.0f));
							y = - (plane.m_x * x0 + d) / plane.m_y;
							contactsOut[count] = dgVector (x0 + r, y, dgFloat32 (0.0f), dgFloat32 (0.0f));
							count ++;
						}
					}
				}
			}

			if (count < 2) {
				dgVector dp (m_silhuette[3] - m_silhuette[2]);
				den = normal1 % dp;
				if (dgAbsf (den) > dgFloat32 (0.0f)) {
					test0 = - plane.Evalue (m_silhuette[2]) / den;
					if ((test0 <= dgFloat32 (1.0)) && (test0 >= dgFloat32 (0.0f))) {
						contactsOut[count] = m_silhuette[2] + dp.Scale (test0);
						count ++;
					}
				}
			}

			if (count < 2) {
				test0 = plane.m_w + plane.m_x * m_height[0];
				if (dgAbsf (test0) < m_radius) {
					r = m_height[0];
					d = plane.m_w + r * plane.m_x;

					a = plane.m_x * plane.m_x + plane.m_y * plane.m_y;
					b = dgFloat32 (2.0f) * plane.m_x * d;
					c = d * d - m_radius * m_radius * plane.m_y * plane.m_y;
					desc = b * b - dgFloat32 (4.0f) * a * c;
					if (desc > dgFloat32 (0.0f)) {
						_DG_ASSERTE (dgAbsf (a) > dgFloat32 (0.0f));
						desc = dgSqrt (desc);
						a = - dgFloat32 (0.5f) * b / a; 
						x0 = a + desc;
						x1 = a - desc;
						if (x0 < dgFloat32 (0.0f)) {
							x0 = x1;
						}
						if (x0 > 0.0f) {
							_DG_ASSERTE (x0 >= dgFloat32 (0.0f));
							_DG_ASSERTE (dgAbsf (plane.m_y) > dgFloat32 (0.0f));
							y = - (plane.m_x * x0 + d) / plane.m_y;
							contactsOut[count] = dgVector (x0 + r, y, dgFloat32 (0.0f), dgFloat32 (0.0f));
							count ++;
						}
					}
				}
			}
		}

		for (i = 0; i < count; i ++) {
			y = contactsOut[i].m_y;
			z = contactsOut[i].m_z;
			contactsOut[i].m_y = y * cosAng - z * sinAng; 
			contactsOut[i].m_z = z * cosAng + y * sinAng;
		}
	}
	return count;
}


void dgCollisionCapsule::GetCollisionInfo(dgCollisionInfo* info) const
{
	dgCollisionConvex::GetCollisionInfo(info);

	info->m_capsule.m_r0 = m_radius;
	info->m_capsule.m_r1 = m_radius;
	info->m_capsule.m_height = dgFloat32 (2.0f) * (m_radius + m_height[0]);
	info->m_offsetMatrix = GetOffsetMatrix();
//	strcpy (info->m_collisionType, "capsule");
	info->m_collisionType = m_collsionId;

}


void dgCollisionCapsule::Serialize(dgSerialize callback, void* const userData) const
{
	dgVector size (m_radius, dgFloat32 (2.0f) * (m_radius + m_height[0]), dgFloat32 (0.0f), dgFloat32 (0.0f));

	SerializeLow(callback, userData);
	callback (userData, &size, sizeof (dgVector));
}
