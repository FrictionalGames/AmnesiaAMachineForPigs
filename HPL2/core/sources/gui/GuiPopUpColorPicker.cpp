/*
 * Copyright © 2011-2020 Frictional Games
 * 
 * This file is part of Amnesia: A Machine For Pigs.
 * 
 * Amnesia: A Machine For Pigs is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version. 

 * Amnesia: A Machine For Pigs is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Amnesia: A Machine For Pigs.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "gui/GuiPopUpColorPicker.h"

#include "gui/GuiTypes.h"

#include "system/LowLevelSystem.h"
#include "system/Platform.h"
#include "system/String.h"

#include "math/Math.h"
#include "math/MathTypes.h"

#include "graphics/Graphics.h"
#include "graphics/Color.h"
#include "graphics/LowLevelGraphics.h"
#include "graphics/FontData.h"
#include "graphics/Bitmap.h"
#include "graphics/Texture.h"

#include "gui/Gui.h"
#include "gui/GuiSkin.h"
#include "gui/GuiSet.h"
#include "gui/GuiGfxElement.h"

#include "gui/WidgetCheckBox.h"
#include "gui/WidgetTextBox.h"
#include "gui/WidgetWindow.h"
#include "gui/WidgetButton.h"
#include "gui/WidgetFrame.h"
#include "gui/WidgetSlider.h"
#include "gui/WidgetLabel.h"
#include "gui/WidgetImage.h"

#include "resources/Resources.h"
#include "resources/XmlDocument.h"


namespace hpl {

	//-------------------------------------------------------------------------------

	/////////////////////////////////////////////////////////////////////////////////
	// GRAPHIC PICKER MODE - CONSTRUCTORS
	/////////////////////////////////////////////////////////////////////////////////

	//-------------------------------------------------------------------------------

	iGraphicPickerMode::iGraphicPickerMode(cGuiPopUpColorPicker* apPicker, 
											const tWString& asName, int alSliderParamIndex, 
											const cVector3f& avMaxValues) : mpPicker(apPicker), msName(asName), mlSliderParamIndex(alSliderParamIndex), mvMaxValues(avMaxValues)
	{
		mpBoxBmp = apPicker->mpColorBoxBitmap;
		mpBoxTex = apPicker->mpColorBoxTexture;
		mpSliderBmp = apPicker->mpColorSliderBitmap;
		mpSliderTex = apPicker->mpColorSliderTexture;

		///////////////////////////////////////////////
		// Determine row and column indices
		cVector2l vIndices;
		int j=0;
		for(int i=0; i<3; ++i)
		{
			if(i==mlSliderParamIndex) continue;

			vIndices.v[j] = i;
			++j;
		}

		mlRowIndex = vIndices.x;
		mlColIndex = vIndices.y;
	}

	//-------------------------------------------------------------------------------

	/////////////////////////////////////////////////////////////////////////////////
	// GRAPHIC PICKER MODE - PUBLIC METHODS
	/////////////////////////////////////////////////////////////////////////////////

	//-------------------------------------------------------------------------------

	bool iGraphicPickerMode::IsCurrent()
	{
		return mpPicker->mpCurrentMode==this;
	}

	//-------------------------------------------------------------------------------

	void iGraphicPickerMode::OnSetBox(const cVector2f& avX)
	{
		mvColorMapPos = avX;
	}

	void iGraphicPickerMode::OnSetSlider(float afX)
	{
		RebuildBox();
		
		mfColorSliderPos = afX;

		UpdateSliderMarkerColor();
	}

	//-------------------------------------------------------------------------------

	void iGraphicPickerMode::OnInputEnter(cWidgetTextBox* apInput)
	{
		iWidget* pParent = apInput->GetParent();
		iGraphicPickerMode* pInputMode = static_cast<iGraphicPickerMode*>(pParent->GetUserData());

		SaveSliderValue(GetSliderParamValue());
		pInputMode->OnInputEnterSpecific(apInput);
			
		if(IsCurrent() && SliderValueChanged(GetSliderParamValue()))
		{
			RebuildBox();
			RebuildSlider();
		}

		UpdateMarkers();
	}

	//-------------------------------------------------------------------------------

	void iGraphicPickerMode::UpdateMarkers()
	{
		mvColorMapPos = GetPosInMap();
		mfColorSliderPos = GetPosInSlider();

		UpdateSliderMarkerColor();
	}

	//-------------------------------------------------------------------------------

	bool iGraphicPickerMode::SetHSBValue(int alIndex, float afX, bool abUpdateInput)
	{
		return mpPicker->SetHSBValue(alIndex, afX, abUpdateInput);
	}

	bool iGraphicPickerMode::SetRGBValue(int alIndex, float afX, bool abUpdateInput)
	{
		return mpPicker->SetRGBValue(alIndex, afX, abUpdateInput);
	}

	void iGraphicPickerMode::SetRGB(float afR, float afG, float afB)
	{
		mpPicker->SetRGB(afR, afG, afB);
	}

	//-------------------------------------------------------------------------------

	/////////////////////////////////////////////////////////////////////////////////
	// GRAPHIC PICKER MODE - PROTECTED METHODS
	/////////////////////////////////////////////////////////////////////////////////

	//-------------------------------------------------------------------------------

	void iGraphicPickerMode::OnDrawBox(iWidget* apWidget)
	{
		// Draws the marker on the map
		cVector3f vPos = apWidget->GetGlobalPosition();
		vPos.z += 0.01f;

		const cVector2f& vSize = apWidget->GetSize();

		mpPicker->mpSet->DrawGfx(mpPicker->mpGfxHMarker, vPos+cVector3f(0,mvColorMapPos.y*(vSize.y-1),0), cVector2f(vSize.x,1), cColor(mvColorMapPos.y,1), eGuiMaterial_Diffuse);
		mpPicker->mpSet->DrawGfx(mpPicker->mpGfxVMarker, vPos+cVector3f(mvColorMapPos.x*(vSize.x-1),0,0), cVector2f(1,vSize.y), cColor(1,1), eGuiMaterial_Diffuse);
	}

	//-------------------------------------------------------------------------------

	void iGraphicPickerMode::OnDrawSlider(iWidget* apWidget)
	{
		cGuiGfxElement* pPointer = mpPicker->mpGfxHMarker;
		cVector3f vPos = apWidget->GetGlobalPosition();
		vPos.y += (1.0f-mfColorSliderPos)*apWidget->GetSize().y;
		vPos.z += 0.01f;

		// Draws the marker on the slider
		mpPicker->mpSet->DrawGfx(pPointer, vPos, cVector2f(20,1), mSliderMarkerCol, eGuiMaterial_Alpha);
	}

	//-------------------------------------------------------------------------------

	void iGraphicPickerMode::UpdateSliderMarkerColor()
	{
		unsigned char pixelData[3];
			
		mpSliderBmp->GetPixel(0,0, 
								cVector3l(cMath::FastPositiveFloatToInt(mfColorSliderPos*mpSliderBmp->GetSize().x),0,0), 
								pixelData);

		mSliderMarkerCol.r = 1.0f - UCharColorToFloat(pixelData[0]);
		mSliderMarkerCol.g = 1.0f - UCharColorToFloat(pixelData[1]);
		mSliderMarkerCol.b = 1.0f - UCharColorToFloat(pixelData[2]);
	}

	//-------------------------------------------------------------------------------

	/////////////////////////////////////////////////////////////////////////////////
	// HSB MODE - CONSTRUCTORS
	/////////////////////////////////////////////////////////////////////////////////

	//-------------------------------------------------------------------------------

	cHSBMode::cHSBMode(cGuiPopUpColorPicker* apPicker, int alIndex) : iGraphicPickerMode(apPicker, _W("HSB"), alIndex, cVector3f(360.0f, 1.0f, 1.0f))
	{
	}

	//-------------------------------------------------------------------------------

	/////////////////////////////////////////////////////////////////////////////////
	// HSB MODE - PUBLIC METHODS
	/////////////////////////////////////////////////////////////////////////////////

	//-------------------------------------------------------------------------------

	void cHSBMode::OnSetBox(const cVector2f& avPos)
	{
		cVector2f vMapValues = cVector2f(mvMaxValues.v[mlRowIndex]*avPos.x, 
			mvMaxValues.v[mlColIndex]*(1-avPos.y));

		bool bUpdateBox = false;

		if(mpPicker->GetHSB().v[mlRowIndex]!=vMapValues.x)
		{
			bUpdateBox = true;
			SetHSBValue(mlRowIndex, vMapValues.x);
		}

		if(mpPicker->GetHSB().v[mlColIndex]!=vMapValues.y)
		{
			bUpdateBox = true;
			SetHSBValue(mlColIndex, vMapValues.y);
		}

		if(bUpdateBox)	iGraphicPickerMode::OnSetBox(avPos);
	}

	//-------------------------------------------------------------------------------

	void cHSBMode::OnSetSlider(float afValue)
	{
		float fValue = mvMaxValues.v[mlSliderParamIndex]*afValue;

		if(fValue==mpPicker->GetHSB().v[mlSliderParamIndex]) return;

		if(SetHSBValue(mlSliderParamIndex, fValue))
			iGraphicPickerMode::OnSetSlider(afValue);
	}

	//-------------------------------------------------------------------------------

	void cHSBMode::OnInputEnterSpecific(cWidgetTextBox* apInput)
	{
		int lIndex = apInput->GetUserValue();
		float fValue = apInput->GetNumericValue()* mvMaxValues.v[lIndex]/apInput->GetUpperBound();
			
		SetHSBValue(lIndex, fValue, false);
	}

	//-------------------------------------------------------------------------------

	void cHSBMode::RebuildBox()
	{
		/////////////////////////////////////////////////////////////////////////////////
		// Builds a color map for this HSB mode. If the slider param is H,
		//  the map will be built on S and B, if slider is S, on H and B and so on

		const cVector3l& vSize = mpBoxBmp->GetSize();
		cVector3f vHSB;
		cVector3f vStepAmounts = cVector3f(mvMaxValues.x/float(vSize.x-1), mvMaxValues.y/float(vSize.x-1), mvMaxValues.z/float(vSize.x-1));
		cVector2f vStep;
		cVector2l vIndices;
		cColor temp;
		unsigned char byteArray[4];
		byteArray[3] = 255;

		////////////////////////////////////////////////
		// Sets up initial values and step sizes
		vHSB.v[mlSliderParamIndex] = mpPicker->GetHSB().v[mlSliderParamIndex];
		vHSB.v[mlRowIndex] = 0.0f;
		vHSB.v[mlColIndex] = 0.0f;

		vStep.x = vStepAmounts.v[mlRowIndex];
		vStep.y = vStepAmounts.v[mlColIndex];
		
		////////////////////////////////////////////////////////////////
		// Build the helper bitmap
		for(int y=vSize.y-1; y>=0; --y)
		{
			for(int x=0; x<vSize.x; ++x)
			{
				// Update temp color
				cMath::HSBToRGBHelper(vHSB, temp);
				
				// Convert to byte array
				byteArray[0] = FloatColorToUChar(temp.r);
				byteArray[1] = FloatColorToUChar(temp.g);
				byteArray[2] = FloatColorToUChar(temp.b);

				// Set to bitmap
				mpBoxBmp->SetPixel(0, 0, cVector3l(x, y, 0), byteArray);

				vHSB.v[mlRowIndex] += vStep.x;
			}
			
			vHSB.v[mlRowIndex] = 0.0f;
			vHSB.v[mlColIndex] += vStep.y;
		}
		
		////////////////////////////////////////////////////////////
		// Dump helper bitmap onto texture
		mpBoxTex->SetRawData(0, 0, vSize, ePixelFormat_RGB, mpBoxBmp->GetData(0, 0)->mpData);
	}

	//-------------------------------------------------------------------------------

	void cHSBMode::RebuildSlider()
	{
		const cVector3l& vSize = mpSliderBmp->GetSize();
		cVector3f vHSB;
		cVector3f vStepAmounts = cVector3f(mvMaxValues.x/float(vSize.x-1), mvMaxValues.y/float(vSize.x-1), mvMaxValues.z/float(vSize.x-1));
		
		cColor temp;
		unsigned char byteArray[4];
		byteArray[3] = 255;

		vHSB.v[mlSliderParamIndex] = 0.0f;
		vHSB.v[mlRowIndex] = mvMaxValues.v[mlRowIndex];
		vHSB.v[mlColIndex] = mvMaxValues.v[mlColIndex];

		float step = vStepAmounts.v[mlSliderParamIndex];
		
		bool bHue = mlSliderParamIndex==0;
		if(bHue)
		{
			/////////////////////////////////////////////////////////////////
			// If Hue, display a nice and wonderful rainbow like slider
			for(int x=0; x<vSize.x; ++x)
			{			
				cMath::HSBToRGBHelper(vHSB, temp);

				byteArray[0] = FloatColorToUChar(temp.r);
				byteArray[1] = FloatColorToUChar(temp.g);
				byteArray[2] = FloatColorToUChar(temp.b);

				mpSliderBmp->SetPixel(0, 0, cVector3l(x, 0, 0), byteArray);

				vHSB.v[mlSliderParamIndex] += step;
			}
		}
		else
		{
			/////////////////////////////////////////////////////////////////
			// Else, a gray scale showing amount of each param set is enough
			for(int x=0; x<vSize.x; ++x)
			{
				byteArray[0] = byteArray[1] = byteArray[2] = FloatColorToUChar(vHSB.v[mlSliderParamIndex]);

				mpSliderBmp->SetPixel(0, 0, cVector3l(x, 0, 0), byteArray);

				vHSB.v[mlSliderParamIndex] += step;
			}
		}
		

		////////////////////////////////////////////////////////////
		// Dump helper bitmap onto texture
		mpSliderTex->SetRawData(0, 0, vSize, ePixelFormat_RGB, mpSliderBmp->GetData(0, 0)->mpData);
	}

	//-------------------------------------------------------------------------------

	/////////////////////////////////////////////////////////////////////////////////
	// HSB MODE - PROTECTED METHODS
	/////////////////////////////////////////////////////////////////////////////////

	//-------------------------------------------------------------------------------

	float cHSBMode::GetSliderParamValue()
	{
		return mpPicker->GetHSB().v[mlSliderParamIndex];
	}

	//-------------------------------------------------------------------------------

	cVector2f cHSBMode::GetPosInMap()
	{
		const cVector3f& vHSB = mpPicker->GetHSB();
		return cVector2f(vHSB.v[mlRowIndex]/mvMaxValues.v[mlRowIndex], 
							1.0f-vHSB.v[mlColIndex]/mvMaxValues.v[mlColIndex]);
	}

	float cHSBMode::GetPosInSlider()
	{
		const cVector3f& vHSB = mpPicker->GetHSB();
		return vHSB.v[mlSliderParamIndex]/mvMaxValues.v[mlSliderParamIndex];
	}

	//-------------------------------------------------------------------------------

	/////////////////////////////////////////////////////////////////////////////////
	// RGB MODE - CONSTRUCTORS
	/////////////////////////////////////////////////////////////////////////////////

	//-------------------------------------------------------------------------------

	cRGBMode::cRGBMode(cGuiPopUpColorPicker* apPicker, int alSliderParamIndex) : iGraphicPickerMode(apPicker, _W("RGB"), alSliderParamIndex, cVector3f(1.0f))
	{
	}

	//-------------------------------------------------------------------------------

	/////////////////////////////////////////////////////////////////////////////////
	// RGB MODE - PUBLIC METHODS
	/////////////////////////////////////////////////////////////////////////////////

	//-------------------------------------------------------------------------------

	void cRGBMode::OnSetBox(const cVector2f& avPos)
	{
		cVector3l vSize = mpBoxBmp->GetSize()-1;
		cVector3l vPos = cVector3l(int(avPos.x*vSize.x), int(avPos.y*vSize.y), 0);

		unsigned char byteArray[3];
		mpBoxBmp->GetPixel(0,0, vPos, byteArray);

		SetRGB( UCharColorToFloat(byteArray[0]),
				UCharColorToFloat(byteArray[1]),
				UCharColorToFloat(byteArray[2]));

		iGraphicPickerMode::OnSetBox(avPos);
	}

	void cRGBMode::OnSetSlider(float afValue)
	{
		afValue = mvMaxValues.v[mlSliderParamIndex]*afValue;

		if(afValue==mpPicker->GetColor().v[mlSliderParamIndex]) return;

		if(SetRGBValue(mlSliderParamIndex, afValue))
			iGraphicPickerMode::OnSetSlider(afValue);
	}

	//-------------------------------------------------------------------------------

	/////////////////////////////////////////////////////////////////////////////////
	// RGB MODE - PROTECTED METHODS
	/////////////////////////////////////////////////////////////////////////////////

	//-------------------------------------------------------------------------------

	void cRGBMode::OnInputEnterSpecific(cWidgetTextBox* apInput)
	{
		int lIndex = apInput->GetUserValue();
		float fValue = apInput->GetNumericValue() * mvMaxValues.v[mlSliderParamIndex]/apInput->GetUpperBound();
			
		SetRGBValue(lIndex, fValue, false);
	}

	//-------------------------------------------------------------------------------

	void cRGBMode::RebuildBox()
	{
		const cVector3l& vSize = mpBoxBmp->GetSize();
		cVector3f vRGB;
		float fStep = 1.0f/float(vSize.x-1);
		cColor temp;
		unsigned char byteArray[4];
		byteArray[3] = 255;
		
		vRGB.v[mlSliderParamIndex] = mpPicker->GetColor().v[mlSliderParamIndex];
		vRGB.v[mlRowIndex] = 0.0f;
		vRGB.v[mlColIndex] = 0.0f;
				
		for(int y=vSize.y-1; y>=0; --y)
		{
			for(int x=0; x<vSize.x; ++x)
			{
				byteArray[0] = FloatColorToUChar(vRGB.x);
				byteArray[1] = FloatColorToUChar(vRGB.y);
				byteArray[2] = FloatColorToUChar(vRGB.z);

				mpBoxBmp->SetPixel(0, 0, cVector3l(x, y, 0), byteArray);

				vRGB.v[mlRowIndex] += fStep;
			}
			
			vRGB.v[mlRowIndex] = 0.0f;
			vRGB.v[mlColIndex] += fStep;
		}
				
		mpBoxTex->SetRawData(0, 0, vSize, ePixelFormat_RGB, mpBoxBmp->GetData(0, 0)->mpData);
	}

	//-------------------------------------------------------------------------------

	void cRGBMode::RebuildSlider()
	{
		const cVector3l& vSize = mpSliderBmp->GetSize();
		cVector3f vRGB;
		unsigned char byteArray[4];
		byteArray[3] = 255;
		float step = 1.0f/float(vSize.x-1);
				
		vRGB.v[mlSliderParamIndex] = 0.0f;
		vRGB.v[mlRowIndex] = 0.0f;
		vRGB.v[mlColIndex] = 0.0f;
		
		for(int x=0; x<vSize.x; ++x)
		{		
			byteArray[0] = FloatColorToUChar(vRGB.x);
			byteArray[1] = FloatColorToUChar(vRGB.y);
			byteArray[2] = FloatColorToUChar(vRGB.z);


			mpSliderBmp->SetPixel(0, 0, cVector3l(x, 0, 0), byteArray);
						
			vRGB.v[mlSliderParamIndex] += step;
		}
				
		mpSliderTex->SetRawData(0, 0, vSize, ePixelFormat_RGB, mpSliderBmp->GetData(0, 0)->mpData);
	}

	//-------------------------------------------------------------------------------

	float cRGBMode::GetSliderParamValue()
	{
		return mpPicker->GetColor().v[mlSliderParamIndex];
	}

	//-------------------------------------------------------------------------------

	cVector2f cRGBMode::GetPosInMap()
	{
		const cColor& col = mpPicker->GetColor();
		return cVector2f(col.v[mlRowIndex], 1.0f-col.v[mlColIndex]);
	}

	float cRGBMode::GetPosInSlider()
	{
		const cColor& col = mpPicker->GetColor();
		return col.v[mlSliderParamIndex];
	}

	//-------------------------------------------------------------------------------

	/////////////////////////////////////////////////////////////////////////////////
	// COLOR PICKER - CONSTRUCTORS
	/////////////////////////////////////////////////////////////////////////////////

	//-------------------------------------------------------------------------------

	static cVector2f gvInputSize = cVector2f(55, 25);
	static cVector2f gvInputFontSize = cVector2f(12);
	static cVector3f gvInputOffset = cVector3f(30, -5, 0);

	size_t cGuiPopUpColorPicker::mlRecentColorNum = 10;
	tColorList cGuiPopUpColorPicker::mlstRecentColors = tColorList(mlRecentColorNum);

	//-------------------------------------------------------------------------------

	cGuiPopUpColorPicker::cGuiPopUpColorPicker(cGraphics *apGraphics,
											   cGuiSet* apSet, 
											   cColor* apDestColor, 
											   void *apCallbackObject, tGuiCallbackFunc apCallback,
											   void *apUpdateColorCallbackObject, tGuiCallbackFunc apUpdateColorCallback) : iGuiPopUp(apSet, true, cVector2f(600,350))
	{
		mpCallbackObject = apCallbackObject;
		mpCallback = apCallback;
		mpGraphics = apGraphics;

		SetUpdateColorCallback(apUpdateColorCallbackObject, apUpdateColorCallback);

		mpCurrentMode = NULL;

		cGraphics* pGfx = mpGraphics;
		iLowLevelGraphics* pLowLevelGfx = pGfx->GetLowLevel();

		////////////////////////////////////////////////////////////
		// Create helper bitmaps and textures

		// Color map
		mpColorBoxBitmap = hplNew(cBitmap,());
		mpColorBoxBitmap->SetUpData(1,1);
		mpColorBoxBitmap->CreateData(cVector3l(256, 256, 1), ePixelFormat_RGB, 0, 0);
		mpColorBoxBitmap->Clear(cColor(1,1,1,1), 0, 0);

		mpColorBoxTexture = pLowLevelGfx->CreateTexture("ColorBox", eTextureType_2D, eTextureUsage_Normal);
		mpColorBoxTexture->SetUseMipMaps(false);
		mpColorBoxTexture->CreateFromBitmap(mpColorBoxBitmap);
		mpColorBoxTexture->SetWrapSTR(eTextureWrap_Clamp);
		mpColorBoxTexture->SetFilter(eTextureFilter_Nearest);
		
		// Color slider
		mpColorSliderBitmap = hplNew(cBitmap,());
		mpColorSliderBitmap->SetUpData(1,1);
		mpColorSliderBitmap->CreateData(cVector3l(256, 1, 1) , ePixelFormat_RGB, 0, 0);
		mpColorSliderBitmap->Clear(cColor(1,1,1,1), 0, 0);

		mpColorSliderTexture = pLowLevelGfx->CreateTexture("ColorSlider", eTextureType_1D, eTextureUsage_Normal);
		mpColorSliderTexture->SetUseMipMaps(false);
		mpColorSliderTexture->CreateFromBitmap(mpColorSliderBitmap);
		mpColorSliderTexture->SetWrapSTR(eTextureWrap_Clamp);
		mpColorSliderTexture->SetFilter(eTextureFilter_Nearest);
		
		////////////////////////////////////////
		// Set up target color
		mpDestColor = apDestColor;
		if(mpDestColor)	mColor = *mpDestColor;
		else			mColor = cColor(1);

		mfAlpha = mColor.a;
		mfOldAlpha = mfAlpha;
		mbShowTransPreview = true;

		////////////////////////////////////////
		// Build all GUI
		cGui* pGui = mpSet->GetGui();
		mpGfxBGPattern = pGui->CreateGfxImage("gui_def_colorpicker_bgpattern.tga", eGuiMaterial_Diffuse);
		mpGfxColorPointer = pGui->CreateGfxImage("gui_def_colorpicker_pointer.tga", eGuiMaterial_Alpha);
		mpGfxSliderPointer = pGui->CreateGfxImage("gui_def_colorpicker_pointer_slider.tga", eGuiMaterial_Alpha);

		// Build horizontal line marker
		mpGfxHMarker = pGui->CreateGfxFilledRect(cColor(1,1), eGuiMaterial_Diffuse);

		// Build vertical line marker
		{
			unsigned char vByteArray[256];
			float fStep = 1.0f/255.0f;
			for(int i=0; i<256; ++i)
			{
				vByteArray[i] = FloatColorToUChar(1.0f-i*fStep);
			}

			iTexture* pHoriMarkerTexture = mpGraphics->GetLowLevel()->CreateTexture("AlphaSlider", eTextureType_1D, eTextureUsage_Normal);
			pHoriMarkerTexture->CreateFromRawData(cVector3l(256,0,0), ePixelFormat_Luminance, vByteArray);

			mpGfxVMarker = mpSet->GetGui()->CreateGfxFilledRect(cColor(1,1), eGuiMaterial_Diffuse);
			mpGfxVMarker->AddTexture(pHoriMarkerTexture, cVector2f(1,0), cVector2f(1,1), cVector2f(0,1), cVector2f(0,0));
			mpGfxVMarker->SetDestroyTexture(true);
		}
		
		Init();
		
		mbImgPressed = false;
	}

    //-------------------------------------------------------------------------------

	cGuiPopUpColorPicker::~cGuiPopUpColorPicker()
	{
		cGraphics* pGfx = mpGraphics;
		
		hplDelete(mpColorBoxBitmap);
		pGfx->DestroyTexture(mpColorBoxTexture);

		hplDelete(mpColorSliderBitmap);
		pGfx->DestroyTexture(mpColorSliderTexture);

		cGui* pGui = mpSet->GetGui();
		pGui->DestroyGfx(mpGfxBGPattern);
		pGui->DestroyGfx(mpGfxHMarker);
		pGui->DestroyGfx(mpGfxVMarker);
		pGui->DestroyGfx(mpGfxColorPointer);
		pGui->DestroyGfx(mpGfxSliderPointer);

		for(size_t i=0; i<mvPickerModeSwitches.size(); ++i)
		{
			iWidget* pCBMode = mvPickerModeSwitches[i];
			iGraphicPickerMode* pMode = static_cast<iGraphicPickerMode*>(pCBMode->GetUserData());

			hplDelete(pMode);
		}
	}

	//-------------------------------------------------------------------------------
	
	/////////////////////////////////////////////////////////////////////////////////
	// COLOR PICKER - PUBLIC METHODS
	/////////////////////////////////////////////////////////////////////////////////

	//-------------------------------------------------------------------------------

	void cGuiPopUpColorPicker::SetColor(const cColor& aX)
	{
		if(mColor==aX && mfAlpha==aX.a) return;

		mColor = aX;
		mfAlpha = aX.a;

		UpdateRGBInputs(true);
		UpdateColorRGB();

		mpCurrentMode->RebuildBox();
		mpCurrentMode->RebuildSlider();
		mpCurrentMode->UpdateMarkers();		
	}

	//-------------------------------------------------------------------------------

	const cColor& cGuiPopUpColorPicker::GetOldColor()
	{
		return mpFPreviousColor->GetBackGroundColor();
	}

	//-------------------------------------------------------------------------------
	
	void cGuiPopUpColorPicker::SetUpdateColorCallback(void *apCallbackObject, tGuiCallbackFunc apCallback)
	{
		mpUpdateCallbackObject = apCallbackObject;
		mpUpdateCallback = apCallback;
	}

	//-------------------------------------------------------------------------------

	void cGuiPopUpColorPicker::LoadRecentColorList(cXmlElement* apElem)
	{
		mlstRecentColors.clear();
		for(int i=1; i<=10; ++i)
		{
			cColor c = apElem->GetAttributeColor("RecentColor"+cString::ToString(i, 1), cColor(0,1));
			mlstRecentColors.push_back(c);			
		}
	}

	void cGuiPopUpColorPicker::SaveRecentColorList(cXmlElement* apElem)
	{
		int i=1;
		tColorListIt it = mlstRecentColors.begin();
		for(;it!=mlstRecentColors.end(); ++it)
		{
			const cColor& c = *it;

			apElem->SetAttributeColor("RecentColor"+cString::ToString(i, 1), c);
		}
	}

	//-------------------------------------------------------------------------------
	
	/////////////////////////////////////////////////////////////////////////////////
	// COLOR PICKER - PROTECTED METHODS
	/////////////////////////////////////////////////////////////////////////////////

	//-------------------------------------------------------------------------------

	bool cGuiPopUpColorPicker::SetHSBValue(int alIdx, float afX, bool abUpdateInput)
	{
		if(mvHSB.v[alIdx]==afX) return false;

		mvHSB.v[alIdx] = afX;
		mvHSBValueUpdated[alIdx] = abUpdateInput;

		UpdateHSBInputs();
		UpdateColorHSB();

		return true;
	}

	//-------------------------------------------------------------------------------

	bool cGuiPopUpColorPicker::SetRGBValue(int alIdx, float afX, bool abUpdateInput)
	{
		if(mColor.v[alIdx]==afX) return false;

		mColor.v[alIdx] = afX;
		mvRGBValueUpdated[alIdx] = abUpdateInput;

		UpdateRGBInputs();
		UpdateColorRGB();

		return true;
	}

	void cGuiPopUpColorPicker::SetRGB(float afR, float afG, float afB)
	{
		if(mColor.r==afR && mColor.g==afG && mColor.b==afB) 
			return;

		mColor.r = afR;
		mColor.g = afG;
		mColor.b = afB;

		UpdateRGBInputs(true);
		UpdateColorRGB();
	}

	//-------------------------------------------------------------------------------

	void cGuiPopUpColorPicker::CreateHSBInputs(iWidget* apParent, cVector3f& avPos)
	{
		tFloatVec vMin, vMax;
		tColorPickerModeVec vModes;
		vMin.resize(3);
		vMax.resize(3);

		vMin[0] = 0;	vMax[0] = 360;
		vMin[1] = 0;	vMax[1] = 100;
		vMin[2] = 0;	vMax[2] = 100;

		vModes.push_back(hplNew(cHSBMode,(this, 0)));
		vModes.push_back(hplNew(cHSBMode,(this, 1)));
		vModes.push_back(hplNew(cHSBMode,(this, 2)));

		CreateInputs(apParent, avPos, kGuiCallback(Color_InputValueEnter), vMin, vMax, mvHSBInputs, mvHSBValueUpdated, vModes);
	}

	//-------------------------------------------------------------------------------

	void cGuiPopUpColorPicker::CreateRGBInputs(iWidget* apParent, cVector3f& avPos)
	{
		tColorPickerModeVec vModes;
		
		vModes.push_back(hplNew(cRGBMode,(this, 0)));
		vModes.push_back(hplNew(cRGBMode,(this, 1)));
		vModes.push_back(hplNew(cRGBMode,(this, 2)));
		
		CreateInputs(apParent, avPos, kGuiCallback(Color_InputValueEnter), tFloatVec(3, 0.0f), tFloatVec(3, 255.0f), mvRGBInputs, mvRGBValueUpdated, vModes);
	}

	//-------------------------------------------------------------------------------

	void cGuiPopUpColorPicker::CreateInputs(iWidget* apParent, cVector3f& avPos, tGuiCallbackFunc apCallback,
											 const tFloatVec& avMin, const tFloatVec& avMax, std::vector<cWidgetTextBox*>& avContainer, tBoolVec& avUpdatedContainer, tColorPickerModeVec avModes)
	{
		cVector3f vPos = avPos;

		for(size_t i=0; i<avModes.size(); ++i)
		{
			////////////////////////////////////////////////////////////////////////////////////////
			// Checkbox / radio button wannabe, sets the contained mode to current if checked
			iGraphicPickerMode* pMode = avModes[i];
			cWidgetCheckBox* pCBModeSelector = mpSet->CreateWidgetCheckBox(vPos, -1, tWString(1,pMode->GetSubname()), apParent);
			pCBModeSelector->SetDefaultFontSize(gvInputFontSize);
			pCBModeSelector->AddCallback(eGuiMessage_CheckChange, this, kGuiCallback(PickerMode_OnSelect));
			pCBModeSelector->SetUserData(pMode);
			pCBModeSelector->SetUserValue(i);
			mvPickerModeSwitches.push_back(pCBModeSelector);
			
			////////////////////////////////////////////////////////////////////////////////////////
			// Textbox: Amount of this param
			cWidgetTextBox* pInput = mpSet->CreateWidgetTextBox(gvInputOffset, gvInputSize, _W(""), pCBModeSelector, eWidgetTextBoxInputType_Numeric, 1);
			pInput->SetDefaultFontSize(gvInputFontSize);
			pInput->SetDecimals(0);
			pInput->SetLowerBound(true, avMin[i]);
			pInput->SetUpperBound(true, avMax[i]);
			pInput->SetUserValue(i);
			if(apCallback) pInput->AddCallback(eGuiMessage_TextBoxEnter, this, apCallback);

			avContainer.push_back(pInput);
			avUpdatedContainer.push_back(true);

			vPos.x = avPos.x;
			vPos.y += gvInputSize.y+5.0f;
		}

		avPos = vPos;
	}

	//-------------------------------------------------------------------------------

	bool cGuiPopUpColorPicker::Button_Pressed(iWidget* apWidget, const cGuiMessageData &aData)
	{
		/////////////////////////////
		// Color picking tool ???
		if(apWidget == mvButtons[0])
		{

		}
		/////////////////////////////
		// Accept button
		if(apWidget == mvButtons[1])
		{
			AcceptPickedColor();
			SelfDestruct();
		}
		/////////////////////////////
		// Cancel button
		if(apWidget == mvButtons[2])
		{
			Window_OnClose(apWidget, aData);
		}
		return true;
	}
	kGuiCallbackDeclaredFuncEnd(cGuiPopUpColorPicker, Button_Pressed);

	//-------------------------------------------------------------------------------

	bool cGuiPopUpColorPicker::ColorFrame_OnDraw(iWidget* apWidget, const cGuiMessageData& aData)
	{
		cWidgetFrame* pFrame = static_cast<cWidgetFrame*>(apWidget);
		cVector3f vPos = pFrame->GetGlobalPosition();
		vPos.z += pFrame->GetBackgroundZ() - 0.1f;

		mpSet->DrawGfx(mpGfxBGPattern, vPos, pFrame->GetSize());

		return true;
	}
	kGuiCallbackDeclaredFuncEnd(cGuiPopUpColorPicker, ColorFrame_OnDraw);

	//-------------------------------------------------------------------------------

	bool cGuiPopUpColorPicker::RecentColor_OnMouseDown(iWidget* apWidget, const cGuiMessageData& aData)
	{
		cColor col;

		cWidgetFrame* pFrame = static_cast<cWidgetFrame*>(apWidget);
		cColor* pColor = static_cast<cColor*>(apWidget->GetUserData());
		if(pColor)
			col = *pColor;
		else
			col = mpFPreviousColor->GetBackGroundColor();

		SetColor(col);
		
		return true;
	}
	kGuiCallbackDeclaredFuncEnd(cGuiPopUpColorPicker, RecentColor_OnMouseDown);

	//-------------------------------------------------------------------------------

	bool cGuiPopUpColorPicker::Color_InputValueEnter(iWidget* apWidget, const cGuiMessageData& aData)
	{
		cWidgetTextBox* pInput = static_cast<cWidgetTextBox*>(apWidget);
		mpCurrentMode->OnInputEnter(pInput);
		
		return true;
	}
	kGuiCallbackDeclaredFuncEnd(cGuiPopUpColorPicker, Color_InputValueEnter);

	//-------------------------------------------------------------------------------

	bool cGuiPopUpColorPicker::Alpha_InputValueEnter(iWidget* apWidget, const cGuiMessageData& aData)
	{
		float fValue = mpInpAlpha->GetNumericValue();

		mfAlpha = fValue/255.0f;

		UpdateColorRGB();

		return true;
	}
	kGuiCallbackDeclaredFuncEnd(cGuiPopUpColorPicker, Alpha_InputValueEnter);

	//-------------------------------------------------------------------------------

	bool cGuiPopUpColorPicker::Alpha_Toggle(iWidget* apWidget, const cGuiMessageData& aData)
	{
		UpdateColor();
		mbAlphaValueUpdated = true;
		UpdateRGBInputs();
		mpInpAlpha->SetEnabled(mpCBAlpha->IsChecked());

		return true;
	}
	kGuiCallbackDeclaredFuncEnd(cGuiPopUpColorPicker, Alpha_Toggle);

	//-------------------------------------------------------------------------------

	bool cGuiPopUpColorPicker::PickerMode_OnSelect(iWidget* apWidget, const cGuiMessageData& aData)
	{
		iGraphicPickerMode* pMode = static_cast<iGraphicPickerMode*>(apWidget->GetUserData());
		if(mpCurrentMode!=pMode)
		{
			for(size_t i=0; i<mvPickerModeSwitches.size(); ++i)
			{
				cWidgetCheckBox* pSwitch = mvPickerModeSwitches[i];

				pSwitch->SetChecked(pSwitch==apWidget, false);
			}

			mpCurrentMode = pMode;
			mpCurrentMode->RebuildSlider();
			mpCurrentMode->RebuildBox();
			mpCurrentMode->UpdateMarkers();
		}
		else
		{
			cWidgetCheckBox* pSwitch = static_cast<cWidgetCheckBox*>(apWidget);

			pSwitch->SetChecked(true, false);
		}

		return true;
	}
	kGuiCallbackDeclaredFuncEnd(cGuiPopUpColorPicker, PickerMode_OnSelect);

	//-------------------------------------------------------------------------------

	bool cGuiPopUpColorPicker::Img_OnMouseDown(iWidget* apWidget, const cGuiMessageData& aData)
	{
		if(mbImgPressed) return true;

		mpSet->PushAttentionWidget();
		mpSet->SetAttentionWidget(apWidget);
		mbImgPressed = true;

		return true;
	}
	kGuiCallbackDeclaredFuncEnd(cGuiPopUpColorPicker, Img_OnMouseDown);

	//-------------------------------------------------------------------------------

	bool cGuiPopUpColorPicker::Img_OnMouseUp(iWidget* apWidget, const cGuiMessageData& aData)
	{
		if(mbImgPressed && mpSet->GetAttentionWidget()==apWidget)
		{
			mpSet->PopAttentionWidget();
			mbImgPressed = false;
		}

		return true;
	}
	kGuiCallbackDeclaredFuncEnd(cGuiPopUpColorPicker, Img_OnMouseUp);

	//-------------------------------------------------------------------------------

	bool cGuiPopUpColorPicker::Slider_OnMouseMove(iWidget* apWidget, const cGuiMessageData& aData)
	{
		if(mbImgPressed && mpSet->GetAttentionWidget()==apWidget)
		{
			cVector3f vPos = cVector3f(aData.mvPos)-apWidget->GetGlobalPosition();
			vPos.y = vPos.y/apWidget->GetSize().y;
			vPos.y = cMath::Clamp(vPos.y, 0.0f, 1.0f);

			cGuiMessageData data(1.0f-vPos.y);

			tGuiCallbackFunc pCallback = reinterpret_cast<tGuiCallbackFunc>(apWidget->GetUserData());
			pCallback(this, apWidget, data);
		}
		
		return true;
	}
	kGuiCallbackDeclaredFuncEnd(cGuiPopUpColorPicker, Slider_OnMouseMove);

	//-------------------------------------------------------------------------------

	bool cGuiPopUpColorPicker::ColorBox_OnMouseMove(iWidget* apWidget, const cGuiMessageData& aData)
	{
		if(mbImgPressed && mpSet->GetAttentionWidget()==apWidget)
		{
			cVector3f vPos = cVector3f(aData.mvPos)-apWidget->GetGlobalPosition();

			/////////////////////////////////////////////////////////////////////////
			// Normalize mouse position in color map before passing to pick mode
			vPos.x = vPos.x/apWidget->GetSize().x;
			vPos.x = cMath::Clamp(vPos.x, 0.0f, 1.0f);
			vPos.y = vPos.y/apWidget->GetSize().y;
			vPos.y = cMath::Clamp(vPos.y, 0.0f, 1.0f);

			cVector2f vPos2D = cVector2f(vPos.x, vPos.y);

			mpCurrentMode->OnSetBox(vPos2D);
		}
		return true;
	}
	kGuiCallbackDeclaredFuncEnd(cGuiPopUpColorPicker, ColorBox_OnMouseMove);

	//-------------------------------------------------------------------------------

	bool cGuiPopUpColorPicker::ColorBox_OnDraw(iWidget* apWidget, const cGuiMessageData& aData)
	{
		mpCurrentMode->OnDrawBox(apWidget);

		return true;
	}
	kGuiCallbackDeclaredFuncEnd(cGuiPopUpColorPicker, ColorBox_OnDraw);

	//-------------------------------------------------------------------------------

	bool cGuiPopUpColorPicker::ColorSlider_OnMouseMove(iWidget* apWidget, const cGuiMessageData& aData)
	{
		mpCurrentMode->OnSetSlider(aData.mfVal);
				
		return true;
	}
	kGuiCallbackDeclaredFuncEnd(cGuiPopUpColorPicker, ColorSlider_OnMouseMove);

	//-------------------------------------------------------------------------------

	bool cGuiPopUpColorPicker::ColorSlider_OnDraw(iWidget* apWidget, const cGuiMessageData& aData)
	{
		mpCurrentMode->OnDrawSlider(apWidget);

		return true;
	}
	kGuiCallbackDeclaredFuncEnd(cGuiPopUpColorPicker, ColorSlider_OnDraw);

	//-------------------------------------------------------------------------------

	bool cGuiPopUpColorPicker::AlphaSlider_OnMouseMove(iWidget* apWidget, const cGuiMessageData& aData)
	{
		mfAlpha = aData.mfVal;
		mbAlphaValueUpdated = true;

		UpdateRGBInputs();

		UpdateColorRGB();

		return true;
	}
	kGuiCallbackDeclaredFuncEnd(cGuiPopUpColorPicker, AlphaSlider_OnMouseMove);

	//-------------------------------------------------------------------------------

	bool cGuiPopUpColorPicker::AlphaSlider_OnDraw(iWidget* apWidget, const cGuiMessageData& aData)
	{
		cVector3f vPos = apWidget->GetGlobalPosition();
		float fValue = (1-mfAlpha);
		vPos.y += fValue*255.0f;
		vPos.z += 0.01f;

		// Draws the marker on the slider
		mpSet->DrawGfx(mpGfxHMarker, vPos, cVector2f(20,1), cColor(fValue, 1), eGuiMaterial_Alpha);

		return true;
	}
	kGuiCallbackDeclaredFuncEnd(cGuiPopUpColorPicker, AlphaSlider_OnDraw);

	//-------------------------------------------------------------------------------
	
	bool cGuiPopUpColorPicker::HexInput_OnChangeText(iWidget* apWidget, const cGuiMessageData& aData)
	{
		apWidget->SetText(cString::ToUpperCaseW(apWidget->GetText()));

		return true;
	}
	kGuiCallbackDeclaredFuncEnd(cGuiPopUpColorPicker, HexInput_OnChangeText);

	//-------------------------------------------------------------------------------

	bool cGuiPopUpColorPicker::HexInput_OnEnter(iWidget* apWidget, const cGuiMessageData& aData)
	{
		cWidgetTextBox* pTB = static_cast<cWidgetTextBox*>(apWidget);

		int lMaxLength = pTB->GetMaxTextLength();
		tWString sHex = pTB->GetText();

		int lDiff = lMaxLength-sHex.length();
		sHex.insert(sHex.end(), lDiff, _W('0'));

		cMath::HexWToRGBAHelper(sHex, mColor);
		mfAlpha = mColor.a;

		UpdateRGBInputs(true);
		UpdateColorRGB();

		return true;
	}
	kGuiCallbackDeclaredFuncEnd(cGuiPopUpColorPicker, HexInput_OnEnter);

	//-------------------------------------------------------------------------------

	void cGuiPopUpColorPicker::OnCloseSpecific()
	{
		RestorePreviewToOldColor();
	}
	
	//-------------------------------------------------------------------------------

	/** Builds the window and sets up stuff
	 *
	 */
	void cGuiPopUpColorPicker::Init()
	{
		/////////////////////////////////
		// Init widgets

		// Main window
		mpWindow->SetText(_W("Color Picker"));
		mpWindow->SetStatic(false);
				
		cVector3f vPos = cVector3f(95,50,0.1f);
		
		//////////////////////////////////////////////////////////////////////////////
		// Init Color map and slider texture
		cGuiGfxElement* pImg = NULL;

		pImg = mpSet->GetGui()->CreateGfxTexture(mpColorBoxTexture, false, eGuiMaterial_Diffuse);
		mpImgColorBox = mpSet->CreateWidgetImage("", vPos, 256, eGuiMaterial_Diffuse, false, mpWindow);
		mpImgColorBox->AddCallback(eGuiMessage_MouseDown, this, kGuiCallback(Img_OnMouseDown));
		mpImgColorBox->AddCallback(eGuiMessage_MouseDown, this, kGuiCallback(ColorBox_OnMouseMove));
		mpImgColorBox->AddCallback(eGuiMessage_MouseUp, this, kGuiCallback(Img_OnMouseUp));
		mpImgColorBox->AddCallback(eGuiMessage_MouseMove, this, kGuiCallback(ColorBox_OnMouseMove));
		mpImgColorBox->AddCallback(eGuiMessage_OnDraw, this, kGuiCallback(ColorBox_OnDraw));
		mpImgColorBox->SetImage(pImg);
		
		vPos.x += 255 + 25;

		pImg = mpSet->GetGui()->CreateGfxFilledRect(cColor(1,1), eGuiMaterial_Diffuse);
		pImg->AddTexture(mpColorSliderTexture, cVector2f(1,0), cVector2f(1,1), cVector2f(0,1), cVector2f(0,0));
		pImg->SetDestroyTexture(false);
		mpImgColorSlider = mpSet->CreateWidgetImage("", vPos, cVector2f(20, 256), eGuiMaterial_Diffuse, false, mpWindow);
		mpImgColorSlider->SetUserData((void*)kGuiCallback(ColorSlider_OnMouseMove));
		mpImgColorSlider->AddCallback(eGuiMessage_MouseDown, this, kGuiCallback(Img_OnMouseDown));
		mpImgColorSlider->AddCallback(eGuiMessage_MouseDown, this, kGuiCallback(Slider_OnMouseMove));
		mpImgColorSlider->AddCallback(eGuiMessage_MouseUp, this, kGuiCallback(Img_OnMouseUp));
		mpImgColorSlider->AddCallback(eGuiMessage_MouseMove, this, kGuiCallback(Slider_OnMouseMove));
		mpImgColorSlider->AddCallback(eGuiMessage_OnDraw, this, kGuiCallback(ColorSlider_OnDraw));
		mpImgColorSlider->SetImage(pImg);
		vPos.x += 20 + 25;

		// Create alpha slider
		{
			//mpImgAlphaSliderBG = mpSet->CreateWidgetImage("gui_def_colorpicker_bgpattern.tga", vPos, cVector2f(20, 256), eGuiMaterial_Diffuse, false, mpWindow);

			unsigned char vByteArray[256*3];
			float fStep = 1.0f/255.0f;
			for(int i=0; i<256; ++i)
			{
				vByteArray[i*3] = vByteArray[i*3+1] = vByteArray[i*3+2] = FloatColorToUChar(float(i)*fStep);
			}

			iTexture* pAlphaSliderTexture = mpGraphics->GetLowLevel()->CreateTexture("AlphaSlider", eTextureType_1D, eTextureUsage_Normal);
			pAlphaSliderTexture->CreateFromRawData(cVector3l(256,0,0), ePixelFormat_RGB, vByteArray);

			pImg = mpSet->GetGui()->CreateGfxFilledRect(cColor(1,1), eGuiMaterial_Diffuse);
			pImg->AddTexture(pAlphaSliderTexture, cVector2f(1,0), cVector2f(1,1), cVector2f(0,1), cVector2f(0,0));
			pImg->SetDestroyTexture(true);
		}

		mpImgAlphaSlider = mpSet->CreateWidgetImage("", vPos, cVector2f(20, 256), eGuiMaterial_Diffuse, false, mpWindow);
		mpImgAlphaSlider->SetUserData((void*)kGuiCallback(AlphaSlider_OnMouseMove));
		mpImgAlphaSlider->AddCallback(eGuiMessage_MouseDown, this, kGuiCallback(Img_OnMouseDown));
		mpImgAlphaSlider->AddCallback(eGuiMessage_MouseDown, this, kGuiCallback(Slider_OnMouseMove));
		mpImgAlphaSlider->AddCallback(eGuiMessage_MouseUp, this, kGuiCallback(Img_OnMouseUp));
		mpImgAlphaSlider->AddCallback(eGuiMessage_MouseMove, this, kGuiCallback(Slider_OnMouseMove));
		mpImgAlphaSlider->AddCallback(eGuiMessage_OnDraw, this, kGuiCallback(AlphaSlider_OnDraw));

		mpImgAlphaSlider->SetImage(pImg);

		vPos.x += 20 + 25;
		
		/////////////////////////////////////////////////////////////////////////////
		// Init HSB and RGB inputs
		CreateHSBInputs(mpWindow, vPos);
		CreateRGBInputs(mpWindow, vPos);

		// Init Alpha input
		{
			cVector3f vPosA = vPos + cVector3f(0,10,0);
			mpCBAlpha = mpSet->CreateWidgetCheckBox(vPosA, 0, _W("A"), mpWindow);
			mpCBAlpha->SetDefaultFontSize(gvInputFontSize);
			mpCBAlpha->AddCallback(eGuiMessage_CheckChange, this, kGuiCallback(Alpha_Toggle));
		
			mpInpAlpha = mpSet->CreateWidgetTextBox(gvInputOffset, gvInputSize, _W(""), mpCBAlpha, eWidgetTextBoxInputType_Numeric);
			mpInpAlpha->SetDefaultFontSize(gvInputFontSize);
			mpInpAlpha->SetLowerBound(true, 0.0f);
			mpInpAlpha->SetUpperBound(true, 255.0f);
			mpInpAlpha->SetDecimals(0);
			mpInpAlpha->AddCallback(eGuiMessage_TextBoxEnter, this, kGuiCallback(Alpha_InputValueEnter));
			mbAlphaValueUpdated = true;

			vPos.y += mpInpAlpha->GetSize().y+10;
		}

		// Init Hexcode input
		{
			cVector3f vPosHex = vPos;
			vPosHex.y += 10;

			cWidgetLabel* pLabel = mpSet->CreateWidgetLabel(vPosHex, 0, _W("Hex"), mpWindow);
			pLabel->SetDefaultFontSize(gvInputFontSize);
			pLabel->SetAutogenerateSize(true);

			/*mpInpHexRGB = mpSet->CreateWidgetTextBox(gvInputOffset, gvInputSize, _W(""), pLabel);
			mpInpHexRGB->SetDefaultFontSize(gvInputFontSize);
			mpInpHexRGB->SetMaxTextLength(8);
			mpInpHexRGB->AddCallback(eGuiMessage_TextChange, this, kGuiCallback(HexInput_OnChangeText));
			mpInpHexRGB->AddCallback(eGuiMessage_TextBoxEnter, this, kGuiCallback(HexInput_OnEnter));
			mpInpHexRGB->SetLegalCharCodeLimitEnabled(true);
			mpInpHexRGB->AddLegalCharCodeRange(_W('0'), _W('9'));
			mpInpHexRGB->AddLegalCharCodeRange(_W('A'), _W('F'));
			mpInpHexRGB->AddLegalCharCodeRange(_W('a'), _W('f'));
			*/

			/*mpInpHexAlpha = mpSet->CreateWidgetTextBox(gvInputOffset+cVector3f(gvInputSize.x+5,0,0), gvInputSize*cVector2f(0.5f,1.0f), _W(""), pLabel);
			mpInpHexAlpha->SetDefaultFontSize(gvInputFontSize);
			mpInpHexAlpha->SetMaxTextLength(2);
			mpInpHexAlpha->AddCallback(eGuiMessage_TextChange, this, kGuiCallback(HexInput_OnChangeText));
			mpInpHexAlpha->AddCallback(eGuiMessage_TextBoxEnter, this, kGuiCallback(HexInput_OnEnter));
			mpInpHexAlpha->SetLegalCharCodeLimitEnabled(true);
			mpInpHexAlpha->AddLegalCharCodeRange(_W('0'), _W('9'));
			mpInpHexAlpha->AddLegalCharCodeRange(_W('A'), _W('F'));
			mpInpHexAlpha->AddLegalCharCodeRange(_W('a'), _W('f'));
			*/

			mpInpHexRGBA = mpSet->CreateWidgetTextBox(gvInputOffset, gvInputSize+cVector2f(gvInputSize.x*0.333f,0), _W(""), pLabel);
			mpInpHexRGBA->SetDefaultFontSize(gvInputFontSize);
			mpInpHexRGBA->SetMaxTextLength(8);
			mpInpHexRGBA->AddCallback(eGuiMessage_TextChange, this, kGuiCallback(HexInput_OnChangeText));
			mpInpHexRGBA->AddCallback(eGuiMessage_TextBoxEnter, this, kGuiCallback(HexInput_OnEnter));
			mpInpHexRGBA->SetLegalCharCodeLimitEnabled(false);

			vPos.y += mpInpHexRGBA->GetSize().y+10;

			pLabel = mpSet->CreateWidgetLabel(vPos, 0, _W("Vec"), mpWindow);
			pLabel->SetDefaultFontSize(gvInputFontSize);
			pLabel->SetAutogenerateSize(true);

			mpInpVecRGBA = mpSet->CreateWidgetTextBox(gvInputOffset, gvInputSize+cVector2f(gvInputSize.x*0.333f,0), _W(""), pLabel);
			mpInpVecRGBA->SetDefaultFontSize(gvInputFontSize);
			mpInpVecRGBA->SetLegalCharCodeLimitEnabled(true);
		}
		
		//////////////////////////////////////////////////////////////////////////////
		// Init color previews
		vPos = cVector3f(10, 50, 0.7f);
		mpFCurrentColor = mpSet->CreateWidgetFrame(vPos,50,true,mpWindow);
		mpFCurrentColor->AddCallback(eGuiMessage_OnDraw, this, kGuiCallback(ColorFrame_OnDraw));
		mpFCurrentColor->SetDrawBackground(true);
		mpFCurrentColor->SetBackgroundZ(0.1f);
		
		vPos.x += 25;
		vPos.y += 25;
		vPos.z -= 0.4f;

		mpFPreviousColor = mpSet->CreateWidgetFrame(vPos,50,true,mpWindow);
		mpFPreviousColor->AddCallback(eGuiMessage_OnDraw, this, kGuiCallback(ColorFrame_OnDraw));
		mpFPreviousColor->AddCallback(eGuiMessage_MouseDown, this, kGuiCallback(RecentColor_OnMouseDown));
		mpFPreviousColor->SetBackGroundColor(mColor);
		mpFPreviousColor->SetUserData(NULL);
		mpFPreviousColor->SetDrawBackground(true);
		mpFPreviousColor->SetBackgroundZ(0.1f);

		/////////////////////////////////////////////////////////////////
		// Recent color list
		vPos = cVector3f(40, 140, 0.1f);
		tColorListIt itRecent = mlstRecentColors.begin();
		for(; itRecent!=mlstRecentColors.end(); ++itRecent)
		{
			cColor color = *itRecent;
			color.a = 1.0f;

			cWidgetFrame* pRecent = mpSet->CreateWidgetFrame(vPos,cVector2f(12,12),true,mpWindow);
			pRecent->AddCallback(eGuiMessage_MouseDown, this, kGuiCallback(RecentColor_OnMouseDown));
			pRecent->SetBackGroundColor(color);
			pRecent->SetUserData(&*itRecent);

			pRecent->SetDrawBackground(true);
			pRecent->SetBackgroundZ(0.1f);

			vPos.y += pRecent->GetSize().y + 5;
		}

		// Buttons
		for(int i=0; i<3; ++i)
		{
			mvButtons[i] = mpSet->CreateWidgetButton(	0,
														30,
														_W(""), 
														mpWindow);

			mvButtons[i]->AddCallback(eGuiMessage_ButtonPressed, this, kGuiCallback(Button_Pressed));
		}

		mvButtons[0]->SetEnabled(false);
		mvButtons[0]->SetVisible(false);
		mvButtons[0]->SetPosition(vPos);
		mvButtons[0]->SetSize(23);

		vPos = cVector3f(130, 320, 0.1f);		
		mvButtons[1]->SetText(_W("Ok"));
		mvButtons[1]->SetPosition(vPos);
		mvButtons[1]->SetSize(cVector2f(70,20));
		vPos.x += mvButtons[1]->GetSize().x + 20;
		
		mvButtons[2]->SetText(_W("Cancel"));
		mvButtons[2]->SetPosition(vPos);
		mvButtons[2]->SetSize(cVector2f(70,20));


		////////////////////////////////////////////////////////////////////
		// Set GUI to initial state
		UpdateRGBInputs(true);
		UpdateColorRGB(true);

		mvPickerModeSwitches[0]->SetChecked(true);
		mpCBAlpha->SetChecked(true);
	}

	//-------------------------------------------------------------------------------

	void cGuiPopUpColorPicker::AcceptPickedColor()
	{
		///////////////////////////////////////////////////////////////
		// Compare old color to new and set up stuff if different
		if((mColor == mpFPreviousColor->GetBackGroundColor()) == false)
		{
			if(mpDestColor)
				*mpDestColor = mColor;

			RestorePreviewToOldColor();

			RunCallback(mpCallbackObject, mpCallback, NULL, cGuiMessageData(), true);
			
			AddRecentColor(mColor);
		}
	}

	void cGuiPopUpColorPicker::RestorePreviewToOldColor()
	{
		//////////////////////////////////////////////////////////////////////////////////////////////
		// Restore the old color in the update, as a preview color might have been propagated
		cColor col = GetOldColor();
		cGuiMessageData data;
		data.mpData = &col;

		RunCallback(mpUpdateCallbackObject, mpUpdateCallback, NULL, data, false);
	}

	//-------------------------------------------------------------------------------

	void cGuiPopUpColorPicker::UpdateHSBInputs(bool abForce)
	{
		for(size_t i=0; i<mvHSBInputs.size(); ++i)
		{
			if(abForce==false && mvHSBValueUpdated[i]==false) continue;

			// If fValue is Saturation or Brightness, do a lil conversion (from range 0-1 to 0-100) - Hue does not need conversion
			float fValue = mvHSB.v[i];
			if(i>0)								
				fValue*=100.0f;
			
			mvHSBInputs[i]->SetNumericValue(fValue);
			mvHSBValueUpdated[i] = false;
		}
	}

	//-------------------------------------------------------------------------------

	void cGuiPopUpColorPicker::UpdateRGBInputs(bool abForce)
	{
		///////////////////////////////
		// Update RGB
		for(size_t i=0; i<mvRGBInputs.size(); ++i)
		{
			if(abForce==false && mvRGBValueUpdated[i]==false) continue;

			float fValue = mColor.v[i]*255.0f;
			
			mvRGBInputs[i]->SetNumericValue(fValue);
			mvRGBValueUpdated[i] = false;
		}

		///////////////////////////////
		// Update alpha
		if(abForce || mbAlphaValueUpdated)
		{
			mpInpAlpha->SetNumericValue(mColor.a*255.0f);
			mbAlphaValueUpdated = false;
		}
	}

	//-------------------------------------------------------------------------------

	void cGuiPopUpColorPicker::UpdateColor(bool abForce)
	{
		////////////////////////////////////////////////////
		// Apply alpha if active
		bool bAlphaActive = mpCBAlpha->IsChecked();
		mColor.a = bAlphaActive? mfAlpha : 1.0f;
	
		cColor previewColor = mpFCurrentColor->GetBackGroundColor();

		if(abForce==false && 
			previewColor.r==mColor.r && previewColor.g==mColor.g && previewColor.b==mColor.b &&
			mColor.a==mfOldAlpha) return; 
		
		/////////////////////////////////////////////////////
		// Color has changed, GUI needs update
		previewColor = mColor;
		if(mbShowTransPreview==false)
			previewColor.a = 1.0f;

		mpFCurrentColor->SetBackGroundColor(previewColor);
		//mpInpHexRGB->SetText(cMath::RGBToHexW(mColor));
		//mpInpHexAlpha->SetText(cMath::UCharToHexStringW(FloatColorToUChar(mfAlpha)));
		mpInpHexRGBA->SetText(cMath::RGBAToHexW(mColor));
		//mpImgAlphaSlider->SetColorMul(cColor(mColor.r, mColor.g, mColor.b, 1));

		tWString sVecRGBA;
		for(int i=0;i<4;++i) 
		{
			if(i>0) sVecRGBA += _W(",");
			sVecRGBA += cString::ToStringW(mColor.v[i], 3, true);
		}
		mpInpVecRGBA->SetText(sVecRGBA);

		// Call update callback if set
		cGuiMessageData data;
		data.mpData = &mColor;

		RunCallback(mpUpdateCallbackObject, mpUpdateCallback, NULL, data, false);

		mfOldAlpha = mColor.a;
	}

	void cGuiPopUpColorPicker::UpdateColorHSB(bool abForce)
	{
		cMath::HSBToRGBHelper(mvHSB, mColor);
		UpdateRGBInputs(true);

		UpdateColor(abForce);
	}

	void cGuiPopUpColorPicker::UpdateColorRGB(bool abForce)
	{
		cMath::RGBToHSBHelper(mColor, mvHSB);
		if(mvHSB.x==-1) mvHSB.x = 0; // -1 means undefined, just set some value.

		UpdateHSBInputs(true);

		UpdateColor(abForce);
	}

	//-------------------------------------------------------------------------------

	void cGuiPopUpColorPicker::AddRecentColor(const cColor& aX)
	{
		mlstRecentColors.remove(aX);
		mlstRecentColors.push_front(aX);

		while(mlstRecentColors.size()>mlRecentColorNum)
			mlstRecentColors.pop_back();
	}

	//-------------------------------------------------------------------------------

};




