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
#include "dgRandom.h"

#define RAND_MUL 31415821u
static dgUnsigned32 randSeed = RAND_MUL;

void dgApi dgRandomize (dgUnsigned32 Seed)
{
	randSeed = Seed;
}

dgUnsigned32 dgApi dgRandom()
{
//	__asm {
//		mov	eax, randSeed 
//		mov	edx, RAND_MUL	
//		mul	edx
//		inc	eax
//		mov	randSeed, eax 
//	}
//	return randSeed;

	randSeed = RAND_MUL * randSeed + 1; 
	return randSeed;
}


