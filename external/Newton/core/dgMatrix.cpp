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
#include "dgMatrix.h"
#include "dgQuaternion.h"


static dgMatrix zeroMatrix (dgVector (dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f)),
							dgVector (dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f)),
							dgVector (dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f)),
							dgVector (dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f)));

static dgMatrix identityMatrix (dgVector (dgFloat32(1.0f), dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f)),
								dgVector (dgFloat32(0.0f), dgFloat32(1.0f), dgFloat32(0.0f), dgFloat32(0.0f)),
								dgVector (dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(1.0f), dgFloat32(0.0f)),
								dgVector (dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(1.0f)));

const dgMatrix& dgGetIdentityMatrix()
{
	return identityMatrix;
}

const dgMatrix& dgGetZeroMatrix ()
{
	return zeroMatrix;
}


dgMatrix::dgMatrix (
	const dgQuaternion &rotation, 
	const dgVector &position)
{
	dgFloat32 x2;
	dgFloat32 y2;
	dgFloat32 z2;
	dgFloat32 w2;
	dgFloat32 xy;
	dgFloat32 xz;
	dgFloat32 xw;
	dgFloat32 yz;
	dgFloat32 yw;
	dgFloat32 zw;

	x2 = dgFloat32 (2.0f) * rotation.m_q1 * rotation.m_q1;
	y2 = dgFloat32 (2.0f) * rotation.m_q2 * rotation.m_q2;
	z2 = dgFloat32 (2.0f) * rotation.m_q3 * rotation.m_q3;
	w2 = dgFloat32 (2.0f) * rotation.m_q0 * rotation.m_q0;
	_DG_ASSERTE (dgAbsf (w2 + x2 + y2 + z2 - dgFloat32(2.0f)) <dgFloat32 (1.0e-3f));

	xy = dgFloat32 (2.0f) * rotation.m_q1 * rotation.m_q2;
	xz = dgFloat32 (2.0f) * rotation.m_q1 * rotation.m_q3;
	xw = dgFloat32 (2.0f) * rotation.m_q1 * rotation.m_q0;
	yz = dgFloat32 (2.0f) * rotation.m_q2 * rotation.m_q3;
	yw = dgFloat32 (2.0f) * rotation.m_q2 * rotation.m_q0;
	zw = dgFloat32 (2.0f) * rotation.m_q3 * rotation.m_q0;

	m_front = dgVector (dgFloat32(1.0f) - y2 - z2, xy + zw,                   xz - yw				    , dgFloat32(0.0f));
	m_up    = dgVector (xy - zw,                   dgFloat32(1.0f) - x2 - z2, yz + xw					, dgFloat32(0.0f));
	m_right = dgVector (xz + yw,                   yz - xw,                   dgFloat32(1.0f) - x2 - y2 , dgFloat32(0.0f));


	m_posit.m_x = position.m_x;
	m_posit.m_y = position.m_y;
	m_posit.m_z = position.m_z;
	m_posit.m_w = dgFloat32(1.0f);
}

dgMatrix dgMatrix::operator* (const dgMatrix &B) const
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



void dgMatrix::TransformTriplex (
	void* const dstPtr, 
	dgInt32 dstStrideInBytes,
	const void* const srcPtr, 
	dgInt32 srcStrideInBytes, 
	dgInt32 count) const
{
   dgInt32 i;
   dgFloat32 x;
   dgFloat32 y;
   dgFloat32 z;
	dgFloat32* dst;
	dgFloat32* src;

	dst = (dgFloat32*) dstPtr;
	src = (dgFloat32*) srcPtr;

	dstStrideInBytes /= sizeof (dgFloat32);
	srcStrideInBytes /= sizeof (dgFloat32);

	for (i = 0 ; i < count; i ++ ) {
		x = src[0];
		y = src[1];
		z = src[2];
		dst[0] = x * m_front.m_x + y * m_up.m_x + z * m_right.m_x + m_posit.m_x;
		dst[1] = x * m_front.m_y + y * m_up.m_y + z * m_right.m_y + m_posit.m_y;
		dst[2] = x * m_front.m_z + y * m_up.m_z + z * m_right.m_z + m_posit.m_z;
		dst += dstStrideInBytes;
		src += srcStrideInBytes;
	}
}


void dgMatrix::TransformBBox (
	const dgVector& p0local, 
	const dgVector& p1local, 
	dgVector& p0, 
	dgVector& p1) const
{
	dgInt32 i;
	dgVector box[8];

	box[0][0] = p0local[0];
	box[0][1] = p0local[1];
	box[0][2] = p0local[2];
	box[0][3] = dgFloat32(1.0f);

	box[1][0] = p0local[0];
	box[1][1] = p0local[1];
	box[1][2] = p1local[2];
	box[1][3] = dgFloat32(1.0f);

	box[2][0] = p0local[0];
	box[2][1] = p1local[1];
	box[2][2] = p0local[2];
	box[2][3] = dgFloat32(1.0f);

	box[3][0] = p0local[0];
	box[3][1] = p1local[1];
	box[3][2] = p1local[2];
	box[3][3] = dgFloat32(1.0f);

	box[4][0] = p1local[0];
	box[4][1] = p0local[1];
	box[4][2] = p0local[2];
	box[4][3] = dgFloat32(1.0f);

	box[5][0] = p1local[0];
	box[5][1] = p0local[1];
	box[5][2] = p1local[2];
	box[1][3] = dgFloat32(1.0f);

	box[6][0] = p1local[0];
	box[6][1] = p1local[1];
	box[6][2] = p0local[2];
	box[6][3] = dgFloat32(1.0f);

	box[7][0] = p1local[0];
	box[7][1] = p1local[1];
	box[7][2] = p1local[2];
	box[7][3] = dgFloat32(1.0f);

	TransformTriplex (box, sizeof (dgVector), box, sizeof (dgVector), 8);

	p0 = box[0];
	p1 = box[0];
	for (i = 1; i < 8; i ++) {
		p0.m_x = GetMin (p0.m_x, box[i].m_x);
		p0.m_y = GetMin (p0.m_y, box[i].m_y);
		p0.m_z = GetMin (p0.m_z, box[i].m_z);

		p1.m_x = GetMax (p1.m_x, box[i].m_x);
		p1.m_y = GetMax (p1.m_y, box[i].m_y);
		p1.m_z = GetMax (p1.m_z, box[i].m_z);
	}
}



dgMatrix dgMatrix::Symetric3by3Inverse () const
{
	dgFloat32 x11;
	dgFloat32 x12;
	dgFloat32 x13;
	dgFloat32 x22;
	dgFloat32 x23;
	dgFloat32 x33;
	dgFloat64 det;

	const dgMatrix& mat = *this;
	det = mat[0][0] * mat[1][1] * mat[2][2] + 
			mat[0][1] * mat[1][2] * mat[0][2] * dgFloat32 (2.0f) -
			mat[0][2] * mat[1][1] * mat[0][2] -
			mat[0][1] * mat[0][1] * mat[2][2] -
			mat[0][0] * mat[1][2] * mat[1][2];

	det = dgFloat32 (1.0f) / det;

	x11 = (dgFloat32)(det * (mat[1][1] * mat[2][2] - mat[1][2] * mat[1][2]));  
	x22 = (dgFloat32)(det * (mat[0][0] * mat[2][2] - mat[0][2] * mat[0][2]));  
	x33 = (dgFloat32)(det * (mat[0][0] * mat[1][1] - mat[0][1] * mat[0][1]));  

	x12 = (dgFloat32)(det * (mat[1][2] * mat[2][0] - mat[1][0] * mat[2][2]));  
	x13 = (dgFloat32)(det * (mat[1][0] * mat[2][1] - mat[1][1] * mat[2][0]));  
	x23 = (dgFloat32)(det * (mat[0][1] * mat[2][0] - mat[0][0] * mat[2][1]));  


#ifdef _DG_DEBUG
	dgMatrix matInv (dgVector (x11, x12, x13, dgFloat32(0.0f)),
				     dgVector (x12, x22, x23, dgFloat32(0.0f)),
					 dgVector (x13, x23, x33, dgFloat32(0.0f)),
					 dgVector (dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(1.0f)));

	dgMatrix test (matInv * mat);
	_DG_ASSERTE (dgAbsf (test[0][0] - dgFloat32(1.0f)) < dgFloat32(0.01f));
	_DG_ASSERTE (dgAbsf (test[1][1] - dgFloat32(1.0f)) < dgFloat32(0.01f));
	_DG_ASSERTE (dgAbsf (test[2][2] - dgFloat32(1.0f)) < dgFloat32(0.01f));
#endif

	return dgMatrix (dgVector (x11, x12, x13, dgFloat32(0.0f)),
					 dgVector (x12, x22, x23, dgFloat32(0.0f)),
					 dgVector (x13, x23, x33, dgFloat32(0.0f)),
					 dgVector (dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(0.0f), dgFloat32(1.0f)));
}


static inline void ROT(
	dgMatrix &a,
	dgInt32 i,
	dgInt32 j,
	dgInt32 k,
	dgInt32 l, 
	dgFloat32 s,
	dgFloat32 tau) 
{
	dgFloat32 g;
	dgFloat32 h;
	g = a[i][j]; 
	h = a[k][l]; 
	a[i][j] = g - s * (h + g * tau); 
	a[k][l] = h + s * (g - h * tau);
}

// from numerical recipes in c
// Jacobian method for computing the eigenvectors of a symmetric matrix
void dgMatrix::EigenVectors (dgVector &eigenValues)
{
	dgInt32 i;
	dgInt32 nrot;
	dgFloat32 t;
	dgFloat32 s;
	dgFloat32 h;
	dgFloat32 g;
	dgFloat32 c;
	dgFloat32 sm;
	dgFloat32 tau;
	dgFloat32 theta;
	dgFloat32 thresh;
	dgFloat32 b[3];
	dgFloat32 z[3];
	dgFloat32 d[3];

	dgMatrix eigenVectors (dgGetIdentityMatrix());
	dgMatrix &mat = *this;

	b[0] = mat[0][0]; 
	b[1] = mat[1][1];
	b[2] = mat[2][2];

	d[0] = mat[0][0]; 
	d[1] = mat[1][1]; 
	d[2] = mat[2][2]; 

	z[0] = dgFloat32 (0.0f);
	z[1] = dgFloat32 (0.0f);
	z[2] = dgFloat32 (0.0f);

	nrot = 0;
	for (i = 0; i < 50; i++) {
		sm = dgAbsf(mat[0][1]) + dgAbsf(mat[0][2]) + dgAbsf(mat[1][2]);

		if (sm < dgEPSILON * dgFloat32(1.0e-5f)) {
			_DG_ASSERTE (dgAbsf((eigenVectors.m_front % eigenVectors.m_front) - dgFloat32(1.0f)) < dgEPSILON);
			_DG_ASSERTE (dgAbsf((eigenVectors.m_up % eigenVectors.m_up) - dgFloat32(1.0f)) < dgEPSILON);
			_DG_ASSERTE (dgAbsf((eigenVectors.m_right % eigenVectors.m_right) - dgFloat32(1.0f)) < dgEPSILON);

			// order the eigenvalue vectors	
			dgVector tmp (eigenVectors.m_front * eigenVectors.m_up);
			if (tmp % eigenVectors.m_right < dgFloat32(0.0f)) {
				eigenVectors.m_right = eigenVectors.m_right.Scale (-dgFloat32(1.0f));
			}

			eigenValues = dgVector (d[0], d[1], d[2], dgFloat32 (0.0f));
			*this = eigenVectors.Inverse();
			return;
		}

		if (i < 3) {
			thresh = (dgFloat32)(0.2f / 9.0f) * sm;
		}	else {
			thresh = dgFloat32 (0.0f);
		}


		// First row
		g = dgFloat32 (100.0f) * dgAbsf(mat[0][1]);
		if ((i > 3) && (dgAbsf(d[0]) + g == dgAbsf(d[0])) && (dgAbsf(d[1]) + g == dgAbsf(d[1]))) {
			mat[0][1] = dgFloat32 (0.0f);
		} else if (dgAbsf(mat[0][1]) > thresh) {
			h = d[1] - d[0];
			if (dgAbsf(h) + g == dgAbsf(h)) {
				t = mat[0][1] / h;
			} else {
				theta = dgFloat32 (0.5f) * h / mat[0][1];
				t = dgFloat32(1.0f) / (dgAbsf(theta) + dgSqrt(dgFloat32(1.0f) + theta * theta));
				if (theta < dgFloat32 (0.0f)) {
					t = -t;
				}
			}
			c = dgFloat32(1.0f) / dgSqrt (dgFloat32 (1.0f) + t * t); 
			s = t * c; 
			tau = s / (dgFloat32(1.0f) + c); 
			h = t * mat[0][1];
			z[0] -= h; 
			z[1] += h; 
			d[0] -= h; 
			d[1] += h;
			mat[0][1] = dgFloat32(0.0f);
			ROT (mat, 0, 2, 1, 2, s, tau); 
			ROT (eigenVectors, 0, 0, 0, 1, s, tau); 
			ROT (eigenVectors, 1, 0, 1, 1, s, tau); 
			ROT (eigenVectors, 2, 0, 2, 1, s, tau); 

			nrot++;
		}


		// second row
		g = dgFloat32 (100.0f) * dgAbsf(mat[0][2]);
		if ((i > 3) && (dgAbsf(d[0]) + g == dgAbsf(d[0])) && (dgAbsf(d[2]) + g == dgAbsf(d[2]))) {
			mat[0][2] = dgFloat32 (0.0f);
		} else if (dgAbsf(mat[0][2]) > thresh) {
			h = d[2] - d[0];
			if (dgAbsf(h) + g == dgAbsf(h)) {
				t = (mat[0][2]) / h;
			}	else {
				theta = dgFloat32 (0.5f) * h / mat[0][2];
				t = dgFloat32(1.0f) / (dgAbsf(theta) + dgSqrt(dgFloat32(1.0f) + theta * theta));
				if (theta < dgFloat32 (0.0f)) {
					t = -t;
				}
			}
			c = dgFloat32(1.0f) / dgSqrt(dgFloat32 (1.0f) + t * t); 
			s = t * c; 
			tau = s / (dgFloat32(1.0f) + c); 
			h = t * mat[0][2];
			z[0] -= h; 
			z[2] += h; 
			d[0] -= h; 
			d[2] += h;
			mat[0][2]=dgFloat32 (0.0f);
			ROT (mat, 0, 1, 1, 2, s, tau); 
			ROT (eigenVectors, 0, 0, 0, 2, s, tau); 
			ROT (eigenVectors, 1, 0, 1, 2, s, tau); 
			ROT (eigenVectors, 2, 0, 2, 2, s, tau); 
		}

		// third row
		g = dgFloat32 (100.0f) * dgAbsf(mat[1][2]);
		if ((i > 3) && (dgAbsf(d[1]) + g == dgAbsf(d[1])) && (dgAbsf(d[2]) + g == dgAbsf(d[2]))) {
			mat[1][2] = dgFloat32 (0.0f);
		} else if (dgAbsf(mat[1][2]) > thresh) {
			h = d[2] - d[1];
			if (dgAbsf(h) + g == dgAbsf(h)) {
				t = mat[1][2] / h;
			}	else {
				theta = dgFloat32 (0.5f) * h / mat[1][2];
				t = dgFloat32(1.0f) / (dgAbsf(theta) + dgSqrt(dgFloat32(1.0f) + theta * theta));
				if (theta < dgFloat32 (0.0f)) {
					t = -t;
				}
			}
			c = dgFloat32(1.0f) / dgSqrt(dgFloat32 (1.0f) + t*t); 
			s = t * c; 
			tau = s / (dgFloat32(1.0f) + c); 

			h = t * mat[1][2];
			z[1] -= h; 
			z[2] += h; 
			d[1] -= h; 
			d[2] += h;
			mat[1][2] = dgFloat32 (0.0f);
			ROT (mat, 0, 1, 0, 2, s, tau); 
			ROT (eigenVectors, 0, 1, 0, 2, s, tau); 
			ROT (eigenVectors, 1, 1, 1, 2, s, tau); 
			ROT (eigenVectors, 2, 1, 2, 2, s, tau); 
			nrot++;
		}

		b[0] += z[0]; d[0] = b[0]; z[0] = dgFloat32 (0.0f);
		b[1] += z[1]; d[1] = b[1]; z[1] = dgFloat32 (0.0f);
		b[2] += z[2]; d[2] = b[2]; z[2] = dgFloat32 (0.0f);
	}

	eigenValues = dgVector (d[0], d[1], d[2], dgFloat32 (0.0f));
	*this = dgGetIdentityMatrix();
} 	






dgVector dgMatrix::CalcPitchYawRoll () const
{
	dgFloat32 yaw;
	dgFloat32 roll;
	dgFloat32 pitch;
	const dgFloat32 minSin = dgFloat32(0.99995f);

	const dgMatrix& matrix = *this;

	roll = dgFloat32(0.0f);
	pitch  = dgFloat32(0.0f);
	yaw = dgAsin (-ClampValue (matrix[0][2], dgFloat32(-0.999999f), dgFloat32(0.999999f)));

	_DG_ASSERTE (dgCheckFloat (yaw));
	if (matrix[0][2] < minSin) {
		if (matrix[0][2] > (-minSin)) {
			roll = dgAtan2 (matrix[0][1], matrix[0][0]);
			pitch = dgAtan2 (matrix[1][2], matrix[2][2]);
		} else {
			pitch = dgAtan2 (matrix[1][0], matrix[1][1]);
		}
	} else {
		pitch = -dgAtan2 (matrix[1][0], matrix[1][1]);
	}

#ifdef _DG_DEBUG
	dgMatrix m (dgPitchMatrix (pitch) * dgYawMatrix(yaw) * dgRollMatrix(roll));
	for (dgInt32 i = 0; i < 3; i ++) {
		for (dgInt32 j = 0; j < 3; j ++) {
			dgFloat32 error = dgAbsf (m[i][j] - matrix[i][j]);
			_DG_ASSERTE (error < 5.0e-2f);
		}
	}
#endif

	return dgVector (pitch, yaw, roll, dgFloat32(0.0f));
}


