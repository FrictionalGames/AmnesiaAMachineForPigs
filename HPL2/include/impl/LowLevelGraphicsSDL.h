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

#ifndef HPL_LOWLEVELGRAPHICS_SDL_H
#define HPL_LOWLEVELGRAPHICS_SDL_H


#include "graphics/LowLevelGraphics.h"
#include "math/MathTypes.h"


namespace hpl {


	class cLowLevelGraphicsSDL : public iLowLevelGraphics
	{
	public:
		cLowLevelGraphicsSDL();
		~cLowLevelGraphicsSDL();

		/////////////////////////////////////////////////////
		/////////////// GENERAL SETUP ///////////////////////
		/////////////////////////////////////////////////////

		[[deprecated("replaced with BGFX")]]
		bool Init(	int alWidth, int alHeight, int alDisplay, int alBpp, int abFullscreen, int alMultisampling,
					eGpuProgramFormat aGpuProgramFormat,const tString& asWindowCaption,
					const cVector2l &avWindowPos);


		[[deprecated("replaced with BGFX")]]
		int GetCaps(eGraphicCaps aType);

		[[deprecated("replaced with BGFX")]]
		void ShowCursor(bool abX);

		[[deprecated("replaced with BGFX")]]
        void SetWindowGrab(bool abX);

		[[deprecated("replaced with BGFX")]]
        void SetRelativeMouse(bool abX);

		[[deprecated("replaced with BGFX")]]
        void SetWindowCaption(const tString &asName);

		[[deprecated("replaced with BGFX")]]
        bool GetWindowMouseFocus();

		[[deprecated("replaced with BGFX")]]
        bool GetWindowInputFocus();

		[[deprecated("replaced with BGFX")]]
        bool GetWindowIsVisible();

		[[deprecated("replaced with BGFX")]]
		bool GetFullscreenModeActive() { return false; }

		[[deprecated("replaced with BGFX")]]
		void SetVsyncActive(bool abX, bool abAdaptive);

		[[deprecated("replaced with BGFX")]]
		void SetMultisamplingActive(bool abX);

		[[deprecated("replaced with BGFX")]]
		void SetGammaCorrection(float afX);
		[[deprecated("replaced with BGFX")]]
		float GetGammaCorrection();

		[[deprecated("replaced with BGFX")]]
		int GetMultisampling(){ return mlMultisampling;}

		[[deprecated("replaced with BGFX")]]
		cVector2f GetScreenSizeFloat();
		[[deprecated("replaced with BGFX")]]
		const cVector2l GetScreenSizeInt();

		/////////////////////////////////////////////////////
		/////////////// DATA CREATION //////////////////////
		/////////////////////////////////////////////////////

		[[deprecated("replaced with BGFX")]]
		iFontData* CreateFontData(const tString &asName);

		[[deprecated("replaced with BGFX")]]
		iTexture* CreateTexture(const tString &asName, eTextureType aType, eTextureUsage aUsage);

		[[deprecated("replaced with BGFX")]]
		iVertexBuffer* CreateVertexBuffer(	eVertexBufferType aType,
											eVertexBufferDrawType aDrawType,
											eVertexBufferUsageType aUsageType,
											int alReserveVtxSize=0,int alReserveIdxSize=0);

		[[deprecated("replaced with BGFX")]]
		iFrameBuffer* CreateFrameBuffer(const tString& asName);
		[[deprecated("replaced with BGFX")]]
		iDepthStencilBuffer* CreateDepthStencilBuffer(const cVector2l& avSize, int alDepthBits, int alStencilBits);

		[[deprecated("replaced with BGFX")]]
		void ClearFrameBuffer(tClearFrameBufferFlag aFlags);

		[[deprecated("replaced with BGFX")]]
		void SetClearColor(const cColor& aCol);
		[[deprecated("replaced with BGFX")]]
		void SetClearDepth(float afDepth);
		[[deprecated("replaced with BGFX")]]
		void SetClearStencil(int alVal);

		[[deprecated("replaced with BGFX")]]
		void CopyFrameBufferToTexure(	iTexture* apTex, const cVector2l &avPos,
									const cVector2l &avSize, const cVector2l &avTexOffset=0);
		[[deprecated("replaced with BGFX")]]
		cBitmap* CopyFrameBufferToBitmap(const cVector2l &avScreenPos=0, const cVector2l &avScreenSize=-1);

		[[deprecated("replaced with BGFX")]]
		void WaitAndFinishRendering();
		[[deprecated("replaced with BGFX")]]
		void FlushRendering();
		[[deprecated("replaced with BGFX")]]
		void SwapBuffers();

		[[deprecated("replaced with BGFX")]]
		void SetCurrentFrameBuffer(iFrameBuffer* apFrameBuffer, const cVector2l &avPos = 0, const cVector2l& avSize = -1);
		[[deprecated("replaced with BGFX")]]
		iFrameBuffer* GetCurrentFrameBuffer() { return nullptr; }


		[[deprecated("replaced with BGFX")]]
		void SetColorWriteActive(bool abR,bool abG,bool abB,bool abA);
		[[deprecated("replaced with BGFX")]]
		void SetDepthWriteActive(bool abX);

		[[deprecated("replaced with BGFX")]]
		void SetCullActive(bool abX);
		[[deprecated("replaced with BGFX")]]
		void SetCullMode(eCullMode aMode);


		[[deprecated("replaced with BGFX")]]
		void SetDepthTestActive(bool abX);
		[[deprecated("replaced with BGFX")]]
		void SetDepthTestFunc(eDepthTestFunc aFunc);

		[[deprecated("replaced with BGFX")]]
		void SetAlphaTestActive(bool abX);
		[[deprecated("replaced with BGFX")]]
		void SetAlphaTestFunc(eAlphaTestFunc aFunc,float afRef);

		[[deprecated("replaced with BGFX")]]
		void SetStencilActive(bool abX);
		[[deprecated("replaced with BGFX")]]
		void SetStencilWriteMask(unsigned int alMask);
		[[deprecated("replaced with BGFX")]]
		void SetStencil(eStencilFunc aFunc,int alRef, unsigned int aMask,
						eStencilOp aFailOp,eStencilOp aZFailOp,eStencilOp aZPassOp);
		[[deprecated("replaced with BGFX")]]
		void SetStencilTwoSide(	eStencilFunc aFrontFunc,eStencilFunc aBackFunc,
								int alRef, unsigned int aMask,
								eStencilOp aFrontFailOp,eStencilOp aFrontZFailOp,eStencilOp aFrontZPassOp,
								eStencilOp aBackFailOp,eStencilOp aBackZFailOp,eStencilOp aBackZPassOp);

		[[deprecated("replaced with BGFX")]]
		void SetScissorActive(bool abX);
		[[deprecated("replaced with BGFX")]]
		void SetScissorRect(const cVector2l& avPos, const cVector2l& avSize);

		[[deprecated("replaced with BGFX")]]
		void SetClipPlane(int alIdx, const cPlanef& aPlane);
		[[deprecated("replaced with BGFX")]]
		cPlanef GetClipPlane(int alIdx);

		[[deprecated("replaced with BGFX")]]
		void SetClipPlaneActive(int alIdx, bool abX);

		[[deprecated("replaced with BGFX")]]
		void SetColor(const cColor &aColor);

		[[deprecated("replaced with BGFX")]]
		void SetBlendActive(bool abX);
		[[deprecated("replaced with BGFX")]]
		void SetBlendFunc(eBlendFunc aSrcFactor, eBlendFunc aDestFactor);

		[[deprecated("replaced with BGFX")]]
		void SetBlendFuncSeparate(	eBlendFunc aSrcFactorColor, eBlendFunc aDestFactorColor,
									eBlendFunc aSrcFactorAlpha, eBlendFunc aDestFactorAlpha);

		[[deprecated("replaced with BGFX")]]
		void SetPolygonOffsetActive(bool abX);
		[[deprecated("replaced with BGFX")]]
		void SetPolygonOffset(float afBias,float afSlopeScaleBias);

		[[deprecated("replaced with BGFX")]]
		void PushMatrix(eMatrix aMtxType);
		[[deprecated("replaced with BGFX")]]
		void PopMatrix(eMatrix aMtxType);
		[[deprecated("replaced with BGFX")]]
		void SetIdentityMatrix(eMatrix aMtxType);

		[[deprecated("replaced with BGFX")]]
		void SetMatrix(eMatrix aMtxType, const cMatrixf& a_mtxA);

		[[deprecated("replaced with BGFX")]]
		void SetOrthoProjection(const cVector2f& avSize, float afMin, float afMax);
		[[deprecated("replaced with BGFX")]]
		void SetOrthoProjection(const cVector3f& avMin, const cVector3f& avMax);

		[[deprecated("replaced with BGFX")]]
		void SetTexture(unsigned int alUnit,iTexture* apTex);

		[[deprecated("replaced with BGFX")]]
		void DrawTriangle(tVertexVec& avVtx);

		[[deprecated("replaced with BGFX")]]
		void DrawQuad(	const cVector3f &avPos,const cVector2f &avSize, const cColor& aColor=cColor(1,1));

		[[deprecated("replaced with BGFX")]]
		void DrawQuad(	const cVector3f &avPos,const cVector2f &avSize,
						const cVector2f &avMinTexCoord,const cVector2f &avMaxTexCoord,
						const cColor& aColor=cColor(1,1));

		[[deprecated("replaced with BGFX")]]
		void DrawQuad(	const cVector3f &avPos,const cVector2f &avSize,
						const cVector2f &avMinTexCoord0,const cVector2f &avMaxTexCoord0,
						const cVector2f &avMinTexCoord1,const cVector2f &avMaxTexCoord1,
						const cColor& aColor=cColor(1,1));

		[[deprecated("replaced with BGFX")]]
		void DrawQuad(const tVertexVec &avVtx);
		[[deprecated("replaced with BGFX")]]
		void DrawQuad(const tVertexVec &avVtx, const cColor aCol);
		[[deprecated("replaced with BGFX")]]
		void DrawQuad(const tVertexVec &avVtx,const float afZ);
		[[deprecated("replaced with BGFX")]]
		void DrawQuad(const tVertexVec &avVtx,const float afZ,const cColor &aCol);
		[[deprecated("replaced with BGFX")]]
		void DrawQuadMultiTex(const tVertexVec &avVtx,const tVector3fVec &avExtraUvs);

		[[deprecated("replaced with BGFX")]]
		void DrawLine(const cVector3f& avBegin, const cVector3f& avEnd, cColor aCol);
		[[deprecated("replaced with BGFX")]]
		void DrawLine(const cVector3f& avBegin, const cColor& aBeginCol, const cVector3f& avEnd, const cColor& aEndCol);

		[[deprecated("replaced with BGFX")]]
		void DrawBoxMinMax(const cVector3f& avMin, const cVector3f& avMax, cColor aCol);
		[[deprecated("replaced with BGFX")]]
		void DrawSphere(const cVector3f& avPos, float afRadius, cColor aCol);
		[[deprecated("replaced with BGFX")]]
		void DrawSphere(const cVector3f& avPos, float afRadius, cColor aColX, cColor aColY, cColor aColZ);

		[[deprecated("replaced with BGFX")]]
		void DrawLineQuad(const cRect2f& aRect, float afZ, cColor aCol);
		[[deprecated("replaced with BGFX")]]
		void DrawLineQuad(const cVector3f &avPos,const cVector2f &avSize, cColor aCol);

		/////////////////////////////////////////////////////
		/////////// VERTEX BATCHING /////////////////////////
		/////////////////////////////////////////////////////

		[[deprecated("replaced with BGFX")]]
		void AddVertexToBatch(const cVertex *apVtx);
		[[deprecated("replaced with BGFX")]]
		void AddVertexToBatch(const cVertex *apVtx, const cVector3f* avTransform);
		[[deprecated("replaced with BGFX")]]
		void AddVertexToBatch(const cVertex *apVtx, const cMatrixf* aMtx);

		[[deprecated("replaced with BGFX")]]
		void AddVertexToBatch_Size2D(const cVertex *apVtx, const cVector3f* avTransform,
										const cColor* apCol,const float& mfW, const float& mfH);

		[[deprecated("replaced with BGFX")]]
		void AddVertexToBatch_Raw(	const cVector3f& avPos, const cColor &aColor,
									const cVector3f& avTex);


		[[deprecated("replaced with BGFX")]]
		void AddTexCoordToBatch(unsigned int alUnit,const cVector3f *apCoord);
		[[deprecated("replaced with BGFX")]]
		void SetBatchTextureUnitActive(unsigned int alUnit,bool abActive);

		[[deprecated("replaced with BGFX")]]
		void AddIndexToBatch(int alIndex);

		[[deprecated("replaced with BGFX")]]
		void FlushTriBatch(tVtxBatchFlag aTypeFlags, bool abAutoClear=true);
		[[deprecated("replaced with BGFX")]]
		void FlushQuadBatch(tVtxBatchFlag aTypeFlags, bool abAutoClear=true);
		[[deprecated("replaced with BGFX")]]
		void ClearBatch();

	private:
		int mlMultisampling;

		float mfGammaCorrection;


		//////////////////////////////////////
		//Matrix helper
		void SetMatrixMode(eMatrix mType);

		//////////////////////////////////////
		//Batch helper
		void SetUpBatchArrays();

		//////////////////////////////////////
		//Vtx helper
		void SetVtxBatchStates(tVtxBatchFlag aFlags);
	};
};
#endif // HPL_LOWLEVELGRAPHICS_SDL_H
