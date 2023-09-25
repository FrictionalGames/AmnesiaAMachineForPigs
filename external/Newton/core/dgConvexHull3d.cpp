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

#include "dgStdafx.h"
#include "dgStack.h"
#include "dgGoogol.h"
#include "dgConvexHull3d.h"
#include "dgSmallDeterminant.h"

#define DG_VERTEX_CLUMP_SIZE_3D		8 
class dgAABBPointTree3d
{
	public:
#ifdef _DG_DEBUG
	dgAABBPointTree3d()
	{
		static dgInt32 id = 0;
		m_id = id;
		id ++;
	}
	dgInt32 m_id;
#endif

	dgBigVector m_box[2];
	dgAABBPointTree3d* m_left;
	dgAABBPointTree3d* m_right;
	dgAABBPointTree3d* m_parent;

};

class dgAABBPointTree3dClump: public dgAABBPointTree3d
{
	public:
	dgInt32 m_count;
	dgInt32 m_indices[DG_VERTEX_CLUMP_SIZE_3D];
};


dgConvexHull3DFace::dgConvexHull3DFace()
{
	m_mark = 0; 
	m_twin[0] = NULL;
	m_twin[1] = NULL;
	m_twin[2] = NULL;
}

dgFloat64 dgConvexHull3DFace::Evalue (const dgBigVector* const pointArray, const dgBigVector& point) const
{
	const dgBigVector& p0 = pointArray[m_index[0]];
	const dgBigVector& p1 = pointArray[m_index[1]];
	const dgBigVector& p2 = pointArray[m_index[2]];

	dgFloat64 matrix[3][3];
	for (dgInt32 i = 0; i < 3; i ++) {
		matrix[0][i] = p2[i] - p0[i];
		matrix[1][i] = p1[i] - p0[i];
		matrix[2][i] = point[i] - p0[i];
	}

	dgFloat64 error;
	dgFloat64 det = Determinant3x3 (matrix, &error);
	dgFloat64 precision  = dgFloat64 (1.0f) / dgFloat64 (1<<24);
	dgFloat64 errbound = error * precision; 
	if (fabs(det) > errbound) {
		return det;
	}

	dgGoogol exactMatrix[3][3];
	for (dgInt32 i = 0; i < 3; i ++) {
		exactMatrix[0][i] = dgGoogol(p2[i]) - dgGoogol(p0[i]);
		exactMatrix[1][i] = dgGoogol(p1[i]) - dgGoogol(p0[i]);
		exactMatrix[2][i] = dgGoogol(point[i]) - dgGoogol(p0[i]);
	}

	dgGoogol exactDet (Determinant3x3(exactMatrix));
	det = exactDet.GetAproximateValue();
	return det;
}

dgBigPlane dgConvexHull3DFace::GetPlaneEquation (const dgBigVector* const pointArray) const
{
	const dgBigVector& p0 = pointArray[m_index[0]];
	const dgBigVector& p1 = pointArray[m_index[1]];
	const dgBigVector& p2 = pointArray[m_index[2]];
	dgBigPlane plane (p0, p1, p2);
	plane = plane.Scale (1.0f / sqrt (plane % plane));
	return plane;
}


dgConvexHull3d::dgConvexHull3d (dgMemoryAllocator* const allocator)
	:dgList<dgConvexHull3DFace>(allocator), m_count (0), m_diag(), m_points(1024, allocator)
{
}


dgConvexHull3d::dgConvexHull3d(dgMemoryAllocator* const allocator, const dgFloat32* const vertexCloud, dgInt32 strideInByte, dgInt32 count, dgFloat32 distTol)
	:dgList<dgConvexHull3DFace>(allocator),  m_count (0), m_diag(), m_points(count, allocator) 
{
#if (defined (WIN32) || defined(_WIN32))
	dgUnsigned32 controlWorld = dgControlFP (0xffffffff, 0);
	dgControlFP (_PC_53, _MCW_PC);
#endif

	

	dgInt32 treeCount = count / (DG_VERTEX_CLUMP_SIZE_3D>>1); 
	if (treeCount < 4) {
		treeCount = 4;
	}
	treeCount *= 2;

	dgStack<dgBigVector> points (count);
	dgStack<dgBigVector> convexPoints (count);
	dgStack<dgAABBPointTree3dClump> treePool (treeCount);

	count = InitVertexArray(&convexPoints[0], &points[0], vertexCloud, strideInByte, count, &treePool[0], treePool.GetSizeInBytes());
	if (m_count >= 4) {
		CalculateConvexHull (&treePool[0], &convexPoints[0], &points[0], count, distTol);

		m_points[m_count].m_x = 0.0f;
		dgVector* const points = &m_points[0];
		const dgBigVector* const hullPoints = &convexPoints[0];
		for (dgInt32 i = 0; i < m_count; i ++) {
			points[i] = dgVector (dgFloat32 (hullPoints[i].m_x), dgFloat32 (hullPoints[i].m_y), dgFloat32 (hullPoints[i].m_z), dgFloat32 (0.0f));
		}
	
	}

#if (defined (WIN32) || defined(_WIN32))
	dgControlFP (controlWorld, _MCW_PC);
#endif

}

dgConvexHull3d::~dgConvexHull3d(void)
{
}



dgInt32 dgConvexHull3d::ConvexCompareVertex(const dgBigVector* const  A, const dgBigVector* const B, void* const context)
{
	for (dgInt32 i = 0; i < 3; i ++) {
		if ((*A)[i] < (*B)[i]) {
			return -1;
		} else if ((*A)[i] > (*B)[i]) {
			return 1;
		}
	}
	return 0;
}



dgAABBPointTree3d* dgConvexHull3d::BuildTree (dgAABBPointTree3d* const parent, dgBigVector* const points, dgInt32 count, dgInt32 baseIndex, dgInt8** memoryPool, dgInt32& maxMemSize) const
{
	dgAABBPointTree3d* tree = NULL;

	_DG_ASSERTE (count);
	dgBigVector minP ( dgFloat32 (1.0e15f),  dgFloat32 (1.0e15f),  dgFloat32 (1.0e15f), dgFloat32 (0.0f)); 
	dgBigVector maxP (-dgFloat32 (1.0e15f), -dgFloat32 (1.0e15f), -dgFloat32 (1.0e15f), dgFloat32 (0.0f)); 
	if (count <= DG_VERTEX_CLUMP_SIZE_3D) {

		dgAABBPointTree3dClump* const clump = new (*memoryPool) dgAABBPointTree3dClump;
		*memoryPool += sizeof (dgAABBPointTree3dClump);
		maxMemSize -= sizeof (dgAABBPointTree3dClump);
		_DG_ASSERTE (maxMemSize >= 0);


		clump->m_count = count;
		for (dgInt32 i = 0; i < count; i ++) {
			clump->m_indices[i] = i + baseIndex;

			const dgBigVector& p = points[i];
			minP.m_x = GetMin (p.m_x, minP.m_x); 
			minP.m_y = GetMin (p.m_y, minP.m_y); 
			minP.m_z = GetMin (p.m_z, minP.m_z); 

			maxP.m_x = GetMax (p.m_x, maxP.m_x); 
			maxP.m_y = GetMax (p.m_y, maxP.m_y); 
			maxP.m_z = GetMax (p.m_z, maxP.m_z); 
		}

		clump->m_left = NULL;
		clump->m_right = NULL;
		tree = clump;

	} else {
		dgBigVector median (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));
		dgBigVector varian (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));
		for (dgInt32 i = 0; i < count; i ++) {

			const dgBigVector& p = points[i];
			minP.m_x = GetMin (p.m_x, minP.m_x); 
			minP.m_y = GetMin (p.m_y, minP.m_y); 
			minP.m_z = GetMin (p.m_z, minP.m_z); 

			maxP.m_x = GetMax (p.m_x, maxP.m_x); 
			maxP.m_y = GetMax (p.m_y, maxP.m_y); 
			maxP.m_z = GetMax (p.m_z, maxP.m_z); 

			median += p;
			varian += p.CompProduct (p);
		}

		varian = varian.Scale (dgFloat32 (count)) - median.CompProduct(median);
		dgInt32 index = 0;
		dgFloat64 maxVarian = dgFloat64 (-1.0e10f);
		for (dgInt32 i = 0; i < 3; i ++) {
			if (varian[i] > maxVarian) {
				index = i;
				maxVarian = varian[i];
			}
		}
		dgBigVector center = median.Scale (dgFloat64 (1.0f) / dgFloat64 (count));

		dgFloat64 test = center[index];

		dgInt32 i0 = 0;
		dgInt32 i1 = count - 1;
		do {    
			for (; i0 <= i1; i0 ++) {
				dgFloat64 val = points[i0][index];
				if (val > test) {
					break;
				}
			}

			for (; i1 >= i0; i1 --) {
				dgFloat64 val = points[i1][index];
				if (val < test) {
					break;
				}
			}

			if (i0 < i1)	{
				Swap(points[i0], points[i1]);
				i0++; 
				i1--;
			}
		} while (i0 <= i1);

		if (i0 == 0){
			i0 = count / 2;
		}
		if (i0 == (count - 1)){
			i0 = count / 2;
		}

		tree = new (*memoryPool) dgAABBPointTree3d;
		*memoryPool += sizeof (dgAABBPointTree3d);
		maxMemSize -= sizeof (dgAABBPointTree3d);
		_DG_ASSERTE (maxMemSize >= 0);

		_DG_ASSERTE (i0);
		_DG_ASSERTE (count - i0);

		tree->m_left = BuildTree (tree, points, i0, baseIndex, memoryPool, maxMemSize);
		tree->m_right = BuildTree (tree, &points[i0], count - i0, i0 + baseIndex, memoryPool, maxMemSize);
	}

	_DG_ASSERTE (tree);
	tree->m_parent = parent;
	tree->m_box[0] = minP - dgBigVector (dgFloat64 (1.0e-3f), dgFloat64 (1.0e-3f), dgFloat64 (1.0e-3f), dgFloat64 (1.0f));
	tree->m_box[1] = maxP + dgBigVector (dgFloat64 (1.0e-3f), dgFloat64 (1.0e-3f), dgFloat64 (1.0e-3f), dgFloat64 (1.0f));
	return tree;
}





dgInt32 dgConvexHull3d::InitVertexArray(dgBigVector* const convexPoints, dgBigVector* const points, const dgFloat32* const vertexCloud, dgInt32 strideInBytes, dgInt32 count, void* const memoryPool, dgInt32 maxMemSize)
{
	dgInt32 stride = dgInt32 (strideInBytes / sizeof (dgFloat32));
	for (dgInt32 i = 0; i < count; i ++) {
		dgInt32 index = i * stride;
		points[i] = dgBigVector (vertexCloud[index], vertexCloud[index + 1], vertexCloud[index + 2], dgFloat64 (0.0f));
	}

	dgSort(points, count, ConvexCompareVertex);

	dgInt32 indexCount = 0;
	for (int i = 1; i < count; i ++) {
		for (; i < count; i ++) {
			if (ConvexCompareVertex (&points[indexCount], &points[i], NULL)) {
				indexCount ++;
				points[indexCount] = points[i];
				break;
			}
		}
	}
	count = indexCount + 1;
	if (count < 4) {
		m_count = 0;
		_DG_ASSERTE (0);
		return count;
	}

	dgAABBPointTree3d* tree = BuildTree (NULL, points, count, 0, (dgInt8**) &memoryPool, maxMemSize);

	dgBigVector boxSize (tree->m_box[1] - tree->m_box[0]);	
	m_diag = dgFloat32 (sqrt (boxSize % boxSize));

	dgStack<dgBigVector> normalArrayPool (256);
	dgBigVector* const normalArray = &normalArrayPool[0];
	dgInt32 normalCount = BuildNormalList (&normalArray[0]);

	dgInt32 index = SupportVertex (&tree, points, normalArray[0]);
	convexPoints[0] = points[index];
	points[index].m_w = dgFloat64 (1.0f);

	bool validTetrahedrum = false;
	dgBigVector e1 (dgFloat64 (0.0f), dgFloat64 (0.0f), dgFloat64 (0.0f), dgFloat64 (0.0f)) ;
	for (dgInt32 i = 1; i < normalCount; i ++) {
		dgInt32 index = SupportVertex (&tree, points, normalArray[i]);
		_DG_ASSERTE (index >= 0);

		e1 = points[index] - convexPoints[0];
		dgFloat64 error2 = e1 % e1;
		if (error2 > (dgFloat32 (1.0e-4f) * m_diag * m_diag)) {
			convexPoints[1] = points[index];
			points[index].m_w = dgFloat64 (1.0f);
			validTetrahedrum = true;
			break;
		}
	}
	if (!validTetrahedrum) {
		m_count = 0;
		_DG_ASSERTE (0);
		return count;
	}

	validTetrahedrum = false;
	dgBigVector e2(dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));;
	dgBigVector normal (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));
	for (dgInt32 i = 2; i < normalCount; i ++) {
		dgInt32 index = SupportVertex (&tree, points, normalArray[i]);
		_DG_ASSERTE (index >= 0);
		e2 = points[index] - convexPoints[0];
		normal = e1 * e2;
		dgFloat64 error2 = sqrt (normal % normal);
		if (error2 > (dgFloat32 (1.0e-4f) * m_diag * m_diag)) {
			convexPoints[2] = points[index];
			points[index].m_w = dgFloat64 (1.0f);
			validTetrahedrum = true;
			break;
		}
	}

	if (!validTetrahedrum) {
		m_count = 0;
		_DG_ASSERTE (0);
		return count;
	}

	// find the largest possible tetrahedron
	validTetrahedrum = false;
	dgBigVector e3(dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));;
	for (dgInt32 i = 3; i < normalCount; i ++) {
		dgInt32 index = SupportVertex (&tree, points, normalArray[i]);
		_DG_ASSERTE (index >= 0);

		//make sure the volume of the fist tetrahedral is no negative
		e3 = points[index] - convexPoints[0];
		dgFloat64 error2 = normal % e3;
//		if (fabs (error2) > (dgFloat64 (1.0e-4f) * m_diag * m_diag * m_diag)) {
		if (fabs (error2) > (dgFloat64 (1.0e-4f) * m_diag * m_diag)) {
			// we found a valid tetrahedra, about and start build the hull by adding the rest of the points
			convexPoints[3] = points[index];
			points[index].m_w = dgFloat64 (1.0f);
			validTetrahedrum = true;
			break;
		}
	}
	if (!validTetrahedrum) {
		// the point do no form a convex hull
		m_count = 0;
		_DG_ASSERTE (0);
		return count;
	}

	m_count = 4;
	dgFloat64 volume = TetrahedrumVolume (convexPoints[0], convexPoints[1], convexPoints[2], convexPoints[3]);
	if (volume > dgFloat64 (0.0f)) {
		Swap(convexPoints[2], convexPoints[3]);
	}
	_DG_ASSERTE (TetrahedrumVolume(convexPoints[0], convexPoints[1], convexPoints[2], convexPoints[3]) < dgFloat64(0.0f));

	return count;
}

dgFloat64 dgConvexHull3d::TetrahedrumVolume (const dgBigVector& p0, const dgBigVector& p1, const dgBigVector& p2, const dgBigVector& p3) const
{
	dgBigVector p1p0 (p1 - p0);
	dgBigVector p2p0 (p2 - p0);
	dgBigVector p3p0 (p3 - p0);
	return (p1p0 * p2p0) % p3p0;
}


void dgConvexHull3d::TessellateTriangle (dgInt32 level, const dgVector& p0, const dgVector& p1, const dgVector& p2, dgInt32& count, dgBigVector* const ouput, dgInt32& start) const
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

		TessellateTriangle  (level - 1, p0,  p01, p20, count, ouput, start);
		TessellateTriangle  (level - 1, p1,  p12, p01, count, ouput, start);
		TessellateTriangle  (level - 1, p2,  p20, p12, count, ouput, start);
		TessellateTriangle  (level - 1, p01, p12, p20, count, ouput, start);

	} else {
		dgBigPlane n (p0, p1, p2);
		n = n.Scale (dgFloat64(1.0f) / sqrt (n % n));
		n.m_w = dgFloat64(0.0f);
		ouput[start] = n;
		start += 8;
		count ++;
	}
}


dgInt32 dgConvexHull3d::SupportVertex (dgAABBPointTree3d** const treePointer, const dgBigVector* const points, const dgBigVector& dir) const
{
/*
	dgFloat64 dist = dgFloat32 (-1.0e10f);
	dgInt32 index = -1;
	for (dgInt32 i = 0; i < count; i ++) {
		//dgFloat64 dist1 = dir.DotProduct4 (points[i]);
		dgFloat64 dist1 = dir % points[i];
		if (dist1 > dist) {
			dist = dist1;
			index = i;
		}
	}
	_DG_ASSERTE (index != -1);
	return index;
*/

	#define DG_STACK_DEPTH_3D 64
	dgFloat64 aabbProjection[DG_STACK_DEPTH_3D];
	const dgAABBPointTree3d *stackPool[DG_STACK_DEPTH_3D];

	dgInt32 index = -1;
	dgInt32 stack = 1;
	stackPool[0] = *treePointer;
	aabbProjection[0] = dgFloat32 (1.0e20f);
	dgFloat64 maxProj = dgFloat64 (-1.0e20f); 
	dgInt32 ix = (dir[0] > dgFloat64 (0.0f)) ? 1 : 0;
	dgInt32 iy = (dir[1] > dgFloat64 (0.0f)) ? 1 : 0;
	dgInt32 iz = (dir[2] > dgFloat64 (0.0f)) ? 1 : 0;
	while (stack) {
		stack--;
		dgFloat64 boxSupportValue = aabbProjection[stack];
		if (boxSupportValue > maxProj) {
			const dgAABBPointTree3d* const me = stackPool[stack];

			if (me->m_left && me->m_right) {
				dgBigVector leftSupportPoint (me->m_left->m_box[ix].m_x, me->m_left->m_box[iy].m_y, me->m_left->m_box[iz].m_z, dgFloat32 (0.0));
				dgFloat64 leftSupportDist = leftSupportPoint % dir;

				dgBigVector rightSupportPoint (me->m_right->m_box[ix].m_x, me->m_right->m_box[iy].m_y, me->m_right->m_box[iz].m_z, dgFloat32 (0.0));
				dgFloat64 rightSupportDist = rightSupportPoint % dir;


				if (rightSupportDist >= leftSupportDist) {
					aabbProjection[stack] = leftSupportDist;
					stackPool[stack] = me->m_left;
					stack++;
					_DG_ASSERTE (stack < DG_STACK_DEPTH_3D);
					aabbProjection[stack] = rightSupportDist;
					stackPool[stack] = me->m_right;
					stack++;
					_DG_ASSERTE (stack < DG_STACK_DEPTH_3D);
				} else {
					aabbProjection[stack] = rightSupportDist;
					stackPool[stack] = me->m_right;
					stack++;
					_DG_ASSERTE (stack < DG_STACK_DEPTH_3D);
					aabbProjection[stack] = leftSupportDist;
					stackPool[stack] = me->m_left;
					stack++;
					_DG_ASSERTE (stack < DG_STACK_DEPTH_3D);
				}

			} else {
				dgAABBPointTree3dClump* const clump = (dgAABBPointTree3dClump*) me;
				for (dgInt32 i = 0; i < clump->m_count; i ++) {
					const dgBigVector& p = points[clump->m_indices[i]];
					_DG_ASSERTE (p.m_x >= clump->m_box[0].m_x);
					_DG_ASSERTE (p.m_x <= clump->m_box[1].m_x);
					_DG_ASSERTE (p.m_y >= clump->m_box[0].m_y);
					_DG_ASSERTE (p.m_y <= clump->m_box[1].m_y);
					_DG_ASSERTE (p.m_z >= clump->m_box[0].m_z);
					_DG_ASSERTE (p.m_z <= clump->m_box[1].m_z);
					if (p.m_w == dgFloat64 (0.0f)) {
						dgFloat64 dist = p % dir;
						if (dist > maxProj) {
							maxProj = dist;
							index = clump->m_indices[i];
						}
					} else {
						clump->m_indices[i] = clump->m_indices[clump->m_count - 1];
						clump->m_count = clump->m_count - 1;
						i --;
					}
				}

				if (clump->m_count == 0) {
					dgAABBPointTree3d* const parent = clump->m_parent;
					if (parent) {	
						dgAABBPointTree3d* const sibling = (parent->m_left != clump) ? parent->m_left : parent->m_right;
						_DG_ASSERTE (sibling != clump);
						dgAABBPointTree3d* const grandParent = parent->m_parent;
						if (grandParent) {
							sibling->m_parent = grandParent;
							if (grandParent->m_right == parent) {
								grandParent->m_right = sibling;
							} else {
								grandParent->m_left = sibling;
							}
						} else {
							sibling->m_parent = NULL;
							*treePointer = sibling;
						}
					}
				}
			}
		}
	}

	_DG_ASSERTE (index != -1);
	return index;
}


dgInt32 dgConvexHull3d::BuildNormalList (dgBigVector* const normalArray) const
{
	dgVector p0 ( dgFloat32 (1.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f)); 
	dgVector p1 (-dgFloat32 (1.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f)); 
	dgVector p2 ( dgFloat32 (0.0f), dgFloat32 (1.0f), dgFloat32 (0.0f), dgFloat32 (0.0f)); 
	dgVector p3 ( dgFloat32 (0.0f),-dgFloat32 (1.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));
	dgVector p4 ( dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (1.0f), dgFloat32 (0.0f));
	dgVector p5 ( dgFloat32 (0.0f), dgFloat32 (0.0f),-dgFloat32 (1.0f), dgFloat32 (0.0f));

	dgInt32 count = 0;
	dgInt32 subdivitions = 1;

	dgInt32 start = 0;
	TessellateTriangle  (subdivitions, p4, p0, p2, count, normalArray, start);
	start = 1;
	TessellateTriangle  (subdivitions, p5, p3, p1, count, normalArray, start);
	start = 2;
	TessellateTriangle  (subdivitions, p5, p1, p2, count, normalArray, start);
	start = 3;
	TessellateTriangle  (subdivitions, p4, p3, p0, count, normalArray, start);
	start = 4;
	TessellateTriangle  (subdivitions, p4, p2, p1, count, normalArray, start);
	start = 5;
	TessellateTriangle  (subdivitions, p5, p0, p3, count, normalArray, start);
	start = 6;
	TessellateTriangle  (subdivitions, p5, p2, p0, count, normalArray, start);
	start = 7;
	TessellateTriangle  (subdivitions, p4, p1, p3, count, normalArray, start);
	return count;
}

dgConvexHull3d::dgListNode* dgConvexHull3d::AddFace (dgInt32 i0, dgInt32 i1, dgInt32 i2)
{
	dgListNode* const node = Append();
	dgConvexHull3DFace& face = node->GetInfo();

	face.m_index[0] = i0; 
	face.m_index[1] = i1; 
	face.m_index[2] = i2; 
	return node;
}

void dgConvexHull3d::DeleteFace (dgListNode* const node) 
{
	Remove (node);
}

bool dgConvexHull3d::Sanity() const
{
/*
	for (dgListNode* node = GetFirst(); node; node = node->GetNext()) {
		dgConvexHull3DFace* const face = &node->GetInfo();		
		for (dgInt32 i = 0; i < 3; i ++) {
			dgListNode* const twinNode = face->m_twin[i];
			if (!twinNode) {
				return false;
			}

			dgInt32 count = 0;
			dgListNode* me = NULL;
			dgConvexHull3DFace* const twinFace = &twinNode->GetInfo();
			for (dgInt32 j = 0; j < 3; j ++) {
				if (twinFace->m_twin[j] == node) {
					count ++;
					me = twinFace->m_twin[j];
				}
			}
			if (count != 1) {
				return false;
			}
			if (me != node) {
				return false;
			}
		}
	}
*/
	return true;
}

void dgConvexHull3d::CalculateConvexHull (
	dgAABBPointTree3d* vertexTree, 
	dgBigVector* const hullVertexArray, 
	dgBigVector* const points, 
	dgInt32 count, 
	dgFloat32 distTol)
{
	distTol = dgAbsf (distTol) * m_diag;
	dgListNode* const f0Node = AddFace (0, 1, 2);
	dgListNode* const f1Node = AddFace (0, 2, 3);
	dgListNode* const f2Node = AddFace (2, 1, 3);
	dgListNode* const f3Node = AddFace (1, 0, 3);

	dgConvexHull3DFace* const f0 = &f0Node->GetInfo();
	dgConvexHull3DFace* const f1 = &f1Node->GetInfo();
	dgConvexHull3DFace* const f2 = &f2Node->GetInfo();
	dgConvexHull3DFace* const f3 = &f3Node->GetInfo();

	f0->m_twin[0] = (dgList<dgConvexHull3DFace>::dgListNode*)f3Node; 
	f0->m_twin[1] = (dgList<dgConvexHull3DFace>::dgListNode*)f2Node; 
	f0->m_twin[2] = (dgList<dgConvexHull3DFace>::dgListNode*)f1Node;

	f1->m_twin[0] = (dgList<dgConvexHull3DFace>::dgListNode*)f0Node; 
	f1->m_twin[1] = (dgList<dgConvexHull3DFace>::dgListNode*)f2Node; 
	f1->m_twin[2] = (dgList<dgConvexHull3DFace>::dgListNode*)f3Node;

	f2->m_twin[0] = (dgList<dgConvexHull3DFace>::dgListNode*)f0Node; 
	f2->m_twin[1] = (dgList<dgConvexHull3DFace>::dgListNode*)f3Node; 
	f2->m_twin[2] = (dgList<dgConvexHull3DFace>::dgListNode*)f1Node;

	f3->m_twin[0] = (dgList<dgConvexHull3DFace>::dgListNode*)f0Node; 
	f3->m_twin[1] = (dgList<dgConvexHull3DFace>::dgListNode*)f1Node; 
	f3->m_twin[2] = (dgList<dgConvexHull3DFace>::dgListNode*)f2Node;
	
	dgList<dgListNode*> boundaryFaces(GetAllocator());

	boundaryFaces.Append(f0Node);
	boundaryFaces.Append(f1Node);
	boundaryFaces.Append(f2Node);
	boundaryFaces.Append(f3Node);

	dgStack<dgListNode*> stackPool(1024 + m_count);
	dgStack<dgListNode*> coneListPool(1024 + m_count);
	dgStack<dgListNode*> deleteListPool(1024 + m_count);

	dgListNode** const stack = &stackPool[0];
	dgListNode** const coneList = &stackPool[0];
	dgListNode** const deleteList = &deleteListPool[0];

	count -= 4;
	dgInt32 currentIndex = 4;
	while (boundaryFaces.GetCount() && count) {

		dgListNode* const faceNode = boundaryFaces.GetFirst()->GetInfo();
		dgConvexHull3DFace* const face = &faceNode->GetInfo();
		dgBigPlane planeEquation (face->GetPlaneEquation (hullVertexArray));

		dgInt32 index = SupportVertex (&vertexTree, points, planeEquation);
		const dgBigVector& p = points[index];
		dgFloat64 dist = planeEquation.Evalue(p);
		if ((dist >= distTol) && (face->Evalue(hullVertexArray, p) > dgFloat64(0.0f))) {
			_DG_ASSERTE (Sanity());
			
			_DG_ASSERTE (faceNode);
			stack[0] = faceNode;

			dgInt32 stackIndex = 1;
			dgInt32 deletedCount = 0;

			while (stackIndex) {
				stackIndex --;
				dgListNode* const node = stack[stackIndex];
				dgConvexHull3DFace* const face = &node->GetInfo();

				if (!face->m_mark && (face->Evalue(hullVertexArray, p) > dgFloat64(0.0f))) { 
					#ifdef _DG_DEBUG
					for (dgInt32 i = 0; i < deletedCount; i ++) {
						_DG_ASSERTE (deleteList[i] != node);
					}
					#endif

					deleteList[deletedCount] = node;
					deletedCount ++;
					_DG_ASSERTE (deletedCount < dgInt32 (deleteListPool.GetElementsCount()));
					face->m_mark = 1;
					for (dgInt32 i = 0; i < 3; i ++) {
						dgListNode* const twinNode = (dgListNode*)face->m_twin[i];
						_DG_ASSERTE (twinNode);
						dgConvexHull3DFace* const twinFace = &twinNode->GetInfo();
						if (!twinFace->m_mark) {
							stack[stackIndex] = twinNode;
							stackIndex ++;
							_DG_ASSERTE (stackIndex < dgInt32 (stackPool.GetElementsCount()));
						}
					}
				}
			}

//			Swap (hullVertexArray[index], hullVertexArray[currentIndex]);
			hullVertexArray[currentIndex] = points[index];
			points[index].m_w = dgFloat64 (1.0f);

			dgInt32 newCount = 0;
			for (dgInt32 i = 0; i < deletedCount; i ++) {
				dgListNode* const node = deleteList[i];
				dgConvexHull3DFace* const face = &node->GetInfo();
				_DG_ASSERTE (face->m_mark == 1);
				for (dgInt32 j0 = 0; j0 < 3; j0 ++) {
					dgListNode* const twinNode = face->m_twin[j0];
					dgConvexHull3DFace* const twinFace = &twinNode->GetInfo();
					if (!twinFace->m_mark) {
						dgInt32 j1 = (j0 == 2) ? 0 : j0 + 1;
						dgListNode* const newNode = AddFace (currentIndex, face->m_index[j0], face->m_index[j1]);
						boundaryFaces.Addtop(newNode);

						dgConvexHull3DFace* const newFace = &newNode->GetInfo();
						newFace->m_twin[1] = twinNode;
						for (dgInt32 k = 0; k < 3; k ++) {
							if (twinFace->m_twin[k] == node) {
								twinFace->m_twin[k] = newNode;
							}
						}
						coneList[newCount] = newNode;
						newCount ++;
						_DG_ASSERTE (newCount < dgInt32 (coneListPool.GetElementsCount()));
					}
				}
			}
			
			for (dgInt32 i = 0; i < newCount - 1; i ++) {
				dgListNode* const nodeA = coneList[i];
				dgConvexHull3DFace* const faceA = &nodeA->GetInfo();
				_DG_ASSERTE (faceA->m_mark == 0);
				for (dgInt32 j = i + 1; j < newCount; j ++) {
					dgListNode* const nodeB = coneList[j];
					dgConvexHull3DFace* const faceB = &nodeB->GetInfo();
					_DG_ASSERTE (faceB->m_mark == 0);
					if (faceA->m_index[2] == faceB->m_index[1]) {
						faceA->m_twin[2] = nodeB;
						faceB->m_twin[0] = nodeA;
						break;
					}
				}

				for (dgInt32 j = i + 1; j < newCount; j ++) {
					dgListNode* const nodeB = coneList[j];
					dgConvexHull3DFace* const faceB = &nodeB->GetInfo();
					_DG_ASSERTE (faceB->m_mark == 0);
					if (faceA->m_index[1] == faceB->m_index[2]) {
						faceA->m_twin[0] = nodeB;
						faceB->m_twin[2] = nodeA;
						break;
					}
				}
			}

			for (dgInt32 i = 0; i < deletedCount; i ++) {
				dgListNode* const node = deleteList[i];
				boundaryFaces.Remove (node);
				DeleteFace (node); 
			}

			currentIndex ++;
			count --;
		} else {
			boundaryFaces.Remove (faceNode);
		}
	}
	m_count = currentIndex;
}



