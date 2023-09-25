/*
   AngelCode Scripting Library
   Copyright (c) 2003-2010 Andreas Jonsson

   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any
   damages arising from the use of this software.

   Permission is granted to anyone to use this software for any
   purpose, including commercial applications, and to alter it and
   redistribute it freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you
	  must not claim that you wrote the original software. If you use
	  this software in a product, an acknowledgment in the product
	  documentation would be appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and
	  must not be misrepresented as being the original software.

   3. This notice may not be removed or altered from any source
	  distribution.

   The original version of this library can be located at:
   http://www.angelcode.com/angelscript/

   Andreas Jonsson
   andreas@angelcode.com
*/

/*
 * Implements the AMD64 calling convention for gcc-based 64bit Unices
 *
 * Author: Ionut "gargltk" Leonte <ileonte@bitdefender.com>
 *
 * Initial author: niteice
 */

#include "as_config.h"

#ifndef AS_MAX_PORTABILITY
#ifdef AS_X64_GCC

#include "as_scriptengine.h"
#include "as_texts.h"

BEGIN_AS_NAMESPACE

enum argTypes { x64ENDARG = 0, x64INTARG = 1, x64FLOATARG = 2, x64DOUBLEARG = 3, x64VARIABLE = 4 };
typedef asQWORD ( *funcptr_t )( void );

#define X64_MAX_ARGS			 32
#define MAX_CALL_INT_REGISTERS	  6
#define MAX_CALL_SSE_REGISTERS	  8
#define CALLSTACK_MULTIPLIER	  2
#define X64_CALLSTACK_SIZE		  ( X64_MAX_ARGS + MAX_CALL_SSE_REGISTERS + 3 )

// TODO: Should this really be different on Mac and other systems? Probably the Mac way is the correct one
#define ASM_GET_REG( name, dest )				 \
	__asm__ __volatile__ (						 \
		"movq %" name ", %0\n"					 \
		:										 \
		: "m" ( dest )							 \
	)

static asDWORD GetReturnedFloat()
{
	float	retval;
	asDWORD ret;

	__asm__ __volatile__ (
		"lea	  %0, %%rax\n"
		"movss	  %%xmm0, (%%rax)"
		: /* no output */
		: "m" (retval)
		: "%rax"
	);

	/* We need to avoid implicit conversions from float to unsigned - we need
	   a bit-wise-correct-and-complete copy of the value */
	memcpy( &ret, &retval, sizeof( ret ) );

	return ( asDWORD )ret;
}

static asQWORD GetReturnedDouble()
{
	double	retval;
	asQWORD ret;

	__asm__ __volatile__ (
		"lea	 %0, %%rax\n"
		"movlpd	 %%xmm0, (%%rax)"
		: /* no optput */
		: "m" (retval)
		: "%rax"
	);

	/* We need to avoid implicit conversions from double to unsigned long long - we need
	   a bit-wise-correct-and-complete copy of the value */
	memcpy( &ret, &retval, sizeof( ret ) );

	return ret;
}

static asQWORD __attribute ((__noinline__)) X64_CallFunction( const asDWORD* pArgs,
															  const asBYTE *pArgsType,
															  void *func )
{
	volatile asQWORD retval;
	volatile asQWORD stackcnt = 0;
	int i;

	for (i = MAX_CALL_INT_REGISTERS + MAX_CALL_SSE_REGISTERS;
		 pArgsType[i] != x64ENDARG && ( i < X64_MAX_ARGS + MAX_CALL_SSE_REGISTERS + 3);
		 i++ ) {
		stackcnt++;
	}

	__asm__ __volatile__ (
		// %0 - stackcnt, %1 - Args, %2 - func
		"  movq %0, %%rcx \n"
		"  movq %1, %%r10 \n"
		"  movq %2, %%r11 \n"

		// Backup stack pointer in R15 that is guaranteed to maintain its value over function calls
		"  movq %%rsp, %%r15 \n"

#ifdef __OPTIMIZE__
		// Make sure the stack unwind logic knows we've backed up the stack pointer in register r15
		// This should only be done if any optimization is done. If no optimization (-O0) is used,
		// then the compiler already backups the rsp before entering the inline assembler code
		".cfi_def_cfa_register r15 \n"
#endif

		// Skip the first 128 bytes on the stack frame, called "red zone",
		// that might be used by the compiler to store temporary values
		"  sub $128, %%rsp \n"

		// Make sure the stack pointer will be aligned to 16 bytes when the function is called
		"  movq %%rcx, %%rdx \n"
		"  salq $3, %%rdx \n"
		"  movq %%rsp, %%rax \n"
		"  sub %%rdx, %%rax \n"
		"  and $15, %%rax \n"
		"  sub %%rax, %%rsp \n"

		// Push the stack parameters, i.e. the arguments that won't be loaded into registers
// Push the stack parameters, i.e. the arguments that won't be loaded into registers
		"  movq %%rcx, %%rsi \n"
		"  testl %%esi, %%esi \n"
		"  jle 2f \n"
		"  subl $1, %%esi \n"
		"  xorl %%edx, %%edx \n"
		"  leaq 8(, %%rsi, 8), %%rcx \n"
		"1: \n"
		"  movq 112(%%r10, %%rdx), %%rax \n"
		"  pushq %%rax \n"
		"  addq $8, %%rdx \n"
		"  cmpq %%rcx, %%rdx \n"
		"  jne 1b \n"
		"2: \n"

		// Populate integer and floating point parameters
		"movq %%r10, %%rax \n"
		"movq	(%%rax), %%rdi\n"
		"movq  8(%%rax), %%rsi\n"
		"movq 16(%%rax), %%rdx\n"
		"movq 24(%%rax), %%rcx\n"
		"movq 32(%%rax), %%r8\n"
		"movq 40(%%rax), %%r9\n"
		"movq 48(%%rax), %%xmm0\n"
		"movq 56(%%rax), %%xmm1\n"
		"movq 64(%%rax), %%xmm2\n"
		"movq 72(%%rax), %%xmm3\n"
		"movq 80(%%rax), %%xmm4\n"
		"movq 88(%%rax), %%xmm5\n"
		"movq 96(%%rax), %%xmm6\n"
		"movq 104(%%rax), %%xmm7\n"

		// Call the function
		"callq *%%r11 \n"

		// Restore stack pointer
		"movq %%r15, %%rsp\n"
#ifdef __OPTIMIZE__
		// Inform the stack unwind logic that the stack pointer has been restored
		// This should only be done if any optimization is done. If no optimization (-O0) is used,
		// then the compiler already backups the rsp before entering the inline assembler code
		".cfi_def_cfa_register rsp \n"
#endif
		"movq %%rax, %3\n"

		: : "g" (stackcnt), "g" (pArgs), "g" (func), "m" (retval)
		: "%xmm0", "%xmm1", "%xmm2", "%xmm3", "%xmm4", "%xmm5", "%xmm6", "%xmm7",
		  "%rdi", "%rsi", "%rax", "%rdx", "%rcx", "%r8", "%r9", "%r10", "%r11", "%r15");

	return retval;
}

// returns true if the given parameter is a 'variable argument'
inline bool IsVariableArgument( asCDataType type )
{
	return ( type.GetTokenType() == ttQuestion ) ? true : false;
}

int CallSystemFunction( int id, asCContext *context, void *objectPointer )
{
	asCScriptEngine			   *engine			   = context->engine;
	asCScriptFunction		   *descr			   = engine->scriptFunctions[id];
	asSSystemFunctionInterface *sysFunc			   = engine->scriptFunctions[id]->sysFuncIntf;
	int							callConv		   = sysFunc->callConv;

	asQWORD						retQW			   = 0;
	asQWORD						retQW2			   = 0;
	void					   *func			   = ( void * )sysFunc->func;
	int							paramSize		   = sysFunc->paramSize;
	asDWORD					   *args			   = context->regs.stackPointer;
	asDWORD					   *stack_pointer	   = context->regs.stackPointer;
	void					   *retPointer		   = 0;
	void					   *obj				   = 0;
	funcptr_t				   *vftable			   = NULL;
	int							popSize			   = paramSize;
	int							totalArgumentCount = 0;
	int							n				   = 0;
	int							base_n			   = 0;
	int							a				   = 0;
	int							param_pre		   = 0;
	int							param_post		   = 0;
	int							argIndex		   = 0;
	int							argumentCount	   = 0;

	asDWORD	 tempBuff[CALLSTACK_MULTIPLIER * X64_CALLSTACK_SIZE] = { 0 };
	asBYTE	 tempType[X64_CALLSTACK_SIZE] = { 0 };

	asDWORD	 paramBuffer[CALLSTACK_MULTIPLIER * X64_CALLSTACK_SIZE] = { 0 };
	asBYTE	 argsType[X64_CALLSTACK_SIZE] = { 0 };

	asBYTE	 argsSet[X64_CALLSTACK_SIZE]  = { 0 };

	if( callConv == ICC_GENERIC_FUNC || callConv == ICC_GENERIC_METHOD ) {
		return context->CallGeneric( id, objectPointer );
	}

	context->regs.objectType = descr->returnType.GetObjectType();
	if( descr->returnType.IsObject() && !descr->returnType.IsReference() && !descr->returnType.IsObjectHandle() ) {
		// Allocate the memory for the object
		retPointer = engine->CallAlloc( descr->returnType.GetObjectType() );

		if( sysFunc->hostReturnInMemory ) {
			// The return is made in memory
			callConv++;
		}
	}

	argumentCount = ( int )descr->parameterTypes.GetLength();
	assert( argumentCount <= X64_MAX_ARGS );

	// TODO: optimize: argsType should be computed in PrepareSystemFunction
	for( a = 0; a < argumentCount; ++a, ++argIndex ) {
		// get the base type
		argsType[argIndex] = x64INTARG;
		if ( descr->parameterTypes[a].IsFloatType() && !descr->parameterTypes[a].IsReference() ) {
			argsType[argIndex] = x64FLOATARG;
		}
		if ( descr->parameterTypes[a].IsDoubleType() && !descr->parameterTypes[a].IsReference() ) {
			argsType[argIndex] = x64DOUBLEARG;
		}
		if ( descr->parameterTypes[a].GetSizeOnStackDWords() == 2 && !descr->parameterTypes[a].IsDoubleType() && !descr->parameterTypes[a].IsReference() ) {
			argsType[argIndex] = x64INTARG;
		}

		if ( IsVariableArgument( descr->parameterTypes[a] ) ) {
			argsType[argIndex] = x64VARIABLE;
		}
	}
	assert( argIndex == argumentCount );

	for ( a = 0; a < argumentCount && totalArgumentCount <= X64_MAX_ARGS; a++ ) {
		switch ( argsType[a] ) {
			case x64ENDARG:
			case x64INTARG:
			case x64FLOATARG:
			case x64DOUBLEARG: {
				if ( totalArgumentCount < X64_MAX_ARGS )
					tempType[totalArgumentCount++] = argsType[a];
				break;
			}
			case x64VARIABLE: {
				if ( totalArgumentCount < X64_MAX_ARGS )
					tempType[totalArgumentCount++] = x64VARIABLE;
				if ( totalArgumentCount < X64_MAX_ARGS )
					tempType[totalArgumentCount++] = x64INTARG;
				break;
			}
		}
	}

	assert( totalArgumentCount <= X64_MAX_ARGS );
	if ( totalArgumentCount > argumentCount ) {
		memcpy( argsType, tempType, totalArgumentCount );
	}
	memset( tempType, 0, sizeof( tempType ) );

	// TODO: This should be checked in PrepareSystemFunction
#ifndef COMPLEX_OBJS_PASSED_BY_REF
	if( sysFunc->takesObjByVal ) {
		/* I currently know of no way we can predict register usage for passing complex
		   objects by value when the compiler does not pass them by reference instead. I
		   will quote the example from the AMD64 ABI to demonstrate this:

		   (http://www.x86-64.org/documentation/abi.pdf - page 22)

		------------------------------ BEGIN EXAMPLE -------------------------------

		Let us consider the following C code:

		typedef struct {
			int a, b;
			double d;
		} structparm;

		structparm s;
		int e, f, g, h, i, j, k;
		long double ld;
		double m, n;

		extern void func (int e, int f,
			structparm s, int g, int h,
			long double ld, double m,
			double n, int i, int j, int k);

		func (e, f, s, g, h, ld, m, n, i, j, k);

		Register allocation for the call:
		--------------------------+--------------------------+-------------------
		General Purpose Registers | Floating Point Registers | Stack Frame Offset
		--------------------------+--------------------------+-------------------
		 %rdi: e				  | %xmm0: s.d				 | 0:  ld
		 %rsi: f				  | %xmm1: m				 | 16: j
		 %rdx: s.a,s.b			  | %xmm2: n				 | 24: k
		 %rcx: g				  |							 |
		 %r8:  h				  |							 |
		 %r9:  i				  |							 |
		--------------------------+--------------------------+-------------------
		*/

		context->SetInternalException( TXT_INVALID_CALLING_CONVENTION );
		if( retPointer ) {
			engine->CallFree( retPointer );
		}
		return 0;
	}
#endif

	obj = objectPointer;
	if ( !obj && callConv >= ICC_THISCALL ) {
		// The object pointer should be popped from the context stack
		popSize += AS_PTR_SIZE;

		// Check for null pointer
		obj = ( void * )( *( ( asQWORD * )( args ) ) );
		stack_pointer += AS_PTR_SIZE;
		if( !obj ) {
			context->SetInternalException( TXT_NULL_POINTER_ACCESS );
			if( retPointer ) {
				engine->CallFree( retPointer );
			}
			return 0;
		}

		// Add the base offset for multiple inheritance
		obj = ( void * )( ( asQWORD )obj + sysFunc->baseOffset );
	}

	if ( obj && ( callConv == ICC_VIRTUAL_THISCALL || callConv == ICC_VIRTUAL_THISCALL_RETURNINMEM ) ) {
		vftable = *( ( funcptr_t ** )obj );
		func	= ( void * )vftable[( asQWORD )func >> 3];
	}

	switch ( callConv ) {
		case ICC_CDECL_RETURNINMEM:
		case ICC_STDCALL_RETURNINMEM: {
			if ( totalArgumentCount ) {
				memmove( argsType + 1, argsType, totalArgumentCount );
			}
			memcpy( paramBuffer, &retPointer, sizeof( retPointer ) );
			argsType[0] = x64INTARG;
			base_n = 1;

			param_pre = 1;

			break;
		}
		case ICC_THISCALL:
		case ICC_VIRTUAL_THISCALL:
		case ICC_CDECL_OBJFIRST: {
			if ( totalArgumentCount ) {
				memmove( argsType + 1, argsType, totalArgumentCount );
			}
			memcpy( paramBuffer, &obj, sizeof( obj ) );
			argsType[0] = x64INTARG;

			param_pre = 1;

			break;
		}
		case ICC_THISCALL_RETURNINMEM:
		case ICC_VIRTUAL_THISCALL_RETURNINMEM:
		case ICC_CDECL_OBJFIRST_RETURNINMEM: {
			if ( totalArgumentCount ) {
				memmove( argsType + 2, argsType, totalArgumentCount );
			}
			memcpy( paramBuffer, &retPointer, sizeof( retPointer ) );
			memcpy( paramBuffer + CALLSTACK_MULTIPLIER, &obj, sizeof( obj ) );
			argsType[0] = x64INTARG;
			argsType[1] = x64INTARG;

			param_pre = 2;

			break;
		}
		case ICC_CDECL_OBJLAST: {
			memcpy( paramBuffer + totalArgumentCount * CALLSTACK_MULTIPLIER, &obj, sizeof( obj ) );
			argsType[totalArgumentCount] = x64INTARG;

			param_post = 1;

			break;
		}
		case ICC_CDECL_OBJLAST_RETURNINMEM: {
			if ( totalArgumentCount ) {
				memmove( argsType + 1, argsType, totalArgumentCount );
			}
			memcpy( paramBuffer, &retPointer, sizeof( retPointer ) );
			argsType[0] = x64INTARG;
			memcpy( paramBuffer + ( totalArgumentCount + 1 ) * CALLSTACK_MULTIPLIER, &obj, sizeof( obj ) );
			argsType[totalArgumentCount + 1] = x64INTARG;

			param_pre = 1;
			param_post = 1;

			break;
		}
		default: {
			base_n = 0;
			break;
		}
	}

	int adjust = 0;
	for( n = 0; n < ( int )( param_pre + totalArgumentCount + param_post ); n++ ) {
		int copy_count = 0;
		if ( n >= param_pre && n < ( int )( param_pre + totalArgumentCount ) ) {
			copy_count = descr->parameterTypes[n - param_pre - adjust].GetSizeOnStackDWords();

			if ( argsType[n] == x64VARIABLE ) {
				adjust += 1;
				argsType[n] = x64INTARG;
				n += 1;
			}
		}
		if ( copy_count > CALLSTACK_MULTIPLIER ) {
			if ( copy_count > CALLSTACK_MULTIPLIER + 1 ) {
				context->SetInternalException( TXT_INVALID_CALLING_CONVENTION );
				return 0;
			}

			memcpy( paramBuffer + ( n - 1 ) * CALLSTACK_MULTIPLIER, stack_pointer, AS_PTR_SIZE * sizeof( asDWORD ) );
			stack_pointer += AS_PTR_SIZE;
			memcpy( paramBuffer + n * CALLSTACK_MULTIPLIER, stack_pointer, sizeof( asDWORD ) );
			stack_pointer += 1;
		} else {
			if ( copy_count ) {
				memcpy( paramBuffer + n * CALLSTACK_MULTIPLIER, stack_pointer, copy_count * sizeof( asDWORD ) );
				stack_pointer += copy_count;
			}
		}
	}

	// If we are returning a object not by reference, we need to make the
	// pointer to the space allocated to the object the first parameter.
	if( descr->returnType.IsObject() && ( descr->returnType.GetObjectType()->flags & asOBJ_APP_CLASS_CA ) == asOBJ_APP_CLASS_CA &&
		!descr->returnType.IsReference() && !sysFunc->hostReturnInMemory )
	{
		if ( totalArgumentCount )
		{
			memmove( paramBuffer + CALLSTACK_MULTIPLIER, paramBuffer, ( CALLSTACK_MULTIPLIER * ( X64_CALLSTACK_SIZE - 1 ) ) );
			memmove( argsType + 1, argsType, X64_CALLSTACK_SIZE - 1 );
		}
		memcpy( paramBuffer, &retPointer, sizeof( retPointer ) );
		argsType[ 0 ] = x64INTARG;
	}

	/*
	 * Q: WTF is going on here !?
	 *
	 * A: The idea is to pre-arange the parameters so that X64_CallFunction() can do
	 * it's little magic which must work regardless of how the compiler decides to
	 * allocate registers. Basically:
	 * - the first MAX_CALL_INT_REGISTERS entries in tempBuff and tempType will
	 *	 contain the values/types of the x64INTARG parameters - that is the ones who
	 *	 go into the registers. If the function has less then MAX_CALL_INT_REGISTERS
	 *	 integer parameters then the last entries will be set to 0
	 * - the next MAX_CALL_SSE_REGISTERS entries will contain the float/double arguments
	 *	 that go into the floating point registers. If the function has less than
	 *	 MAX_CALL_SSE_REGISTERS floating point parameters then the last entries will
	 *	 be set to 0
	 * - index MAX_CALL_INT_REGISTERS + MAX_CALL_SSE_REGISTERS marks the start of the
	 *	 parameters which will get passed on the stack. These are added to the array
	 *	 in reverse order so that X64_CallFunction() can simply push them to the stack
	 *	 without the need to perform further tests
	 */
	int		used_int_regs = 0;
	int		used_sse_regs = 0;
	int		idx			  = 0;
	base_n = 0;
	for ( n = 0; ( n < X64_CALLSTACK_SIZE ) && ( used_int_regs < MAX_CALL_INT_REGISTERS ); n++ ) {
		if ( argsType[n] == x64INTARG ) {
			idx = base_n;
			argsSet[n] = 1;
			tempType[idx] = argsType[n];
			memcpy( tempBuff + idx * CALLSTACK_MULTIPLIER, paramBuffer + n * CALLSTACK_MULTIPLIER, CALLSTACK_MULTIPLIER * sizeof( asDWORD ) );
			base_n++;
			used_int_regs++;
		}
	}
	base_n = 0;
	for ( n = 0; ( n < X64_CALLSTACK_SIZE ) && ( used_sse_regs < MAX_CALL_SSE_REGISTERS ); n++ ) {
		if ( argsType[n] == x64FLOATARG || argsType[n] == x64DOUBLEARG ) {
			idx = MAX_CALL_INT_REGISTERS + base_n;
			argsSet[n] = 1;
			tempType[idx] = argsType[n];
			memcpy( tempBuff + idx * CALLSTACK_MULTIPLIER, paramBuffer + n * CALLSTACK_MULTIPLIER, CALLSTACK_MULTIPLIER * sizeof( asDWORD ) );
			base_n++;
			used_sse_regs++;
		}
	}
	base_n = 0;
	for ( n = X64_CALLSTACK_SIZE - 1; n >= 0; n-- ) {
		if ( argsType[n] != x64ENDARG && !argsSet[n] ) {
			idx = MAX_CALL_INT_REGISTERS + MAX_CALL_SSE_REGISTERS + base_n;
			argsSet[n] = 1;
			tempType[idx] = argsType[n];
			memcpy( tempBuff + idx * CALLSTACK_MULTIPLIER, paramBuffer + n * CALLSTACK_MULTIPLIER, CALLSTACK_MULTIPLIER * sizeof( asDWORD ) );
			base_n++;
		}
	}

	context->isCallingSystemFunction = true;
	retQW = X64_CallFunction( tempBuff, tempType, ( asDWORD * )func );
	ASM_GET_REG( "%rdx", retQW2 );
	context->isCallingSystemFunction = false;

#ifdef COMPLEX_OBJS_PASSED_BY_REF
	if( sysFunc->takesObjByVal ) {
		// Need to free the complex objects passed by value
		stack_pointer = context->regs.stackPointer;
		if ( !objectPointer && callConv >= ICC_THISCALL ) {
			stack_pointer += AS_PTR_SIZE;
		}
		for( n = 0; n < ( int )descr->parameterTypes.GetLength(); n++ ) {
			if ( descr->parameterTypes[n].IsObject() && !descr->parameterTypes[n].IsReference() && ( descr->parameterTypes[n].GetObjectType()->flags & COMPLEX_MASK ) ) {
				obj = ( void * )( *( asQWORD * )stack_pointer );
				asSTypeBehaviour *beh = &descr->parameterTypes[n].GetObjectType()->beh;
				if( beh->destruct ) {
					engine->CallObjectMethod(obj, beh->destruct);
				}

				engine->CallFree(obj);
			}

			stack_pointer += descr->parameterTypes[n].GetSizeInMemoryDWords();
		}
	}
#endif

	// Store the returned value in our stack
	if( descr->returnType.IsObject() && !descr->returnType.IsReference() )
	{
		if( descr->returnType.IsObjectHandle() )
		{
			context->regs.objectRegister = ( void * )( size_t )retQW;

			if( sysFunc->returnAutoHandle && context->regs.objectRegister )
			{
				engine->CallObjectMethod( context->regs.objectRegister, descr->returnType.GetObjectType()->beh.addref );
			}
		}
		else
		{
			if ( !sysFunc->hostReturnInMemory && ( descr->returnType.GetObjectType()->flags & asOBJ_APP_CLASS_CA ) != asOBJ_APP_CLASS_CA )
			{
				if ( sysFunc->hostReturnSize == 1 )
				{
					*( asDWORD * )retPointer = ( asDWORD )retQW;
				}
				else if ( sysFunc->hostReturnSize == 2 )
				{
					*( asQWORD * )retPointer = retQW;
				}
				else if ( sysFunc->hostReturnSize == 3 )
				{
					*( asQWORD * )retPointer			 = retQW;
					*( ( ( asDWORD * )retPointer ) + 2 ) = ( asDWORD )retQW2;
				}
				else
				{
					*( asQWORD * )retPointer			 = retQW;
					*( ( ( asQWORD * )retPointer ) + 1 ) = retQW2;
				}
			}

			// Store the object in the register
			context->regs.objectRegister = retPointer;
		}
	}
	else
	{
		// Store value in valueRegister
		if( sysFunc->hostReturnFloat )
		{
			if( sysFunc->hostReturnSize == 1 )
			{
				*(asDWORD*)&context->regs.valueRegister = GetReturnedFloat();
			}
			else
			{
				context->regs.valueRegister = GetReturnedDouble();
			}
		}
		else if ( sysFunc->hostReturnSize == 1 )
		{
			*( asDWORD * )&context->regs.valueRegister = ( asDWORD )retQW;
		}
		else
		{
			context->regs.valueRegister = retQW;
		}
	}

	if( sysFunc->hasAutoHandles )
	{
		args = context->regs.stackPointer;
		if( callConv >= ICC_THISCALL && !objectPointer )
		{
			args += AS_PTR_SIZE;
		}

		int spos = 0;
		for( n = 0; n < ( int )descr->parameterTypes.GetLength(); n++ )
		{
			if( sysFunc->paramAutoHandles[n] && (*(size_t*)&args[spos] != 0) )
			{
				// Call the release method on the type
				engine->CallObjectMethod( ( void * )*( size_t * )&args[spos], descr->parameterTypes[n].GetObjectType()->beh.release );
				args[spos] = 0;
			}

			if( descr->parameterTypes[n].IsObject() && !descr->parameterTypes[n].IsObjectHandle() && !descr->parameterTypes[n].IsReference() )
			{
				spos += AS_PTR_SIZE;
			}
			else
			{
				spos += descr->parameterTypes[n].GetSizeOnStackDWords();
			}
		}
	}

	return popSize;
}

END_AS_NAMESPACE

#endif // AS_X64_GCC
#endif // AS_MAX_PORTABILITY
