/*
 * Copyright © 2009-2020 Frictional Games
 *
 * This file is part of Amnesia: The Dark Descent.
 *
 * Amnesia: The Dark Descent is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Amnesia: The Dark Descent is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Amnesia: The Dark Descent.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "engine/Interface.h"
#include "graphics/ForgeRenderer.h"
#include "impl/LegacyVertexBuffer.h"
#include "math/MathTypes.h"

#include <assert.h>
#include <stdlib.h>

#include "system/LowLevelSystem.h"
#include "system/Platform.h"

#include "impl/LowLevelGraphicsSDL.h"
#include "impl/SDLFontData.h"

#include "graphics/Bitmap.h"
#include <windowing/NativeWindow.h>

#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#endif

#include "SDL2/SDL_syswm.h"


#ifndef WIN32
	#if defined __ppc__ || defined(__LP64__)
		#define CALLBACK
	#else
		#define CALLBACK __attribute__ ((__stdcall__))
	#endif
#endif

namespace hpl {

	cLowLevelGraphicsSDL::cLowLevelGraphicsSDL()
	{
		mlMultisampling =0;
	}

	cLowLevelGraphicsSDL::~cLowLevelGraphicsSDL()
	{
	}

	bool cLowLevelGraphicsSDL::Init(int alWidth, int alHeight, int alDisplay, int alBpp, int abFullscreen,
		int alMultisampling, eGpuProgramFormat aGpuProgramFormat,const tString& asWindowCaption,
		const cVector2l &avWindowPos)
	{
		mlMultisampling = alMultisampling;


        SetWindowGrab(true);


		//Turn off cursor as default
		ShowCursor(false);

		return true;
	}


	int cLowLevelGraphicsSDL::GetCaps(eGraphicCaps aType)
	{

		return 0;
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsSDL::ShowCursor(bool abX)
	{
		if(auto* window = Interface<window::NativeWindowWrapper>::Get()) {
			if(abX) {
				window->ShowHardwareCursor();
			} else {
				window->HideHardwareCursor();
			}
		}
	}

	//-----------------------------------------------------------------------

    void cLowLevelGraphicsSDL::SetWindowGrab(bool abX)
    {
        if(auto* window = Interface<window::NativeWindowWrapper>::Get()) {
			if(abX) {
				window->GrabCursor();
			} else {
				window->ReleaseCursor();
			}
		}
    }

	void cLowLevelGraphicsSDL::SetRelativeMouse(bool abX)
	{
		if(auto* window = Interface<window::NativeWindowWrapper>::Get()) {
			if(abX) {
				window->ConstrainCursor();
			} else {
				window->UnconstrainCursor();
			}
		}
	}

    void cLowLevelGraphicsSDL::SetWindowCaption(const tString &asName)
    {
		if(auto* window = Interface<window::NativeWindowWrapper>::Get()) {
			window->SetWindowTitle(asName);
		}
    }

    bool cLowLevelGraphicsSDL::GetWindowMouseFocus()
    {
		if(auto* window = Interface<window::NativeWindowWrapper>::Get()) {
			return any(window->GetWindowStatus() & window::WindowStatus::WindowStatusInputMouseFocus);
		}
		return false;
    }

    bool cLowLevelGraphicsSDL::GetWindowInputFocus()
    {
		if(auto* window = Interface<window::NativeWindowWrapper>::Get()) {
			return any(window->GetWindowStatus() & window::WindowStatus::WindowStatusInputFocus);
		}
		return false;
    }

    bool cLowLevelGraphicsSDL::GetWindowIsVisible()
    {
		if(auto* window = Interface<window::NativeWindowWrapper>::Get()) {
			return any(window->GetWindowStatus() & window::WindowStatus::WindowStatusVisible);
		}
		return false;
    }

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsSDL::SetVsyncActive(bool abX, bool abAdaptive)
	{
//         ;
// #if SDL_VERSION_ATLEAST(2, 0, 0)
//         SDL_GL_SetSwapInterval(abX ? (abAdaptive ? -1 : 1) : 0);
// #elif defined(WIN32)
// 		if(WGLEW_EXT_swap_control)
// 		{
// 			wglSwapIntervalEXT(abX ? (abAdaptive ? -1 : 1) : 0);
// 		}
// #elif defined(__linux__) || defined(__FreeBSD__)
// 		if (GLX_SGI_swap_control)
// 		{
// 			GLXSWAPINTERVALPROC glXSwapInterval = (GLXSWAPINTERVALPROC)glXGetProcAddress((GLubyte*)"glXSwapIntervalSGI");
// 			glXSwapInterval(abX ? (abAdaptive ? -1 : 1) : 0);
// 		}
// 		else if (GLX_MESA_swap_control)
// 		{
// 			GLXSWAPINTERVALPROC glXSwapInterval = (GLXSWAPINTERVALPROC)glXGetProcAddress((GLubyte*)"glXSwapIntervalMESA");
// 			glXSwapInterval(abX ? (abAdaptive ? -1 : 1) : 0);
// 		}
// #elif defined(__APPLE__)
// 		CGLContextObj ctx = CGLGetCurrentContext();
// 		GLint swap = abX ? 1 : 0;
// 		CGLSetParameter(ctx, kCGLCPSwapInterval, &swap);
// #endif
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsSDL::SetMultisamplingActive(bool abX)
	{
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsSDL::SetGammaCorrection(float afX)
	{
		mfGammaCorrection = afX;
		if(auto* window = Interface<window::NativeWindowWrapper>::Get()) {
		    window->SetWindowBrightness(afX);
        }
        // SDL_SetWindowBrightness(hpl::entry_sdl::getWindow(), mfGammaCorrection);
	}

	float cLowLevelGraphicsSDL::GetGammaCorrection()
	{
        return Interface<ForgeRenderer>::Get()->GetGamma();

	}

	//-----------------------------------------------------------------------

	cVector2f cLowLevelGraphicsSDL::GetScreenSizeFloat()
	{
		if(auto* window = Interface<window::NativeWindowWrapper>::Get()) {
			auto size = window->GetWindowSize();
			return cVector2f(static_cast<float>(size.x), static_cast<float>(size.y));
		}
		return cVector2f(0,0);
	}

	const cVector2l cLowLevelGraphicsSDL::GetScreenSizeInt()
	{
		if(auto* window = Interface<window::NativeWindowWrapper>::Get()) {
			return window->GetWindowSize();
		}
		return cVector2l(0,0);
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// DATA CREATION
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	iFontData* cLowLevelGraphicsSDL::CreateFontData(const tString &asName)
	{

		return hplNew( cSDLFontData, (asName, this) );
	}


	iTexture* cLowLevelGraphicsSDL::CreateTexture(const tString &asName,eTextureType aType,   eTextureUsage aUsage)
	{
		// BX_ASSERT(false, "deprecated CreateTexture");
		return nullptr;
	}

	//-----------------------------------------------------------------------

	iVertexBuffer* cLowLevelGraphicsSDL::CreateVertexBuffer(eVertexBufferType aType,
															eVertexBufferDrawType aDrawType,
															eVertexBufferUsageType aUsageType,
															int alReserveVtxSize,int alReserveIdxSize)
	{
		return new LegacyVertexBuffer(aDrawType, aUsageType, alReserveVtxSize, alReserveIdxSize);
	}

	//-----------------------------------------------------------------------

	iFrameBuffer* cLowLevelGraphicsSDL::CreateFrameBuffer(const tString& asName)
	{
		// BX_ASSERT(false, "interface is deprecated");
		return nullptr;
	}

	//-----------------------------------------------------------------------

	iDepthStencilBuffer* cLowLevelGraphicsSDL::CreateDepthStencilBuffer(const cVector2l& avSize, int alDepthBits, int alStencilBits)
	{
		// BX_ASSERT(false, "interface is deprecated");
		return nullptr;
	}

	void cLowLevelGraphicsSDL::ClearFrameBuffer(tClearFrameBufferFlag aFlags)
	{
		// BX_ASSERT(false, "interface is deprecated");
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsSDL::SetClearColor(const cColor& aCol){
		// BX_ASSERT(false, "interface is deprecated");
	}
	void cLowLevelGraphicsSDL::SetClearDepth(float afDepth){
		// BX_ASSERT(false, "interface is deprecated");

	}
	void cLowLevelGraphicsSDL::SetClearStencil(int alVal){
		// BX_ASSERT(false, "interface is deprecated");
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsSDL::CopyFrameBufferToTexure(iTexture* apTex, const cVector2l &avPos,
		const cVector2l &avSize, const cVector2l &avTexOffset)
	{
		// BX_ASSERT(false, "interface is deprecated");
	}

	//-----------------------------------------------------------------------

	cBitmap* cLowLevelGraphicsSDL::CopyFrameBufferToBitmap(	const cVector2l &avScreenPos,const cVector2l &avScreenSize)
	{
		// BX_ASSERT(false, "TODO: need to replace this with a helper method no need to couple this");

		return nullptr;
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsSDL::SetCurrentFrameBuffer(iFrameBuffer* apFrameBuffer, const cVector2l &avPos, const cVector2l& avSize)
	{
		// BX_ASSERT(false, "Deprecated");
	}

	void cLowLevelGraphicsSDL::WaitAndFinishRendering()
	{
		// BX_ASSERT(false, "Deprecated");
	}

	void cLowLevelGraphicsSDL::FlushRendering()
	{
		// BX_ASSERT(false, "Deprecated");
	}

	void cLowLevelGraphicsSDL::SwapBuffers()
	{
		// BX_ASSERT(false, "Deprecated");
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// RENDER STATE
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsSDL::SetColorWriteActive(bool abR,bool abG,bool abB,bool abA)
	{
		// BX_ASSERT(false, "interface is deprecated");
	}

	void cLowLevelGraphicsSDL::SetDepthWriteActive(bool abX)
	{
		// BX_ASSERT(false, "interface is deprecated");
	}

	void cLowLevelGraphicsSDL::SetDepthTestActive(bool abX)
	{
	}

	void cLowLevelGraphicsSDL::SetDepthTestFunc(eDepthTestFunc aFunc)
	{
		// BX_ASSERT(false, "interface is deprecated");
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsSDL::SetAlphaTestActive(bool abX)
	{
		// BX_ASSERT(false, "interface is deprecated");
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsSDL::SetAlphaTestFunc(eAlphaTestFunc aFunc,float afRef)
	{
		// BX_ASSERT(false, "interface is deprecated");
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsSDL::SetStencilActive(bool abX)
	{
		// BX_ASSERT(false, "interface is deprecated");
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsSDL::SetStencilWriteMask(unsigned int alMask)
	{
		// BX_ASSERT(false, "interface is deprecated");
	}
	//-----------------------------------------------------------------------

	void cLowLevelGraphicsSDL::SetStencil(eStencilFunc aFunc,int alRef, unsigned int aMask,
		eStencilOp aFailOp,eStencilOp aZFailOp,eStencilOp aZPassOp)
	{
		// BX_ASSERT(false, "interface is deprecated");
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsSDL::SetStencilTwoSide(eStencilFunc aFrontFunc,eStencilFunc aBackFunc,
		int alRef, unsigned int aMask,
		eStencilOp aFrontFailOp,eStencilOp aFrontZFailOp,eStencilOp aFrontZPassOp,
		eStencilOp aBackFailOp,eStencilOp aBackZFailOp,eStencilOp aBackZPassOp)
	{
		// BX_ASSERT(false, "interface is deprecated");
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsSDL::SetCullActive(bool abX)
	{
		// BX_ASSERT(false, "interface is deprecated");
	}

	void cLowLevelGraphicsSDL::SetCullMode(eCullMode aMode)
	{
		// BX_ASSERT(false, "interface is deprecated");
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsSDL::SetScissorActive(bool abX)
	{
		// BX_ASSERT(false, "interface is deprecated");
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsSDL::SetScissorRect(const cVector2l& avPos, const cVector2l& avSize)
	{
		// BX_ASSERT(false, "interface is deprecated");
	}

	//-----------------------------------------------------------------------


	void cLowLevelGraphicsSDL::SetClipPlane(int alIdx, const cPlanef& aPlane)
	{
		// BX_ASSERT(false, "interface is deprecated");
	}
	cPlanef cLowLevelGraphicsSDL::GetClipPlane(int alIdx)
	{
		// BX_ASSERT(false, "interface is deprecated");
		return cPlanef();
	}
	void cLowLevelGraphicsSDL::SetClipPlaneActive(int alIdx, bool abX)
	{
		// BX_ASSERT(false, "interface is deprecated");
	}


	//-----------------------------------------------------------------------

	void cLowLevelGraphicsSDL::SetBlendActive(bool abX)
	{
		// BX_ASSERT(false, "interface is deprecated");
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsSDL::SetBlendFunc(eBlendFunc aSrcFactor, eBlendFunc aDestFactor)
	{
		// BX_ASSERT(false, "interface is deprecated");
	}

	//-----------------------------------------------------------------------


	void cLowLevelGraphicsSDL::SetBlendFuncSeparate(eBlendFunc aSrcFactorColor, eBlendFunc aDestFactorColor,
		eBlendFunc aSrcFactorAlpha, eBlendFunc aDestFactorAlpha)
	{
		// BX_ASSERT(false, "interface is deprecated");
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsSDL::SetPolygonOffsetActive(bool abX)
	{
		// BX_ASSERT(false, "interface is deprecated");

	}


	void cLowLevelGraphicsSDL::SetPolygonOffset(float afBias, float afSlopeScaleBias)
	{
		// BX_ASSERT(false, "interface is deprecated");
	}

	//-----------------------------------------------------------------------


	//////////////////////////////////////////////////////////////////////////
	// MATRIX
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------


	void cLowLevelGraphicsSDL::PushMatrix(eMatrix aMtxType)
	{
		// BX_ASSERT(false, "interface is deprecated");
	}

	//-----------------------------------------------------------------------


	void cLowLevelGraphicsSDL::PopMatrix(eMatrix aMtxType)
	{
		// BX_ASSERT(false, "interface is deprecated");
	}
	//-----------------------------------------------------------------------

	void cLowLevelGraphicsSDL::SetMatrix(eMatrix aMtxType, const cMatrixf& a_mtxA)
	{
		// BX_ASSERT(false, "interface is deprecated");
	}

	//-----------------------------------------------------------------------


	void cLowLevelGraphicsSDL::SetIdentityMatrix(eMatrix aMtxType)
	{
		// BX_ASSERT(false, "interface is deprecated");
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsSDL::SetOrthoProjection(const cVector2f& avSize, float afMin, float afMax)
	{
		// BX_ASSERT(false, "interface is deprecated");
	}

	void cLowLevelGraphicsSDL::SetOrthoProjection(const cVector3f& avMin, const cVector3f& avMax)
	{
		// BX_ASSERT(false, "interface is deprecated");
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// TEXTURE OPERATIONS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------


	void cLowLevelGraphicsSDL::SetTexture(unsigned int alUnit,iTexture* apTex)
	{
		// BX_ASSERT(false, "interface is deprecated");
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsSDL::SetColor(const cColor &aColor)
	{
		// BX_ASSERT(false, "interface is deprecated");
	}

	//-----------------------------------------------------------------------



	//////////////////////////////////////////////////////////////////////////
	// DRAWING
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsSDL::DrawTriangle(tVertexVec& avVtx)
	{
		// BX_ASSERT(false, "interface is deprecated");

	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsSDL::DrawQuad(const cVector3f &avPos,const cVector2f &avSize,const cColor& aColor)
	{
		// BX_ASSERT(false, "interface is deprecated");

	}

	void cLowLevelGraphicsSDL::DrawQuad(const cVector3f &avPos,const cVector2f &avSize,
		const cVector2f &avMinTexCoord,const cVector2f &avMaxTexCoord,
		const cColor& aColor)
	{
		// BX_ASSERT(false, "interface is deprecated");

	}

	void cLowLevelGraphicsSDL::DrawQuad(const cVector3f &avPos,const cVector2f &avSize,
		const cVector2f &avMinTexCoord0,const cVector2f &avMaxTexCoord0,
		const cVector2f &avMinTexCoord1,const cVector2f &avMaxTexCoord1,
		const cColor& aColor)
	{
		// BX_ASSERT(false, "interface is deprecated");

	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsSDL::DrawQuad(const tVertexVec &avVtx)
	{
		// BX_ASSERT(false, "interface is deprecated");

	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsSDL::DrawQuadMultiTex(const tVertexVec &avVtx,const tVector3fVec &avExtraUvs)
	{

		// BX_ASSERT(false, "interface is deprecated");
	}


	//-----------------------------------------------------------------------

	void cLowLevelGraphicsSDL::DrawQuad(const tVertexVec &avVtx, const cColor aCol)
	{
		// BX_ASSERT(false, "interface is deprecated");

	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsSDL::DrawQuad(const tVertexVec &avVtx,const float afZ)
	{
		// BX_ASSERT(false, "interface is deprecated");

	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsSDL::DrawQuad(const tVertexVec &avVtx,const float afZ,const cColor &aCol)
	{
		// BX_ASSERT(false, "interface is deprecated");

	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsSDL::DrawLine(const cVector3f& avBegin, const cVector3f& avEnd, cColor aCol)
	{
		// BX_ASSERT(false, "interface is deprecated");

	}

	void cLowLevelGraphicsSDL::DrawLine(const cVector3f& avBegin, const cColor& aBeginCol, const cVector3f& avEnd, const cColor& aEndCol)
	{
		// BX_ASSERT(false, "interface is deprecated");

	}

	void cLowLevelGraphicsSDL::DrawBoxMinMax(const cVector3f& avMin, const cVector3f& avMax, cColor aCol)
	{
		// BX_ASSERT(false, "interface is deprecated");

	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsSDL::DrawSphere(const cVector3f& avPos, float afRadius, cColor aCol)
	{
		// BX_ASSERT(false, "interface is deprecated");

	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsSDL::DrawSphere(const cVector3f& avPos, float afRadius, cColor aColX, cColor aColY, cColor aColZ)
	{
		// BX_ASSERT(false, "interface is deprecated");

	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsSDL::DrawLineQuad(const cRect2f& aRect, float afZ, cColor aCol)
	{
		// BX_ASSERT(false, "interface is deprecated");
	}

	void cLowLevelGraphicsSDL::DrawLineQuad(const cVector3f &avPos,const cVector2f &avSize, cColor aCol)
	{
		// BX_ASSERT(false, "interface is deprecated");
	}



	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// VERTEX BATCHING
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsSDL::AddVertexToBatch(const cVertex *apVtx)
	{
		// BX_ASSERT(false, "interface is deprecated");
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsSDL::AddVertexToBatch(const cVertex *apVtx, const cVector3f* avTransform)
	{
		// BX_ASSERT(false, "interface is deprecated");
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsSDL::AddVertexToBatch(const cVertex *apVtx, const cMatrixf* aMtx)
	{
		// BX_ASSERT(false, "interface is deprecated");

	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsSDL::AddVertexToBatch_Size2D(const cVertex *apVtx, const cVector3f* avTransform,
		const cColor* apCol,const float& mfW, const float& mfH)
	{
		// BX_ASSERT(false, "interface is deprecated");
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsSDL::AddVertexToBatch_Raw(	const cVector3f& avPos, const cColor &aColor,
		const cVector3f& avTex)
	{
		// BX_ASSERT(false, "interface is deprecated");
	}


	//-----------------------------------------------------------------------

	void cLowLevelGraphicsSDL::AddIndexToBatch(int alIndex)
	{
		// BX_ASSERT(false, "interface is deprecated");

	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsSDL::AddTexCoordToBatch(unsigned int alUnit,const cVector3f *apCoord)
	{
		// BX_ASSERT(false, "interface is deprecated");

	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsSDL::SetBatchTextureUnitActive(unsigned int alUnit,bool abActive)
	{
		// BX_ASSERT(false, "interface is deprecated");

	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsSDL::FlushTriBatch(tVtxBatchFlag aTypeFlags, bool abAutoClear)
	{
		// BX_ASSERT(false, "interface is deprecated");
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsSDL::FlushQuadBatch(tVtxBatchFlag aTypeFlags, bool abAutoClear)
	{
		// BX_ASSERT(false, "interface is deprecated");
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsSDL::ClearBatch()
	{
		// BX_ASSERT(false, "interface is deprecated");
	}

	void cLowLevelGraphicsSDL::SetUpBatchArrays()
	{
		// BX_ASSERT(false, "interface is deprecated");
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsSDL::SetVtxBatchStates(tVtxBatchFlag aFlags)
	{
		// BX_ASSERT(false, "interface is deprecated");
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsSDL::SetMatrixMode(eMatrix mType)
	{
		// BX_ASSERT(false, "interface is deprecated");
	}

}
