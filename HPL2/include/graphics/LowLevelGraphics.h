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

#pragma once

#include "math/MathTypes.h"
#include "system/SystemTypes.h"
#include "graphics/GraphicsTypes.h"

namespace hpl {

	class iFontData;
	class iGpuShader;
	class iTexture;
	class iVertexBuffer;
	class cBitmap;
	class iFrameBuffer;
	class iDepthStencilBuffer;

	//----------------------------------------

	class iLowLevelGraphics
	{
	public:
		virtual ~iLowLevelGraphics(){}


		/////////////////////////////////////////////////////
		/////////////// GENERAL SETUP ///////////////////////
		/////////////////////////////////////////////////////

		virtual bool Init(	int alWidth, int alHeight, int alDisplay, int alBpp, int abFullscreen, int alMultisampling,
							eGpuProgramFormat aGpuProgramFormat, const tString& asWindowCaption,
							const cVector2l &avWindowPos)=0;

		/**
		 * Get the capabilities of the graphics. Th return value depends on the capability
		 * \return
		 */
		[[deprecated("interface is deprecated")]]
		virtual int GetCaps(eGraphicCaps aType)=0;

		/**
		 * Show the cursor or not. Default is false
		 */
		[[deprecated("interface is deprecated")]]
		virtual void ShowCursor(bool abX)=0;

		[[deprecated("interface is deprecated")]]
		virtual void SetWindowGrab(bool abX)=0;

		[[deprecated("interface is deprecated")]]
		virtual void SetRelativeMouse(bool abX)=0;

		[[deprecated("interface is deprecated")]]
        virtual void SetWindowCaption(const tString& asName)=0;

		[[deprecated("interface is deprecated")]]
        virtual bool GetWindowMouseFocus()=0;

		[[deprecated("interface is deprecated")]]
        virtual bool GetWindowInputFocus()=0;

		[[deprecated("interface is deprecated")]]
        virtual bool GetWindowIsVisible()=0;

		[[deprecated("interface is deprecated")]]
		virtual int GetMultisampling()=0;

		/**
		 * Get Size of screen
		 */
		[[deprecated("interface is deprecated")]]
		virtual cVector2f GetScreenSizeFloat()=0;
		[[deprecated("interface is deprecated")]]
		virtual const cVector2l GetScreenSizeInt()=0;

		/*
		* Get fullscreen mode
		*/
		[[deprecated("interface is deprecated")]]
		virtual bool GetFullscreenModeActive() = 0;

		/**
		 * Set if vsync should be active
		 */
		[[deprecated("interface is deprecated")]]
		virtual void SetVsyncActive(bool abX, bool abAdaptive = false)=0;

		[[deprecated("interface is deprecated")]]
		virtual void SetMultisamplingActive(bool abX)=0;

		[[deprecated("interface is deprecated")]]
		virtual void SetGammaCorrection(float afX)=0;
		[[deprecated("interface is deprecated")]]
		virtual float GetGammaCorrection()=0;

		/////////////////////////////////////////////////////
		/////////////// DATA CREATION ///////////////////////
		/////////////////////////////////////////////////////

		[[deprecated("interface is deprecated")]]
		virtual iFontData* CreateFontData(const tString &asName)=0;

		[[deprecated("interface is deprecated")]]
		virtual iTexture* CreateTexture(const tString &asName,eTextureType aType,  eTextureUsage aUsage)=0;

		[[deprecated("interface is deprecated")]]
		virtual iVertexBuffer* CreateVertexBuffer(	eVertexBufferType aType,
													eVertexBufferDrawType aDrawType,
													eVertexBufferUsageType aUsageType,
													int alReserveVtxSize=0,int alReserveIdxSize=0)=0;

		[[deprecated("interface is deprecated")]]
		virtual iFrameBuffer* CreateFrameBuffer(const tString& asName)=0;

		[[deprecated("interface is deprecated")]]
		virtual iDepthStencilBuffer* CreateDepthStencilBuffer(const cVector2l& avSize, int alDepthBits, int alStencilBits)=0;

		[[deprecated("interface is deprecated")]]
		virtual void ClearFrameBuffer(tClearFrameBufferFlag aFlags)=0;

		[[deprecated("interface is deprecated")]]
		virtual void SetClearColor(const cColor& aCol)=0;

		[[deprecated("interface is deprecated")]]
		virtual void SetClearDepth(float afDepth)=0;

		[[deprecated("interface is deprecated")]]
		virtual void SetClearStencil(int alVal)=0;

		[[deprecated("interface is deprecated")]]
		virtual void FlushRendering()=0;

		[[deprecated("interface is deprecated")]]
		virtual void WaitAndFinishRendering()=0;

		[[deprecated("interface is deprecated")]]
		virtual void SwapBuffers()=0;

		/**
		* Copies the current frame buffer to a texture.
		* \param apTex The texture the context is copied to.
		* \param &avPos The position in conext
		* \param &avSize The size of the screen.
		* \param &avTexOffset The position on the texture.
		*/

		[[deprecated("interface is deprecated")]]
		virtual void CopyFrameBufferToTexure(	iTexture* apTex, const cVector2l &avPos,
												const cVector2l &avSize, const cVector2l &avTexOffset=0)=0;

		/**
		 * Copies a portion of the screen to the full size of a bitmap.
		 * \param &avScreenPos The position on the screen
		 * \param &avScreenSize The size of the portion of the screen. -1 = use full screen size.
		 * \return a bitmap that contains the data. The bitmap is created by the method and must be deleted by the user.
		 */

		[[deprecated("interface is deprecated")]]
		virtual cBitmap* CopyFrameBufferToBitmap(const cVector2l &avScreenPos=0, const cVector2l &avScreenSize=-1)=0;

		/**
		* All further drawing operations are rendered to this texture.
		* \param pTex Texture to render to. NULL = screen (frame buffer)
		*/

		[[deprecated("interface is deprecated")]]
		virtual void SetCurrentFrameBuffer(iFrameBuffer* apFrameBuffer, const cVector2l &avPos = 0, const cVector2l& avSize = -1)=0;

		/**
		* Returns current FrameBuffer
		*
		*/

		[[deprecated("interface is deprecated")]]
		virtual iFrameBuffer* GetCurrentFrameBuffer()=0;

		[[deprecated("interface is deprecated")]]
		virtual void SetColorWriteActive(bool abR,bool abG,bool abB,bool abA)=0;

		[[deprecated("interface is deprecated")]]
		virtual void SetDepthWriteActive(bool abX)=0;

		[[deprecated("interface is deprecated")]]
		virtual void SetCullActive(bool abX)=0;

		[[deprecated("interface is deprecated")]]
		virtual void SetCullMode(eCullMode aMode)=0;


		[[deprecated("interface is deprecated")]]
		virtual void SetDepthTestActive(bool abX)=0;

		[[deprecated("interface is deprecated")]]
		virtual void SetDepthTestFunc(eDepthTestFunc aFunc)=0;


		[[deprecated("interface is deprecated")]]
		virtual void SetAlphaTestActive(bool abX)=0;

		[[deprecated("interface is deprecated")]]
		virtual void SetAlphaTestFunc(eAlphaTestFunc aFunc,float afRef)=0;


		[[deprecated("interface is deprecated")]]
		virtual void SetStencilActive(bool abX)=0;

		[[deprecated("interface is deprecated")]]
		virtual void SetStencilWriteMask(unsigned int alMask)=0;

		[[deprecated("interface is deprecated")]]
		virtual void SetStencil(eStencilFunc aFunc,int alRef, unsigned int aMask,
								eStencilOp aFailOp,eStencilOp aZFailOp,eStencilOp aZPassOp)=0;

		[[deprecated("interface is deprecated")]]
		virtual void SetStencilTwoSide(eStencilFunc aFrontFunc,eStencilFunc aBackFunc,
										int alRef, unsigned int aMask,
										eStencilOp aFrontFailOp,eStencilOp aFrontZFailOp,eStencilOp aFrontZPassOp,
										eStencilOp aBackFailOp,eStencilOp aBackZFailOp,eStencilOp aBackZPassOp)=0;

		[[deprecated("interface is deprecated")]]
		virtual void SetScissorActive(bool abX)=0;

		[[deprecated("interface is deprecated")]]
		virtual void SetScissorRect(const cVector2l& avPos, const cVector2l& avSize)=0;

		[[deprecated("interface is deprecated")]]
		virtual void SetClipPlane(int alIdx, const cPlanef& aPlane)=0;

		[[deprecated("interface is deprecated")]]
		virtual cPlanef GetClipPlane(int alIdx)=0;

		[[deprecated("interface is deprecated")]]
		virtual void SetClipPlaneActive(int alIdx, bool abX)=0;

		[[deprecated("interface is deprecated")]]
		virtual void SetColor(const cColor &aColor)=0;

		[[deprecated("interface is deprecated")]]
		virtual void SetBlendActive(bool abX)=0;

		[[deprecated("interface is deprecated")]]
		virtual void SetBlendFunc(eBlendFunc aSrcFactor, eBlendFunc aDestFactor)=0;
		[[deprecated("interface is deprecated")]]
		virtual void SetBlendFuncSeparate(	eBlendFunc aSrcFactorColor, eBlendFunc aDestFactorColor,
											eBlendFunc aSrcFactorAlpha, eBlendFunc aDestFactorAlpha)=0;

		[[deprecated("interface is deprecated")]]
		virtual void SetPolygonOffsetActive(bool abX)=0;
		[[deprecated("interface is deprecated")]]
		virtual void SetPolygonOffset(float afBias, float afSlopeScaleBias)=0;


		/////////////////////////////////////////////////////
		/////////// MATRIX //////////////////////////////////
		/////////////////////////////////////////////////////

		[[deprecated("interface is deprecated")]]
		virtual void PushMatrix(eMatrix aMtxType)=0;

		[[deprecated("interface is deprecated")]]
		virtual void PopMatrix(eMatrix aMtxType)=0;

		[[deprecated("interface is deprecated")]]
		virtual void SetIdentityMatrix(eMatrix aMtxType)=0;

		[[deprecated("interface is deprecated")]]
		virtual void SetMatrix(eMatrix aMtxType, const cMatrixf& a_mtxA)=0;

		[[deprecated("interface is deprecated")]]
		virtual void SetOrthoProjection(const cVector2f& avSize, float afMin, float afMax)=0;

		[[deprecated("interface is deprecated")]]
		virtual void SetOrthoProjection(const cVector3f& avMin, const cVector3f& avMax)=0;

		/////////////////////////////////////////////////////
		/////////// TEXTURE OPERATIONS ///////////////////////
		/////////////////////////////////////////////////////

		[[deprecated("interface is deprecated")]]
		virtual void SetTexture(unsigned int alUnit,iTexture* apTex)=0;


		/////////////////////////////////////////////////////
		/////////// DRAWING ///////////////////////////////
		/////////////////////////////////////////////////////

		[[deprecated("interface is deprecated")]]
		virtual void DrawTriangle(tVertexVec& avVtx)=0;

		[[deprecated("interface is deprecated")]]
		virtual void DrawQuad(	const cVector3f &avPos,const cVector2f &avSize, const cColor& aColor=cColor(1,1))=0;


		[[deprecated("interface is deprecated")]]
		virtual void DrawQuad(	const cVector3f &avPos,const cVector2f &avSize,
								const cVector2f &avMinTexCoord,const cVector2f &avMaxTexCoord,
								const cColor& aColor=cColor(1,1))=0;

		[[deprecated("interface is deprecated")]]
		virtual void DrawQuad(	const cVector3f &avPos,const cVector2f &avSize,
								const cVector2f &avMinTexCoord0,const cVector2f &avMaxTexCoord0,
								const cVector2f &avMinTexCoord1,const cVector2f &avMaxTexCoord1,
								const cColor& aColor=cColor(1,1))=0;

		[[deprecated("interface is deprecated")]]
		virtual void DrawQuad(const tVertexVec &avVtx)=0;

		[[deprecated("interface is deprecated")]]
		virtual void DrawQuad(const tVertexVec &avVtx, const cColor aCol)=0;

		[[deprecated("interface is deprecated")]]
		virtual void DrawQuad(const tVertexVec &avVtx,const float afZ)=0;

		[[deprecated("interface is deprecated")]]
		virtual void DrawQuad(const tVertexVec &avVtx,const float afZ,const cColor &aCol)=0;

		[[deprecated("interface is deprecated")]]
		virtual void DrawQuadMultiTex(const tVertexVec &avVtx,const tVector3fVec &avExtraUvs)=0;


		[[deprecated("interface is deprecated")]]
		virtual void DrawLine(const cVector3f& avBegin, const cVector3f& avEnd, cColor aCol)=0;

		[[deprecated("interface is deprecated")]]
		virtual void DrawLine(const cVector3f& avBegin, const cColor& aBeginCol, const cVector3f& avEnd, const cColor& aEndCol)=0;

		[[deprecated("interface is deprecated")]]
		virtual void DrawBoxMinMax(const cVector3f& avMin, const cVector3f& avMax, cColor aCol)=0;

		[[deprecated("interface is deprecated")]]
		virtual void DrawSphere(const cVector3f& avPos, float afRadius, cColor aCol)=0;

		[[deprecated("interface is deprecated")]]
		virtual void DrawSphere(const cVector3f& avPos, float afRadius, cColor aColX, cColor aColY, cColor aColZ)=0;


		[[deprecated("interface is deprecated")]]
		virtual void DrawLineQuad(const cRect2f& aRect, float afZ, cColor aCol)=0;
		[[deprecated("interface is deprecated")]]
		virtual void DrawLineQuad(const cVector3f &avPos,const cVector2f &avSize, cColor aCol)=0;

		/////////////////////////////////////////////////////
		/////////// VERTEX BATCHING /////////////////////////
		/////////////////////////////////////////////////////


		[[deprecated("interface is deprecated")]]
		virtual void AddVertexToBatch(const cVertex *apVtx)=0;

		[[deprecated("interface is deprecated")]]
		virtual void AddVertexToBatch(const cVertex *apVtx, const cVector3f* avTransform)=0;

		[[deprecated("interface is deprecated")]]
		virtual void AddVertexToBatch(const cVertex *apVtx, const cMatrixf* aMtx)=0;


		[[deprecated("interface is deprecated")]]
		virtual void AddVertexToBatch_Size2D(const cVertex *apVtx, const cVector3f* avTransform,
									const cColor* apCol,const float& mfW, const float& mfH)=0;

		[[deprecated("interface is deprecated")]]
		virtual void AddVertexToBatch_Raw(	const cVector3f& avPos, const cColor &aColor,
											const cVector3f& avTex)=0;


		[[deprecated("interface is deprecated")]]
		virtual void AddIndexToBatch(int alIndex)=0;

		[[deprecated("interface is deprecated")]]
		virtual void AddTexCoordToBatch(unsigned int alUnit,const cVector3f *apCoord)=0;

		[[deprecated("interface is deprecated")]]
		virtual void SetBatchTextureUnitActive(unsigned int alUnit,bool abActive)=0;

		[[deprecated("interface is deprecated")]]
		virtual void FlushTriBatch(tVtxBatchFlag aTypeFlags, bool abAutoClear=true)=0;

		[[deprecated("interface is deprecated")]]
		virtual void FlushQuadBatch(tVtxBatchFlag aTypeFlags, bool abAutoClear=true)=0;

		[[deprecated("interface is deprecated")]]
		virtual void ClearBatch()=0;

		/////////////////////////////////////////////////////
		/////////// STATIC VARIABLES /////////////////////////
		/////////////////////////////////////////////////////

		[[deprecated("interface is deprecated")]]
		static void SetForceShaderModel3And4Off(bool abX){ mbForceShaderModel3And4Off = abX;}

		[[deprecated("interface is deprecated")]]
		static bool GetForceShaderModel3And4Off(){	return mbForceShaderModel3And4Off;}

	protected:
		static bool mbForceShaderModel3And4Off;
	};
};
