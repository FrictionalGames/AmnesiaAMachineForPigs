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

// dgVehicleConstraint.h: interface for the dgVehicleConstraint class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DGVEHICLECONSTRAINT_H__D8169ACD_1350_488D_8391_2EB2FF8850C8__INCLUDED_)
#define AFX_DGVEHICLECONSTRAINT_H__D8169ACD_1350_488D_8391_2EB2FF8850C8__INCLUDED_


#include "dgConstraint.h"

//template<class T> class dgPool;

#define MIN_SIDE_SLIP_SPEED 0.5f
#define MIN_SIDE_SLIDE_SPEED 0.5f

class dgWorld;
class dgTireCollision;
class dgVehicleConstraint;
typedef void (dgApi *OnVehicleUpdate) (const dgVehicleConstraint& vehicle);


class dgVehicleConstraint: public dgConstraint  
{
	public:
	class dgTire 
	{
		public:
		dgMatrix m_localMatrix;
		dgMatrix m_tireCollisionOffset;
		dgQuaternion m_baseRotation;
		dgVector  m_rotationPin;
		dgVector m_tireNormalLoad;

		dgFloat32 m_mass;
		dgFloat32 m_width;
		dgFloat32 m_radius;
		dgFloat32 m_suspesionShock;
		dgFloat32 m_suspesionSpring;
		dgFloat32 m_suspesionLength;
		void* m_collisionID;
		void* m_userData;
		dgTireCollision* m_collision;
		dgVehicleConstraint* m_vehicle;

		// calculated contacts
		dgFloat32 m_inertia;
		dgFloat32 m_invMass;
		dgFloat32 m_invInertia;

		dgFloat32 m_steerAngle;
		dgFloat32 m_tireForce;
		dgFloat32 m_tireTorque;
//		dgFloat32 m_tireNormalLoad___;

		dgFloat32 m_lateralSpeed;
		dgFloat32 m_longitudinalSpeed;
		dgFloat32 m_sideSlipCoef;
		dgFloat32 m_maxSideSpeed;
		dgFloat32 m_longiSlideCoef;
		dgFloat32 m_maxSlideSpeed;
		dgFloat32 m_brakeAcceleration;
		dgFloat32 m_brakeFrictionTorque;

		// variables;
		dgFloat32 m_parametricPosit;
		dgFloat32 m_parametricAngle;
		dgFloat32 m_parametricSpeed;
		dgFloat32 m_parametricOmega;

		dgFloat32 m_paramT;
		dgUnsigned32 m_bodyIndex;
		bool m_isAirborne;
		bool m_lostTraction;
		bool m_lostSideGrip;
	};


	class WheelsSet: public dgList<dgTire>
	{
		public:
		WheelsSet();
	};


	dgVehicleConstraint(dgWorld* world);
	virtual ~dgVehicleConstraint();

	void InitVehicle(const dgVector& upDir);
	void SetTireCallback (OnVehicleUpdate update); 

	void* AddTire (const dgMatrix& position, const dgVector& pin, dgFloat32 mass, dgFloat32 width, dgFloat32 radius, 
		dgFloat32 suspesionShock, dgFloat32 suspesionSpring, dgFloat32 suspesionLength, void* collisionID, void* usedData);


	void Reset (); 
	void RemoveTire (void* index); 
	

	void* GetFirstTireIndex() const;
	void* GetNextTireIndex(void* index) const;

	bool IsTireAirBorned (void* index) const;
	bool TireLostSideGrip (void* index) const;
	bool TireLostTraction (void* index) const;

	void* GetTireUserData (void* index) const;
	dgFloat32 GetTireOmega (void* index) const;
	dgFloat32 GetSteerAngle (void* index) const;
	dgFloat32 GetTireNormalLoad (void* index) const;
	dgFloat32 GetTireLateralSpeed (void* index) const;
	dgFloat32 GetTireLongitudinalSpeed (void* index) const;
	void GetTireMatrix (void* index, dgMatrix& matrix) const;
	void GetTireLocalMatrix (void* index, dgMatrix& matrix) const;
	dgFloat32 TireCalculateMaxBrakeAcceleration (void* index) const;


	void SetSteerAngle (void* index, dgFloat32 angle);
	void SetTireTorque (void* index, dgFloat32 torque);
	void SetTireMaxSideSleepSpeed (void* index, dgFloat32 speed);
	void SetTireMaxLongitudinalSlideSpeed (void* index, dgFloat32 speed);
	void SetTireSideSleepCoeficient (void* index, dgFloat32 coeficient);
	void SetTireLongitudinalSlideCoeficient (void* index, dgFloat32 coeficient);
	void TireSetBrakeAcceleration (void* index, dgFloat32 acceleration, dgFloat32 torqueMaxFriction);


	private:
//	static dgVehicleConstraint* Create(dgWorld* world);
//	virtual void Remove(dgWorld* world);

	void AddContacts();
	virtual void SetDestructorCallback (OnConstraintDestroy destructor);
	virtual dgUnsigned32 JacobianDerivative (dgContraintDescritor& params); 

	virtual void JointAccelerations(const dgJointAccelerationDecriptor& params) {_DG_ASSERTE (0);} 
	virtual void JointAccelerationsSimd(const dgJointAccelerationDecriptor& params){_DG_ASSERTE (0);} 
	virtual void JointVelocityCorrection(const dgJointAccelerationDecriptor& params){_DG_ASSERTE (0);} 


	dgVector m_upPin;
	WheelsSet m_wheels;
	dgWorld* m_world;
	dgContraintDescritor* m_param;
	OnVehicleUpdate m_vehicleUpdate;
	OnConstraintDestroy m_destructor;
	dgInt64 m_updateLRU;
	dgInt16 m_tireCount;

	friend class dgWorld;
	friend class dgVehicleCache;
	friend class dgVehicleForceMiniSolver;
//	friend class dgPool<dgVehicleConstraint>;
};


class dgVehicleCache: public dgTree<dgBody*, void*>
{
	public:
	void UpdateVehicleContacts ();
	void CacheVehicle (dgBody* body);
};

class dgVehicleList: public dgTree<dgVehicleConstraint*, void*>
{
	public:
	 dgVehicleList();
	~dgVehicleList();

	dgFloat32 m_minFrec;
};

class dgVehicleConstraintArray: 
	public dgVehicleCache, 
	public dgVehicleList
{
	public:
	DG_CLASS_ALLOCATOR(allocator)
};


inline void dgVehicleCache::CacheVehicle (dgBody* body) 
{
	Insert (body, body);
}


inline void* dgVehicleConstraint::GetFirstTireIndex() const
{
	return m_wheels.GetFirst();
}

inline void* dgVehicleConstraint::GetNextTireIndex(void* index) const
{
	dgVehicleConstraint::WheelsSet::dgListNode *node;
	node = (dgVehicleConstraint::WheelsSet::dgListNode *) index;

	return node->GetNext();
}

inline void* dgVehicleConstraint::GetTireUserData (void* index) const
{
	dgVehicleConstraint::WheelsSet::dgListNode *node;
	node = (dgVehicleConstraint::WheelsSet::dgListNode *) index;

	return node->GetInfo().m_userData;
}


inline dgFloat32 dgVehicleConstraint::GetSteerAngle (void* index) const
{
	dgVehicleConstraint::WheelsSet::dgListNode *node;
	node = (dgVehicleConstraint::WheelsSet::dgListNode *) index;

	return node->GetInfo().m_steerAngle;
}


inline dgFloat32 dgVehicleConstraint::GetTireOmega (void* index) const
{
	dgVehicleConstraint::WheelsSet::dgListNode *node;
	node = (dgVehicleConstraint::WheelsSet::dgListNode *) index;

	return node->GetInfo().m_parametricOmega;
}


inline dgFloat32 dgVehicleConstraint::GetTireNormalLoad (void* index) const
{
	dgFloat32 load; 
	dgVehicleConstraint::WheelsSet::dgListNode *node;
	node = (dgVehicleConstraint::WheelsSet::dgListNode *) index;
//	return node->GetInfo().m_tireNormalLoad;

//	load = node->GetInfo().m_tireNormalLoad___;
	load = node->GetInfo().m_tireNormalLoad % m_body0->m_matrix.RotateVector(m_upPin);
//_DG_ASSERTE (dgAbsf(load - node->GetInfo().m_tireNormalLoad_ % m_body0->m_matrix.RotateVector(m_upPin)) < 0.1f);
	return load;
}


inline void dgVehicleConstraint::SetTireTorque (void* index, dgFloat32 torque)
{
	dgVehicleConstraint::WheelsSet::dgListNode *node;
	node = (dgVehicleConstraint::WheelsSet::dgListNode *) index;

	node->GetInfo().m_tireTorque = torque;
}



inline void dgVehicleConstraint::SetTireMaxSideSleepSpeed (void* index, dgFloat32 speed)
{
	dgVehicleConstraint::WheelsSet::dgListNode *node;
	node = (dgVehicleConstraint::WheelsSet::dgListNode *) index;
	node->GetInfo().m_maxSideSpeed = GetMax (dgAbsf (speed), dgFloat32(MIN_SIDE_SLIP_SPEED));
}


inline void dgVehicleConstraint::SetTireMaxLongitudinalSlideSpeed (void* index, dgFloat32 speed)
{
	dgVehicleConstraint::WheelsSet::dgListNode *node;
	node = (dgVehicleConstraint::WheelsSet::dgListNode *) index;
	node->GetInfo().m_maxSlideSpeed = GetMax (dgAbsf (speed), dgFloat32(MIN_SIDE_SLIDE_SPEED));
}


inline void dgVehicleConstraint::SetTireSideSleepCoeficient (void* index, dgFloat32 coeficient)
{
	dgVehicleConstraint::WheelsSet::dgListNode *node;
	node = (dgVehicleConstraint::WheelsSet::dgListNode *) index;
	node->GetInfo().m_sideSlipCoef = 0.25f * ClampValue (dgAbsf(coeficient), dgFloat32(0.0f), dgFloat32(1.0f));
}

inline void dgVehicleConstraint::SetTireLongitudinalSlideCoeficient (void* index, dgFloat32 coeficient)
{
	dgVehicleConstraint::WheelsSet::dgListNode *node;
	node = (dgVehicleConstraint::WheelsSet::dgListNode *) index;
	node->GetInfo().m_longiSlideCoef = 0.25f * ClampValue (dgAbsf(coeficient), dgFloat32(0.0f), dgFloat32(1.0f));
}


inline void dgVehicleConstraint::TireSetBrakeAcceleration (void* index, dgFloat32 acceleration, dgFloat32 torqueMaxFriction) 
{
	dgVehicleConstraint::WheelsSet::dgListNode *node;

	node = (dgVehicleConstraint::WheelsSet::dgListNode *) index;
	dgTire& tire = node->GetInfo();
	tire.m_brakeAcceleration = acceleration;
	tire.m_brakeFrictionTorque = dgAbsf (torqueMaxFriction);
}

inline dgFloat32 dgVehicleConstraint::TireCalculateMaxBrakeAcceleration (void* index) const
{
	dgVehicleConstraint::WheelsSet::dgListNode *node;
	node = (dgVehicleConstraint::WheelsSet::dgListNode *) index;

	return (node->GetInfo().m_parametricOmega * m_param->m_invTimestep * 0.99f);
}

inline dgFloat32 dgVehicleConstraint::GetTireLateralSpeed (void* index) const
{
	dgVehicleConstraint::WheelsSet::dgListNode *node;
	node = (dgVehicleConstraint::WheelsSet::dgListNode *) index;

	return -node->GetInfo().m_lateralSpeed;
}

inline dgFloat32 dgVehicleConstraint::GetTireLongitudinalSpeed (void* index) const
{
	dgVehicleConstraint::WheelsSet::dgListNode *node;
	node = (dgVehicleConstraint::WheelsSet::dgListNode *) index;

	return -node->GetInfo().m_longitudinalSpeed;
}

inline bool dgVehicleConstraint::IsTireAirBorned (void* index) const
{
	dgVehicleConstraint::WheelsSet::dgListNode *node;
	node = (dgVehicleConstraint::WheelsSet::dgListNode *) index;

	return node->GetInfo().m_isAirborne;
}


inline bool dgVehicleConstraint::TireLostSideGrip (void* index) const
{
	dgVehicleConstraint::WheelsSet::dgListNode *node;
	node = (dgVehicleConstraint::WheelsSet::dgListNode *) index;
	return node->GetInfo().m_lostSideGrip;
}


inline bool dgVehicleConstraint::TireLostTraction (void* index) const
{
	dgVehicleConstraint::WheelsSet::dgListNode *node;
	node = (dgVehicleConstraint::WheelsSet::dgListNode *) index;
	return node->GetInfo().m_lostTraction;
}



#endif 




