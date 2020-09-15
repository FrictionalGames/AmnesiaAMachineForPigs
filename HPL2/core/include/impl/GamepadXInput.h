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

#ifndef HPL_GAMEPAD_XINPUT_H
#define HPL_GAMEPAD_XINPUT_H

#define USE_XINPUT 0

#if USE_XINPUT

#include <windows.h>
#include <XInput.h>

#include <vector>
#include <list>
#include "system/SystemTypes.h"
#include "input/Gamepad.h"

namespace hpl {

	
	//------------------------------

	///////////////////////////////////////////////////////////////////////
	// Gamepad layout
	// ----------Buttons---------
	// 0 --> A
	// 1 --> B
	// 2 --> X
	// 3 --> Y
	// 4 --> LB
	// 5 --> RB
	// 6 --> Back
	// 7 --> Start
	// 8 --> LA
	// 9 --> RA
	// ------------Hat-------------
	// Up --> DPAD-Up
	// Down --> DPAD-Down
	// Left --> DPAD-Left
	// Right --> DPAD-Right
	// ------------Axis-------------
	// 0 --> (-) LStick-Left, (+) LStick-Right
	// 1 --> (-) LStick-Forward, (+) LStick-Back
	// 2 --> (-) RStick-Down, (+) RStick-Up
	// 3 --> (-) RStick-Left, (+) RStick-Right
	// 4 --> (+) LTrigger
	// 5 --> (+) RTrigger

	//---------------------------------------------------------------------

	class cGamepadXInput : public iGamepad
	{
	public:
		cGamepadXInput(int alIndex);
		~cGamepadXInput();

		tString GetGamepadName() { return msGamepadName; }

		int GetNumButtons();
		int GetNumAxes();
		int GetNumHats();
		int GetNumBalls();

		void Update();

		//Gamepad specific
		bool HasInputUpdates();
		cGamepadInputData GetInputUpdate();
		
		bool ButtonIsDown(eGamepadButton aButton);
		cGamepadInputData GetButton();
		bool ButtonIsPressed();
		bool ButtonIsReleased();
		cGamepadInputData GetReleasedButton();

		float GetAxisValue(eGamepadAxis aAxis);
		float GetAxisDeadZoneRadiusValue();
		void SetAxisDeadZoneRadiusValue(float afValue);
		bool AxesUpdated();
		cGamepadInputData GetUpdatedAxis();

		eGamepadHatState GetHatCurrentState(eGamepadHat aHat);
		bool HatIsInState(eGamepadHat aHat, eGamepadHatState aState);
		bool HatsChanged();
		cGamepadInputData GetHatState();

		cVector2l GetBallAbsPos(eGamepadBall aBall);
		cVector2l GetBallRelPos(eGamepadBall aBall);

		static int GetNumConnected();
		static bool IsConnected(int alIndex);
		static bool GetWasConnected(int alIndex) { return mbDeviceConnected[alIndex]; }
		static int GetDeviceChange();
	private:
		static void SetWasConnected(int alIndex, bool abVal) { mbDeviceConnected[alIndex] = abVal; }

		void UpdateAxis(int alAxis, float afVal);
		void UpdateTrigger(float afLVal, float afRVal);

		eGamepadButton		XInputToButton(size_t alButton);
		float				XInputToAxisValue(SHORT alAxisValue);
		float				XInputToTriggerValue(BYTE alTriggerValue);

		tString msGamepadName;

		std::vector<float>				mvAxisArray;
		std::vector<eGamepadAxisRange>	mvAxisRange;
		std::vector<bool>				mvButtonArray;
		std::vector<eGamepadHatState>	mvHatArray;
		std::vector<cVector2l>			mvBallAbsPosArray;
		std::vector<cVector2l>			mvBallRelPosArray;

		std::list<cGamepadInputData> mlstInputUpdates;

		std::list<cGamepadInputData> mlstButtonsPressed;
		std::list<cGamepadInputData> mlstButtonsReleased;

		std::list<cGamepadInputData> mlstAxisChanges;

		std::list<cGamepadInputData> mlstHatStateChanges;
		float mfLeftTrigger;
		float mfRightTrigger;

		static float mfInvAxisMax;
		static float mfDeadZoneRadius;

		XINPUT_STATE mState;

		static bool mbDeviceConnected[4];
	};

};

#endif // USE_XINPUT
#endif // HPL_GAMEPAD_XINPUT_H
