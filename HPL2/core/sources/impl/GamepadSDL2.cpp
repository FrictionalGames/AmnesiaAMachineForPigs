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

#if USE_SDL2

#include "impl/GamepadSDL2.h"

#include "impl/LowLevelInputSDL.h"

#include "math/Math.h"
#include <limits>

namespace hpl {
	
	//////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	float cGamepadSDL2::mfInvAxisMax = 1.0f/(float)cMath::Min(cMath::Abs(std::numeric_limits<Sint16>::min()), std::numeric_limits<Sint16>::max());
	float cGamepadSDL2::mfDeadZoneRadius = 3200.0f*cGamepadSDL2::mfInvAxisMax;

	//-----------------------------------------------------------------------
#pragma optimize("", off)
	cGamepadSDL2::cGamepadSDL2(cLowLevelInputSDL *apLowLevelInputSDL, int alIndex) : iGamepad("SDL Portable Gamepad", alIndex)
	{
		mpLowLevelInputSDL = apLowLevelInputSDL;

		mpHandle = SDL_GameControllerOpen(mlIndex);

		if(mpHandle)
		{
            SDL_Joystick *joy = SDL_GameControllerGetJoystick(mpHandle);
			
            mlInstance = SDL_JoystickInstanceID(joy);

			msGamepadName = tString(SDL_GameControllerName(mpHandle));

			mvButtonArray.assign(SDL_CONTROLLER_BUTTON_MAX, false);

			mvAxisArray.assign(SDL_CONTROLLER_AXIS_MAX, 0.0f);
            
            // @todo open up the assiciated haptic device and provide rumble!
		}
		//ClearKeyList();
		
#ifdef WIN32
		mvRemappedAxisArray.resize(mvAxisArray.size());
		mvRemappedButtonArray.resize(mvButtonArray.size());
		mvHatArray.resize(mvButtonArray.size());
#endif
	}
	
	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PUBLIC METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	int cGamepadSDL2::GetNumButtons()
	{
		return (int)mvButtonArray.size();
	}

	int cGamepadSDL2::GetNumAxes()
	{
		return (int)mvAxisArray.size();
	}

	int cGamepadSDL2::GetNumHats()
	{
		return 0;
	}

	int cGamepadSDL2::GetNumBalls()
	{
		return 0;
	}

	//-----------------------------------------------------------------------


	void cGamepadSDL2::Update()
	{
		cGamepadInputData inputUpdate;

		mlstInputUpdates.clear();
		mlstButtonsPressed.clear();
		mlstButtonsReleased.clear();

		std::list<SDL_Event>::iterator it = mpLowLevelInputSDL->mlstEvents.begin();
        for(; it != mpLowLevelInputSDL->mlstEvents.end(); ++it)
		{
			SDL_Event *pEvent = &(*it);

            switch (pEvent->type) {
                case SDL_CONTROLLERAXISMOTION:
                    if (mlInstance == pEvent->caxis.which) {
                        eGamepadAxis axis = SDLToAxis(pEvent->caxis.axis);
                        float fAxisValue = SDLToAxisValue(pEvent->caxis.value);
                        
                        if(cMath::Abs(fAxisValue) < mfDeadZoneRadius)
                            fAxisValue = 0.0f;
                        
                        if(fAxisValue!=mvAxisArray[axis])
                        {
                            inputUpdate = cGamepadInputData(mlIndex, eGamepadInputType_Axis, axis, fAxisValue);
                            
                            mlstAxisChanges.push_back(inputUpdate);
                            mlstInputUpdates.push_back(inputUpdate);
                        }
                        mvAxisArray[axis] = fAxisValue;                        
                    }
                    break;
                case SDL_CONTROLLERBUTTONDOWN:
                case SDL_CONTROLLERBUTTONUP:
                    if (mlInstance == pEvent->cbutton.which) {
                        eGamepadButton button = SDLToButton(pEvent->cbutton.button);
                        inputUpdate = cGamepadInputData(mlIndex, eGamepadInputType_Button, button, 0.0f);

                        bool bPressed;
                        if(pEvent->cbutton.state==SDL_RELEASED)
                        {
                            inputUpdate.mfInputValue = 0.0f;
                            mlstButtonsReleased.push_back(inputUpdate);
                            bPressed = false;
                        }
                        else
                        {
                            inputUpdate.mfInputValue = 1.0f;
                            mlstButtonsPressed.push_back(inputUpdate);
                            bPressed = true;
                        }
                        
                        mlstInputUpdates.push_back(inputUpdate);
                        mvButtonArray[button] = bPressed;                        
                    }
                    break;
            }
		}

#ifdef WIN32
		//////////////////
		// Remapp buttons to SDL1 version, cant use SDL2 directly because then the patch would destroy binds
		if(mvAxisArray.size() >= 6 && mvRemappedAxisArray.size() >= 6)
		{
			mvRemappedAxisArray[0] = mvAxisArray[0];
			mvRemappedAxisArray[1] = mvAxisArray[1];
			mvRemappedAxisArray[2] = (mvAxisArray[5] * mvAxisArray[5] > 0) ? -mvAxisArray[5] : mvAxisArray[4];
			mvRemappedAxisArray[3] = mvAxisArray[3];
			mvRemappedAxisArray[4] = mvAxisArray[2];
			mvRemappedAxisArray[5] = 0;
		}

		if(mvButtonArray.size() >= 15 && mvRemappedButtonArray.size() >= 15 && mvHatArray.size() >= 15)
		{
			mvRemappedButtonArray[0] = mvButtonArray[0];
			mvRemappedButtonArray[1] = mvButtonArray[1];
			mvRemappedButtonArray[2] = mvButtonArray[2];
			mvRemappedButtonArray[3] = mvButtonArray[3];
			mvRemappedButtonArray[4] = mvButtonArray[9];
			mvRemappedButtonArray[5] = mvButtonArray[10];
			mvRemappedButtonArray[6] = mvButtonArray[4];
			mvRemappedButtonArray[7] = mvButtonArray[6];
			mvRemappedButtonArray[8] = mvButtonArray[7];
			mvRemappedButtonArray[9] = mvButtonArray[8];

			mvHatArray[0] = eGamepadHatState_Centered;
			
			bool bUp = mvButtonArray[11];
			bool bDown = mvButtonArray[12];
			bool bLeft = mvButtonArray[13];
			bool bRight = mvButtonArray[14];

			if(bUp)
			{
				mvHatArray[0] = eGamepadHatState_Up;

				if(bLeft)
				{
					mvHatArray[0] = eGamepadHatState_LeftUp;
				}
				else if(bRight)
				{
					mvHatArray[0] = eGamepadHatState_RightUp;
				}
			}
			else if(bDown)
			{
				mvHatArray[0] = eGamepadHatState_Down;

				if(bLeft)
				{
					mvHatArray[0] = eGamepadHatState_LeftDown;
				}
				else if(bRight)
				{
					mvHatArray[0] = eGamepadHatState_RightDown;
				}
			}
			else
			{
				if(bLeft)
				{
					mvHatArray[0] = eGamepadHatState_Left;
				}
				else if(bRight)
				{
					mvHatArray[0] = eGamepadHatState_Right;
				}
			}
		}
#endif
	}

	//-----------------------------------------------------------------------

	bool cGamepadSDL2::HasInputUpdates()
	{
		return mlstInputUpdates.empty()==false;
	}

	cGamepadInputData cGamepadSDL2::GetInputUpdate()
	{
		cGamepadInputData input = mlstInputUpdates.front();
		mlstInputUpdates.pop_front();

		switch(input.mInputType)
		{
		case eGamepadInputType_Button:
			{
				if(input.mfInputValue==0.0f)
					mlstButtonsReleased.remove(input);
				else
					mlstButtonsPressed.remove(input);
			}
			break;
		case eGamepadInputType_Axis:
			mlstAxisChanges.remove(input);
			break;
		}

		return input;
	}

	//-----------------------------------------------------------------------

    bool cGamepadSDL2::ButtonIsDown(eGamepadButton aButton)
	{
        if ((size_t)aButton >= mvButtonArray.size()) return false;
#ifdef WIN32
		return mvRemappedButtonArray[aButton];
#else
		return mvButtonArray[aButton];
#endif
	}

	//-----------------------------------------------------------------------


	cGamepadInputData cGamepadSDL2::GetButton()
	{
		cGamepadInputData button = mlstButtonsPressed.front();
		mlstButtonsPressed.pop_front();
		mlstInputUpdates.remove(button);

		return button;
	}

	//-----------------------------------------------------------------------

	bool cGamepadSDL2::ButtonIsPressed()
	{
		return mlstButtonsPressed.empty()==false;
	}

	//-----------------------------------------------------------------------

	cGamepadInputData cGamepadSDL2::GetReleasedButton()
	{
		cGamepadInputData button = mlstButtonsReleased.front();
		mlstButtonsReleased.pop_front();
		mlstInputUpdates.remove(button);

		return button;
	}

	//-----------------------------------------------------------------------

	bool cGamepadSDL2::ButtonIsReleased()
	{
		return mlstButtonsReleased.empty()==false;
	}

	//-----------------------------------------------------------------------

	float cGamepadSDL2::GetAxisValue(eGamepadAxis aAxis)
	{
        if ((size_t)aAxis >= mvAxisArray.size()) return 0;
#ifdef WIN32
		return mvRemappedAxisArray[aAxis];
#else
		return mvAxisArray[aAxis];
#endif
	}

	float cGamepadSDL2::GetAxisDeadZoneRadiusValue()
	{
		return mfDeadZoneRadius;
	}

	void cGamepadSDL2::SetAxisDeadZoneRadiusValue(float afValue)
	{
		mfDeadZoneRadius = afValue;
	}

	cGamepadInputData cGamepadSDL2::GetUpdatedAxis()
	{
		cGamepadInputData axis = mlstAxisChanges.front();
		mlstAxisChanges.pop_front();
		mlstInputUpdates.remove(axis);

		return axis;
	}

	bool cGamepadSDL2::AxesUpdated()
	{
		return mlstAxisChanges.empty()==false;
	}

	//-----------------------------------------------------------------------

	eGamepadHatState cGamepadSDL2::GetHatCurrentState(eGamepadHat aHat)
	{
#ifdef WIN32
		return mvHatArray[aHat];
#else
		return eGamepadHatState_Centered;
#endif
	}

	bool cGamepadSDL2::HatIsInState(eGamepadHat aHat, eGamepadHatState aState)
	{
		return (GetHatCurrentState(aHat)&aState)!=0;
	}

	bool cGamepadSDL2::HatsChanged()
	{
		return false;
	}

	cGamepadInputData cGamepadSDL2::GetHatState()
	{
		return cGamepadInputData();
	}
	
	//-----------------------------------------------------------------------

	cVector2l cGamepadSDL2::GetBallAbsPos(eGamepadBall aBall)
	{
		return cVector2l(0,0);
	}

	cVector2l cGamepadSDL2::GetBallRelPos(eGamepadBall aBall)
	{
		return cVector2l(0,0);
	}
	
	//-----------------------------------------------------------------------

	/////////////////////////////////////////////////////////////////////////
	// PRIVATE METHODS
	/////////////////////////////////////////////////////////////////////////
	
	//-----------------------------------------------------------------------
	
	eGamepadButton cGamepadSDL2::SDLToButton(Uint8 alButton)
	{
		return static_cast<eGamepadButton>(alButton);
	}

	eGamepadAxis	cGamepadSDL2::SDLToAxis(Uint8 alAxis)
	{
		return static_cast<eGamepadAxis>(alAxis);
	}

	float cGamepadSDL2::SDLToAxisValue(Sint16 alAxisValue)
	{
		return cMath::Clamp((float)alAxisValue*mfInvAxisMax, -1.0f, 1.0f);
	}
	
	eGamepadHat cGamepadSDL2::SDLToHat(Uint8 alHat)
	{
		return static_cast<eGamepadHat>(alHat);
	}

	eGamepadHatState cGamepadSDL2::SDLToHatState(Uint8 alHatState)
	{
		return static_cast<eGamepadHatState>(alHatState);
	}

	eGamepadBall cGamepadSDL2::SDLToBall(Uint8 alBall)
	{
		return static_cast<eGamepadBall>(alBall);
	}

	//-----------------------------------------------------------------------

}
#endif // !USE_SDL2