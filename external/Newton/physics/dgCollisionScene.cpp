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
#include "dgIntersections.h"
#include "dgCollisionScene.h"
#include "dgCollisionNull.h"

#define DG_SCENE_AABB_SCALE		dgFloat32 (4.0f)
#define DG_SCENE_AABB_INV_SCALE	(dgFloat32 (1.0f) / DG_SCENE_AABB_SCALE)


dgCollisionScene::dgNode::dgNode ()
	:m_parent(NULL)
	,m_left(NULL)
	,m_right(NULL)
	,m_fitnessNode(NULL)
{
}

dgCollisionScene::dgNode::dgNode (dgNode* const sibling, dgNode* const myNode)
	:m_parent(sibling->m_parent)
	,m_left(sibling)
	,m_right(myNode)
	,m_fitnessNode(NULL)
{
	if (m_parent) {
		if (m_parent->m_left == sibling) {
			m_parent->m_left = this;
		} else {
			_DG_ASSERTE (m_parent->m_right == sibling);
			m_parent->m_right = this;
		}
	}

	sibling->m_parent = this;
	myNode->m_parent = this;

	dgNode* const left = m_left;
	dgNode* const right = m_right;

	m_minBox = dgVector (GetMin (left->m_minBox.m_x, right->m_minBox.m_x), GetMin (left->m_minBox.m_y, right->m_minBox.m_y), GetMin (left->m_minBox.m_z, right->m_minBox.m_z), dgFloat32 (0.0f));
	m_maxBox = dgVector (GetMax (left->m_maxBox.m_x, right->m_maxBox.m_x), GetMax (left->m_maxBox.m_y, right->m_maxBox.m_y), GetMax (left->m_maxBox.m_z, right->m_maxBox.m_z), dgFloat32 (0.0f));

	dgVector side0 (m_maxBox - m_minBox);
	dgVector side1 (side0.m_y, side0.m_z, side0.m_x, dgFloat32 (0.0f));
	m_surfaceArea = side0 % side1;
}


dgCollisionScene::dgNode::~dgNode ()
{
	if (m_left) {
		delete (m_left);
	}
	if (m_right) {
		delete (m_right);
	}
}

dgCollisionScene::dgProxy::dgProxy (dgCollision* m_shape, /*const dgMatrix& matrix,*/ dgCollisionScene* const owner)
	:dgNode ()
	,m_shape (m_shape)
	,m_owner (owner)
{
	dgVector boxP0;
	dgVector boxP1;

	m_matrix = m_shape->GetOffsetMatrix()/* * matrix*/;
	m_shape->CalcAABB (m_matrix, boxP0, boxP1);

	dgVector p0 (boxP0.CompProduct(dgVector (DG_SCENE_AABB_SCALE, DG_SCENE_AABB_SCALE, DG_SCENE_AABB_SCALE, dgFloat32 (0.0f))));
	dgVector p1 (boxP1.CompProduct(dgVector (DG_SCENE_AABB_SCALE, DG_SCENE_AABB_SCALE, DG_SCENE_AABB_SCALE, dgFloat32 (0.0f))));

	m_minBox.m_x = dgFloor (p0.m_x) * DG_SCENE_AABB_INV_SCALE; 
	m_minBox.m_y = dgFloor (p0.m_y) * DG_SCENE_AABB_INV_SCALE;  
	m_minBox.m_z = dgFloor (p0.m_z) * DG_SCENE_AABB_INV_SCALE;  
	m_minBox.m_w = dgFloat32 (0.0f);

	m_maxBox.m_x = dgFloor (p1.m_x + dgFloat32 (1.0f)) * DG_SCENE_AABB_INV_SCALE;  
	m_maxBox.m_y = dgFloor (p1.m_y + dgFloat32 (1.0f)) * DG_SCENE_AABB_INV_SCALE;  
	m_maxBox.m_z = dgFloor (p1.m_z + dgFloat32 (1.0f)) * DG_SCENE_AABB_INV_SCALE;  
	m_maxBox.m_w = dgFloat32 (0.0f);

	dgVector side0 (m_maxBox - m_minBox);
	dgVector side1 (side0.m_y, side0.m_z, side0.m_x, dgFloat32 (0.0f));
	m_surfaceArea = side0 % side1;
}

dgCollisionScene::dgProxy::~dgProxy()
{
}


dgCollisionScene::dgCollisionScene(dgWorld* world)
	:dgCollision (world->GetAllocator(), 0, dgGetIdentityMatrix(), m_sceneCollision)
	,m_lock(0)
	,m_list(world->GetAllocator())
	,m_fitnessList(world->GetAllocator())
{
	m_world = world;
	m_rootNode = NULL;

	m_rtti |= dgCollisionScene_RTTI;
	SetCollisionBBox (dgVector (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f)),
					  dgVector (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f)));

}

dgCollisionScene::dgCollisionScene (dgWorld* const world, dgDeserialize deserialization, void* const userData)
	:dgCollision (world, deserialization, userData)
	,m_lock(0)
	,m_list(world->GetAllocator())
	,m_fitnessList(world->GetAllocator())
{
	_DG_ASSERTE (0);
/*
	dgInt32 stack;
	dgInt32 nodeCount;
	dgInt32 data[4];
	dgNode *stackPool[64];

	m_world = world;
	m_rtti |= dgCollisionScene_RTTI;

	deserialization (userData, &data, sizeof (data));
	dgStack<dgList<dgProxy>::dgListNode*> array(data[0]);

	for (dgInt32 i = 0; i < data[0]; i ++) {
		
		dgVector p0;
		dgVector p1;
		dgMatrix matrix;
		dgCollision* collision;
		deserialization (userData, &matrix, sizeof (dgMatrix));
		deserialization (userData, &p0, sizeof (dgVector));
		deserialization (userData, &p1, sizeof (dgVector));

		collision = m_world->CreateFromSerialization (deserialization, userData);

		array[i] = (dgList<dgProxy>::dgListNode*)AddProxy (collision);

		dgProxy& proxy = array[i]->GetInfo();
		proxy.m_matrix = matrix;
		proxy.m_boxP0 = p0;
		proxy.m_boxP1 = p1;

		collision->Release();
	}

	m_rootNode = NULL;
	deserialization (userData, &nodeCount, sizeof (dgInt32));

	m_rootNode = new (m_allocator) dgNode;
	m_rootNode->m_parent = NULL;

	stack = 1;
	stackPool[0] = m_rootNode;
	while (stack) {
		dgInt32 left;
		dgInt32 right;

		stack --;
		dgNode* const node = stackPool[stack];

		deserialization (userData, &node->m_p0, sizeof (dgVector));
		deserialization (userData, &node->m_p1, sizeof (dgVector));
		deserialization (userData, &left, sizeof (dgInt32));
		deserialization (userData, &right, sizeof (dgInt32));

		node->m_nodeIsDirty = false;
		node->m_leftIsProxy = dgInt8 (left);
		node->m_rightIsProxy = dgInt8 (right);

		if (node->m_rightIsProxy) {
			dgInt32 proxyIndex;
			deserialization  (userData, &proxyIndex, sizeof (dgInt32));
			node->m_rightProxi = array[proxyIndex];

		} else {
			dgNode* const child = new (m_allocator) dgNode;
			child->m_parent = node;
			node->m_rightNode = child;
			_DG_ASSERTE (node->m_rightIsProxy == false);
			_DG_ASSERTE (stack < sizeof (stackPool) / sizeof (dgNode*));
			stackPool[stack] = node->m_rightNode;
			stack++;
		}

		if (node->m_leftIsProxy) {
			dgInt32 proxyIndex;
			deserialization  (userData, &proxyIndex, sizeof (dgInt32));
			node->m_leftProxi = array[proxyIndex];

		} else {
			dgNode* const child = new (m_allocator) dgNode;
			child->m_parent = node;
			node->m_leftNode = child;
			_DG_ASSERTE (node->m_leftIsProxy == false);
			_DG_ASSERTE (stack < sizeof (stackPool) / sizeof (dgNode*));
			stackPool[stack] = node->m_leftNode;
			stack++;
		}
	}
*/
}

dgCollisionScene::~dgCollisionScene(void)
{
	for (dgList<dgProxy*>::dgListNode* node = m_list.GetFirst(); node; ) {
		dgList<dgProxy*>::dgListNode* const proxyNode = node;
		node = node->GetNext();
		RemoveProxy(proxyNode);
	}
}

void dgCollisionScene::Serialize(dgSerialize callback, void* const userData) const
{
	_DG_ASSERTE (0);
/*
	dgInt32 index;
	dgInt32 stack;
	dgInt32 data[4];
	dgNode *stackPool[64];

	SerializeLow(callback, userData);

	data[0] = m_list.GetCount();
	data[1] = 0;
	data[2] = 0;
	data[3] = 0;
	callback (userData, &data, sizeof (data));

	index = 0;
	for (dgList<dgProxy>::dgListNode* node = m_list.GetFirst(); node; node = node->GetNext()) {
		dgProxy& proxy = node->GetInfo();
		proxy.m_boxP0.m_w = dgFloat32 (index); 
		index ++;

		callback (userData, &proxy.m_matrix, sizeof (dgMatrix));
		callback (userData, &proxy.m_boxP0, sizeof (dgVector));
		callback (userData, &proxy.m_boxP1, sizeof (dgVector));
		m_world->Serialize (proxy.m_shape, callback, userData);
	}

#ifdef _DG_DEBUG
	index = 0;
	stack = 1;
	stackPool[0] = m_rootNode;
	while (stack) {
		stack --;
		dgNode* const node = stackPool[stack];
		index ++;

		if (node->m_rightIsProxy == false) {
			_DG_ASSERTE (node->m_rightNode);
			_DG_ASSERTE (stack < sizeof (stackPool) / sizeof (dgNode*));
			stackPool[stack] = node->m_rightNode;
			stack++;
		}
	
		if (node->m_leftIsProxy == false) {
			_DG_ASSERTE (node->m_leftNode);
			_DG_ASSERTE (stack < sizeof (stackPool) / sizeof (dgNode*));
			stackPool[stack] = node->m_leftNode;
			stack++;
		}
	}
	_DG_ASSERTE (index == (m_list.GetCount() - 1));
#endif

	index = m_list.GetCount() - 1;
	callback (userData, &index, sizeof (dgInt32));

	stack = 1;
	stackPool[0] = m_rootNode;
	while (stack) {
		stack --;
		const dgNode* const node = stackPool[stack];

		dgInt32 left  = node->m_leftIsProxy;
		dgInt32 right = node->m_rightIsProxy;
		callback (userData, &node->m_p0, sizeof (dgVector));
		callback (userData, &node->m_p1, sizeof (dgVector));
		callback (userData, &left, sizeof (dgInt32));
		callback (userData, &right, sizeof (dgInt32));

		if (node->m_rightIsProxy) {
			dgInt32 proxyIndex;
			const dgProxy& sceneProxy = node->m_rightProxi->GetInfo();
			proxyIndex = dgInt32 (sceneProxy.m_boxP0.m_w);
			callback (userData, &proxyIndex, sizeof (dgInt32));
		} else {
			_DG_ASSERTE (node->m_rightNode);
			_DG_ASSERTE (node->m_rightIsProxy == false);
			_DG_ASSERTE (stack < sizeof (stackPool) / sizeof (dgNode*));
			stackPool[stack] = node->m_rightNode;
			stack++;
		}

		if (node->m_leftIsProxy) {
			dgInt32 proxyIndex;
			const dgProxy& sceneProxy = node->m_leftProxi->GetInfo();
			proxyIndex = dgInt32 (sceneProxy.m_boxP0.m_w);
			callback (userData, &proxyIndex, sizeof (dgInt32));

		} else {
			_DG_ASSERTE (node->m_leftNode);
			_DG_ASSERTE (node->m_leftIsProxy == false);
			_DG_ASSERTE (stack < sizeof (stackPool) / sizeof (dgNode*));
			stackPool[stack] = node->m_leftNode;
			stack++;
		}
	}
*/
}








void dgCollisionScene::SetProxyMatrix (void* proxy, const dgMatrix& matrix)
{
	dgVector boxP0;
	dgVector boxP1;

	dgProxy* const entry = ((dgList<dgProxy*>::dgListNode*) proxy)->GetInfo();

	entry->m_matrix = entry->m_shape->GetOffsetMatrix() * matrix;
	entry->m_shape->CalcAABB (entry->m_matrix, boxP0, boxP1);

	dgVector p0 (boxP0.CompProduct(dgVector (DG_SCENE_AABB_SCALE, DG_SCENE_AABB_SCALE, DG_SCENE_AABB_SCALE, dgFloat32 (0.0f))));
	dgVector p1 (boxP1.CompProduct(dgVector (DG_SCENE_AABB_SCALE, DG_SCENE_AABB_SCALE, DG_SCENE_AABB_SCALE, dgFloat32 (0.0f))));

	p0.m_x = dgFloor (p0.m_x) * DG_SCENE_AABB_INV_SCALE; 
	p0.m_y = dgFloor (p0.m_y) * DG_SCENE_AABB_INV_SCALE;  
	p0.m_z = dgFloor (p0.m_z) * DG_SCENE_AABB_INV_SCALE;  

	p1.m_x = dgFloor (p1.m_x + dgFloat32 (1.0f)) * DG_SCENE_AABB_INV_SCALE;  
	p1.m_y = dgFloor (p1.m_y + dgFloat32 (1.0f)) * DG_SCENE_AABB_INV_SCALE;  
	p1.m_z = dgFloor (p1.m_z + dgFloat32 (1.0f)) * DG_SCENE_AABB_INV_SCALE;  

	entry->m_minBox = p0;
	entry->m_maxBox = p1;

	dgVector side0 (p1 - p0);
	dgVector side1 (side0.m_y, side0.m_z, side0.m_x, dgFloat32 (0.0f));
	entry->m_surfaceArea = side0 % side1;

	for (dgNode* parent = entry->m_parent; parent; parent = parent->m_parent) {
		dgVector minBox;
		dgVector maxBox;
		dgFloat32 area = CalculateSurfaceArea (parent->m_left, parent->m_right, minBox, maxBox);
		if (!((parent->m_minBox.m_x < minBox.m_x) || (parent->m_minBox.m_y < minBox.m_y) || (parent->m_minBox.m_z < minBox.m_z) ||
			(parent->m_maxBox.m_x > maxBox.m_x) || (parent->m_maxBox.m_y < maxBox.m_y) || (parent->m_maxBox.m_z < maxBox.m_z))) {
				break;
		}

		m_world->dgGetIndirectLock (&m_lock);
		parent->m_minBox = minBox;
		parent->m_maxBox = maxBox;
		parent->m_surfaceArea = area;
		m_world->dgReleaseIndirectLock (&m_lock);
	}
}

dgMatrix dgCollisionScene::GetProxyMatrix (void* const proxy)
{
	dgProxy* const entry = ((dgList<dgProxy*>::dgListNode*) proxy)->GetInfo();
	return entry->m_shape->GetOffsetMatrix().Inverse() * entry->m_matrix;
}


void dgCollisionScene::SetCollisionCallback (dgCollisionMeshCollisionCallback debugCallback)
{
_DG_ASSERTE (0);
/*
	for (dgList<dgProxy>::dgListNode* node = m_list.GetFirst(); node; node = node->GetNext()) {
		const dgCollisionScene::dgProxy& entry = node->GetInfo();
		if (entry.m_shape->IsType (dgCollisionMesh_RTTI)) {
			dgCollisionMesh* mesh;
			mesh = (dgCollisionMesh*)entry.m_shape;
			mesh->SetCollisionCallback (debugCallback);
		}
	}
*/
}




void dgCollisionScene::CalcAABB (const dgMatrix& matrix, dgVector& p0, dgVector& p1) const
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

void dgCollisionScene::CalcAABBSimd(const dgMatrix &matrix,	dgVector &p0, dgVector &p1) const
{
	CalcAABB(matrix, p0, p1);
}







void dgCollisionScene::DebugCollision (const dgMatrix& matrix, OnDebugCollisionMeshCallback callback, void* const userData) const
{
	for (dgList<dgProxy*>::dgListNode* node = m_list.GetFirst(); node; node = node->GetNext()) {
		const dgCollisionScene::dgProxy* entry = node->GetInfo();
		dgMatrix proxyMatrix (entry->m_matrix * matrix);
		entry->m_shape->DebugCollision (entry->m_shape->GetOffsetMatrix() * proxyMatrix, callback, userData);
	}
}


dgFloat32 dgCollisionScene::GetVolume () const
{
	_DG_ASSERTE (0);
	return dgFloat32 (0.0f);
}


dgInt32 dgCollisionScene::CalculateSignature () const
{
	_DG_ASSERTE (0);
	return 0;
}

dgFloat32 dgCollisionScene::GetBoxMinRadius () const 
{
	return dgFloat32 (0.0f);
}

dgFloat32 dgCollisionScene::GetBoxMaxRadius () const 
{
	return dgFloat32 (0.0f);
}

void dgCollisionScene::SetCollisionBBox (const dgVector& p0, const dgVector& p1)
{
	_DG_ASSERTE (p0.m_x <= p1.m_x);
	_DG_ASSERTE (p0.m_y <= p1.m_y);
	_DG_ASSERTE (p0.m_z <= p1.m_z);

	m_boxSize = (p1 - p0).Scale (dgFloat32 (0.5f)); 
	m_boxOrigin = (p1 + p0).Scale (dgFloat32 (0.5f)); 
}

void dgCollisionScene::CalculateInertia (dgVector& inertia, dgVector& origin) const
{
	inertia.m_x = dgFloat32 (0.0f);
	inertia.m_y = dgFloat32 (0.0f);
	inertia.m_z = dgFloat32 (0.0f);

	origin.m_x = dgFloat32 (0.0f);
	origin.m_y = dgFloat32 (0.0f);
	origin.m_z = dgFloat32 (0.0f);
}

dgVector dgCollisionScene::CalculateVolumeIntegral (const dgMatrix& globalMatrix, GetBuoyancyPlane bouyancyPlane, void* const context) const
{
	_DG_ASSERTE (0);
	return dgVector (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));
}


void dgCollisionScene::GetCollisionInfo(dgCollisionInfo* info) const
{
	dgCollision::GetCollisionInfo(info);
	info->m_offsetMatrix = dgGetIdentityMatrix();
	info->m_sceneCollision.m_childrenProxyCount = m_list.GetCount();
}


bool dgCollisionScene::OOBBTest (const dgMatrix& matrix, const dgCollisionConvex* const shape, void* const cacheOrder) const
{
	_DG_ASSERTE (0);
	return true;
}



dgFloat32 dgCollisionScene::RayCastSimd (const dgVector& localP0, const dgVector& localP1, dgContactPoint& contactOut, OnRayPrecastAction preFilter, const dgBody* const body, void* const userData) const
{
_DG_ASSERTE (0);
return 0;
/*
	dgInt32 stack;
	dgFloat32 maxParam;
	dgContactPoint tmpContactOut;
	const dgNode *stackPool[64];

	if (!m_rootNode) {
		return dgFloat32 (1.2f);
	}

	
	
	stack = 1;
	stackPool[0] = m_rootNode;
	maxParam = dgFloat32 (1.2f);

	FastRayTest ray (localP0, localP1);
	while (stack) {
		const dgNode *me;

		stack --;
		me = stackPool[stack];
		if (ray.BoxTestSimd (me->m_p0, me->m_p1)) {
			if (me->m_leftIsProxy) {
				const dgProxy& proxy = me->m_leftProxi->GetInfo();
				if (ray.BoxTestSimd (proxy.m_boxP0, proxy.m_boxP1)) {
					dgFloat32 param;
					dgVector l0 (proxy.m_matrix.UntransformVector (localP0));
					dgVector l1 (proxy.m_matrix.UntransformVector (localP1));
					param = proxy.m_shape->RayCastSimd (l0, l1, tmpContactOut, preFilter, body, userData);
					_DG_ASSERTE (param >= dgFloat32 (0.0f));
					if (param < maxParam) {
						contactOut.m_normal = proxy.m_matrix.RotateVector(tmpContactOut.m_normal);
						maxParam = param;
						ray.Reset (maxParam) ;
					}
				}
			} else {
				_DG_ASSERTE (me->m_leftIsProxy == false);
				_DG_ASSERTE (stack < sizeof (stackPool) / sizeof (dgNode*));
				stackPool[stack] = me->m_leftNode;
				stack++;
			}

			if (me->m_rightIsProxy) {
				const dgProxy& proxy = me->m_rightProxi->GetInfo();
				if (ray.BoxTestSimd (proxy.m_boxP0, proxy.m_boxP1)) {
					dgFloat32 param;
					dgVector l0 (proxy.m_matrix.UntransformVector (localP0));
					dgVector l1 (proxy.m_matrix.UntransformVector (localP1));
					param = proxy.m_shape->RayCastSimd (l0, l1, tmpContactOut, preFilter, body, userData);
					_DG_ASSERTE (param >= dgFloat32 (0.0f));
					if (param < maxParam) {
						contactOut.m_normal = proxy.m_matrix.RotateVector(tmpContactOut.m_normal);
						maxParam = param;
						ray.Reset (maxParam) ;
					}
				}

			} else {
				_DG_ASSERTE (me->m_rightIsProxy == false);
				_DG_ASSERTE (stack < sizeof (stackPool) / sizeof (dgNode*));
				stackPool[stack] = me->m_rightNode;
				stack++;
			}
		}
	}
	return maxParam;
*/
}


dgVector dgCollisionScene::SupportVertex (const dgVector& dir) const
{
	_DG_ASSERTE (0);
	return dgVector (0, 0, 0, 0);
}



dgFloat32 dgCollisionScene::RayCast (const dgVector& localP0, const dgVector& localP1, dgContactPoint& contactOut, OnRayPrecastAction preFilter, const dgBody* const body, void* const userData) const
{
	_DG_ASSERTE (0);
	return 0;
/*
	dgInt32 stack;
	dgFloat32 maxParam;
	dgContactPoint tmpContactOut;
	const dgNode *stackPool[64];

	if (!m_rootNode) {
		return dgFloat32 (1.2f);
	}

	stack = 1;
	stackPool[0] = m_rootNode;
	maxParam = dgFloat32 (1.2f);

	FastRayTest ray (localP0, localP1);
	while (stack) {
		const dgNode *me;
		stack --;
		me = stackPool[stack];
		if (ray.BoxTest (me->m_p0, me->m_p1)) {
			if (me->m_leftIsProxy) {
				const dgProxy& proxy = me->m_leftProxi->GetInfo();
				if (ray.BoxTest (proxy.m_boxP0, proxy.m_boxP1)) {
					dgFloat32 param;
					dgVector l0 (proxy.m_matrix.UntransformVector (localP0));
					dgVector l1 (proxy.m_matrix.UntransformVector (localP1));
					param = proxy.m_shape->RayCast (l0, l1, tmpContactOut, preFilter, body, userData);
					_DG_ASSERTE (param >= dgFloat32 (0.0f));
					if (param < maxParam) {
						contactOut.m_normal = proxy.m_matrix.RotateVector(tmpContactOut.m_normal);
						maxParam = param;
						ray.Reset (maxParam);
					}
				}
			} else {
				_DG_ASSERTE (me->m_leftNode);
				_DG_ASSERTE (stack < sizeof (stackPool) / sizeof (dgNode*));
				stackPool[stack] = me->m_leftNode;
				stack++;
			}

			if (me->m_rightIsProxy) {
				const dgProxy& proxy = me->m_rightProxi->GetInfo();
				if (ray.BoxTest (proxy.m_boxP0, proxy.m_boxP1)) {
					dgFloat32 param;
					dgVector l0 (proxy.m_matrix.UntransformVector (localP0));
					dgVector l1 (proxy.m_matrix.UntransformVector (localP1));
					param = proxy.m_shape->RayCast (l0, l1, tmpContactOut, preFilter, body, userData);
					_DG_ASSERTE (param >= dgFloat32 (0.0f));
					if (param < maxParam) {
						contactOut.m_normal = proxy.m_matrix.RotateVector(tmpContactOut.m_normal);
						maxParam = param;
						ray.Reset (maxParam) ;
					}
				}

			} else {
				_DG_ASSERTE (me->m_rightNode);
				_DG_ASSERTE (stack < sizeof (stackPool) / sizeof (dgNode*));
				stackPool[stack] = me->m_rightNode;
				stack++;
			}
		}
	}
	return maxParam;
*/
}

void dgCollisionScene::CollidePairSimd (dgCollidingPairCollector::dgPair* const pair, dgCollisionParamProxi& proxi) const
{
_DG_ASSERTE (0);
/*
	dgInt32 stack;
	dgWorld* world;
	const dgNode *stackPool[64];

	_DG_ASSERTE (pair->m_body1->GetCollision() == this);
	_DG_ASSERTE (pair->m_body1->GetCollision()->IsType(dgCollision::dgCollisionScene_RTTI));

	dgVector p0;
	dgVector p1;

	world = pair->m_body1->GetWorld();
	dgMatrix matrix (pair->m_body0->m_matrix * pair->m_body1->m_matrix.Inverse());
	pair->m_body0->GetCollision()->CalcAABB (matrix, p0, p1);

	stack = 0;
	if (m_rootNode->m_leftIsProxy) {
		const dgProxy& sceneProxy = m_rootNode->m_leftProxi->GetInfo();
		if (!sceneProxy.m_shape->IsType (dgCollision::dgCollisionNull_RTTI)) {
			if (dgOverlapTestSimd (sceneProxy.m_boxP0, sceneProxy.m_boxP1, p0, p1)) {
				world->SceneContactsSimd (sceneProxy, pair, proxi);
			}
		}
	} else if (m_rootNode->m_leftNode) {
		stackPool[stack] = m_rootNode->m_leftNode;
		stack++;
	}

	if (m_rootNode->m_rightIsProxy) {
		const dgProxy& sceneProxy = m_rootNode->m_rightProxi->GetInfo();
		if (!sceneProxy.m_shape->IsType (dgCollision::dgCollisionNull_RTTI)) {
			if (dgOverlapTestSimd (sceneProxy.m_boxP0, sceneProxy.m_boxP1, p0, p1)) {
				world->SceneContactsSimd (sceneProxy, pair, proxi);
			}
		}
	} else {
		_DG_ASSERTE (m_rootNode->m_rightNode);
		stackPool[stack] = m_rootNode->m_rightNode;
		stack++;
	}

	while (stack) {

		stack --;
		const dgNode* const me = stackPool[stack];
		if (dgOverlapTestSimd (me->m_p0, me->m_p1, p0, p1)) {
			if (me->m_leftIsProxy) {
				const dgProxy& sceneProxy = me->m_leftProxi->GetInfo();
				if (dgOverlapTestSimd (sceneProxy.m_boxP0, sceneProxy.m_boxP1, p0, p1)) {
					world->SceneContactsSimd (sceneProxy, pair, proxi);
				}
			} else {
				_DG_ASSERTE (me->m_leftNode);
				_DG_ASSERTE (stack < sizeof (stackPool) / sizeof (dgNode*));
				stackPool[stack] = me->m_leftNode;
				stack++;
			}

			if (me->m_rightIsProxy) {
				const dgProxy& sceneProxy = me->m_rightProxi->GetInfo();
				if (dgOverlapTestSimd (sceneProxy.m_boxP0, sceneProxy.m_boxP1, p0, p1)) {
					world->SceneContactsSimd (sceneProxy, pair, proxi);
				}
			} else {
				_DG_ASSERTE (me->m_rightNode);
				_DG_ASSERTE (stack < sizeof (stackPool) / sizeof (dgNode*));
				stackPool[stack] = me->m_rightNode;
				stack++;
			}
		}
	}
*/
}

void dgCollisionScene::CollidePair (dgCollidingPairCollector::dgPair* const pair, dgCollisionParamProxi& proxi) const
{
_DG_ASSERTE (0);
/*
	dgInt32 stack;
	dgWorld* world;
	const dgNode *stackPool[64];

	_DG_ASSERTE (pair->m_body1->GetCollision() == this);
	_DG_ASSERTE (pair->m_body1->GetCollision()->IsType(dgCollision::dgCollisionScene_RTTI));

	dgVector p0;
	dgVector p1;

	world = pair->m_body1->GetWorld();
	dgMatrix matrix (pair->m_body0->m_matrix * pair->m_body1->m_matrix.Inverse());
	pair->m_body0->GetCollision()->CalcAABB (matrix, p0, p1);

	stack = 0;
	if (m_rootNode->m_leftIsProxy) {
		const dgProxy& sceneProxy = m_rootNode->m_leftProxi->GetInfo();
		if (!sceneProxy.m_shape->IsType (dgCollision::dgCollisionNull_RTTI)) {
			if (dgOverlapTest (sceneProxy.m_boxP0, sceneProxy.m_boxP1, p0, p1)) {
					world->SceneContacts (sceneProxy, pair, proxi);
			}
		}
	} else if (m_rootNode->m_leftNode) {
		stackPool[stack] = m_rootNode->m_leftNode;
		stack++;
	}

	if (m_rootNode->m_rightIsProxy) {
		const dgProxy& sceneProxy = m_rootNode->m_rightProxi->GetInfo();
		if (!sceneProxy.m_shape->IsType (dgCollision::dgCollisionNull_RTTI)) {
			if (dgOverlapTest (sceneProxy.m_boxP0, sceneProxy.m_boxP1, p0, p1)) {
				world->SceneContacts (sceneProxy, pair, proxi);
			}
		}
	} else {
		_DG_ASSERTE (m_rootNode->m_rightNode);
		stackPool[stack] = m_rootNode->m_rightNode;
		stack++;
	}

	while (stack) {

		stack --;
		const dgNode* const me = stackPool[stack];
		if (dgOverlapTest (me->m_p0, me->m_p1, p0, p1)) {
			if (me->m_leftIsProxy) {
				const dgProxy& sceneProxy = me->m_leftProxi->GetInfo();
				if (dgOverlapTest (sceneProxy.m_boxP0, sceneProxy.m_boxP1, p0, p1)) {
					world->SceneContacts (sceneProxy, pair, proxi);
				}
			} else {
				_DG_ASSERTE (me->m_leftNode);
				_DG_ASSERTE (stack < sizeof (stackPool) / sizeof (dgNode*));
				stackPool[stack] = me->m_leftNode;
				stack++;
			}

			if (me->m_rightIsProxy) {
				const dgProxy& sceneProxy = me->m_rightProxi->GetInfo();
				if (dgOverlapTest (sceneProxy.m_boxP0, sceneProxy.m_boxP1, p0, p1)) {
					world->SceneContacts (sceneProxy, pair, proxi);
				}
			} else {
				_DG_ASSERTE (me->m_rightNode);
				_DG_ASSERTE (stack < sizeof (stackPool) / sizeof (dgNode*));
				stackPool[stack] = me->m_rightNode;
				stack++;
			}
		}
	}
*/
}


void dgCollisionScene::ImproveNodeFitness (dgNode* const node)
{
	_DG_ASSERTE (node->m_left);
	_DG_ASSERTE (node->m_right);

	if (node->m_parent)	{
		if (node->m_parent->m_left == node) {
			dgFloat32 cost0 = node->m_surfaceArea;

			dgVector cost1P0;
			dgVector cost1P1;		
			dgFloat32 cost1 = CalculateSurfaceArea (node->m_right, node->m_parent->m_right, cost1P0, cost1P1);

			dgVector cost2P0;
			dgVector cost2P1;		
			dgFloat32 cost2 = CalculateSurfaceArea (node->m_left, node->m_parent->m_right, cost2P0, cost2P1);

			if ((cost1 <= cost0) && (cost1 <= cost2)) {
				dgNode* const parent = node->m_parent;
				node->m_minBox = parent->m_minBox;
				node->m_maxBox = parent->m_maxBox;
				node->m_surfaceArea = parent->m_surfaceArea; 
				if (parent->m_parent) {
					if (parent->m_parent->m_left == parent) {
						parent->m_parent->m_left = node;
					} else {
						_DG_ASSERTE (parent->m_parent->m_right == parent);
						parent->m_parent->m_right = node;
					}
				} else {
					m_rootNode = node;
				}
				node->m_parent = parent->m_parent;
				parent->m_parent = node;
				node->m_right->m_parent = parent;
				parent->m_left = node->m_right;
				node->m_right = parent;
				parent->m_minBox = cost1P0;
				parent->m_maxBox = cost1P1;		
				parent->m_surfaceArea = cost1;


			} else if ((cost2 <= cost0) && (cost2 <= cost1)) {
				dgNode* const parent = node->m_parent;
				node->m_minBox = parent->m_minBox;
				node->m_maxBox = parent->m_maxBox;
				node->m_surfaceArea = parent->m_surfaceArea; 

				if (parent->m_parent) {
					if (parent->m_parent->m_left == parent) {
						parent->m_parent->m_left = node;
					} else {
						_DG_ASSERTE (parent->m_parent->m_right == parent);
						parent->m_parent->m_right = node;
					}
				} else {
					m_rootNode = node;
				}
				node->m_parent = parent->m_parent;
				parent->m_parent = node;
				node->m_left->m_parent = parent;
				parent->m_left = node->m_left;
				node->m_left = parent;

				parent->m_minBox = cost2P0;
				parent->m_maxBox = cost2P1;		
				parent->m_surfaceArea = cost2;
			}
		} else {
			dgFloat32 cost0 = node->m_surfaceArea;

			dgVector cost1P0;
			dgVector cost1P1;		
			dgFloat32 cost1 = CalculateSurfaceArea (node->m_left, node->m_parent->m_left, cost1P0, cost1P1);

			dgVector cost2P0;
			dgVector cost2P1;		
			dgFloat32 cost2 = CalculateSurfaceArea (node->m_right, node->m_parent->m_left, cost2P0, cost2P1);


			if ((cost1 <= cost0) && (cost1 <= cost2)) {

				dgNode* const parent = node->m_parent;
				node->m_minBox = parent->m_minBox;
				node->m_maxBox = parent->m_maxBox;
				node->m_surfaceArea = parent->m_surfaceArea; 
				if (parent->m_parent) {
					if (parent->m_parent->m_left == parent) {
						parent->m_parent->m_left = node;
					} else {
						_DG_ASSERTE (parent->m_parent->m_right == parent);
						parent->m_parent->m_right = node;
					}
				} else {
					m_rootNode = node;
				}
				node->m_parent = parent->m_parent;
				parent->m_parent = node;
				node->m_left->m_parent = parent;
				parent->m_right = node->m_left;
				node->m_left = parent;

				parent->m_minBox = cost1P0;
				parent->m_maxBox = cost1P1;		
				parent->m_surfaceArea = cost1;

			} else if ((cost2 <= cost0) && (cost2 <= cost1)) {
				dgNode* const parent = node->m_parent;
				node->m_minBox = parent->m_minBox;
				node->m_maxBox = parent->m_maxBox;
				node->m_surfaceArea = parent->m_surfaceArea; 
				if (parent->m_parent) {
					if (parent->m_parent->m_left == parent) {
						parent->m_parent->m_left = node;
					} else {
						_DG_ASSERTE (parent->m_parent->m_right == parent);
						parent->m_parent->m_right = node;
					}
				} else {
					m_rootNode = node;
				}
				node->m_parent = parent->m_parent;
				parent->m_parent = node;
				node->m_right->m_parent = parent;
				parent->m_right = node->m_right;
				node->m_right = parent;

				parent->m_minBox = cost2P0;
				parent->m_maxBox = cost2P1;		
				parent->m_surfaceArea = cost2;
			}
		}
	}

	_DG_ASSERTE (!m_rootNode->m_parent);
}



void dgCollisionScene::ImproveTotalFitness()
{
	dgInt32 maxPasses = 2 * exp_2 (m_fitnessList.GetCount()) + 1;

	dgFloat64 newCost = dgFloat32 (1.0e20f);
	dgFloat64 prevCost = newCost;
	do {
		prevCost = newCost;
		for (dgList<dgNode*>::dgListNode* node = m_fitnessList.GetFirst(); node; node = node->GetNext()) {
			ImproveNodeFitness (node->GetInfo());
		}

		newCost	= dgFloat32 (0.0f);
		for (dgList<dgNode*>::dgListNode* node = m_fitnessList.GetFirst(); node; node = node->GetNext()) {
			newCost += node->GetInfo()->m_surfaceArea;
		}
		maxPasses --;
	} while (maxPasses && (newCost < prevCost));
}

dgFloat32 dgCollisionScene::CalculateSurfaceArea (const dgNode* const node0, const dgNode* const node1, dgVector& minBox, dgVector& maxBox) const
{
	minBox = dgVector (GetMin (node0->m_minBox.m_x, node1->m_minBox.m_x), GetMin (node0->m_minBox.m_y, node1->m_minBox.m_y), GetMin (node0->m_minBox.m_z, node1->m_minBox.m_z), dgFloat32 (0.0f));
	maxBox = dgVector (GetMax (node0->m_maxBox.m_x, node1->m_maxBox.m_x), GetMax (node0->m_maxBox.m_y, node1->m_maxBox.m_y), GetMax (node0->m_maxBox.m_z, node1->m_maxBox.m_z), dgFloat32 (0.0f));		
	dgVector side0 (maxBox - minBox);
	dgVector side1 (side0.m_y, side0.m_z, side0.m_x, dgFloat32 (0.0f));
	return side0 % side1;
}


void* dgCollisionScene::GetFirstProxy () const
{
	return m_list.GetFirst();
}

void* dgCollisionScene::GetNextProxy (void* const proxy) const
{
	dgList<dgProxy*>::dgListNode* const node = (dgList<dgProxy*>::dgListNode*) proxy;
	return node->GetNext();
}


void* dgCollisionScene::AddProxy (dgCollision* const shape/*, const dgMatrix& matrix*/)
{
	shape->AddRef();
	dgProxy* const newNode = new (m_world->GetAllocator()) dgProxy (shape, /*matrix,*/ this);
	newNode->m_myNode = m_list.Append(newNode);

	if (!m_rootNode) {
		m_rootNode = newNode;
	} else {
		dgVector p0;
		dgVector p1;		
		dgNode* sibling = m_rootNode;
		dgFloat32 surfaceArea = CalculateSurfaceArea (newNode, sibling, p0, p1);
		while(sibling->m_left && sibling->m_right) {
			if (surfaceArea > sibling->m_surfaceArea) {
				break;
			} 

			sibling->m_minBox = p0;
			sibling->m_maxBox = p1;
			sibling->m_surfaceArea = surfaceArea;

			dgVector leftP0;
			dgVector leftP1;		
			dgFloat32 leftSurfaceArea = CalculateSurfaceArea (newNode, sibling->m_left, leftP0, leftP1);

			dgVector rightP0;
			dgVector rightP1;		
			dgFloat32 rightSurfaceArea = CalculateSurfaceArea (newNode, sibling->m_right, rightP0, rightP1);

			if (leftSurfaceArea < rightSurfaceArea) {
				sibling = sibling->m_left;
				p0 = leftP0;
				p1 = leftP1;
				surfaceArea = leftSurfaceArea;
			} else {
				sibling = sibling->m_right;
				p0 = rightP0;
				p1 = rightP1;
				surfaceArea = rightSurfaceArea;
			}
		} 

		dgNode* const parent = new (m_world->GetAllocator()) dgNode (sibling, newNode);
		parent->m_fitnessNode = m_fitnessList.Append (parent);

		if (!parent->m_parent) {
			m_rootNode = parent;
		}
	}

	return newNode->m_myNode;
}

void dgCollisionScene::RemoveProxy (void* const proxy)
{
	dgList<dgProxy*>::dgListNode* const node = (dgList<dgProxy*>::dgListNode*) proxy;

	dgProxy* const treeNode = node->GetInfo();
	m_world->ReleaseCollision (treeNode->m_shape);

	m_list.Remove (node);
	
	if (!treeNode->m_parent) {
		delete (m_rootNode);
		m_rootNode = NULL;
	} else if (!treeNode->m_parent->m_parent) {
		m_fitnessList.Remove(treeNode->m_parent->m_fitnessNode);

		dgNode* const root = m_rootNode;
		if (treeNode->m_parent->m_left == treeNode) {
			m_rootNode = treeNode->m_parent->m_right;
			treeNode->m_parent->m_right = NULL;
		} else {
			m_rootNode = treeNode->m_parent->m_left;
			treeNode->m_parent->m_left= NULL;
		}
		m_rootNode->m_parent = NULL;
		delete (root);

	} else {
		m_fitnessList.Remove(treeNode->m_parent->m_fitnessNode);
		dgNode* const root = treeNode->m_parent->m_parent;
		if (treeNode->m_parent == root->m_left) {
			if (treeNode->m_parent->m_right == treeNode) {
				root->m_left = treeNode->m_parent->m_left;
				treeNode->m_parent->m_left = NULL;
			} else {
				root->m_left = treeNode->m_parent->m_right;
				treeNode->m_parent->m_right = NULL;
			}
			root->m_left->m_parent = root;
		} else {
			if (treeNode->m_parent->m_right == treeNode) {
				root->m_right = treeNode->m_parent->m_left;
				treeNode->m_parent->m_left = NULL;
			} else {
				root->m_right = treeNode->m_parent->m_right;
				treeNode->m_parent->m_right = NULL;
			}
			root->m_right->m_parent = root;
		}
		delete (treeNode->m_parent);
	}
}
