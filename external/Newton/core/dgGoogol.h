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

#ifndef __dgGoogol__
#define __dgGoogol__


#include "dgStdafx.h"
#include "dgVector.h"
//#define DG_GOOGOL_SIZE	32
#define DG_GOOGOL_SIZE		16


class dgGoogol
{
	public:
	dgGoogol(void);
	dgGoogol(dgFloat64 value);
	~dgGoogol(void);

	dgFloat64 GetAproximateValue() const;
	void InitFloatFloat (dgFloat64 value);

	dgGoogol operator+ (const dgGoogol &A) const; 
	dgGoogol operator- (const dgGoogol &A) const; 
	dgGoogol operator* (const dgGoogol &A) const; 

	private:
	inline void PackFloat ();
	inline void AddFloat (dgFloat64 A, dgFloat64 B, dgFloat64& x, dgFloat64& y) const;
	inline void MulFloat (dgFloat64 A, dgFloat64 B, dgFloat64& x, dgFloat64& y) const;
	inline void SplitFloat (dgFloat64 A, dgFloat64& hi, dgFloat64& lo) const;
	inline dgGoogol ScaleFloat(dgFloat64 scale) const;

	dgInt32 m_significantCount;
	dgFloat64 m_elements[DG_GOOGOL_SIZE];
};

class dgHugeVector: public dgTemplateVector<dgGoogol>
{
	public:
	dgHugeVector (const dgBigVector& a)
		:dgTemplateVector<dgGoogol>()
	{
		m_x = a.m_x;
		m_y = a.m_y;
		m_z = a.m_z;
		m_w = a.m_w; 
	}
	dgHugeVector (const dgTemplateVector<dgGoogol>& a)
		:dgTemplateVector<dgGoogol>(a)
	{
	}

	dgHugeVector (dgFloat64 x, dgFloat64 y, dgFloat64 z, dgFloat64 w)
		:dgTemplateVector<dgGoogol>()
	{
		m_x = x;
		m_y = y;
		m_z = z;
		m_w = w; 
	}
};



#endif
