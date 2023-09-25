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

// dgVehicleConstraint.cpp: implementation of the dgVehicleConstraint class.
//
//////////////////////////////////////////////////////////////////////

#include "dgPhysicsStdafx.h"
#include "dgBody.h"
#include "dgWorld.h"
#include "dgCollisionBox.h"
#include "dgBodyMasterList.h"
#include "dgCompoundCollision.h"
#include "dgVehicleConstraint.h"
#include "dgWorldDynamicUpdate.h"
#include "dgCollisionChamferCylinder.h"

#define DG_MAX_VEHICLE_WHEELS						32
#define DG_MAX_VEHICLE_DOF							(DG_MAX_VEHICLE_WHEELS * 8)
#define DG_MAX_CONTACTS_PER_TIRES					6
#define DG_TIRES_LATERAL_FRICTION_MULTIPLIER		dgFloat32 (2.0f)
#define DG_TIRES_LONGITUDINAL_FRICTION_MULTIPLIER	dgFloat32 (3.0f)
#define DG_VEHICLE_DIAG_DAMP						dgFloat32 (0.001f)
#define DG_MIN_FREC									dgFloat32 (179.0f)

//#define DG_VEHICLE_SEIDLE

struct TiresContacts
{
	dgVehicleConstraint::dgTire* m_tyre;
	dgContact* m_contact;
	dgList<dgContactMaterial>::dgListNode *m_node;
};



class dgTireCollision: public dgCollisionChamferCylinder
{
	public:
	dgTireCollision (dgWorld* world, dgVehicleConstraint::dgTire* tire, const dgMatrix& matrix, const dgVector& updir)
		:dgCollisionChamferCylinder(0, dgFloat32 (1.0f), dgFloat32 (1.0f), matrix)
	{
		_DG_ASSERTE (0);
/*
//		SetAsTire(true);
		m_tire = tire;
		m_scale.m_x = tire->m_width;
		m_scale.m_y = tire->m_radius;
		m_scale.m_z = tire->m_radius;
		
		m_offsetY = dgFloat32 (0.0f);

		m_invScale.m_x = dgFloat32 (1.0f) / m_scale.m_x;
		m_invScale.m_y = dgFloat32 (1.0f) / m_scale.m_y;
		m_invScale.m_z = dgFloat32 (1.0f) / m_scale.m_z;

		m_criticalRadius2 = tire->m_radius * dgFloat32 (0.75f);
		m_criticalRadius2 *= m_criticalRadius2;

		m_activeContactsCount = 0;
		m_masterContactCount = 0;
		tire->m_collision = this; 
		SetUserData (tire->m_collisionID);

		// make sure the transform will be active
		SetMatrix (matrix, updir); 
		m_offsetActive = true;

		m_world = world;
*/
	}

	void ReleaseTire ()
	{
		dgRef::Release();
	}

	void SelectContacts (const dgMatrix &matrix, const dgVector& updir, TiresContacts* contacts, dgInt32 count)
	{
		_DG_ASSERTE (0);
/*
		dgInt32 i;
		dgInt32 j;
		dgInt32 contactCount;
		dgFloat32 val;
		dgVector normals[64];
		dgContact* contact[64];
		dgList<dgContactMaterial>::dgListNode *array[64];
		
		contactCount = 1;
		normals[0].m_w = dgFloat32 (1.0e10f);
		const dgMatrix& localMatrix = GetOffsetMatrix();
		for (i = 0; i < count; i ++) {
			if (contacts[i].m_node) {
				dgContactMaterial& contactPoint = contacts[i].m_node->GetInfo();
				if ((contactPoint.m_collision0 == this) || (contactPoint.m_collision1 == this)) {

					dgVector normal (contactPoint.m_normal.Scale (dgFloat32 (-1.0f)));
					dgVector p (localMatrix.UntransformVector(matrix.UntransformVector (contactPoint.m_point)));
					val = p % p + dgFloat32 (1.0e-12f);
					if (val > m_criticalRadius2) {
						normal = matrix.RotateVector(localMatrix.RotateVector(p.Scale (dgRsqrt (val))));
					}
					normal.m_w = - (normal % updir);
					for (j = contactCount - 1; normal.m_w > normals[j].m_w; j --) {
						array[j + 1] = array[j];
						normals[j + 1] = normals[j];
						contact[j + 1] = contact[j];
					}
					normals[j + 1] = normal;
					array[j + 1] = contacts[i].m_node;
					contact[j + 1] = contacts[i].m_contact;
					contactCount ++;
					contacts[i].m_node = NULL;
				}
			}
		}


		val = dgFloat32 (-10.0f);
		m_masterContactCount = 0;
		m_collidingPairsCount = 0;
		for (i = 1; i < contactCount; i ++) {
			if (m_masterContactCount < DG_MAX_CONTACTS_PER_TIRES) {
				if (dgAbsf (normals[i].m_w - val) > dgFloat32 (0.02f)) {
					dgContactMaterial& material = array[i]->GetInfo();
					m_contactsMaterials[m_masterContactCount] = &material;
					m_contactsMaterials[m_masterContactCount]->m_normal = normals[i];
					for (j = 0; j < m_collidingPairsCount; j ++) {
						const CollidingPair &pair = m_collidingPairs[j];

						if (((pair.m_pair0.m_body == material.m_body0) && (pair.m_pair1.m_body == material.m_body1)) ||
							((pair.m_pair0.m_body == material.m_body1) && (pair.m_pair1.m_body == material.m_body0))) {
							break;
						}
					}

					if (j == m_collidingPairsCount) {
						CollidingPair &pair = m_collidingPairs[m_collidingPairsCount];
						pair.m_pair0.m_body = material.m_body0;
						pair.m_pair1.m_body = material.m_body1;
						pair.m_pair0.m_collision = material.m_collision0;
						pair.m_pair1.m_collision = material.m_collision1;
						m_collidingPairsCount ++;
					}
					m_masterContactCount ++;
				}
			}
			contact[i]->RotateToEnd (array[i]);
			contact[i]->m_activeContacts --; 
			_DG_ASSERTE (contact[i]->m_activeContacts >= 0);
		}


		for (i = 0; i < m_masterContactCount; i ++) {
			dgContactMaterial& material = *m_contactsMaterials[i];
			m_activeContactsContacts[i].m_pair0.m_body = material.m_body0;
			m_activeContactsContacts[i].m_pair0.m_collision = material.m_collision0;
			m_activeContactsContacts[i].m_pair1.m_body = material.m_body1;
			m_activeContactsContacts[i].m_pair1.m_collision = material.m_collision1;
			m_activeContactsContacts[i].m_point = material.m_point;
			m_activeContactsContacts[i].m_normal = material.m_normal;
			m_activeContactsContacts[i].m_penetration = material.m_penetration;
			m_activeContactsContacts[i].m_staticFriction0 = material.m_staticFriction0;
			m_activeContactsContacts[i].m_staticFriction1 = material.m_staticFriction1;
			m_activeContactsContacts[i].m_dynamicFriction0 = material.m_dynamicFriction0;
			m_activeContactsContacts[i].m_dynamicFriction1 = material.m_dynamicFriction1;
		}
		m_activeContactsCount = m_masterContactCount;
		_DG_ASSERTE (m_activeContactsCount <= DG_MAX_CONTACTS_PER_TIRES);
*/
	}


	void SetMatrix (const dgMatrix& matrix, const dgVector& dir) 
	{
		m_offset = matrix;
		m_offset.m_posit += dir.Scale (m_offsetY);
	}

	dgVector SupportVertex (const dgVector& dir) const
	{
		dgVector dir1 (dir.CompProduct (m_scale));
		dir1 = dir1.Scale (dgRsqrt (dir1 % dir1 + dgFloat32 (1.0e-12f)));
		dgVector p (dgCollisionChamferCylinder::SupportVertex (dir1));
		return dgVector (p.CompProduct (m_scale)); 
	}

	dgVector SupportVertexSimd (const dgVector& dir) const
	{
		_DG_ASSERTE (0);
		return dgVector (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));
	}

	dgInt32 CalculatePlaneIntersection (const dgVector& normal, const dgVector& point, dgVector contactsOut[]) const
	{
		dgInt32 i;
		dgInt32 count;

		dgVector n (normal.CompProduct (m_scale));
		n = n.Scale (dgRsqrt (n % n));

		dgVector p (point.CompProduct (m_invScale));
		count = dgCollisionChamferCylinder::CalculatePlaneIntersection (n, p, contactsOut);
		for (i = 0; i < count; i ++) {
			contactsOut[i] = contactsOut[i].CompProduct (m_scale);
		}

		return count;
	}

	dgInt32 CalculatePlaneIntersectionSimd (
		const dgVector& normal, 
		const dgVector& origin, 
		dgVector contactsOut[]) const
	{
		_DG_ASSERTE (0);
		return 0; 
	}


	void CalcAABB (const dgMatrix &matrix, dgVector& p0, dgVector& p1) const
	{
		dgMatrix mat (matrix);
		mat.m_front = mat.m_front.Scale (m_scale.m_x);
		mat.m_up = mat.m_up.Scale (m_scale.m_y);
		mat.m_right = mat.m_right.Scale (m_scale.m_z);
		_DG_ASSERTE (0);
		dgCollisionChamferCylinder::CalcAABB (mat, p0, p1);
	}


	dgFloat32 RayCast (const dgVector& p0, const dgVector& p1, dgContactPoint& contactOut, OnRayPrecastAction preFilter, const dgBody* const body, void* const userData) const
	{
		dgVector q0 (p0.CompProduct (m_invScale)); 
		dgVector q1 (p1.CompProduct (m_invScale));  
		return dgCollisionChamferCylinder::RayCast (q0, q1, contactOut, preFilter, body, userData);
	}


	void DebugCollision (const dgMatrix& matrixPtr, OnDebugCollisionMeshCallback callback, void* const userData) const
	{
		_DG_ASSERTE (0);
		dgMatrix saveMatrix (m_offset);
		dgMatrix& mat = ((dgTireCollision*) this)->m_offset;  
		mat.m_front = mat.m_front.Scale (m_scale.m_x);
		mat.m_up = mat.m_up.Scale (m_scale.m_y);
		mat.m_right = mat.m_right.Scale (m_scale.m_z);
		dgCollisionChamferCylinder::DebugCollision (matrixPtr, callback, userData);
		((dgTireCollision*) this)->m_offset = saveMatrix;
	}

	struct BodyCollisionPair
	{
		dgBody *m_body;
		dgCollision *m_collision;
	};

	struct CollidingPair
	{
		BodyCollisionPair m_pair0;
		BodyCollisionPair m_pair1;
	};

	struct Contact: public CollidingPair
	{
		dgVector m_point;
		dgVector m_normal;
		dgFloat32 m_staticFriction0;
		dgFloat32 m_staticFriction1;
		dgFloat32 m_dynamicFriction0;
		dgFloat32 m_dynamicFriction1;
		dgFloat32 m_penetration;
	};

	dgVector m_scale;
	dgVector m_invScale;

	dgWorld* m_world;
	dgFloat32 m_offsetY;
	dgFloat32 m_criticalRadius2;
	
	dgInt32 m_activeContactsCount;
	dgInt32 m_masterContactCount;
	dgInt32 m_collidingPairsCount;
	Contact m_activeContactsContacts[DG_MAX_CONTACTS_PER_TIRES];
	CollidingPair m_collidingPairs[DG_MAX_CONTACTS_PER_TIRES];
	dgContactMaterial* m_contactsMaterials[DG_MAX_CONTACTS_PER_TIRES];
	dgVehicleConstraint::dgTire* m_tire;
};



DG_MSC_VECTOR_ALIGMENT 
class dgVehicleForceMiniSolver
{
	public:
	dgVehicleForceMiniSolver ()
	{	

	}

	~dgVehicleForceMiniSolver()
	{
	}

	inline void MatrixTimeVector (dgInt32 rowsCount, const dgFloat32* x, dgFloat32* b)
	{
		dgInt32 i0;
		dgInt32 j0;
		dgFloat32 val;
		dgFloat32 acc;

		for (i0 = 0; i0 < m_bodies; i0 ++) {
			m_y[i0].m_linear[0] = dgFloat32 (0.0f);
			m_y[i0].m_linear[1] = dgFloat32 (0.0f);
			m_y[i0].m_linear[2] = dgFloat32 (0.0f);
			m_y[i0].m_linear[3] = dgFloat32 (0.0f);
			m_y[i0].m_angular[0] = dgFloat32 (0.0f);
			m_y[i0].m_angular[1] = dgFloat32 (0.0f);
			m_y[i0].m_angular[2] = dgFloat32 (0.0f);
			m_y[i0].m_angular[3] = dgFloat32 (0.0f);
		}

		for (i0 = 0; i0 < rowsCount; i0 ++) {
			val = x[i0]; 
			j0 = m_jacobianIndexArray[i0].m_m0;
			m_y[j0].m_linear += m_Jt[i0].m_jacobian_IM0.m_linear.Scale (val);
			m_y[j0].m_angular += m_Jt[i0].m_jacobian_IM0.m_angular.Scale (val);

			j0 = m_jacobianIndexArray[i0].m_m1;
			m_y[j0].m_linear += m_Jt[i0].m_jacobian_IM1.m_linear.Scale (val);
			m_y[j0].m_angular += m_Jt[i0].m_jacobian_IM1.m_angular.Scale (val);
		}

		for (i0 = 0; i0 < rowsCount; i0 ++) {
			j0 = m_jacobianIndexArray[i0].m_m0;
			acc  = ((m_JMinv[i0].m_jacobian_IM0.m_linear % m_y[j0].m_linear) + 
				(m_JMinv[i0].m_jacobian_IM0.m_angular % m_y[j0].m_angular));

			j0 = m_jacobianIndexArray[i0].m_m1;
			acc += ((m_JMinv[i0].m_jacobian_IM1.m_linear % m_y[j0].m_linear) +
				(m_JMinv[i0].m_jacobian_IM1.m_angular % m_y[j0].m_angular));

			b[i0] = acc + x[i0] * m_diagDamp[i0];
		}
	}


#ifndef DG_VEHICLE_SEIDLE
	inline void CalculateForces(dgInt32 forceRows)
	{
		dgInt32 i;
		dgInt32 j;
		dgInt32 passes;
		dgInt32 clampedForce;
		dgFloat32 ak;
		dgFloat32 bk;
		dgFloat32 val;
		dgFloat32 akNum;
		dgFloat32 akDen;
		dgFloat32 force;
		dgFloat32 clipVal;
		dgFloat32 accNorm;

		akNum = dgFloat32 (0.0f);
		clipVal = dgFloat32 (0.0f);
		for (i = 0; i < forceRows; i ++) {
			val = m_accel[i];
			bk = val * m_invDJMinvJt[i];
			akNum += val * bk;
			m_deltaForce[i] = bk;
		}

		accNorm = DG_FREEZE_MAG * dgFloat32 (2.0f);
		passes = 6 + dgInt32 (dgSqrt (dgFloat32 (forceRows)));
		for (i = 0; (i < passes) && (accNorm > DG_FREEZE_MAG); i ++) {
			MatrixTimeVector (forceRows, m_deltaForce, m_deltaAccel);

			akDen = dgFloat32 (0.0f);
			for (j = 0; j < forceRows; j ++) {
				akDen += m_deltaAccel[j] * m_deltaForce[j];
			}

			_DG_ASSERTE (akDen >= dgFloat32 (0.0f));
			akDen = GetMax (akDen, dgFloat32 (1.0e-16f));
			_DG_ASSERTE (dgAbsf (akDen) >= dgFloat32 (1.0e-16f));

			ak = akNum / akDen;

			clampedForce = -1;
			for (j = 0; j < forceRows; j ++) {
				if (m_activeRow[j]) {
					force = m_force[j] + ak * m_deltaForce[j];
					if (force < m_bilateralForceBounds[j].m_low) {
						_DG_ASSERTE (((m_bilateralForceBounds[j].m_low - m_force[j]) / m_deltaForce[j]) <= ak);
						ak = (m_bilateralForceBounds[j].m_low - m_force[j]) / m_deltaForce[j];
						clampedForce = j;
						clipVal = m_bilateralForceBounds[j].m_low;
					} else if (force > m_bilateralForceBounds[j].m_upper) {
						_DG_ASSERTE (((m_bilateralForceBounds[j].m_upper - m_force[j]) / m_deltaForce[j]) <= ak);
						ak = (m_bilateralForceBounds[j].m_upper - m_force[j]) / m_deltaForce[j];
						clampedForce = j;
						clipVal = m_bilateralForceBounds[j].m_upper;
					}
				}
			}


			if (clampedForce != -1) {
				akNum = dgFloat32 (0.0f);
				accNorm = dgFloat32 (0.0f);
				m_activeRow[clampedForce] = dgFloat32 (0.0f);
				for (j = 0; j < forceRows; j ++) {
					m_force[j] += ak * m_deltaForce[j];
					m_accel[j] -= ak * m_deltaAccel[j];

#ifdef _MSC_VER
					_DG_ASSERTE (dgCheckFloat(m_force[j]));
					_DG_ASSERTE (dgCheckFloat(m_accel[j]));
#endif

					val = m_accel[j] * m_invDJMinvJt[j] * m_activeRow[j];
					m_deltaForce[j] = val;
					akNum += val * m_accel[j];
					accNorm = GetMax (dgAbsf (m_accel[j] * m_activeRow[j]), accNorm);
				}
				m_force[clampedForce] = clipVal;
				i = -1;

			} else {
				accNorm = dgFloat32 (0.0f);
				for (j = 0; j < forceRows; j ++) {
					m_force[j] += ak * m_deltaForce[j];
					m_accel[j] -= ak * m_deltaAccel[j];
					accNorm = GetMax (dgAbsf (m_accel[j] * m_activeRow[j]), accNorm);

#ifdef _MSC_VER
					_DG_ASSERTE (dgCheckFloat(m_force[j]));
					_DG_ASSERTE (dgCheckFloat(m_accel[j]));
#endif
				}


				if (accNorm > DG_FREEZE_MAG) {
					bk = akNum;
					akNum = dgFloat32 (0.0f);
					for (j = 0; j < forceRows; j ++) {
						val = m_accel[j] * m_invDJMinvJt[j] * m_activeRow[j];
						m_deltaAccel[j] = val;
						akNum += m_accel[j] * val;
					}

					_DG_ASSERTE (bk >= dgFloat32 (0.0f));
					bk = GetMax (bk, dgFloat32(1.0e-17f));
					bk = akNum / bk;
					for (j = 0; j < forceRows; j ++) {
						m_deltaForce[j] = m_deltaAccel[j] + bk * m_deltaForce[j];
					}
				}
			}
		}
//dgTrace (("acc %d %f\n", i, accNorm));
	}

#else
/*
	dgFloat32 IncrementalDotProduct (
		dgInt32 rowIndex,
		dgInt32 colIndex, 
		dgFloat32 deltaValue)
	{
		dgInt32 j0;

		j0 = m_jacobianIndexArray[colIndex].m_m0;
		m_y[j0].m_linear += m_Jt[colIndex].m_jacobian_IM0.m_linear.Scale (deltaValue);
		m_y[j0].m_angular += m_Jt[colIndex].m_jacobian_IM0.m_angular.Scale (deltaValue);

		j0 = m_jacobianIndexArray[colIndex].m_m1;
		m_y[j0].m_linear += m_Jt[colIndex].m_jacobian_IM1.m_linear.Scale (deltaValue);
		m_y[j0].m_angular += m_Jt[colIndex].m_jacobian_IM1.m_angular.Scale (deltaValue);

		//	acc = (m_force[rowIndex] * m_diagDamp[rowIndex]);
		j0 = m_jacobianIndexArray[rowIndex].m_m0;
		dgVector acc (m_JMinv[rowIndex].m_jacobian_IM0.m_linear.CompProduct(m_y[j0].m_linear));
		acc += m_JMinv[rowIndex].m_jacobian_IM0.m_angular.CompProduct (m_y[j0].m_angular);

		j0 = m_jacobianIndexArray[rowIndex].m_m1;
		acc += m_JMinv[rowIndex].m_jacobian_IM1.m_linear.CompProduct (m_y[j0].m_linear);
		acc += m_JMinv[rowIndex].m_jacobian_IM1.m_angular.CompProduct (m_y[j0].m_angular);

		return acc.m_x + acc.m_y + acc.m_z + m_force[rowIndex] * m_diagDamp[rowIndex];
	}
*/

	inline void CalculateForces(dgInt32 forceRows)
	{
		dgInt32 i;
		dgInt32 j;
		dgInt32 k;
		dgInt32 n;
		dgInt32 passes;
//		dgInt32 clampedForce;
		dgFloat32 ak;
		dgFloat32 bk;
		dgFloat32 val;
//		dgFloat32 akNum;
//		dgFloat32 akDen;
//		dgFloat32 force;
//		dgFloat32 clipVal;
		dgFloat32 accNorm;

		for (i = 0; i < m_bodies; i ++) {
			m_y[i].m_linear[0] = 0.0f;
			m_y[i].m_linear[1] = 0.0f;
			m_y[i].m_linear[2] = 0.0f;
			m_y[i].m_linear[3] = 0.0f;
			m_y[i].m_angular[0] = 0.0f;
			m_y[i].m_angular[1] = 0.0f;
			m_y[i].m_angular[2] = 0.0f;
			m_y[i].m_angular[3] = 0.0f;
		}


		i = 0;
		ak = dgFloat32 (0.0f);
		passes = 2 + dgInt32 (dgSqrt (dgFloat32 (forceRows)));
		passes = 100000;
		accNorm = dgFloat32 (1.0e10f);
        for (k = 0; (k < passes) && ((accNorm > DG_FREEZE_MAG)); k ++) {
			accNorm = dgFloat32 (0.0f);
			for (j = 0; j < forceRows; j ++) {
				n = m_jacobianIndexArray[i].m_m0;
				m_y[n].m_linear += m_Jt[i].m_jacobian_IM0.m_linear.Scale (ak);
				m_y[n].m_angular += m_Jt[i].m_jacobian_IM0.m_angular.Scale (ak);

				n = m_jacobianIndexArray[i].m_m1;
				m_y[n].m_linear += m_Jt[i].m_jacobian_IM1.m_linear.Scale (ak);
				m_y[n].m_angular += m_Jt[i].m_jacobian_IM1.m_angular.Scale (ak);

				n = m_jacobianIndexArray[j].m_m0;
				dgVector acc (m_JMinv[j].m_jacobian_IM0.m_linear.CompProduct(m_y[n].m_linear));
				acc += m_JMinv[j].m_jacobian_IM0.m_angular.CompProduct (m_y[n].m_angular);

				n = m_jacobianIndexArray[j].m_m1;
				acc += m_JMinv[j].m_jacobian_IM1.m_linear.CompProduct (m_y[n].m_linear);
				acc += m_JMinv[j].m_jacobian_IM1.m_angular.CompProduct (m_y[n].m_angular);

				val = m_accel[j] - (acc.m_x + acc.m_y + acc.m_z + m_force[j] * m_diagDamp[j]);
				bk = (val + m_force[j] * m_diagJMinvJt[j]) * m_invDJMinvJt[j];

//				m_activeRow[j] = dgFloat32 (1.0f);
				if (bk < m_bilateralForceBounds[j].m_low) {
					bk = m_bilateralForceBounds[j].m_low;
//					m_activeRow[j] = dgFloat32 (0.0f);
					val = dgFloat32 (0.0f);
				} else if (bk > m_bilateralForceBounds[j].m_upper) {
					bk = m_bilateralForceBounds[j].m_upper;
//					m_activeRow[j] = dgFloat32 (0.0f);
					val = dgFloat32 (0.0f);
				}

				accNorm = GetMax (fabsf (val), accNorm);

				ak = bk - m_force[j];
				i = j;
				m_force[j] = bk;
			}
		}

//dgTrace (("(%d %d) %f\n", forceRows, k, accNorm));
/*
		akNum = dgFloat32 (0.0f);
		accNorm = dgFloat32(0.0f);
		MatrixTimeVector (forceRows, m_force, m_deltaAccel);
		for (i = 0; i < forceRows; i ++) {
			m_accel[i] -= m_deltaAccel[i];
			bk = m_accel[i] * m_invDJMinvJt[i] * m_activeRow[i];
			akNum += m_accel[i] * bk;
			m_deltaForce[i] = bk;
			accNorm = GetMax (dgAbsf (m_accel[i] * m_activeRow[i]), accNorm);                                              
		}
	 	clipVal = dgFloat32 (0.0f);

		passes = 4;
		for (i = 0; (i < passes) && (accNorm > DG_FREEZE_MAG); i ++) {

			MatrixTimeVector (forceRows, m_deltaForce, m_deltaAccel);

			akDen = dgFloat32 (0.0f);
			for (j = 0; j < forceRows; j ++) {
				akDen += m_deltaAccel[j] * m_deltaForce[j];
			}

			_DG_ASSERTE (akDen >= dgFloat32 (0.0f));
			akDen = GetMax (akDen, dgFloat32 (1.0e-16f));
			_DG_ASSERTE (dgAbsf (akDen) >= dgFloat32 (1.0e-16f));

			ak = akNum / akDen;

			clampedForce = -1;
			for (j = 0; j < forceRows; j ++) {
				if (m_activeRow[j]) {
					force = m_force[j] + ak * m_deltaForce[j];
					if (force < m_bilateralForceBounds[j].m_low) {
						_DG_ASSERTE (((m_bilateralForceBounds[j].m_low - m_force[j]) / m_deltaForce[j]) <= ak);
						ak = (m_bilateralForceBounds[j].m_low - m_force[j]) / m_deltaForce[j];
						clampedForce = j;
						clipVal = m_bilateralForceBounds[j].m_low;
					} else if (force > m_bilateralForceBounds[j].m_upper) {
						_DG_ASSERTE (((m_bilateralForceBounds[j].m_upper - m_force[j]) / m_deltaForce[j]) <= ak);
						ak = (m_bilateralForceBounds[j].m_upper - m_force[j]) / m_deltaForce[j];
						clampedForce = j;
						clipVal = m_bilateralForceBounds[j].m_upper;
					}
				}
			}


			if (clampedForce != -1) {
				akNum = dgFloat32 (0.0f);
				accNorm = dgFloat32 (0.0f);
				m_activeRow[clampedForce] = dgFloat32 (0.0f);
				for (j = 0; j < forceRows; j ++) {
					m_force[j] += ak * m_deltaForce[j];
					m_accel[j] -= ak * m_deltaAccel[j];

#ifdef _MSC_VER
					_DG_ASSERTE (dgCheckFloat(m_force[j]));
					_DG_ASSERTE (dgCheckFloat(m_accel[j]));
#endif

					val = m_accel[j] * m_invDJMinvJt[j] * m_activeRow[j];
					m_deltaForce[j] = val;
					akNum += val * m_accel[j];
					accNorm = GetMax (dgAbsf (m_accel[j] * m_activeRow[j]), accNorm);
				}
				m_force[clampedForce] = clipVal;
				i = -1;

			} else {
				accNorm = dgFloat32 (0.0f);
				for (j = 0; j < forceRows; j ++) {
					m_force[j] += ak * m_deltaForce[j];
					m_accel[j] -= ak * m_deltaAccel[j];
					accNorm = GetMax (dgAbsf (m_accel[j] * m_activeRow[j]), accNorm);

#ifdef _MSC_VER
					_DG_ASSERTE (dgCheckFloat(m_force[j]));
					_DG_ASSERTE (dgCheckFloat(m_accel[j]));
#endif
				}


				if (accNorm > DG_FREEZE_MAG) {
					bk = akNum;
					akNum = dgFloat32 (0.0f);
					for (j = 0; j < forceRows; j ++) {
						val = m_accel[j] * m_invDJMinvJt[j] * m_activeRow[j];
						m_deltaAccel[j] = val;
						akNum += m_accel[j] * val;
					}

					_DG_ASSERTE (bk >= dgFloat32 (0.0f));
					bk = GetMax (bk, dgFloat32(1.0e-17f));
					bk = akNum / bk;
					for (j = 0; j < forceRows; j ++) {
						m_deltaForce[j] = m_deltaAccel[j] + bk * m_deltaForce[j];
					}
				}
			}
		}
*/


	}
#endif


	void CalculateTireLinearJacobians (
		dgInt32 tireIndex,
		dgInt32 jacobialIndex, 
		const dgConstraint::dgPointParam& param, 
		const dgVector& dir)
	{
		m_jacobianIndexArray[jacobialIndex].m_m0 = 1;
		m_jacobianIndexArray[jacobialIndex].m_m1 = tireIndex;

		dgJacobian &jacobian0 = m_Jt[jacobialIndex].m_jacobian_IM0; 
		dgVector r0CrossDir (param.m_r0 * dir);
		jacobian0.m_linear[0] = dir.m_x;
		jacobian0.m_linear[1] = dir.m_y;
		jacobian0.m_linear[2] = dir.m_z;
		jacobian0.m_linear[3] = dgFloat32 (0.0f);
		jacobian0.m_angular[0] = r0CrossDir.m_x;
		jacobian0.m_angular[1] = r0CrossDir.m_y;
		jacobian0.m_angular[2] = r0CrossDir.m_z;
		jacobian0.m_angular[3] = dgFloat32 (0.0f);

		dgJacobian &jacobian1 = m_Jt[jacobialIndex].m_jacobian_IM1; 
		dgVector r1CrossDir (dir * param.m_r1);
		jacobian1.m_linear[0] = -dir.m_x;
		jacobian1.m_linear[1] = -dir.m_y;
		jacobian1.m_linear[2] = -dir.m_z;
		jacobian1.m_linear[3] = dgFloat32 (0.0f);
		jacobian1.m_angular[0] = r1CrossDir.m_x;
		jacobian1.m_angular[1] = r1CrossDir.m_y;
		jacobian1.m_angular[2] = r1CrossDir.m_z;
		jacobian1.m_angular[3] = dgFloat32 (0.0f);

		dgVector centrError (param.m_centripetal1 - param.m_centripetal0);

		m_diagDamp[jacobialIndex] = dgFloat32 (0.0f);
		m_accel[jacobialIndex] = centrError % dir; 
		m_bilateralForceBounds[jacobialIndex].m_low = DG_MIN_BOUND;
		m_bilateralForceBounds[jacobialIndex].m_upper = DG_MAX_BOUND;
	}

	void CalculateTireAngularJacobians (dgInt32 tireIndex, dgInt32 jacobialIndex, const dgVector& dir)
	{
		m_jacobianIndexArray[jacobialIndex].m_m0 = 1;
		m_jacobianIndexArray[jacobialIndex].m_m1 = tireIndex;

		dgJacobian &jacobian0 = m_Jt[jacobialIndex].m_jacobian_IM0; 

		jacobian0.m_linear[0] = dgFloat32 (0.0f);
		jacobian0.m_linear[1] = dgFloat32 (0.0f);
		jacobian0.m_linear[2] = dgFloat32 (0.0f);
		jacobian0.m_linear[3] = dgFloat32 (0.0f);
		jacobian0.m_angular[0] = dir.m_x;
		jacobian0.m_angular[1] = dir.m_y;
		jacobian0.m_angular[2] = dir.m_z;
		jacobian0.m_angular[3] = dgFloat32 (0.0f);

		dgJacobian &jacobian1 = m_Jt[jacobialIndex].m_jacobian_IM1; 

		jacobian1.m_linear[0] = dgFloat32 (0.0f);
		jacobian1.m_linear[1] = dgFloat32 (0.0f);
		jacobian1.m_linear[2] = dgFloat32 (0.0f);
		jacobian1.m_linear[3] = dgFloat32 (0.0f);
		jacobian1.m_angular[0] = -dir.m_x;
		jacobian1.m_angular[1] = -dir.m_y;
		jacobian1.m_angular[2] = -dir.m_z;
		jacobian1.m_angular[3] = dgFloat32 (0.0f);

		m_diagDamp[jacobialIndex] = dgFloat32 (0.0f);
		m_accel[jacobialIndex] = dgFloat32 (0.0f);  
		m_bilateralForceBounds[jacobialIndex].m_low = DG_MIN_BOUND;
		m_bilateralForceBounds[jacobialIndex].m_upper = DG_MAX_BOUND;
	}

	void CalculateConstraintForces (dgVehicleConstraint& vehicle, dgFloat32 timestep);

	class dgJacobianIndex 
	{	
		public:
		dgInt32 m_m0;
		dgInt32 m_m1;
	};

	dgBody m_sentinel;

	dgJacobian m_y[DG_MAX_VEHICLE_WHEELS * 2];
	dgJacobianPair m_Jt[DG_MAX_VEHICLE_DOF * 2];
	dgJacobianPair m_JMinv[DG_MAX_VEHICLE_DOF * 2];
	

	dgFloat32 m_force[DG_MAX_VEHICLE_DOF * 2];
	dgFloat32 m_accel[DG_MAX_VEHICLE_DOF * 2];
	dgFloat32 m_deltaForce[DG_MAX_VEHICLE_DOF * 2];
	dgFloat32 m_deltaAccel[DG_MAX_VEHICLE_DOF * 2];
	dgFloat32 m_diagDamp[DG_MAX_VEHICLE_DOF * 2];
	dgFloat32 m_invDJMinvJt[DG_MAX_VEHICLE_DOF * 2];
#ifdef DG_VEHICLE_SEIDLE
	dgFloat32 m_diagJMinvJt[DG_MAX_VEHICLE_DOF * 2];
#else
	dgFloat32 m_activeRow[DG_MAX_VEHICLE_DOF * 2];
#endif
	dgBilateralBounds m_bilateralForceBounds[DG_MAX_VEHICLE_DOF * 2];

	dgVector m_tireForce[DG_MAX_VEHICLE_WHEELS];
	dgVector m_tireTorque[DG_MAX_VEHICLE_WHEELS];
	dgVector m_tireVeloc[DG_MAX_VEHICLE_WHEELS];
	dgVector m_tireOmega[DG_MAX_VEHICLE_WHEELS];


	dgVector m_bodyForce[DG_MAX_VEHICLE_WHEELS * 2];
	dgVector m_bodyTorque[DG_MAX_VEHICLE_WHEELS * 2];
	dgBody *m_bodyArray[DG_MAX_VEHICLE_WHEELS * 2];
	dgVehicleConstraint::dgTire* m_tires[DG_MAX_VEHICLE_WHEELS];

	dgJacobianIndex m_jacobianIndexArray[DG_MAX_VEHICLE_DOF * 2]; 

	dgInt32 m_rows;
	dgInt32 m_bodies;
	dgInt32 m_vehicleBodiesCount;
}DG_GCC_VECTOR_ALIGMENT;



dgVehicleConstraint::WheelsSet::WheelsSet()
{
}


dgVehicleList::dgVehicleList()
	:dgTree<dgVehicleConstraint*, void*>()
{
	m_minFrec = DG_MIN_FREC;
}

dgVehicleList::~dgVehicleList()
{
	_DG_ASSERTE (GetCount() == 0);
}




//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////



dgVehicleConstraint::dgVehicleConstraint(dgWorld* world)
	:dgConstraint(), m_wheels() 
{
	_DG_ASSERTE ((sizeof (dgVehicleConstraint) & 15) == 0);


//	dgVehicleConstraint* constraint;
	m_world = world;
	dgVehicleConstraintArray& array = *world;

//	constraint = array.GetElement();

	_DG_ASSERTE ((sizeof(dgVehicleConstraint) & 15) == 0);
	//	constraint->Init ();
	
	m_maxDOF = 0;
	m_updateLRU = 0;
	m_destructor = NULL;
	m_constId = dgVehicleContraintId;

	m_tireCount = 0;
	m_vehicleUpdate = NULL;

	dgVehicleList& vehicleList = array; 
	vehicleList.Insert(this, this);
}

dgVehicleConstraint::~dgVehicleConstraint()
{
	_DG_ASSERTE (0);
/*
	dgVehicleConstraintArray& array = *m_world;

	dgVehicleList& vehicleList = array; 
	vehicleList.Remove(this);

	if (m_destructor) {
		m_destructor(*this);
	}

	dgVehicleConstraint::WheelsSet::dgListNode *node;
	while (m_wheels.GetCount()) {
		node = m_wheels.GetFirst();
		RemoveTire (node);
	}
//	array.RemoveElement (this);

	dgVehicleCache& vehicleCache = *m_world;
	vehicleCache.Remove(m_body0);

	dgCompoundCollision* vehicelCollision;
	vehicelCollision = (dgCompoundCollision*) m_body0->GetCollision();
	vehicelCollision->SetAsVehicle(false);
*/
}


/*
dgVehicleConstraint* dgVehicleConstraint::Create(dgWorld* world)
{
	dgVehicleConstraint* constraint;

	dgVehicleConstraintArray& array = *world;
	constraint = array.GetElement();

	_DG_ASSERTE ((sizeof(dgVehicleConstraint) & 15) == 0);
//	constraint->Init ();
	constraint->m_maxDOF = 0;
	constraint->m_updateLRU = 0;
	constraint->m_destructor = NULL;
	constraint->m_constId = dgVehicleContraintId;

	//	constraint->m_useRauCast = false;
	//	constraint->m_isBalaced = 0;
	constraint->m_tireCount = 0;
	constraint->m_vehicleUpdate = NULL;

	dgVehicleList& vehicleList = array; 
	vehicleList.Insert(constraint, constraint);

	return constraint;

}

void dgVehicleConstraint::Remove(dgWorld* world)
{
	dgVehicleConstraintArray& array = * world;

	dgVehicleList& vehicleList = array; 
	vehicleList.Remove(this);

	if (m_destructor) {
		m_destructor(*this);
	}

	dgVehicleConstraint::WheelsSet::dgListNode *node;
	while (m_wheels.GetCount()) {
		node = m_wheels.GetFirst();
		RemoveTire (node);
	}
	array.RemoveElement (this);

	dgVehicleCache& vehicleCache = *world;
	vehicleCache.Remove(m_body0);

	dgCompoundCollision* vehicelCollision;
	vehicelCollision = (dgCompoundCollision*) m_body0->GetCollision();
	vehicelCollision->SetAsVehicle(false);
}
*/


void dgVehicleConstraint::Reset ()
{

	WheelsSet::Iterator iter (m_wheels);
	for (iter.Begin(); iter; iter ++) {
		dgTire& tire = *iter;

		tire.m_parametricPosit = dgFloat32 (0.0f);
		tire.m_parametricAngle = dgFloat32 (0.0f);
		tire.m_parametricSpeed = dgFloat32 (0.0f);
		tire.m_parametricOmega = dgFloat32 (0.0f);
	}
}



void dgVehicleConstraint::SetTireCallback (OnVehicleUpdate update) 
{
	m_vehicleUpdate = update;
}


void dgVehicleConstraint::SetSteerAngle (void* index, dgFloat32 angle)
{
	dgVehicleConstraint::WheelsSet::dgListNode *node;
	node = (dgVehicleConstraint::WheelsSet::dgListNode *) index;

	dgTire& tire = node->GetInfo();

	tire.m_steerAngle = angle;

	dgMatrix steerMatrix (dgQuaternion (m_upPin, angle), tire.m_localMatrix.m_posit);
	dgMatrix baseMatrix (tire.m_baseRotation, dgVector (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (1.0f)));

	tire.m_localMatrix = baseMatrix * steerMatrix;		
}



void dgVehicleConstraint::SetDestructorCallback (OnConstraintDestroy destructor)
{
	m_destructor = destructor;
}


void dgVehicleCache::UpdateVehicleContacts ()
{
	dgBody* body;
//	dgLink* link;
	
	dgVehicleConstraint *vehicle;
	dgBodyMasterListRow::dgListNode* link; 

	while (GetRoot()) {
		_DG_ASSERTE (0);
		body = GetRoot()->GetInfo();
		_DG_ASSERTE (body->m_masterNode->GetInfo().GetBody() == body);

//		for (link = body->m_firstConstraintLink; link->m_constraint->GetId() != dgVehicleContraintId; link = link->m_twin->m_next){}
		for (link = body->m_masterNode->GetInfo().GetFirst(); link->GetInfo().m_joint->GetId(); link = link->GetNext()) {}
//		vehicle = (dgVehicleConstraint*) link->m_constraint;
		vehicle = (dgVehicleConstraint*) link->GetInfo().m_joint;
		vehicle->AddContacts ();
		Remove (GetRoot());
	}
}




void dgVehicleConstraint::InitVehicle(const dgVector& upDir)
{
	_DG_ASSERTE (0);
/*
	dgInt32 i;
	dgInt32 collisionCounts;
	dgBody *body;
	dgWorld *world;
	dgCollision *collision;
	dgCollision *collisionArray[256];

	// set the vehicle up vector in local space,
	m_upPin = m_body0->GetMatrix().UnrotateVector (upDir.Scale (dgFloat32 (1.0f) / dgSqrt (upDir % upDir)));

	//replace the collision the body collision geometry for a compound collision
	body = m_body0;
	world = body->GetWorld();

	collisionCounts = 1;
	collisionArray[0] = body->GetCollision();
	if (body->GetCollision()->GetCollisionPrimityType() == m_compoundCollision) {
		dgCompoundCollision *compound;
		compound = (dgCompoundCollision *)body->GetCollision();
		for (i = 0; i < compound->m_count; i ++) {
			collisionArray[i] = compound->m_array[i];
		}
		collisionCounts = compound->m_count;
	}

	for (i = 0; i < collisionCounts; i ++) {
		collisionArray[i]->AddRef();
	}

	collision = world->CreateCompoundCollision (collisionCounts, collisionArray);
	collision->SetAsVehicle(true);

	body->AttachCollision (collision);

	for (i = 0; i < collisionCounts; i ++) {
		world->ReleaseCollision (collisionArray[i]);
	}
	world->ReleaseCollision(collision);
*/
}

void dgVehicleConstraint::GetTireLocalMatrix (void* index, dgMatrix& matrix) const
{
	dgVehicleConstraint::WheelsSet::dgListNode *node;
	node = (dgVehicleConstraint::WheelsSet::dgListNode *) index;

	dgTire& tire = node->GetInfo();

	dgQuaternion rotation (tire.m_rotationPin, tire.m_parametricAngle);
	matrix = dgMatrix(rotation, dgVector (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (1.0f)));
	matrix = tire.m_tireCollisionOffset * matrix * tire.m_localMatrix;
	matrix.m_posit += m_upPin.Scale (tire.m_parametricPosit);
}

void dgVehicleConstraint::GetTireMatrix (void* index, dgMatrix& matrix) const
{
	dgVehicleConstraint::WheelsSet::dgListNode *node;
	node = (dgVehicleConstraint::WheelsSet::dgListNode *) index;

	dgTire& tire = node->GetInfo();

	dgQuaternion rotation (tire.m_rotationPin, tire.m_parametricAngle);
	dgMatrix localMatrix (rotation, dgVector (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (1.0f)));
	localMatrix = localMatrix * tire.m_localMatrix;
	localMatrix.m_posit += m_upPin.Scale (tire.m_parametricPosit);
	matrix = localMatrix * m_body0->m_matrix;
}


void dgVehicleConstraint::AddContacts()
{
_DG_ASSERTE (0);
/*
	dgInt32 i;
	dgInt32 count;
	dgInt32 activeContacts;
	dgLink* ptr;
	dgLink* link;
	
	dgBody* body0;
	dgBody* body1;
	dgContact* contact;
	dgCollision *collision;
	dgList<dgContactMaterial>::dgListNode* node; 
	TiresContacts contacts[DG_MAX_VEHICLE_WHEELS * 8];

	count = 0;
	link = m_body0->m_firstConstraintLink;
	do {
		_DG_ASSERTE (link->m_body == m_body0);
		if (link->m_constraint->GetId() == dgContactConstraintId) {
			contact = (dgContact*)link->m_constraint;
			activeContacts = contact->m_activeContacts;
			node = contact->GetFirst();
			for ( i = 0; i < activeContacts; i ++) {
				dgContactMaterial& contactPoint = node->GetInfo();; 
				body0 = contactPoint.m_body0;
				body1 = contactPoint.m_body1;
				collision = contactPoint.m_collision0;
				if (body0 != m_body0) {
					_DG_ASSERTE (m_body0 == contactPoint.m_body1);
					collision = contactPoint.m_collision1;
					Swap (body0, body1);
				}
				if (collision->IsTire()) {
					body0 = NULL;
					if (body1->m_invMass.m_w != dgFloat32 (0.0f)) {
						ptr = body1->m_firstConstraintLink;
						do {
							if (ptr->m_constraint->GetId() == dgVehicleContraintId) {
								body0 = body1;
								break;
							}
							ptr = ptr->m_twin->m_next;
						} while (ptr != body1->m_firstConstraintLink);
					}
					if (!body0) {
						contacts[count].m_contact = contact;
						contacts[count].m_node = node;
						count ++;
					}
				}
				node = node->GetNext();
			}
		}

		link = link->m_twin->m_next;
	} while (link != m_body0->m_firstConstraintLink);

	WheelsSet::Iterator iter (m_wheels);
	const dgMatrix& matrix = m_body0->GetMatrix();
	const dgVector upDir (matrix.RotateVector (m_upPin));
	for (iter.Begin(); iter; iter ++) {
		dgTire& tire = *iter;
		tire.m_collision->SelectContacts (matrix, upDir, contacts, count);
	}
*/
}


void* dgVehicleConstraint::AddTire (
	const dgMatrix& matrix, 
	const dgVector& pin, 
	dgFloat32 mass, 
	dgFloat32 width, 
	dgFloat32 radius, 
	dgFloat32 suspesionShock, 
	dgFloat32 suspesionSpring, 
	dgFloat32 suspesionLength,
	void* collisionID, 
	void* usedData)
{
	dgBody *body;
	dgWorld *world;
	dgMatrix tireMatrix;
	WheelsSet::dgListNode* wheel;
	dgCompoundCollision* collision;
	dgCollisionChamferCylinder* tireCollision;

	if (m_wheels.GetCount() > DG_MAX_VEHICLE_WHEELS) {
		return 0;
	}


	body = m_body0;
	world = body->GetWorld();

//	m_wheels.Append ();
//	wheel = m_wheels.GetLast();
	wheel = m_wheels.Append ();

	dgTire& tire = wheel->GetInfo();

	_DG_ASSERTE ((((dgUnsigned64)&tire.m_localMatrix) & 15) == 0);

	memset (&tire, 0, sizeof (dgTire));
	tire.m_vehicle = this;
	tire.m_localMatrix = matrix;
	tire.m_baseRotation = dgQuaternion (tire.m_localMatrix);
	tire.m_tireCollisionOffset = dgMatrix (dgQuaternion(m_upPin, dgPI * dgFloat32 (0.5f)), dgVector (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (1.0f)));
	tire.m_rotationPin = pin.Scale (dgFloat32 (1.0f) / dgSqrt (pin % pin));
	tire.m_mass = dgAbsf (mass);
	tire.m_width = dgAbsf (width);
	tire.m_radius = dgAbsf (radius);

	tire.m_steerAngle = dgFloat32 (0.0f);	
	tire.m_steerAngle = dgFloat32 (0.0f);
	tire.m_tireNormalLoad = dgVector (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));


	tire.m_lateralSpeed = dgFloat32 (0.0f);
	tire.m_longitudinalSpeed = dgFloat32 (0.0f);

	tire.m_suspesionShock = dgAbsf (suspesionShock);
	tire.m_suspesionSpring = dgAbsf (suspesionSpring);
	tire.m_suspesionLength = dgAbsf (suspesionLength);

	tire.m_invMass = dgFloat32 (1.0f) / tire.m_mass;
	tire.m_inertia = mass * radius * radius * dgFloat32 (0.4f);
	tire.m_invInertia = dgFloat32 (1.0f) / tire.m_inertia;

	tire.m_collision = NULL;
	tire.m_collisionID = collisionID;
	tire.m_userData = usedData;
	tire.m_isAirborne = false;
	tire.m_lostSideGrip = false;
	tire.m_lostTraction = false;

	// add tire at max decompression 
	collision = (dgCompoundCollision*) body->GetCollision();
	GetTireLocalMatrix(wheel, tireMatrix); 
	tireCollision = new dgTireCollision (world, &tire, tireMatrix, m_upPin);
	collision->AddCollision(tireCollision);

	dgVector p0;
	dgVector p1;
	collision->CalcAABB (body->GetMatrix(), p0, p1);
	return wheel;
}


void dgVehicleConstraint::RemoveTire (void* index)
{
	dgTireCollision* tireCollision;
	dgCompoundCollision* vehicelCollision;
	dgVehicleConstraint::WheelsSet::dgListNode *node;

	if (index) {
		node = (dgVehicleConstraint::WheelsSet::dgListNode *) index;
		vehicelCollision = (dgCompoundCollision*) m_body0->GetCollision();
		dgTire& tire = node->GetInfo();

		tireCollision = tire.m_collision; 
		vehicelCollision->RemoveCollision (tireCollision);
		m_wheels.Remove (node);
		tireCollision->ReleaseTire();
	}
}


void dgVehicleForceMiniSolver::CalculateConstraintForces (dgVehicleConstraint& vehicle, dgFloat32 timestep)
{
_DG_ASSERTE (0);
/*
	dgInt32 i;
	dgInt32 j;
	dgInt32 k;
	dgInt32 rows;
	dgInt32 body2Index;
//	dgInt32 vehicleBodiesCount;
	dgFloat32 dt;
	dgFloat32 invdt;
	dgFloat32 mag2;
	dgFloat32 slip;
	dgFloat32 diag;
	dgFloat32 accel;
	dgFloat32 force;
	dgFloat32 alpha;
	dgFloat32 omega;
	dgFloat32 invMass;
	dgFloat32 springDamp;
	dgFloat32 relVelocErr;
	dgFloat32 penetration;
	dgFloat32 normalForceMag;
	dgFloat32 penetrationVeloc;
	dgFloat32 lateralStaticFriction;
	dgFloat32 lateralKineticFriction;
	dgFloat32 longitudinalStaticFriction;
	dgFloat32 longitudinalKineticFriction;
	dgBody *body;
	dgBody *body2;
	dgTireCollision* collision;
	dgConstraint::dgPointParam pointDataP;

	dt = timestep;
	invdt = dgFloat32 (1.0f) / timestep;

	body = vehicle.GetBody0();
	const dgMatrix& matrix = body->GetMatrix();

	dgVector zeroVector (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));
	const dgVector& localUpDir = vehicle.m_upPin;
	const dgVector upDir (matrix.RotateVector (localUpDir));

	dgVector bodyOmega (body->GetOmega());
	dgVector bodyVeloc (body->GetVelocity());

	rows = 0;
	m_bodies = 2;

	m_bodyArray[1] = body;
	m_bodyArray[0] = &m_sentinel;

	m_bodyForce[0] = zeroVector;
	m_bodyForce[1] = zeroVector;

	m_bodyTorque[0] = zeroVector;
	m_bodyTorque[1] = zeroVector;

	memset (&m_sentinel, 0, sizeof (dgBody));

	springDamp = dgFloat32 (vehicle.m_wheels.GetCount());
	if (!springDamp) {
		springDamp = dgFloat32 (1.0f);
	}
	springDamp = dgPow (dgFloat32 (2.0f), dgFloat32 (1.0f) / springDamp) - dgFloat32 (1.0f);
	// add Jacobian between body chassis and tires
//	dgVector bodyOrigin (matrix.TransformVector (body->GetCentreOfMass()));
	const dgVector& bodyOrigin = body->m_globalCentreOfMass;
	dgVehicleConstraint::WheelsSet::Iterator iter (vehicle.m_wheels);
	for (iter.Begin(); iter; iter ++) {
		dgVehicleConstraint::dgTire& tire = *iter;
		m_tires[m_bodies] = &tire;

		dgVector rightDir (matrix.RotateVector (tire.m_localMatrix.RotateVector(tire.m_rotationPin)));
		dgVector frontDir (upDir * rightDir);

		dgVector p0 (matrix.TransformVector (tire.m_localMatrix.m_posit + localUpDir.Scale (tire.m_parametricPosit)));

		pointDataP.m_r0 = p0 - bodyOrigin;
		pointDataP.m_posit0 = p0;
		pointDataP.m_veloc0 = bodyOmega * pointDataP.m_r0;
		pointDataP.m_centripetal0 = bodyOmega * pointDataP.m_veloc0;
		pointDataP.m_veloc0 += bodyVeloc;

		pointDataP.m_r1 = zeroVector;
		pointDataP.m_posit1 = p0;
		pointDataP.m_centripetal1 = zeroVector;
		pointDataP.m_veloc1 = pointDataP.m_veloc0 + upDir.Scale (tire.m_parametricSpeed);

		tire.m_lateralSpeed = bodyVeloc % rightDir;
		tire.m_longitudinalSpeed = bodyVeloc % frontDir;

		tire.m_bodyIndex = m_bodies;
		m_tireForce[m_bodies] = upDir.Scale(tire.m_tireForce);
		m_tireTorque[m_bodies] = rightDir.Scale(tire.m_tireTorque);

		m_tireVeloc[m_bodies] = pointDataP.m_veloc1;
		m_tireOmega[m_bodies] = bodyOmega + rightDir.Scale(tire.m_parametricOmega);

		CalculateTireLinearJacobians (m_bodies, rows++, pointDataP, frontDir);
		CalculateTireLinearJacobians (m_bodies, rows++, pointDataP, rightDir);
		CalculateTireLinearJacobians (m_bodies, rows, pointDataP, upDir);


		dgFloat32 relPosit = tire.m_parametricPosit;
		dgFloat32 relVeloc = tire.m_parametricSpeed;
		dgFloat32 ks = tire.m_suspesionSpring;
		dgFloat32 kd = tire.m_suspesionShock;
		dgFloat32 ksd = dt * ks;
		dgFloat32 den = dgFloat32 (1.0f) + dt * kd + dt * ksd;
		dgFloat32 num = ks * relPosit + (kd + ksd) * relVeloc;
		m_accel[rows] = num / den;
		m_diagDamp[rows] = springDamp;
		rows ++;

		CalculateTireAngularJacobians (m_bodies, rows++, upDir);
		CalculateTireAngularJacobians (m_bodies, rows++, frontDir);
		if (tire.m_brakeFrictionTorque > dgFloat32 (1.0e-10f)) {
			CalculateTireAngularJacobians (m_bodies, rows, rightDir);
			m_diagDamp[rows] = dgFloat32 (0.0f);
			m_accel[rows] = tire.m_brakeAcceleration;  
			m_bilateralForceBounds[rows].m_low = -tire.m_brakeFrictionTorque;
			m_bilateralForceBounds[rows].m_upper = tire.m_brakeFrictionTorque;
			rows++;
		}
		m_bodies ++;
	}

	// add Jacobian between body tires and floor or other bodies
 	m_vehicleBodiesCount = m_bodies;
	for (iter.Begin(); iter; iter ++) {
		dgVehicleConstraint::dgTire& tire = *iter;

		tire.m_isAirborne = true;
		tire.m_lostSideGrip = false;
		tire.m_lostTraction = false;
		collision = tire.m_collision;
		_DG_ASSERTE (collision->m_activeContactsCount <= collision->m_masterContactCount);
		for (k = 0; k < collision->m_activeContactsCount; k ++) {
			dgVector p0 (matrix.TransformVector (tire.m_localMatrix.m_posit + localUpDir.Scale (tire.m_parametricPosit)));
			penetration = GetMin (collision->m_activeContactsContacts[k].m_penetration, dgFloat32 (0.1f));
			const dgVector& contactPosit = collision->m_activeContactsContacts[k].m_point;
			const dgVector& contactNormal = collision->m_activeContactsContacts[k].m_normal;

			longitudinalStaticFriction = collision->m_activeContactsContacts[k].m_staticFriction0 * DG_TIRES_LONGITUDINAL_FRICTION_MULTIPLIER;
			longitudinalKineticFriction = collision->m_activeContactsContacts[k].m_dynamicFriction0 * DG_TIRES_LONGITUDINAL_FRICTION_MULTIPLIER;

			lateralStaticFriction = collision->m_activeContactsContacts[k].m_staticFriction1 * DG_TIRES_LATERAL_FRICTION_MULTIPLIER;
			lateralKineticFriction = collision->m_activeContactsContacts[k].m_dynamicFriction1 * DG_TIRES_LATERAL_FRICTION_MULTIPLIER;

			body2Index = 0;
			body2 = (collision->m_activeContactsContacts[k].m_pair0.m_body == body) ? 
				     collision->m_activeContactsContacts[k].m_pair1.m_body : 
					 collision->m_activeContactsContacts[k].m_pair0.m_body;

			slip = dgFloat32 (0.0f);
			if (body2->m_invMass.m_w != dgFloat32 (0.0f)) {
				slip = GetMin (tire.m_mass * body2->m_invMass.m_w, dgFloat32 (0.5f));  
				for (body2Index = m_vehicleBodiesCount; body2Index < m_bodies; body2Index ++) {
					if (body2 == m_bodyArray[body2Index]) {
						break;
					}
				}

				
				if (body2Index == m_bodies) {
					m_bodyArray[m_bodies] = body2;

					m_bodyForce[m_bodies] = zeroVector;
					m_bodyTorque[m_bodies] = zeroVector;

//					body->CalcInvInertiaMatrix (m_invInertiaMatrixArray[m_bodies]);
					m_bodies ++;
				}
			} 

			pointDataP.m_r0 = contactPosit - body2->m_globalCentreOfMass;
			pointDataP.m_veloc0 = body2->m_omega * pointDataP.m_r0;
			pointDataP.m_centripetal0 = body2->m_omega * pointDataP.m_veloc0;
			pointDataP.m_veloc0 += body2->m_veloc;

			i = tire.m_bodyIndex;
			pointDataP.m_r1 = contactPosit - p0;
			pointDataP.m_posit1 = contactPosit;
			pointDataP.m_veloc1 = m_tireOmega[i] * pointDataP.m_r1;
			pointDataP.m_centripetal1 = zeroVector;;
			pointDataP.m_veloc1 += m_tireVeloc[i];
			dgVector velocError (pointDataP.m_veloc1 - pointDataP.m_veloc0);

			tire.m_isAirborne = false;
			// calculate the tire normal derivatives
			{
				m_jacobianIndexArray[rows].m_m0 = body2Index;
				m_jacobianIndexArray[rows].m_m1 = i;

				dgJacobian &jacobian0 = m_Jt[rows].m_jacobian_IM0; 
				dgVector r0CrossDir (pointDataP.m_r0 * contactNormal);
				jacobian0.m_linear[0] = contactNormal.m_x;
				jacobian0.m_linear[1] = contactNormal.m_y;
				jacobian0.m_linear[2] = contactNormal.m_z;
				jacobian0.m_linear[3] = dgFloat32 (0.0f);
				jacobian0.m_angular[0] = r0CrossDir.m_x;
				jacobian0.m_angular[1] = r0CrossDir.m_y;
				jacobian0.m_angular[2] = r0CrossDir.m_z;
				jacobian0.m_angular[3] = dgFloat32 (0.0f);

				dgJacobian &jacobian1 = m_Jt[rows].m_jacobian_IM1; 
				dgVector r1CrossDir (contactNormal * pointDataP.m_r1);
				jacobian1.m_linear[0] = -contactNormal.m_x;
				jacobian1.m_linear[1] = -contactNormal.m_y;
				jacobian1.m_linear[2] = -contactNormal.m_z;
				jacobian1.m_linear[3] = dgFloat32 (0.0f);
				jacobian1.m_angular[0] = r1CrossDir.m_x;
				jacobian1.m_angular[1] = r1CrossDir.m_y;
				jacobian1.m_angular[2] = r1CrossDir.m_z;
				jacobian1.m_angular[3] = dgFloat32 (0.0f);

				relVelocErr = velocError % contactNormal;
				penetrationVeloc = dgFloat32 (0.5f) * penetration * invdt;
				relVelocErr *= dgFloat32 (1.1f);

				m_bilateralForceBounds[rows].m_low = dgFloat32 (0.0f);
				m_bilateralForceBounds[rows].m_upper = DG_MAX_BOUND;
				//m_diagDamp[rows] = dgFloat32 (0.0f);
				m_diagDamp[rows] = slip; 
				m_accel[rows] = (penetrationVeloc + relVelocErr) * invdt;
				rows ++;
			}

			// calculate the tire load
			normalForceMag = (m_tireForce[tire.m_bodyIndex] - tire.m_tireNormalLoad) % contactNormal;
//			normalForceMag = localUpDir.Scale ((m_tireForce[tire.m_bodyIndex] - tire.m_tireNormalLoad) % localUpDir) % contactNormal;
			if (normalForceMag > dgFloat32 (0.0f)) {

				dgVector rightDir (matrix.RotateVector (tire.m_localMatrix.RotateVector(tire.m_rotationPin)));
				dgVector frontDir (contactNormal * rightDir);

				mag2 = frontDir % frontDir;
				if (mag2 > dgFloat32 (1.0e-4f)) {
					frontDir = frontDir.Scale (dgRsqrt (mag2));
					rightDir = frontDir * contactNormal;

					// calculate the tire longitudinal forces derivatives
					{
						m_jacobianIndexArray[rows].m_m0 = body2Index;
						m_jacobianIndexArray[rows].m_m1 = i;

						dgJacobian &jacobian0 = m_Jt[rows].m_jacobian_IM0; 

						dgVector r0CrossDir (pointDataP.m_r0 * frontDir);
						jacobian0.m_linear[0] = frontDir.m_x;
						jacobian0.m_linear[1] = frontDir.m_y;
						jacobian0.m_linear[2] = frontDir.m_z;
						jacobian0.m_linear[3] = dgFloat32 (0.0f);
						jacobian0.m_angular[0] = r0CrossDir.m_x;
						jacobian0.m_angular[1] = r0CrossDir.m_y;
						jacobian0.m_angular[2] = r0CrossDir.m_z;
						jacobian0.m_angular[3] = dgFloat32 (0.0f);


						dgJacobian &jacobian1 = m_Jt[rows].m_jacobian_IM1; 
						dgVector r1CrossDir (frontDir * pointDataP.m_r1);
						jacobian1.m_linear[0] = -frontDir.m_x;
						jacobian1.m_linear[1] = -frontDir.m_y;
						jacobian1.m_linear[2] = -frontDir.m_z;
						jacobian1.m_linear[3] = dgFloat32 (0.0f);
						jacobian1.m_angular[0] = r1CrossDir.m_x;
						jacobian1.m_angular[1] = r1CrossDir.m_y;
						jacobian1.m_angular[2] = r1CrossDir.m_z;
						jacobian1.m_angular[3] = dgFloat32 (0.0f);

						relVelocErr = velocError % frontDir;
						m_diagDamp[rows] = tire.m_longiSlideCoef;
						m_accel[rows] = relVelocErr * invdt;

						if (dgAbsf (relVelocErr) < tire.m_maxSlideSpeed) {
							m_bilateralForceBounds[rows].m_low = - normalForceMag * longitudinalStaticFriction;
							m_bilateralForceBounds[rows].m_upper = normalForceMag * longitudinalStaticFriction;
						} else {
							tire.m_lostTraction = true;
							m_bilateralForceBounds[rows].m_low = - normalForceMag * longitudinalKineticFriction;
							m_bilateralForceBounds[rows].m_upper = normalForceMag * longitudinalKineticFriction;
						}
						rows ++;
					}

					// calculate the tire side forces derivatives
					{
						m_jacobianIndexArray[rows].m_m0 = body2Index;
						m_jacobianIndexArray[rows].m_m1 = i;

						dgJacobian &jacobian0 = m_Jt[rows].m_jacobian_IM0; 
						dgVector r0CrossDir (pointDataP.m_r0 * rightDir);
						jacobian0.m_linear[0] = rightDir.m_x;
						jacobian0.m_linear[1] = rightDir.m_y;
						jacobian0.m_linear[2] = rightDir.m_z;
						jacobian0.m_linear[3] = dgFloat32 (0.0f);
						jacobian0.m_angular[0] = r0CrossDir.m_x;
						jacobian0.m_angular[1] = r0CrossDir.m_y;
						jacobian0.m_angular[2] = r0CrossDir.m_z;
						jacobian0.m_angular[3] = dgFloat32 (0.0f);

						dgJacobian &jacobian1 = m_Jt[rows].m_jacobian_IM1; 
						dgVector r1CrossDir (rightDir * pointDataP.m_r1);
						jacobian1.m_linear[0] = -rightDir.m_x;
						jacobian1.m_linear[1] = -rightDir.m_y;
						jacobian1.m_linear[2] = -rightDir.m_z;
						jacobian1.m_linear[3] = dgFloat32 (0.0f);
						jacobian1.m_angular[0] = r1CrossDir.m_x;
						jacobian1.m_angular[1] = r1CrossDir.m_y;
						jacobian1.m_angular[2] = r1CrossDir.m_z;
						jacobian1.m_angular[3] = dgFloat32 (0.0f);

						relVelocErr = velocError % rightDir;
						m_diagDamp[rows] = tire.m_sideSlipCoef;
						m_accel[rows] = relVelocErr * invdt;

						if (dgAbsf (relVelocErr) < tire.m_maxSideSpeed) {
							m_bilateralForceBounds[rows].m_low = - normalForceMag * lateralStaticFriction;
							m_bilateralForceBounds[rows].m_upper = normalForceMag * lateralStaticFriction;
						} else {
							tire.m_lostSideGrip = true;
							m_bilateralForceBounds[rows].m_low = - normalForceMag * lateralKineticFriction;
							m_bilateralForceBounds[rows].m_upper = normalForceMag * lateralKineticFriction;
						}
						rows ++;
					}
				}
			}
		}

	}


//	m_invInertiaMatrixArray[0] = dgGetZeroMatrix();
//	dgMatrix& invInertiaMatrixArray = dgGetZeroMatrix();

	dgMatrix invInertiaMatrixArray;
//	body->CalcInvInertiaMatrix (m_invInertiaMatrixArray[1]);
	body->CalcInvInertiaMatrix (invInertiaMatrixArray);
	invMass = body->m_invMass[3];
	for (i = 0; i < rows; i ++) {
		diag = dgFloat32 (0.0f);
		accel = dgFloat32 (0.0f);

		j = m_jacobianIndexArray[i].m_m0;
		_DG_ASSERTE (j < m_bodies); 
		_DG_ASSERTE ((j == 0) || (j == 1) || (j >= m_vehicleBodiesCount)); 
//		body = m_bodyArray[j];
//		invMass = body->m_invMass[3];
//		dgMatrix* invInertia0;
//		const dgMatrix& invInertia0 = m_invInertiaMatrixArray[j];
		if (j == 1) {
			m_JMinv[i].m_jacobian_IM0.m_linear = m_Jt[i].m_jacobian_IM0.m_linear.Scale (invMass);
			m_JMinv[i].m_jacobian_IM0.m_angular = invInertiaMatrixArray.UnrotateVector (m_Jt[i].m_jacobian_IM0.m_angular);
		} else {
			m_JMinv[i].m_jacobian_IM0.m_linear = zeroVector;
			m_JMinv[i].m_jacobian_IM0.m_angular = zeroVector;
		}

//		m_JMinv[i].m_jacobian_IM0.m_linear = m_Jt[i].m_jacobian_IM0.m_linear.Scale (invMass);
//		m_JMinv[i].m_jacobian_IM0.m_angular = invInertia0.UnrotateVector (m_Jt[i].m_jacobian_IM0.m_angular);

		accel += (m_JMinv[i].m_jacobian_IM0.m_linear % body->m_accel);
		accel += (m_JMinv[i].m_jacobian_IM0.m_angular % body->m_alpha);
		diag += (m_JMinv[i].m_jacobian_IM0.m_linear % m_Jt[i].m_jacobian_IM0.m_linear);
		diag += (m_JMinv[i].m_jacobian_IM0.m_angular % m_Jt[i].m_jacobian_IM0.m_angular);
		dgVehicleConstraint::dgTire& tire = *m_tires[m_jacobianIndexArray[i].m_m1];

		m_JMinv[i].m_jacobian_IM1.m_linear = m_Jt[i].m_jacobian_IM1.m_linear.Scale (tire.m_invMass);
		m_JMinv[i].m_jacobian_IM1.m_angular = m_Jt[i].m_jacobian_IM1.m_angular.Scale (tire.m_invInertia);

		j = tire.m_bodyIndex;
		accel += (m_JMinv[i].m_jacobian_IM1.m_linear % m_tireForce[j]);
		accel += (m_JMinv[i].m_jacobian_IM1.m_angular % m_tireTorque[j]);
		diag += (m_JMinv[i].m_jacobian_IM1.m_linear % m_Jt[i].m_jacobian_IM1.m_linear);
		diag += (m_JMinv[i].m_jacobian_IM1.m_angular % m_Jt[i].m_jacobian_IM1.m_angular);

		m_force[i] = dgFloat32 (0.0f);
		m_accel[i] -= accel;

		_DG_ASSERTE (m_diagDamp[i] >= dgFloat32 (0.0f));
		//		_DG_ASSERTE (m_diagDamp[i] <= dgFloat32 (1.0f));
		slip = DG_VEHICLE_DIAG_DAMP + m_diagDamp[i];
		m_diagDamp[i] = diag * slip;
		diag *= (dgFloat32 (1.0f) + slip);

#ifdef DG_VEHICLE_SEIDLE
		m_diagJMinvJt[i] = diag;
#else
		m_activeRow[i] = dgFloat32 (1.0f);
#endif
		m_invDJMinvJt[i] = dgFloat32 (1.0f) / diag;
	}


	m_rows = rows;
	CalculateForces(rows);

	for (iter.Begin(); iter; iter ++) {
		dgVehicleConstraint::dgTire& tire = *iter;
//		tire.m_tireNormalLoad = dgFloat32 (0.0f);
		tire.m_tireNormalLoad = dgVector (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));
	}


	// apply reaction forces to chassis and tires
	body2 = vehicle.GetBody0();
	for (i = 0; i < m_rows; i ++) {
		force = m_force[i]; 

#ifdef _MSC_VER
		_DG_ASSERTE (dgCheckFloat(force));
#endif

		j = m_jacobianIndexArray[i].m_m0;
		body = m_bodyArray[j];

		m_bodyForce[j] += m_Jt[i].m_jacobian_IM0.m_linear.Scale (force);
		m_bodyTorque[j] += m_Jt[i].m_jacobian_IM0.m_angular.Scale (force);

		dgVehicleConstraint::dgTire& tire = *m_tires[m_jacobianIndexArray[i].m_m1];

		j = tire.m_bodyIndex;
		dgVector forceComponent (m_Jt[i].m_jacobian_IM1.m_linear.Scale (force));
		m_tireForce[j] += forceComponent;
		m_tireTorque[j] += m_Jt[i].m_jacobian_IM1.m_angular.Scale (force);
		if (body2 == body) {
//			tire.m_tireNormalLoad___ -= forceComponent % upDir;
			tire.m_tireNormalLoad -= forceComponent;
//			_DG_ASSERTE (dgAbsf (tire.m_tireNormalLoad___ - (tire.m_tireNormalLoad_ % upDir)) < 0.1f);
		}
	}

	body2->m_accel += m_bodyForce[1];
	body2->m_alpha += m_bodyTorque[1];

	// integrate tire in local space
	dgVector bodyInvMass (body->GetInvMass());
	dgVector localAlpha (matrix.UnrotateVector (bodyOmega));
	dgVector bodyAccel (body->GetForce().Scale (bodyInvMass.m_w));
	dgVector bodyAlpha (matrix.RotateVector (localAlpha.CompProduct (bodyInvMass)));
	for (iter.Begin(); iter; iter ++) {
		dgVehicleConstraint::dgTire& tire = *iter;

		j = tire.m_bodyIndex; 

		dgVector tirePosit (matrix.RotateVector(tire.m_localMatrix.m_posit + localUpDir.Scale (tire.m_parametricPosit)));
		dgVector tireAccel (m_tireForce[j].Scale (tire.m_invMass));

		dgVector crossTireVeloc (bodyOmega * tirePosit);
		dgVector tireRelAccel (tireAccel - (bodyAccel + bodyOmega * crossTireVeloc + bodyAlpha * tirePosit));
		accel = upDir % tireRelAccel;
		tire.m_parametricSpeed += accel * dt;
		tire.m_parametricPosit += tire.m_parametricSpeed * dt;

		dgVector rightDir (matrix.RotateVector (tire.m_localMatrix.RotateVector(tire.m_rotationPin)));
		alpha = rightDir % m_tireTorque[j].Scale (tire.m_invInertia);
		omega = alpha * dt * 0.5f;
		tire.m_parametricOmega += omega;
		tire.m_parametricOmega *= 0.9999f;

		// clamp tire max rpm to 200 meter per seconds
		#define MAX_TIRE_SPEED dgFloat32 (400.0f)
		if (dgAbsf (tire.m_parametricOmega * tire.m_radius) > MAX_TIRE_SPEED) {
			tire.m_parametricOmega = (MAX_TIRE_SPEED / tire.m_radius) * GetSign(tire.m_parametricOmega);
		}

		tire.m_parametricAngle = dgFmod (tire.m_parametricAngle + tire.m_parametricOmega * dt, dgPI2);
		tire.m_parametricOmega += omega;
	}
*/
}

dgUnsigned32 dgVehicleConstraint::JacobianDerivative (dgContraintDescritor& params)
{
	_DG_ASSERTE (0);
/*
	bool isFloating;
	dgInt32 j;
	dgBody *body;
	dgBody *extraBody;
	dgWorld* world;
	dgFloat32 gravity;
	dgFloat32 maxGravity2;
	dgFloat32 timeSlice;
	dgFloat32 bodyMass;
	dgCompoundCollision *compound;
	dgMatrix tireMatrix;
	dgVehicleForceMiniSolver systemSolver;
	

	body = m_body0;
	world = body->m_world;

	if (dgOverlapTest (body->m_minAABB, body->m_maxAABB, world->m_min, world->m_max)) {
		bodyMass = body->GetMass().m_w;
		const dgMatrix matrix = body->GetMatrix();
		const dgVector upDir (matrix.RotateVector (m_upPin));

		WheelsSet::Iterator iter (m_wheels);
	//	dgVector bodyOrigin (body->GetCentreOfMass());
	//	const dgVector& bodyOrigin = body->m_globalCentreOfMass;
		gravity = body->GetForce().Scale (body->GetInvMass().m_w) % upDir;
		
		for (iter.Begin(); iter; iter ++) {
			dgTire& tire = *iter;

			tire.m_tireForce = gravity * tire.m_mass;
			tire.m_tireTorque = dgFloat32 (0.0f);
			tire.m_sideSlipCoef = dgFloat32 (0.0f);
			tire.m_longiSlideCoef = dgFloat32 (0.0f);
			tire.m_maxSideSpeed = MIN_SIDE_SLIP_SPEED;
			tire.m_maxSlideSpeed = MIN_SIDE_SLIDE_SPEED;
			tire.m_brakeAcceleration = dgFloat32 (0.0f);
			tire.m_brakeFrictionTorque = dgFloat32 (0.0f);
		}

		m_param = &params;
		if (m_vehicleUpdate) {
			m_vehicleUpdate (*this);
		}


		timeSlice = params.m_timestep;

		dgVector bodyForce (body->m_accel);
		dgVector bodyTorque (body->m_alpha);
		dgVector bodyVeloc (body->m_veloc);
		dgVector bodyOmega (body->m_omega);
		dgVector bodyglobalCentreOfMass (body->m_globalCentreOfMass);
		dgQuaternion rotation (body->m_rotation);

		maxGravity2 = gravity * gravity * 2000.0f;


		systemSolver.CalculateConstraintForces (*this, timeSlice);
		body->IntegrateNotUpdate (timeSlice);

		dgFloat32 scaleFactor = dgFloat32 (1.0f / 10.0f);
		for (j = systemSolver.m_vehicleBodiesCount; j < systemSolver.m_bodies; j ++) {
			dgFloat32 invMass2;
			dgFloat32 forceMag2;

			extraBody = systemSolver.m_bodyArray[j];

			invMass2 = extraBody->m_invMass.m_w * extraBody->m_invMass.m_w;
			forceMag2 = systemSolver.m_bodyForce[j] % systemSolver.m_bodyForce[j];
			while ((forceMag2 * invMass2 * scaleFactor * scaleFactor) > maxGravity2) {
				scaleFactor *= 0.9f;
			}
			extraBody->m_accel += systemSolver.m_bodyForce[j].Scale (scaleFactor);
			extraBody->m_alpha += systemSolver.m_bodyTorque[j].Scale (scaleFactor);
		}

		for (iter.Begin(); iter; iter ++) {
			dgTire& tire = *iter;
			GetTireLocalMatrix(iter.GetNode(), tireMatrix); 
			tire.m_collision->SetMatrix (tireMatrix, m_upPin);
	//		tire.m_collision->CalculateNewContacts (body->m_matrix);
		}



		body->m_accel = (body->m_veloc - bodyVeloc).Scale (body->m_mass.m_w * params.m_invTimestep);
		dgVector alpha ((body->m_omega - bodyOmega).Scale (params.m_invTimestep));
		body->m_alpha = matrix.RotateVector (body->m_mass.CompProduct (matrix.UnrotateVector(alpha)));
		body->m_veloc = bodyVeloc;
		body->m_omega = bodyOmega;
		body->m_globalCentreOfMass = bodyglobalCentreOfMass;
		body->m_rotation = rotation;
		body->m_matrix = matrix;

		isFloating = true;
		compound = (dgCompoundCollision *)body->GetCollision();
		for (iter.Begin(); iter; iter ++) {
			dgTire& tire = *iter;
			tire.m_collision->m_activeContactsCount = 0;
			tire.m_collision->m_masterContactCount = 0;
			isFloating &= tire.m_isAirborne;
		}

		if (isFloating) {
			dgFloat32 val;
			val = body->m_mass.m_w * body->m_mass.m_w * dgFloat32 (0.01f);
			isFloating = ((body->m_accel % body->m_accel) < val) &&
				((body->m_alpha % body->m_alpha) < val) &&
				((body->m_veloc % body->m_veloc) < val) &&
				((body->m_omega % body->m_omega) < val);
			if (isFloating) {
				body->m_accel = upDir.Scale (body->m_mass.m_w * -dgFloat32 (1.0f));
			}
		}
	} else {
		dgVector zero (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));
		body->m_accel = zero;
		body->m_alpha = zero;
		body->m_veloc = zero;
		body->m_omega = zero;
	}
*/
	return 0;
}


