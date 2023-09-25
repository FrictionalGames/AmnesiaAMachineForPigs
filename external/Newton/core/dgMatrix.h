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

#ifndef __dgMatrix__
#define __dgMatrix__


#include "dgStdafx.h"
#include "dgDebug.h"
#include "dgVector.h"
#include "dgPlane.h"
#include "dgSimd_Instrutions.h"
#include <math.h>

class dgQuaternion;

DG_MSC_VECTOR_ALIGMENT
class dgMatrix
{
	public:
	DG_CLASS_ALLOCATOR(allocator)

	dgMatrix ();
	dgMatrix (const dgVector &front, const dgVector &up, const dgVector &right, const dgVector &posit);
	dgMatrix (const dgQuaternion &rotation, const dgVector &position);

	// create a orthonormal normal vector basis
	dgMatrix (const dgVector &front);


	

	dgVector& operator[] (dgInt32 i);
	const dgVector& operator[] (dgInt32 i) const;

	dgMatrix Inverse () const;
	dgMatrix Transpose () const;
	dgMatrix Transpose4X4 () const;
	dgMatrix Symetric3by3Inverse () const;
	dgVector RotateVector (const dgVector &v) const;
	dgVector UnrotateVector (const dgVector &v) const;
	dgVector TransformVector (const dgVector &v) const;
	dgVector UntransformVector (const dgVector &v) const;
	dgPlane TransformPlane (const dgPlane &localPlane) const;
	dgPlane UntransformPlane (const dgPlane &globalPlane) const;
	void TransformBBox (const dgVector& p0local, const dgVector& p1local, dgVector& p0, dgVector& p1) const; 

	dgVector CalcPitchYawRoll () const;
	void TransformTriplex (void* const dst, dgInt32 dstStrideInBytes,
						   const void* const src, dgInt32 srcStrideInBytes, dgInt32 count) const;

	dgMatrix operator* (const dgMatrix &B) const;
	

	// this function can not be a member of dgMatrix, because
	// dgMatrix a define to handle only orthogonal matrices
	// and this function take a parameter to a symmetric matrix
	void EigenVectors (dgVector &eigenValues);
	void EigenVectors ();


	// simd operations
	dgMatrix InverseSimd () const;
	dgMatrix MultiplySimd (const dgMatrix& B) const;
	dgVector RotateVectorSimd (const dgVector &v) const;
	dgVector UnrotateVectorSimd (const dgVector &v) const;
	dgVector TransformVectorSimd (const dgVector &v) const;
	void TransformVectorsSimd (dgVector* const dst, const dgVector* const src, dgInt32 count) const;

	dgVector m_front;
	dgVector m_up;
	dgVector m_right;
	dgVector m_posit;
}DG_GCC_VECTOR_ALIGMENT;


const dgMatrix& dgGetZeroMatrix ();
const dgMatrix& dgGetIdentityMatrix();



DG_INLINE dgMatrix::dgMatrix ()
{
}

DG_INLINE dgMatrix::dgMatrix (
	const dgVector &front, 
	const dgVector &up,
	const dgVector &right,
	const dgVector &posit)
	:m_front (front), m_up(up), m_right(right), m_posit(posit)
{
}

DG_INLINE dgMatrix::dgMatrix (const dgVector& front)
{
	m_front = front; 
	if (dgAbsf (front.m_z) > dgFloat32 (0.577f)) {
		m_right = front * dgVector (-front.m_y, front.m_z, dgFloat32(0.0f), dgFloat32(0.0f));
	} else {
	  	m_right = front * dgVector (-front.m_y, front.m_x, dgFloat32(0.0f), dgFloat32(0.0f));
	}
  	m_right = m_right.Scale (dgRsqrt (m_right % m_right));
  	m_up = m_right * m_front;

	m_front.m_w = dgFloat32(0.0f);
	m_up.m_w = dgFloat32(0.0f);
	m_right.m_w = dgFloat32(0.0f);
	m_posit = dgVector (dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(1.0f));

	_DG_ASSERTE ((dgAbsf (m_front % m_front) - dgFloat32(1.0f)) < dgFloat32(1.0e-5f)); 
	_DG_ASSERTE ((dgAbsf (m_up % m_up) - dgFloat32(1.0f)) < dgFloat32(1.0e-5f)); 
	_DG_ASSERTE ((dgAbsf (m_right % m_right) - dgFloat32(1.0f)) < dgFloat32(1.0e-5f)); 
	_DG_ASSERTE ((dgAbsf (m_right % (m_front * m_up)) - dgFloat32(1.0f)) < dgFloat32(1.0e-5f)); 
}


DG_INLINE dgVector& dgMatrix::operator[] (dgInt32  i)
{
	_DG_ASSERTE (i < 4);
	_DG_ASSERTE (i >= 0);
	return (&m_front)[i];
}

DG_INLINE const dgVector& dgMatrix::operator[] (dgInt32  i) const
{
	_DG_ASSERTE (i < 4);
	_DG_ASSERTE (i >= 0);
	return (&m_front)[i];
}


DG_INLINE dgMatrix dgMatrix::Transpose () const
{
	return dgMatrix (dgVector (m_front.m_x, m_up.m_x, m_right.m_x, dgFloat32(0.0f)),
					 dgVector (m_front.m_y, m_up.m_y, m_right.m_y, dgFloat32(0.0f)),
					 dgVector (m_front.m_z, m_up.m_z, m_right.m_z, dgFloat32(0.0f)),
					 dgVector (dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(1.0f)));
}

DG_INLINE dgMatrix dgMatrix::Transpose4X4 () const
{
	return dgMatrix (dgVector (m_front.m_x, m_up.m_x, m_right.m_x, m_posit.m_x),
					 dgVector (m_front.m_y, m_up.m_y, m_right.m_y, m_posit.m_y),
					 dgVector (m_front.m_z, m_up.m_z, m_right.m_z, m_posit.m_z),
					 dgVector (m_front.m_w, m_up.m_w, m_right.m_w, m_posit.m_w));
							
}

DG_INLINE dgVector dgMatrix::RotateVector (const dgVector &v) const
{
	return dgVector (v.m_x * m_front.m_x + v.m_y * m_up.m_x + v.m_z * m_right.m_x,
					 v.m_x * m_front.m_y + v.m_y * m_up.m_y + v.m_z * m_right.m_y,
					 v.m_x * m_front.m_z + v.m_y * m_up.m_z + v.m_z * m_right.m_z, v.m_w);
}


DG_INLINE dgVector dgMatrix::UnrotateVector (const dgVector &v) const
{
	return dgVector (v % m_front, v % m_up, v % m_right, v.m_w);
}


DG_INLINE dgVector dgMatrix::TransformVector (const dgVector &v) const
{
//	return m_posit + RotateVector(v);
	return dgVector (v.m_x * m_front.m_x + v.m_y * m_up.m_x + v.m_z * m_right.m_x + m_posit.m_x,
					 v.m_x * m_front.m_y + v.m_y * m_up.m_y + v.m_z * m_right.m_y + m_posit.m_y,
					 v.m_x * m_front.m_z + v.m_y * m_up.m_z + v.m_z * m_right.m_z + m_posit.m_z, v.m_w);

}

DG_INLINE dgVector dgMatrix::UntransformVector (const dgVector &v) const
{
	return UnrotateVector(v - m_posit);
}

DG_INLINE dgPlane dgMatrix::TransformPlane (const dgPlane &localPlane) const
{
	return dgPlane (RotateVector (localPlane), localPlane.m_w - (localPlane % UnrotateVector (m_posit)));  
}

DG_INLINE dgPlane dgMatrix::UntransformPlane (const dgPlane &globalPlane) const
{
	return dgPlane (UnrotateVector (globalPlane), globalPlane.Evalue(m_posit));
}

DG_INLINE void dgMatrix::EigenVectors ()
{
	dgVector eigenValues;
	EigenVectors (eigenValues);
}


/*
DG_INLINE dgMatrix dgMatrix::operator* (const dgMatrix &B) const
{
	const dgMatrix& A = *this;
	return dgMatrix (dgVector (A[0][0] * B[0][0] + A[0][1] * B[1][0] + A[0][2] * B[2][0] + A[0][3] * B[3][0],
							   A[0][0] * B[0][1] + A[0][1] * B[1][1] + A[0][2] * B[2][1] + A[0][3] * B[3][1],
							   A[0][0] * B[0][2] + A[0][1] * B[1][2] + A[0][2] * B[2][2] + A[0][3] * B[3][2],
	                           A[0][0] * B[0][3] + A[0][1] * B[1][3] + A[0][2] * B[2][3] + A[0][3] * B[3][3]),
					 dgVector (A[1][0] * B[0][0] + A[1][1] * B[1][0] + A[1][2] * B[2][0] + A[1][3] * B[3][0],
							   A[1][0] * B[0][1] + A[1][1] * B[1][1] + A[1][2] * B[2][1] + A[1][3] * B[3][1],
							   A[1][0] * B[0][2] + A[1][1] * B[1][2] + A[1][2] * B[2][2] + A[1][3] * B[3][2],
							   A[1][0] * B[0][3] + A[1][1] * B[1][3] + A[1][2] * B[2][3] + A[1][3] * B[3][3]),
					 dgVector (A[2][0] * B[0][0] + A[2][1] * B[1][0] + A[2][2] * B[2][0] + A[2][3] * B[3][0],
							   A[2][0] * B[0][1] + A[2][1] * B[1][1] + A[2][2] * B[2][1] + A[2][3] * B[3][1],
							   A[2][0] * B[0][2] + A[2][1] * B[1][2] + A[2][2] * B[2][2] + A[2][3] * B[3][2],
							   A[2][0] * B[0][3] + A[2][1] * B[1][3] + A[2][2] * B[2][3] + A[2][3] * B[3][3]),
					 dgVector (A[3][0] * B[0][0] + A[3][1] * B[1][0] + A[3][2] * B[2][0] + A[3][3] * B[3][0],
							   A[3][0] * B[0][1] + A[3][1] * B[1][1] + A[3][2] * B[2][1] + A[3][3] * B[3][1],
							   A[3][0] * B[0][2] + A[3][1] * B[1][2] + A[3][2] * B[2][2] + A[3][3] * B[3][2],
							   A[3][0] * B[0][3] + A[3][1] * B[1][3] + A[3][2] * B[2][3] + A[3][3] * B[3][3]));

	
}
*/


DG_INLINE dgMatrix dgPitchMatrix(dgFloat32 ang)
{
	dgFloat32 cosAng;
	dgFloat32 sinAng;
	sinAng = dgSin (ang);
	cosAng = dgCos (ang);
	return dgMatrix (dgVector (dgFloat32(1.0f),  dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f)), 
					 dgVector (dgFloat32(0.0f),  cosAng,          sinAng,          dgFloat32(0.0f)),
					 dgVector (dgFloat32(0.0f), -sinAng,          cosAng,          dgFloat32(0.0f)), 
					 dgVector (dgFloat32(0.0f),  dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(1.0f))); 

}

DG_INLINE dgMatrix dgYawMatrix(dgFloat32 ang)
{
	dgFloat32 cosAng;
	dgFloat32 sinAng;
	sinAng = dgSin (ang);
	cosAng = dgCos (ang);
	return dgMatrix (dgVector (cosAng,          dgFloat32(0.0f), -sinAng,          dgFloat32(0.0f)), 
					 dgVector (dgFloat32(0.0f), dgFloat32(1.0f),  dgFloat32(0.0f), dgFloat32(0.0f)), 
					 dgVector (sinAng,          dgFloat32(0.0f),  cosAng,          dgFloat32(0.0f)), 
					 dgVector (dgFloat32(0.0f), dgFloat32(0.0f),  dgFloat32(0.0f), dgFloat32(1.0f))); 
}

DG_INLINE dgMatrix dgRollMatrix(dgFloat32 ang)
{
	dgFloat32 cosAng;
	dgFloat32 sinAng;
	sinAng = dgSin (ang);
	cosAng = dgCos (ang);
	return dgMatrix (dgVector ( cosAng,          sinAng,          dgFloat32(0.0f), dgFloat32(0.0f)), 
					 dgVector (-sinAng,          cosAng,          dgFloat32(0.0f), dgFloat32(0.0f)),
					 dgVector ( dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(1.0f), dgFloat32(0.0f)), 
					 dgVector ( dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(1.0f))); 
}																		 


DG_INLINE dgMatrix dgMatrix::Inverse () const
{
	return dgMatrix (dgVector (m_front.m_x, m_up.m_x, m_right.m_x, dgFloat32(0.0f)),
					 dgVector (m_front.m_y, m_up.m_y, m_right.m_y, dgFloat32(0.0f)),
					 dgVector (m_front.m_z, m_up.m_z, m_right.m_z, dgFloat32(0.0f)),
					 dgVector (- (m_posit % m_front), - (m_posit % m_up), - (m_posit % m_right), dgFloat32(1.0f)));
}




DG_INLINE dgVector dgMatrix::TransformVectorSimd (const dgVector &v) const
{
#ifdef DG_BUILD_SIMD_CODE
	const dgMatrix& source = *this;
	return dgVector (simd_mul_add_v (
						simd_mul_add_v (
							simd_mul_add_v ((simd_type&) source[3], (simd_type&) source[0], simd_permut_v ((simd_type&) v, (simd_type&) v, PURMUT_MASK(0, 0, 0, 0))), 
																	(simd_type&) source[1], simd_permut_v ((simd_type&) v, (simd_type&) v, PURMUT_MASK(1, 1, 1, 1))), 
																	(simd_type&) source[2], simd_permut_v ((simd_type&) v, (simd_type&) v, PURMUT_MASK(2, 2, 2, 2))));
#else
	return dgVector (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));
#endif
}

DG_INLINE void dgMatrix::TransformVectorsSimd (dgVector* const dst, const dgVector* const src, dgInt32 count) const
{
#ifdef DG_BUILD_SIMD_CODE
	const dgMatrix& source = *this;
	for (dgInt32 i = 0; i < count; i ++) {
		(simd_type&)dst[i] = simd_mul_add_v (
								simd_mul_add_v (
									simd_mul_add_v ((simd_type&) source[3], 
													(simd_type&) source[0], simd_permut_v ((simd_type&) src[i], (simd_type&) src[i], PURMUT_MASK(0, 0, 0, 0))), 
													(simd_type&) source[1], simd_permut_v ((simd_type&) src[i], (simd_type&) src[i], PURMUT_MASK(1, 1, 1, 1))), 
													(simd_type&) source[2], simd_permut_v ((simd_type&) src[i], (simd_type&) src[i], PURMUT_MASK(2, 2, 2, 2)));
	}
#endif
}


DG_INLINE dgVector dgMatrix::RotateVectorSimd (const dgVector &v) const
{
#ifdef DG_BUILD_SIMD_CODE
	const dgMatrix& source = *this;
	return dgVector (simd_mul_add_v (
						simd_mul_add_v (
							simd_mul_v ((simd_type&) source[0], simd_permut_v ((simd_type&) v, (simd_type&) v, PURMUT_MASK(0, 0, 0, 0))), 
									    (simd_type&) source[1], simd_permut_v ((simd_type&) v, (simd_type&) v, PURMUT_MASK(1, 1, 1, 1))), 
										(simd_type&) source[2], simd_permut_v ((simd_type&) v, (simd_type&) v, PURMUT_MASK(2, 2, 2, 2))));

#else
	return dgVector (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));
#endif
}

DG_INLINE dgVector dgMatrix::UnrotateVectorSimd (const dgVector &v) const
{
#ifdef DG_BUILD_SIMD_CODE
	return dgVector (v.DotProductSimd(m_front), v.DotProductSimd(m_up), v.DotProductSimd(m_right), v.m_w);
#else
	return dgVector (dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f), dgFloat32 (0.0f));
#endif
}

DG_INLINE dgMatrix dgMatrix::InverseSimd () const
{
#ifdef DG_BUILD_SIMD_CODE	
	simd_type r0;
	simd_type r1;
	simd_type r2;
	dgMatrix matrix;
	const dgMatrix& source = *this;

	_DG_ASSERTE ((dgUnsigned64(this) & 0x0f) == 0);

	r2 = simd_set1 (dgFloat32 (0.0f));
	r0 = simd_pack_lo_v ((simd_type&) source[0], (simd_type&) source[1]);
	r1 = simd_pack_lo_v ((simd_type&) source[2], r2);
	(simd_type&) matrix[0] = simd_move_lh_v (r0, r1);
	(simd_type&) matrix[1] = simd_move_hl_v (r1, r0);
	r0 = simd_pack_hi_v ((simd_type&) source[0], (simd_type&) source[1]);
	r1 = simd_pack_hi_v ((simd_type&) source[2], r2);
	(simd_type&) matrix[2] = simd_move_lh_v (r0, r1);

	(simd_type&) matrix[3] = simd_sub_v (r2, 
									simd_mul_add_v (
													simd_mul_add_v(simd_mul_v((simd_type&) matrix[0], simd_permut_v ((simd_type&) source[3], (simd_type&) source[3], PURMUT_MASK(3, 0, 0, 0))),
																			  (simd_type&) matrix[1], simd_permut_v ((simd_type&) source[3], (simd_type&) source[3], PURMUT_MASK(3, 1, 1, 1))),
																			  (simd_type&) matrix[2], simd_permut_v ((simd_type&) source[3], (simd_type&) source[3], PURMUT_MASK(3, 2, 2, 2))));
	matrix[3][3] = dgFloat32 (1.0f);
	return matrix;


#else
	return dgGetIdentityMatrix();
#endif
}

DG_INLINE dgMatrix dgMatrix::MultiplySimd (const dgMatrix& B) const
{
#ifdef DG_BUILD_SIMD_CODE	
	const dgMatrix& A = *this;
	return dgMatrix (dgVector (simd_mul_add_v( 
								simd_mul_add_v(
									simd_mul_add_v(simd_mul_v ((simd_type&) B[0], simd_permut_v ((simd_type&) A[0], (simd_type&) A[0], PURMUT_MASK(0, 0, 0, 0))),
															   (simd_type&) B[1], simd_permut_v ((simd_type&) A[0], (simd_type&) A[0], PURMUT_MASK(1, 1, 1, 1))),
															   (simd_type&) B[2], simd_permut_v ((simd_type&) A[0], (simd_type&) A[0], PURMUT_MASK(2, 2, 2, 2))),
															   (simd_type&) B[3], simd_permut_v ((simd_type&) A[0], (simd_type&) A[0], PURMUT_MASK(3, 3, 3, 3)))),

					 dgVector (simd_mul_add_v( 
								simd_mul_add_v(
									simd_mul_add_v(simd_mul_v ((simd_type&) B[0], simd_permut_v ((simd_type&) A[1], (simd_type&) A[1], PURMUT_MASK(0, 0, 0, 0))),
															   (simd_type&) B[1], simd_permut_v ((simd_type&) A[1], (simd_type&) A[1], PURMUT_MASK(1, 1, 1, 1))),
															   (simd_type&) B[2], simd_permut_v ((simd_type&) A[1], (simd_type&) A[1], PURMUT_MASK(2, 2, 2, 2))),
															   (simd_type&) B[3], simd_permut_v ((simd_type&) A[1], (simd_type&) A[1], PURMUT_MASK(3, 3, 3, 3)))),

					dgVector (simd_mul_add_v( 
								simd_mul_add_v(
									simd_mul_add_v(simd_mul_v ((simd_type&) B[0], simd_permut_v ((simd_type&) A[2], (simd_type&) A[2], PURMUT_MASK(0, 0, 0, 0))),
															   (simd_type&) B[1], simd_permut_v ((simd_type&) A[2], (simd_type&) A[2], PURMUT_MASK(1, 1, 1, 1))),
															   (simd_type&) B[2], simd_permut_v ((simd_type&) A[2], (simd_type&) A[2], PURMUT_MASK(2, 2, 2, 2))),
															   (simd_type&) B[3], simd_permut_v ((simd_type&) A[2], (simd_type&) A[2], PURMUT_MASK(3, 3, 3, 3)))),


					dgVector (simd_mul_add_v( 
							   simd_mul_add_v(
								   simd_mul_add_v(simd_mul_v ((simd_type&) B[0], simd_permut_v ((simd_type&) A[3], (simd_type&) A[3], PURMUT_MASK(0, 0, 0, 0))),
															  (simd_type&) B[1], simd_permut_v ((simd_type&) A[3], (simd_type&) A[3], PURMUT_MASK(1, 1, 1, 1))),
															  (simd_type&) B[2], simd_permut_v ((simd_type&) A[3], (simd_type&) A[3], PURMUT_MASK(2, 2, 2, 2))),
															  (simd_type&) B[3], simd_permut_v ((simd_type&) A[3], (simd_type&) A[3], PURMUT_MASK(3, 3, 3, 3)))));
#else
	return dgGetIdentityMatrix();

#endif
}


#endif

