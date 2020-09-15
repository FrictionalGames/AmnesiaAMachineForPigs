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

#ifndef HPL_GUI_POP_UP_COLOR_PICKER_H
#define HPL_GUI_POP_UP_COLOR_PICKER_H

#include "gui/GuiPopUp.h"
#include "graphics/Bitmap.h"

namespace hpl {

	class cGuiSet;
	
	class cGuiPopUpColorPicker;

	class cWidgetWindow;
	class cWidgetButton;
	class cWidgetCheckBox;
	class cWidgetTextBox;

	class cWidgetImage;
	class cWidgetFrame;
	class cWidgetSlider;
	class cWidgetLabel;

	class cXmlElement;
	class cGraphics;

	//---------------------------------------------------------------

	typedef cVector3<int> cVector3l;

	//---------------------------------------------------------------

	class iGraphicPickerMode
	{
	public:
		iGraphicPickerMode(cGuiPopUpColorPicker* apPicker, const tWString& asName, int alSliderParamIndex, const cVector3f& avMaxValues);
		virtual ~iGraphicPickerMode() { }

		const tWString& GetName() { return msName; }
		const wchar_t&	GetSubname() { return msName[mlSliderParamIndex]; }

		bool IsCurrent();

		virtual void OnSetBox(const cVector2f& avX);
		virtual void OnSetSlider(float afValue);
		virtual void OnInputEnter(cWidgetTextBox* apInput);

		virtual void RebuildBox()=0;
		virtual void RebuildSlider()=0;

		void UpdateMarkers();

		void OnDrawBox(iWidget* apWidget);
		void OnDrawSlider(iWidget* apWidget);

		virtual float GetSliderParamValue()=0;

	protected:
		virtual void OnInputEnterSpecific(cWidgetTextBox* apInput)=0;
		virtual cVector2f GetPosInMap()=0;
		virtual float GetPosInSlider()=0;

		bool SetHSBValue(int alIndex, float afX, bool afUpdateInput=true);
		bool SetRGBValue(int alIndex, float afX, bool afUpdateInput=true);
		void SetRGB(float afR, float afG, float afB);

		void SaveSliderValue(float afX) { mfOldSliderParamValue = afX; }
		bool SliderValueChanged(float afX) { return mfOldSliderParamValue!=afX; }

		void UpdateSliderMarkerColor();

		int mlSliderParamIndex;
		int mlRowIndex;
		int mlColIndex;

		float mfOldSliderParamValue;

		cGuiPopUpColorPicker* mpPicker;
		tWString msName;
		 
		cBitmap* mpBoxBmp;
		iTexture* mpBoxTex;
		cBitmap* mpSliderBmp;
		iTexture* mpSliderTex;

		cVector2f mvColorMapPos;
		float mfColorSliderPos;

		cColor mSliderMarkerCol;

		cVector3f mvMaxValues;
	};

	//---------------------------------------------------------------

	typedef std::vector<iGraphicPickerMode*> tColorPickerModeVec;

	//---------------------------------------------------------------

	class cHSBMode : public iGraphicPickerMode
	{
	public:
		cHSBMode(cGuiPopUpColorPicker* apPicker, int alParamIndex);

		void OnSetBox(const cVector2f& avX);
		void OnSetSlider(float afX);
		
		void RebuildBox();
		void RebuildSlider();

		float GetSliderParamValue();

	protected:
		void OnInputEnterSpecific(cWidgetTextBox* apInput);
		cVector2f GetPosInMap();
		float GetPosInSlider();
	};

	//---------------------------------------------------------------
	
	class cRGBMode : public iGraphicPickerMode
	{
	public:
		cRGBMode(cGuiPopUpColorPicker* apPicker, int alIndex);

		void OnSetBox(const cVector2f& avPos);
		void OnSetSlider(float afX);
		
		void RebuildBox();
		void RebuildSlider();

		float GetSliderParamValue();

	protected:
		void OnInputEnterSpecific(cWidgetTextBox* apInput);
		cVector2f GetPosInMap();
		float GetPosInSlider();
	};

	//---------------------------------------------------------------

	class cGuiPopUpColorPicker : public iGuiPopUp
	{
		friend class iGraphicPickerMode;
	public:
		cGuiPopUpColorPicker(cGraphics* apGraphics, cGuiSet* apSet, cColor* apDestColor, 
								void *apCallbackObject, tGuiCallbackFunc apCallback, 
								void *apUpdateColorCallbackObject, tGuiCallbackFunc apUpdateColorCallback);
		virtual ~cGuiPopUpColorPicker();		

		void SetUpdateColorCallback(void* apCallbackObject, tGuiCallbackFunc apCallback);

		void SetColor(const cColor& aX);
		const cColor& GetColor() { return mColor; }
		const cVector3f& GetHSB() { return mvHSB; }

		const cColor& GetOldColor();

		static void LoadRecentColorList(cXmlElement* apElem);
		static void SaveRecentColorList(cXmlElement* apElem);

		static void SetRecentColorNum(int alX);
		static int GetRecentColorNum() { return mlRecentColorNum; }

		void SetShowTransPreview(bool abX) { mbShowTransPreview = abX; }

	protected:
		/////////////////////////////
		// Own functions

		/**
		* \return true if value was set
		*/
		bool SetHSBValue(int alIdx, float afX, bool abUpdateInput);
		/**
		* \return true if value was set
		*/
		bool SetRGBValue(int alIdx, float afX, bool abUpdateInput);

		void SetRGB(float afR, float afG, float afB);

		void CreateHSBInputs(iWidget* apParent, cVector3f& avPos);
		void CreateRGBInputs(iWidget* apParent, cVector3f& avPos);
		void CreateInputs(iWidget* apParent, cVector3f& avPos, tGuiCallbackFunc apCallback,
							const tFloatVec& avMin, const tFloatVec& avMax, std::vector<cWidgetTextBox*>& avContainer, 
							tBoolVec& avUpdatedContainer, tColorPickerModeVec avModes);

		bool Button_Pressed(iWidget* apWidget, const cGuiMessageData& aData);
		kGuiCallbackDeclarationEnd(Button_Pressed);

		bool ColorFrame_OnDraw(iWidget* apWidget, const cGuiMessageData& aData);
		kGuiCallbackDeclarationEnd(ColorFrame_OnDraw);

		bool RecentColor_OnMouseDown(iWidget* apWidget, const cGuiMessageData& aData);
		kGuiCallbackDeclarationEnd(RecentColor_OnMouseDown);

		bool Color_InputValueEnter(iWidget* apWidget, const cGuiMessageData& aData);
		kGuiCallbackDeclarationEnd(Color_InputValueEnter);
 
		bool Alpha_InputValueEnter(iWidget* apWidget, const cGuiMessageData& aData);
		kGuiCallbackDeclarationEnd(Alpha_InputValueEnter);

		bool Alpha_Toggle(iWidget* apWidget, const cGuiMessageData& aData);
		kGuiCallbackDeclarationEnd(Alpha_Toggle);

		bool PickerMode_OnSelect(iWidget* apWidget, const cGuiMessageData& aData);
		kGuiCallbackDeclarationEnd(PickerMode_OnSelect);

		bool Img_OnMouseDown(iWidget* apWidget, const cGuiMessageData& aData);
		kGuiCallbackDeclarationEnd(Img_OnMouseDown);
		bool Img_OnMouseUp(iWidget* apWidget, const cGuiMessageData& aData);
		kGuiCallbackDeclarationEnd(Img_OnMouseUp);

		bool Slider_OnMouseMove(iWidget* apWidget, const cGuiMessageData& aData);
		kGuiCallbackDeclarationEnd(Slider_OnMouseMove);
		
		bool ColorBox_OnMouseMove(iWidget* apWidget, const cGuiMessageData& aData);
		kGuiCallbackDeclarationEnd(ColorBox_OnMouseMove);
		bool ColorBox_OnDraw(iWidget* apWidget, const cGuiMessageData& aData);
		kGuiCallbackDeclarationEnd(ColorBox_OnDraw);

		bool ColorSlider_OnMouseMove(iWidget* apWidget, const cGuiMessageData& aData);
		kGuiCallbackDeclarationEnd(ColorSlider_OnMouseMove);
		bool ColorSlider_OnDraw(iWidget* apWidget, const cGuiMessageData& aData);
		kGuiCallbackDeclarationEnd(ColorSlider_OnDraw);

		bool AlphaSlider_OnMouseMove(iWidget* apWidget, const cGuiMessageData& aData);
		kGuiCallbackDeclarationEnd(AlphaSlider_OnMouseMove);
		bool AlphaSlider_OnDraw(iWidget* apWidget, const cGuiMessageData& aData);
		kGuiCallbackDeclarationEnd(AlphaSlider_OnDraw);

		bool HexInput_OnChangeText(iWidget* apWidget, const cGuiMessageData& aData);
		kGuiCallbackDeclarationEnd(HexInput_OnChangeText);
		bool HexInput_OnEnter(iWidget* apWidget, const cGuiMessageData& aData);
		kGuiCallbackDeclarationEnd(HexInput_OnEnter);

		void OnCloseSpecific();

		void Init();

		void AcceptPickedColor();
		void RestorePreviewToOldColor();

		void ClosePopUp();

		void UpdateHSBInputs(bool abForce=false);
		void UpdateRGBInputs(bool abForce=false);

		void UpdateColor(bool abForce=false);
		void UpdateColorHSB(bool abForce=false);
		void UpdateColorRGB(bool abForce=false);

		void AddRecentColor(const cColor& aX);

		cGraphics* mpGraphics;

		//////////////////////////////
		// Data
		cColor mColor;
		cVector3f mvHSB;
		cColor* mpDestColor;

		float mfAlpha;
		float mfOldAlpha;
		bool mbShowTransPreview;

		iGraphicPickerMode* mpCurrentMode;

		bool mbImgPressed;

		cWidgetFrame* mpFCurrentColor;
		cWidgetFrame* mpFPreviousColor;

		cWidgetImage* mpImgColorBox;
		iTexture*	  mpColorBoxTexture;
		cBitmap*      mpColorBoxBitmap;


		cWidgetImage* mpImgColorSlider;
		iTexture*	  mpColorSliderTexture;
		cBitmap*      mpColorSliderBitmap;

		cWidgetImage* mpImgAlphaSliderBG;
		cWidgetImage* mpImgAlphaSlider;
		
		cGuiGfxElement* mpGfxBGPattern;
		cGuiGfxElement* mpGfxColorPointer;
		cGuiGfxElement* mpGfxSliderPointer;
		
		cGuiGfxElement* mpGfxHMarker;
		cGuiGfxElement* mpGfxVMarker;

		std::vector<cWidgetCheckBox*> mvPickerModeSwitches;

		std::vector<cWidgetTextBox*> mvHSBInputs;
		tBoolVec					 mvHSBValueUpdated;

		std::vector<cWidgetTextBox*> mvRGBInputs;
		tBoolVec					 mvRGBValueUpdated;

		cWidgetCheckBox* mpCBAlpha;
		cWidgetTextBox* mpInpAlpha;
		bool mbAlphaValueUpdated;

		cWidgetTextBox* mpInpHexRGB;
		cWidgetTextBox* mpInpHexAlpha;
		cWidgetTextBox* mpInpHexRGBA;
		cWidgetTextBox* mpInpVecRGBA;
		
		cWidgetButton* mvButtons[3];

		void * mpCallbackObject;
		tGuiCallbackFunc mpCallback;

		void * mpUpdateCallbackObject;
		tGuiCallbackFunc mpUpdateCallback;

		static tColorList mlstRecentColors;
		static size_t mlRecentColorNum;
	};
};

#endif // HPL_GUI_POP_UP_COLOR_PICKER_H
