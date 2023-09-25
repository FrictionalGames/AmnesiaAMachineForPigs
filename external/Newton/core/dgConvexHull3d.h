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

#ifndef __DG_CONVEXHULL_3D__
#define __DG_CONVEXHULL_3D__

#include "dgStdafx.h"
#include "dgList.h"
#include "dgArray.h"
#include "dgPlane.h"
#include "dgVector.h"
#include "dgMatrix.h"
#include "dgQuaternion.h"

class dgMemoryAllocator;
class dgAABBPointTree3d;

class dgConvexHull3DFace
{
	public:
	dgConvexHull3DFace();
	dgInt32 m_index[3]; 
	
	private:
	dgFloat64 Evalue (const dgBigVector* const pointArray, const dgBigVector& point) const;
	dgBigPlane GetPlaneEquation (const dgBigVector* const pointArray) const;

	dgInt32 m_mark;
	dgList<dgConvexHull3DFace>::dgListNode* m_twin[3];
	friend class dgConvexHull3d;
};


class dgConvexHull3d: public dgList<dgConvexHull3DFace>
{
	public:
	dgConvexHull3d(dgMemoryAllocator* const allocator, const dgFloat32* const vertexCloud, dgInt32 strideInBytes, dgInt32 count, dgFloat32 distTol);
	virtual ~dgConvexHull3d();

	dgInt32 GetVertexCount() const;
	const dgVector* GetVertexPool() const;
	const dgVector& GetVertex(dgInt32 i) const;
	

	protected:
	dgConvexHull3d(dgMemoryAllocator* const allocator);

	virtual dgListNode* AddFace (dgInt32 i0, dgInt32 i1, dgInt32 i2);
	virtual void DeleteFace (dgListNode* const node) ;
	virtual dgInt32 InitVertexArray(dgBigVector* const convexPoints, dgBigVector* const points, const dgFloat32* const vertexCloud, dgInt32 strideInBytes, dgInt32 count, void* const memoryPool, dgInt32 maxMemSize);

	void CalculateConvexHull (dgAABBPointTree3d* vertexTree, dgBigVector* const convexPoints, dgBigVector* const points, dgInt32 count, dgFloat32 distTol);
	dgInt32 BuildNormalList (dgBigVector* const normalArray) const;
	dgInt32 SupportVertex (dgAABBPointTree3d** const tree, const dgBigVector* const points, const dgBigVector& dir) const;
	dgFloat64 TetrahedrumVolume (const dgBigVector& p0, const dgBigVector& p1, const dgBigVector& p2, const dgBigVector& p3) const;
	void TessellateTriangle (dgInt32 level, const dgVector& p0, const dgVector& p1, const dgVector& p2, dgInt32& count, dgBigVector* const ouput, dgInt32& start) const;

	dgAABBPointTree3d* BuildTree (dgAABBPointTree3d* const parent, dgBigVector* const points, dgInt32 count, dgInt32 baseIndex, dgInt8** const memoryPool, dgInt32& maxMemSize) const;
	static dgInt32 ConvexCompareVertex(const dgBigVector* const  A, const dgBigVector* const B, void* const context);
	bool Sanity() const;

	dgInt32 m_count;
	dgFloat32 m_diag;
	dgArray<dgVector> m_points;
};


inline dgInt32 dgConvexHull3d::GetVertexCount() const
{
	return m_count;
}

inline const dgVector* dgConvexHull3d::GetVertexPool() const
{
	return &m_points[0];
}

inline const dgVector& dgConvexHull3d::GetVertex(dgInt32 index) const
{
//	return dgVector (dgFloat32 (m_points___[index].m_x), dgFloat32 (m_points___[index].m_y), dgFloat32 (m_points___[index].m_z), dgFloat32 (m_points___[index].m_w));
	return m_points[index];
}


#endif
