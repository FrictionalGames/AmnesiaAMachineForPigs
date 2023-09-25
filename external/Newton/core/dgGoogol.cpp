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




// based of the paper of: Jonathan Richard Shewchuk October 1, 1997
// "Adaptive Precision Floating-Point Arithmetic and Fast Robust Geometric Predicates"
// only using the exact arithmetic 
// more than a million time better than my previews exact arithmetic base on integers

// this class is by far faster an much more complete than n my old Googol big number base in integers

#include "dgStdafx.h"
#include "dgGoogol.h"


static dgFloat64 m_splitter = 0.0f;

dgGoogol::dgGoogol(void)
	:m_significantCount (0)
{
#ifdef _DG_DEBUG
	memset (m_elements, 0, sizeof (m_elements));
#endif
}


dgGoogol::dgGoogol(dgFloat64 value)
{
	InitFloatFloat (value);
}

dgGoogol::~dgGoogol(void)
{
}


dgFloat64 dgGoogol::GetAproximateValue() const
{
	dgFloat64 val = 0.0f;
	for (dgInt32 i = m_significantCount - 1; i >= 0; i --) {
		val += m_elements[i];
	}
	return val;
}

void dgGoogol::InitFloatFloat (dgFloat64 value)
{
	if (m_splitter == 0.0) {
		dgInt32 every_other = 1;
		dgFloat64 check = 1.0;
		dgFloat64 epsilon = 1.0;
		dgFloat64 lastcheck = 0.0f;
		m_splitter = 1.0;
		do {
			lastcheck = check;
			epsilon *= 0.5;
			if (every_other) {
				m_splitter *= 2.0;
			}
			every_other = !every_other;
			check = 1.0 + epsilon;
		} while ((check != 1.0) && (check != lastcheck));
		m_splitter += 1.0;
	}


	m_significantCount = 1;
#ifdef _DG_DEBUG
	memset (m_elements, 0, sizeof (m_elements));
#endif
	m_elements[0] = value;


}



inline void dgGoogol::AddFloat (dgFloat64 a, dgFloat64 b, dgFloat64& x, dgFloat64& y) const
{
	x = a + b; 
	dgFloat64 bvirt = x - a; 
	dgFloat64 avirt = x - bvirt; 
	dgFloat64 bround = b - bvirt; 
	dgFloat64 around = a - avirt; 
	y = around + bround;
}

inline void dgGoogol::PackFloat ()
{
	dgFloat64 elements[DG_GOOGOL_SIZE];
	dgInt32 bottom = m_significantCount - 1;
	dgFloat64 Q = m_elements[bottom];
	for (dgInt32 i = m_significantCount - 2; i >= 0; i--) {
		dgFloat64 q;
		dgFloat64 Qnew;
		dgFloat64 enow = m_elements[i];

		AddFloat (Q, enow, Qnew, q);
		if (q != 0) {
			elements[bottom--] = Qnew;
			Q = q;
		} else {
			Q = Qnew;
		}
	}

	dgInt32 top = 0;
	for (dgInt32 i = bottom + 1; i < m_significantCount; i ++) {
		dgFloat64 q;
		dgFloat64 Qnew;
		dgFloat64 hnow = elements[i];

		AddFloat (hnow, Q, Qnew, q);
		if (q != 0) {
			elements[top] = q;
			top ++;
			_DG_ASSERTE (top < DG_GOOGOL_SIZE);
		}
		Q = Qnew;
	}
	elements[top] = Q;
	m_significantCount = top + 1;
	_DG_ASSERTE (m_significantCount < DG_GOOGOL_SIZE);
	memcpy (m_elements, elements, m_significantCount * sizeof (dgFloat64));
}


inline void dgGoogol::SplitFloat (dgFloat64 a, dgFloat64& ahi, dgFloat64& alo) const
{
	dgFloat64 c = m_splitter * a;
	dgFloat64 abig = c - a; 
	ahi = c - abig; 
	alo = a - ahi;
}

inline void dgGoogol::MulFloat (dgFloat64 a, dgFloat64 b, dgFloat64& x, dgFloat64& y) const
{
	dgFloat64 ahi;
	dgFloat64 alo;
	dgFloat64 bhi;
	dgFloat64 blo;

	x = a * b;

	SplitFloat (a, ahi, alo); 
	SplitFloat (b, bhi, blo); 

	dgFloat64 err1 = x - ahi * bhi; 
	dgFloat64 err2 = err1 - (alo * bhi); 
	dgFloat64 err3 = err2 - (ahi * blo); 
	y = alo * blo - err3;
}


inline dgGoogol dgGoogol::ScaleFloat(dgFloat64 scale) const
{
	dgFloat64 Q;
	dgGoogol tmp;

	MulFloat (m_elements[0], scale, Q, tmp.m_elements[0]);

	dgInt32 hindex = 1;
	for (dgInt32 i = 1; i < m_significantCount; i++) {
		dgFloat64 sum;
		dgFloat64 product0;
		dgFloat64 product1;

		dgFloat64 enow = m_elements[i];
		MulFloat (enow, scale, product1, product0);

		AddFloat (Q, product0, sum, tmp.m_elements[hindex]);
		hindex++;
		_DG_ASSERTE (hindex < DG_GOOGOL_SIZE);

		AddFloat (product1, sum, Q, tmp.m_elements[hindex]);
		hindex++;
		_DG_ASSERTE (hindex < DG_GOOGOL_SIZE);
	}
	tmp.m_elements[hindex] = Q;
	_DG_ASSERTE (hindex < DG_GOOGOL_SIZE);
	tmp.m_significantCount = m_significantCount + m_significantCount;
	_DG_ASSERTE (tmp.m_significantCount < DG_GOOGOL_SIZE);
	return tmp;
}


dgGoogol dgGoogol::operator+ (const dgGoogol &A) const
{
	dgGoogol tmp;
	dgFloat64 q = A.m_elements[0];
	for (dgInt32 i = 0; i < m_significantCount; i++) {
		dgFloat64 Qnew;
		dgFloat64 hnow = m_elements[i];
		AddFloat (q, hnow, Qnew, tmp.m_elements[i]);
		q = Qnew;
	}
	tmp.m_elements[m_significantCount] = q;
	
	dgInt32	significantCount = m_significantCount + 1;
	for (dgInt32 i = 1; i < A.m_significantCount; i++) {
		dgFloat64 q = A.m_elements[i];
		for (dgInt32 j = 0; j < significantCount; j++) {
			dgFloat64 Qnew;
			dgFloat64 hnow = tmp.m_elements[j];
			AddFloat (q, hnow, Qnew, tmp.m_elements[j]);
			q = Qnew;
		}
		tmp.m_elements[significantCount] = q;
		significantCount ++;
		_DG_ASSERTE (significantCount < DG_GOOGOL_SIZE);
	}
	tmp.m_significantCount = significantCount;

	tmp.PackFloat ();
	return tmp;
}


dgGoogol dgGoogol::operator- (const dgGoogol &A) const
{
	dgGoogol tmp (A);
	for (dgInt32 i = 0; i < tmp.m_significantCount; i ++) {
		tmp.m_elements[i] = - tmp.m_elements[i];
	}
	return *this + tmp;
}

dgGoogol dgGoogol::operator* (const dgGoogol &A) const
{
	dgGoogol tmp (ScaleFloat(A.m_elements[0]));	
	for (dgInt32 i = 1; i < A.m_significantCount; i ++) {
		tmp = tmp + ScaleFloat(A.m_elements[i]);
	}
	tmp.PackFloat ();
	return tmp;
}
