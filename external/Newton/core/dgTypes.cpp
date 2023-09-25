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
#include "dgTypes.h"
#include "dgDebug.h"
#include "dgVector.h"
#include "dgMemory.h"
#include "dgStack.h"

#define ROUNDING_MASK		0x0fffff3ff
#define ROUNDING_DOWN		0x000000400

#ifdef _WIN_32_VER
 dgFloat32 dgAbsf(dgFloat32 x) 
{
	_asm fld	x;
	_asm fabs	;
	_asm fstp	x;
	return x;
}

 dgFloat32 dgSqrt(dgFloat32 x)  
{
	_asm fld	x;
	_asm fsqrt	;
	_asm fstp	x;
	return x;
}

 dgFloat32 dgSin(dgFloat32 x)  
{
	_asm fld	x;
	_asm fsin;
	_asm fstp	x;
	return x;
}

 dgFloat32 dgCos(dgFloat32 x)  
{
	_asm fld	x;
	_asm fcos;
	_asm fstp	x;
	return x;
}

 dgFloat32 dgAsin(dgFloat32 x)  
{
	_asm fld	x;
	_asm fld1;             
	_asm fadd   st,st(1); 
	_asm fld1;             
	_asm fsub   st,st(2); 
	_asm fmulp  st(1),st; 
	_asm fsqrt;            
	_asm fpatan;     
	_asm fstp	x;
	return x;
}

 dgFloat32 dgAcos(dgFloat32 x)  
{
	_asm fld	x;
	_asm fld1;             
	_asm fadd   st,st(1); 
	_asm fld1;             
	_asm fsub   st,st(2); 
	_asm fmulp  st(1),st; 
	_asm fsqrt;    
	_asm fxch   st(1); 
	_asm fpatan;     
	_asm fstp	x;
	return x;
}

 dgFloat32 dgAtan2(dgFloat32 x, dgFloat32 y)  
{
	_asm fld	x;
	_asm fld	y;
	_asm fpatan;     
	_asm fstp	x;
	return x;
}



 dgFloat32 dgFloor (dgFloat32 x) 
{
	dgInt32	tmp;
	dgInt32	controlworld;
	__asm  fld		x;
	__asm  fnstcw   controlworld; 
	__asm  mov      eax, controlworld; 
	__asm  and		eax, ROUNDING_MASK;
	__asm  or       eax, ROUNDING_DOWN; 	
	__asm  mov      tmp, eax;
	__asm  fldcw	tmp;
	__asm  frndint;
	__asm  fldcw	controlworld;
	__asm  fstp		x;
	return x;
}


 dgInt32 dgFastInt (dgFloat32 x) 
{
	dgInt32	tmp;
	dgInt32 integer;
	dgInt32	controlworld;

	__asm  fld		x;
	__asm  fnstcw   controlworld; 
	__asm  mov      eax, controlworld; 
	__asm  and		eax, ROUNDING_MASK;
	__asm  or       eax, ROUNDING_DOWN; 	
	__asm  mov      tmp, eax;
	__asm  fldcw	tmp;
	__asm  fistp	integer;
	__asm  fldcw	controlworld;
	return integer;
}

#ifdef __USE_DOUBLE_PRECISION__
void dgSinCos (dgFloat32 ang, dgFloat32& sinAng, dgFloat32& cosAng)
{
	_asm fld		ang;
	_asm fsincos	;
	_asm mov		edx, cosAng; 
	_asm mov		eax, sinAng; 
	_asm fstp		QWORD PTR [edx];
	_asm fstp		QWORD PTR [eax];
}

#else

void dgSinCos (dgFloat32 ang, dgFloat32& sinAng, dgFloat32& cosAng)
{
	_asm fld		ang;
	_asm fsincos	;
	_asm mov		edx, cosAng; 
	_asm mov		eax, sinAng; 
	_asm fstp		DWORD PTR [edx];
	_asm fstp		DWORD PTR [eax];
}
#endif
#endif



void dgApi GetMinMax (dgVector &minOut, dgVector &maxOut, const dgFloat32* const vertexArray, dgInt32 vCount, dgInt32 strideInBytes)
{
	dgInt32 stride = dgInt32 (strideInBytes / sizeof (dgFloat32));
	const dgFloat32* vArray = vertexArray + stride;

	_DG_ASSERTE (stride >= 3);
 	minOut = dgVector (vertexArray[0], vertexArray[1], vertexArray[2], dgFloat32 (0.0f)); 
	maxOut = dgVector (vertexArray[0], vertexArray[1], vertexArray[2], dgFloat32 (0.0f)); 

	for (dgInt32 i = 1; i < vCount; i ++) {
		minOut.m_x = GetMin (minOut.m_x, vArray[0]);
		minOut.m_y = GetMin (minOut.m_y, vArray[1]);
		minOut.m_z = GetMin (minOut.m_z, vArray[2]);

		maxOut.m_x = GetMax (maxOut.m_x, vArray[0]);
		maxOut.m_y = GetMax (maxOut.m_y, vArray[1]);
		maxOut.m_z = GetMax (maxOut.m_z, vArray[2]);

		vArray += stride;
	}
}


#if (defined (WIN32) || defined(_WIN32))

	#if (_MSC_VER >= 1400) && defined (DG_BUILD_SIMD_CODE)
		static bool DetectSSE_3 ()
		{
			__try {
				__m128 i;
				i = _mm_set_ps (dgFloat32 (1.0f), dgFloat32 (2.0f), dgFloat32 (3.0f), dgFloat32 (4.0f));
				i = _mm_hadd_ps (i, i);
				i = _mm_hadd_ps (i, i);
				i = i;
	//			i = _mm_dp_ps(i, i, 0);
	//			i = _mm_dp_ps(i, i, 0);
	//			i = _mm_hadd_ps (i, i);
			} __except (EXCEPTION_EXECUTE_HANDLER) {
				return false;
			}
			return true;
		}
	#else
		static bool DetectSSE_3 ()
		{
			return false;
		}
	#endif

		

	#ifdef DG_BUILD_SIMD_CODE
		static bool DetectSSE ()
		{
			__try {
				__m128 i;
				i = _mm_set_ps (dgFloat32 (1.0f), dgFloat32 (2.0f), dgFloat32 (3.0f), dgFloat32 (4.0f));
				i = _mm_add_ps(i, i);
				i = i;
			} __except (EXCEPTION_EXECUTE_HANDLER) {
				return false;
			}
			return true;
		}
	#else
		static bool DetectSSE ()
		{
			return false;
		}
	#endif


	dgCpuClass dgApi dgGetCpuType ()
	{
		if (DetectSSE_3 ()) {
			return dgSimdPresent;
		}

		if (DetectSSE ()) {
			return dgSimdPresent;
		}
		return dgNoSimdPresent;
	}
#endif


#ifdef __APPLE__

	dgCpuClass dgApi dgGetCpuType ()
	{
		#ifdef __ppc__

			return dgNoSimdPresent;
/*
			#ifdef 	_SCALAR_AROTHMETIC_ONLY
				return dgNoSimdPresent;
			#else
					dgInt32 error;
					dgInt32 hasSimd; 
					size_t length = sizeof( hasSimd ); 

					hasSimd = 0; 
					error = sysctlbyname("hw.optional.altivec", & hasSimd, &length, NULL, 0); 
					if (error) {
						return dgNoSimdPresent;
					}
					if (hasSimd) {
						return dgSimdPresent;
					}
					return dgNoSimdPresent; 
			#endif
*/
		#else
				dgInt32 error;
				dgInt32 hasSimd; 
				size_t length = sizeof( hasSimd ); 

				hasSimd = 0; 
				error = sysctlbyname("hw.optional.sse2", & hasSimd, &length, NULL, 0); 
				if (error) {
					return dgNoSimdPresent;
				}
				if (hasSimd) {
					return dgSimdPresent;
				}
				return dgNoSimdPresent; 
		#endif

	}

#endif

#if (defined (__linux__) || defined (__MINGW32__) || defined (__MINGW64__) || defined (__FreeBSD__))
/*	#define cpuid(func,ax,bx,cx,dx)	__asm__ __volatile__ ("cpuid": "=a" (ax), "=b" (bx), "=c" (cx), "=d" (dx) : "a" (func)); */

#ifndef __x86_64__
	void cpuid(dgUnsigned32 op, dgUnsigned32 reg[4])
	{
		asm volatile(
			"pushl %%ebx      \n\t" /* save %ebx */
			"cpuid            \n\t"
			"movl %%ebx, %1   \n\t" /* save what cpuid just put in %ebx */
			"popl %%ebx       \n\t" /* restore the old %ebx */
			: "=a"(reg[0]), "=r"(reg[1]), "=c"(reg[2]), "=d"(reg[3])
			: "a"(op)
			: "cc");
	}

	static dgInt32 i386_cpuid(void) 
	{ 
//		int a, b, c, d; 
//		cpuid(1,a,b,c,d); 
//		return d; 

		dgUnsigned32 reg[4];
		cpuid(1, reg);
		return reg[3];
	} 
#endif

	dgCpuClass dgApi dgGetCpuType ()
	{
#ifndef __x86_64__
		#define bit_MMX (1 << 23) 
		#define bit_SSE (1 << 25) 
		#define bit_SSE2 (1 << 26) 

#ifndef __USE_DOUBLE_PRECISION__
		if (i386_cpuid() & bit_SSE) {
			return dgSimdPresent;
		}
#endif
		#endif
		return dgNoSimdPresent;
	}
#endif


namespace InternalGeoUtil
{
	static inline dgInt32 dgApi cmp_vertex (const dgFloat32* const v1, const dgFloat32* const v2, dgInt32 firstSortAxis)
	{
		if (v1[firstSortAxis] < v2[firstSortAxis]) {
			return -1;
		}

		if (v1[firstSortAxis] > v2[firstSortAxis]){
			return 1;
		}

		return 0;
	}
	
	static dgInt32 SortVertices (dgFloat32* const vertexList,  dgInt32 stride, dgInt32 floatSize, dgInt32 unsignedSize, dgInt32 vertexCount, dgFloat32 tolerance)
	{
#ifdef __USE_DOUBLE_PRECISION__
		dgInt64* indexPtr;
#else
		dgInt32* indexPtr;
#endif

		dgFloat64 xc = 0;
		dgFloat64 yc = 0;
		dgFloat64 zc = 0;
		dgFloat64 x2c = 0;
		dgFloat64 y2c = 0;
		dgFloat64 z2c = 0;

		dgBigVector minP (1e10, 1e10, 1e10, 0);
		dgBigVector maxP (-1e10, -1e10, -1e10, 0);
		dgInt32 k = 0;
		for (dgInt32 i = 0; i < vertexCount; i ++) {
			dgFloat64 x  = vertexList[k + 2];
			dgFloat64 y  = vertexList[k + 3];
			dgFloat64 z  = vertexList[k + 4];
			k += stride;

			xc += x;
			yc += y;
			zc += z;
			x2c += x * x;
			y2c += y * y; 
			z2c += z * z;
	
			if (x < minP.m_x) {
				minP.m_x = x; 
			}
			if (y < minP.m_y) {
				minP.m_y = y; 
			}
	
			if (z < minP.m_z) {
				minP.m_z = z; 
			}
	
			if (x > maxP.m_x) {
				maxP.m_x = x; 
			}
			if (y > maxP.m_y) {
				maxP.m_y = y; 
			}
	
			if (z > maxP.m_z) {
				maxP.m_z = z; 
			}
		}
	
		dgBigVector del (maxP - minP);
		dgFloat64 minDist = GetMin (del.m_x, del.m_y, del.m_z);
		if (minDist < 1.0e-3) {
			minDist = 1.0e-3;
		}
	
		dgFloat64 tol = tolerance * minDist + 1.0e-4;
		tol = tolerance + 1.0e-4;
		dgFloat64 sweptWindow = 2.0 * tol;
		sweptWindow += 1.0e-4;
	
		x2c = vertexCount * x2c - xc * xc;
		y2c = vertexCount * y2c - yc * yc;
		z2c = vertexCount * z2c - zc * zc;

		dgInt32 firstSortAxis = 2;
		if ((y2c >= x2c) && (y2c >= z2c)) {
			firstSortAxis = 3;
		} else if ((z2c >= x2c) && (z2c >= y2c)) {
			firstSortAxis = 4;
		}


		dgInt32 stack[1024][2];
		stack[0][0] = 0;
		stack[0][1] = vertexCount - 1;
		dgInt32 stackIndex = 1;
		while (stackIndex) {
			stackIndex --;
			dgInt32 lo = stack[stackIndex][0];
			dgInt32 hi = stack[stackIndex][1];
			if ((hi - lo) > 8) {
				dgInt32 i = lo;
				dgInt32 j = hi;
				dgFloat32 val[64]; 
				memcpy (val, &vertexList[((lo + hi) >> 1) * stride], stride * sizeof (dgFloat32));
				do {    
					while (cmp_vertex (&vertexList[i * stride], val, firstSortAxis) < 0) i ++;
					while (cmp_vertex (&vertexList[j * stride], val, firstSortAxis) > 0) j --;

					if (i <= j)	{
						dgFloat32 tmp[64]; 
						//dgIsland tmp (m_islandArray[i]);
						memcpy (tmp, &vertexList[i * stride], stride * sizeof (dgFloat32));
						memcpy (&vertexList[i * stride], &vertexList[j * stride], stride * sizeof (dgFloat32)); 
						memcpy (&vertexList[j * stride], tmp, stride * sizeof (dgFloat32)); 
						i++; 
						j--;
					}
				} while (i <= j);

				if (i < hi) {
					stack[stackIndex][0] = i;
					stack[stackIndex][1] = hi;
					stackIndex ++;
				}
				if (lo < j) {
					stack[stackIndex][0] = lo;
					stack[stackIndex][1] = j;
					stackIndex ++;
				}
				_DG_ASSERTE (stackIndex < sizeof (stack) / (2 * sizeof (stack[0][0])));
			} else {
				for (dgInt32 i = lo + 1; i <= hi ; i++) {
					dgInt32 j;
					dgFloat32 tmp[64]; 
					//dgIsland tmp (m_islandArray[i]);
					memcpy (tmp, &vertexList[i * stride], stride * sizeof (dgFloat32));
					for (j = i; j && (cmp_vertex (&vertexList[(j - 1) * stride], tmp, firstSortAxis) > 0); j --) {
						//m_islandArray[j] = m_islandArray[j - 1];
						memcpy (&vertexList[j * stride], &vertexList[(j - 1)* stride], stride * sizeof (dgFloat32));
					}
					//m_islandArray[j] = tmp;
					memcpy (&vertexList[j * stride], tmp, stride * sizeof (dgFloat32)); 
				}
			}
		}


#ifdef _DG_DEBUG
		for (dgInt32 i = 0; i < (vertexCount - 1); i ++) {
			_DG_ASSERTE (cmp_vertex (&vertexList[i * stride], &vertexList[(i + 1) * stride], firstSortAxis) <= 0);
		}
#endif

		dgInt32 count = 0;
#ifdef __USE_DOUBLE_PRECISION__
		_DG_ASSERTE (0);
		indexPtr = (dgInt64*)vertexList;
#else
		indexPtr = (dgInt32*)vertexList;
#endif
		for (dgInt32 i = 0; i < vertexCount; i ++) {
			dgInt32 m = i * stride;
			dgInt32 index = dgInt32 (indexPtr[m + 0]);
			if (index == dgInt32 (0xffffffff)) {
				dgFloat64 swept = vertexList[m + firstSortAxis] + sweptWindow;
				dgInt32 k = i * stride + stride;
				for (dgInt32 i1 = i + 1; i1 < vertexCount; i1 ++) {

					index = dgInt32 (indexPtr[k + 0]);
					if (index == dgInt32 (0xffffffff)) {
						dgFloat64 val = vertexList[k + firstSortAxis];
						if (val >= swept) {
							break;
						}

						bool test = true;
						dgInt32 t = 0;
						for (; test && (t < floatSize); t ++) {
							//dgFloat32 val;
							dgFloat64 val = dgAbsf (vertexList[m + t + 2] - vertexList[k + t + 2]);
							test = test && (val < tol);
						}
						for (; test && (t < floatSize + unsignedSize); t ++) {
							dgUnsigned32 val0 = *(dgUnsigned32 *)&vertexList[m + t + 2];
							dgUnsigned32 val1 = *(dgUnsigned32 *)&vertexList[k + t + 2];
							test = t && (val0 == val1);
						}
						if (test) {
							indexPtr[k + 0] = count;
						}
					}
					k += stride;
				}

				memcpy (&vertexList[count * stride + 2], &vertexList[m + 2], (stride - 2) * sizeof (dgFloat32));
				indexPtr[m + 0] = count;
				count ++;
			}
		}
				
		return count;
	}



	static dgInt32 QuickSortVertices (
		dgFloat32* const vertList, 
		dgInt32 stride,
		dgInt32 floatSize,
		dgInt32 unsignedSize,
		dgInt32 vertexCount, 
		dgFloat32 tolerance)
	{
		dgInt32 i;
		dgInt32 axis;
		dgInt32 count;
		dgInt32 count0;
		dgInt32 count1;
		dgFloat32 x;
		dgFloat32 y;
		dgFloat32 z;
		dgFloat32 x0;
		dgFloat32 y0;
		dgFloat32 z0;
		dgFloat32 xd;
		dgFloat32 yd;
		dgFloat32 zd;
		dgFloat32 axisVal;

#ifdef __USE_DOUBLE_PRECISION__
		dgInt64* indexPtr;
#else
		dgInt32* indexPtr;
#endif
	
		count = 0;
		if (vertexCount > (3 * 1024 * 32)) {
			x = dgFloat32 (0.0f);
			y = dgFloat32 (0.0f);
			z = dgFloat32 (0.0f);
			xd = dgFloat32 (0.0f);
			yd = dgFloat32 (0.0f);
			zd = dgFloat32 (0.0f);
			
			for (i = 0; i < vertexCount; i ++) {
				x0 = vertList[i * stride + 2];
				y0 = vertList[i * stride + 3];
				z0 = vertList[i * stride + 4];
				x += x0;
				y += y0;
				z += z0;
				xd += x0 * x0;
				yd += y0 * y0;
				zd += z0 * z0;
			}

			xd = vertexCount * xd - x * x;
			yd = vertexCount * yd - y * y;
			zd = vertexCount * zd - z * z;

			axis = 2;
			axisVal = x / vertexCount;
			if ((yd > xd) && (yd > zd)) {
				axis = 3;
				axisVal = y / vertexCount;
			}
			if ((zd > xd) && (zd > yd)) {
				axis = 4;
				axisVal = z / vertexCount;
			}

			dgInt32 i0 = 0;
			dgInt32 i1 = vertexCount - 1;
			do {    
				for ( ;vertList[i0 * stride + axis] < axisVal; i0 ++); 
				for ( ;vertList[i1 * stride + axis] > axisVal; i1 --);
				if (i0 <= i1) {
					for (i = 0; i < stride; i ++) {
						Swap (vertList[i0 * stride + i], vertList[i1 * stride + i]);
					}
					i0 ++; 
					i1 --;
				}
			} while (i0 <= i1);
			_DG_ASSERTE (i0 < vertexCount);

			count0 = QuickSortVertices (&vertList[ 0 * stride], stride, floatSize, unsignedSize, i0, tolerance);
			count1 = QuickSortVertices (&vertList[i0 * stride], stride, floatSize, unsignedSize, vertexCount - i0, tolerance);
			
			count = count0 + count1;

			for (i = 0; i < count1; i ++) {
				memcpy (&vertList[(count0 + i) * stride + 2], &vertList[(i0 + i) * stride + 2], (stride - 2) * sizeof (dgFloat32));
			}


			#ifdef __USE_DOUBLE_PRECISION__
					indexPtr = (dgInt64*)vertList;
			#else
					indexPtr = (dgInt32*)vertList;
			#endif

			for (i = i0; i < vertexCount; i ++) {
				indexPtr[i * stride] += count0;
			}

		} else {
			count = SortVertices (vertList, stride, floatSize, unsignedSize, vertexCount, tolerance);
		}

		return count;
	}
}



dgInt32 dgApi dgVertexListToIndexList (dgFloat32* const vertList, dgInt32 strideInBytes, dgInt32 floatSizeInBytes,  dgInt32 unsignedSizeInBytes, dgInt32 vertexCount, dgInt32* const indexList, dgFloat32 tolerance)
{
#if (defined (WIN32) || defined(_WIN32))
	dgUnsigned32 controlWorld = dgControlFP (0xffffffff, 0);
	dgControlFP (_PC_53, _MCW_PC);
#endif

	if (strideInBytes < dgInt32 (sizeof (dgTriplex))) {
		return 0;
	}
	if ((floatSizeInBytes + unsignedSizeInBytes) < dgInt32(sizeof (dgTriplex))) {
		return 0;
	}

	if ((floatSizeInBytes + unsignedSizeInBytes) > strideInBytes) {
		return 0;
	}

	dgInt32 stride = strideInBytes / dgInt32 (sizeof (dgFloat32));
	dgInt32 stride2 = stride + 2;

	dgStack<dgFloat32>pool (stride2  * vertexCount);
	dgFloat32* const tmpVertexList = &pool[0];

#ifdef __USE_DOUBLE_PRECISION__
	dgInt64* const indexPtr = (dgInt64*)tmpVertexList;
#else
	dgInt32* const indexPtr = (dgInt32*)tmpVertexList;
#endif

	dgInt32 k = 0;
	dgInt32 m = 0;
	for (dgInt32 i = 0; i < vertexCount; i ++) {
		memcpy (&indexPtr [m + 2], &vertList[k], stride * sizeof (dgFloat32));
		indexPtr[m + 0] = 0;
		indexPtr[m + 0] = indexPtr[m + 0] - 1;
		indexPtr[m + 1] = i;
		k += stride;
		m += stride2;
	}
	
	dgInt32 floatSize = dgInt32 (floatSizeInBytes / sizeof (dgFloat32));
	dgInt32 unsignedSize = dgInt32 (unsignedSizeInBytes / sizeof (dgUnsigned32));
	dgInt32 count = InternalGeoUtil::QuickSortVertices (tmpVertexList, stride + 2, floatSize, unsignedSize, vertexCount, tolerance);

	k = 0;
	m = 0;
	for (dgInt32 i = 0; i < count; i ++) {
		k = i * stride;
		m = i * stride2;
		memcpy (&vertList[k], &indexPtr [m + 2], stride * sizeof (dgFloat32));
		k += stride;
		m += stride2;
	}

	m = 0;
	for (dgInt32 i = 0; i < vertexCount; i ++) {
		dgInt32 i1 = dgInt32 (indexPtr [m + 1]);
		dgInt32 index = dgInt32 (indexPtr [m + 0]);
		indexList[i1] = dgInt32 (index);
		m += stride2;
	}

#if (defined (WIN32) || defined(_WIN32))
	dgControlFP (controlWorld, _MCW_PC);
#endif

	return count;
}

