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

#if !USE_SDL2

#include "impl/GamepadSDL.h"

#include "impl/LowLevelInputSDL.h"

#include "math/Math.h"
#include <limits>

#ifdef Win32
#include <conio.h>
#endif

namespace hpl {
	
	//////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	float cGamepadSDL::mfInvAxisMax = 1.0f/(float)cMath::Min(cMath::Abs(std::numeric_limits<Sint16>::min()), std::numeric_limits<Sint16>::max());
	float cGamepadSDL::mfDeadZoneRadius = 3200.0f*cGamepadSDL::mfInvAxisMax;

	//-----------------------------------------------------------------------
	
	cGamepadSDL::cGamepadSDL(cLowLevelInputSDL *apLowLevelInputSDL, int alIndex) : iGamepad("SDL Portable Gamepad", alIndex)
	{
		mpLowLevelInputSDL = apLowLevelInputSDL;

		mpHandle = SDL_JoystickOpen(mlIndex);

		if(mpHandle)
		{
			msGamepadName = tString(SDL_JoystickName(mlIndex));

			mvButtonArray.resize(SDL_JoystickNumButtons(mpHandle));
			mvButtonArray.assign(mvButtonArray.size(), false);

			mvAxisArray.resize(SDL_JoystickNumAxes(mpHandle));
			mvAxisArray.assign(mvAxisArray.size(), 0.0f);

			mvHatArray.resize(SDL_JoystickNumHats(mpHandle));
			mvHatArray.assign(mvHatArray.size(), eGamepadHatState_Centered);

			mvBallAbsPosArray.resize(SDL_JoystickNumBalls(mpHandle));
			mvBallAbsPosArray.assign(mvBallAbsPosArray.size(), cVector2l(0,0));
			mvBallRelPosArray.resize(SDL_JoystickNumBalls(mpHandle));
			mvBallRelPosArray.assign(mvBallRelPosArray.size(), cVector2l(0,0));
		}
		//ClearKeyList();
	}
	
	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PUBLIC METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	int cGamepadSDL::GetNumButtons()
	{
		return (int)mvButtonArray.size();
	}

	int cGamepadSDL::GetNumAxes()
	{
		return (int)mvAxisArray.size();
	}

	int cGamepadSDL::GetNumHats()
	{
		return (int)mvHatArray.size();
	}

	int cGamepadSDL::GetNumBalls()
	{
		return (int)mvBallAbsPosArray.size();
	}

	//-----------------------------------------------------------------------


	void cGamepadSDL::Update()
	{
		cGamepadInputData inputUpdate;

		mlstInputUpdates.clear();
		mlstButtonsPressed.clear();
		mlstButtonsReleased.clear();
		mlstHatStateChanges.clear();

		std::list<SDL_Event>::iterator it = mpLowLevelInputSDL->mlstEvents.begin();
        for(; it != mpLowLevelInputSDL->mlstEvents.end(); ++it)
		{
			SDL_Event *pEvent = &(*it);

			if((pEvent->type != SDL_JOYAXISMOTION	&& 
				pEvent->type != SDL_JOYBUTTONUP		&&
				pEvent->type != SDL_JOYBUTTONDOWN	&&
				pEvent->type != SDL_JOYHATMOTION	&& 
				pEvent->type != SDL_JOYBALLMOTION)	||
				pEvent->jaxis.which	!= mlIndex)
			{
                // The above check works because "which" is at the same location for all joystick events
                // so we only need to check ONE of them the way this check is written
				continue;
			}

			if(pEvent->type==SDL_JOYAXISMOTION)
			{
				eGamepadAxis axis = SDLToAxis(pEvent->jaxis.axis);
				float fAxisValue = SDLToAxisValue(pEvent->jaxis.value);
				
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
			else if(pEvent->type==SDL_JOYHATMOTION)
			{
				eGamepadHat hat = SDLToHat(pEvent->jhat.hat);
				eGamepadHatState state = SDLToHatState(pEvent->jhat.value);

				inputUpdate = cGamepadInputData(mlIndex, eGamepadInputType_Hat, hat, (float)state);
				mlstHatStateChanges.push_back(inputUpdate);

				mlstInputUpdates.push_back(inputUpdate);

				mvHatArray[hat] = state;
			}
			else if(pEvent->type==SDL_JOYBALLMOTION)
			{
				eGamepadBall ball = SDLToBall(pEvent->jball.ball);
				
				cVector2l vDelta = cVector2l(pEvent->jball.xrel, pEvent->jball.yrel);
				mvBallRelPosArray[ball] = vDelta;
				mvBallAbsPosArray[ball] += vDelta;
			}
			else
			{
				eGamepadButton button = SDLToButton(pEvent->jbutton.button);
				inputUpdate = cGamepadInputData(mlIndex, eGamepadInputType_Button, button, 0.0f);

				bool bPressed;
				if(pEvent->type==SDL_JOYBUTTONUP)
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
		}
	}

	//-----------------------------------------------------------------------

	bool cGamepadSDL::HasInputUpdates()
	{
		return mlstInputUpdates.empty()==false;
	}

	cGamepadInputData cGamepadSDL::GetInputUpdate()
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
		case eGamepadInputType_Hat:
			mlstHatStateChanges.remove(input);
			break;
		case eGamepadInputType_Axis:
			mlstAxisChanges.remove(input);
			break;
		}

		return input;
	}

	//-----------------------------------------------------------------------

    bool cGamepadSDL::ButtonIsDown(eGamepadButton aButton)
	{
		return mvButtonArray[aButton];
	}

	//-----------------------------------------------------------------------


	cGamepadInputData cGamepadSDL::GetButton()
	{
		cGamepadInputData button = mlstButtonsPressed.front();
		mlstButtonsPressed.pop_front();
		mlstInputUpdates.remove(button);

		return button;
	}

	//-----------------------------------------------------------------------

	bool cGamepadSDL::ButtonIsPressed()
	{
		return mlstButtonsPressed.empty()==false;
	}

	//-----------------------------------------------------------------------

	cGamepadInputData cGamepadSDL::GetReleasedButton()
	{
		cGamepadInputData button = mlstButtonsReleased.front();
		mlstButtonsReleased.pop_front();
		mlstInputUpdates.remove(button);

		return button;
	}

	//-----------------------------------------------------------------------

	bool cGamepadSDL::ButtonIsReleased()
	{
		return mlstButtonsReleased.empty()==false;
	}

	//-----------------------------------------------------------------------

	float cGamepadSDL::GetAxisValue(eGamepadAxis aAxis)
	{
		return mvAxisArray[aAxis];
	}

	float cGamepadSDL::GetAxisDeadZoneRadiusValue()
	{
		return mfDeadZoneRadius;
	}

	void cGamepadSDL::SetAxisDeadZoneRadiusValue(float afValue)
	{
		mfDeadZoneRadius = afValue;
	}

	cGamepadInputData cGamepadSDL::GetUpdatedAxis()
	{
		cGamepadInputData axis = mlstAxisChanges.front();
		mlstAxisChanges.pop_front();
		mlstInputUpdates.remove(axis);

		return axis;
	}

	bool cGamepadSDL::AxesUpdated()
	{
		return mlstAxisChanges.empty()==false;
	}

	//-----------------------------------------------------------------------

	eGamepadHatState cGamepadSDL::GetHatCurrentState(eGamepadHat aHat)
	{
		return mvHatArray[aHat];
	}

	bool cGamepadSDL::HatIsInState(eGamepadHat aHat, eGamepadHatState aState)
	{
		return (GetHatCurrentState(aHat)&aState)!=0;
	}

	bool cGamepadSDL::HatsChanged()
	{
		return mlstHatStateChanges.empty()==false;
	}

	cGamepadInputData cGamepadSDL::GetHatState()
	{
		cGamepadInputData change = mlstHatStateChanges.front();
		mlstHatStateChanges.pop_front();
		return change;
	}
	
	//-----------------------------------------------------------------------

	cVector2l cGamepadSDL::GetBallAbsPos(eGamepadBall aBall)
	{
		return mvBallAbsPosArray[aBall];
	}

	cVector2l cGamepadSDL::GetBallRelPos(eGamepadBall aBall)
	{
		return mvBallRelPosArray[aBall];
	}
	
	//-----------------------------------------------------------------------

	/////////////////////////////////////////////////////////////////////////
	// PRIVATE METHODS
	/////////////////////////////////////////////////////////////////////////
	
	//-----------------------------------------------------------------------
	
	eGamepadButton cGamepadSDL::SDLToButton(Uint8 alButton)
	{
		return static_cast<eGamepadButton>(alButton);
	}

	eGamepadAxis	cGamepadSDL::SDLToAxis(Uint8 alAxis)
	{
		return static_cast<eGamepadAxis>(alAxis);
	}

	float cGamepadSDL::SDLToAxisValue(Sint16 alAxisValue)
	{
		return cMath::Clamp((float)alAxisValue*mfInvAxisMax, -1.0f, 1.0f);
	}
	
	eGamepadHat cGamepadSDL::SDLToHat(Uint8 alHat)
	{
		return static_cast<eGamepadHat>(alHat);
	}

	eGamepadHatState cGamepadSDL::SDLToHatState(Uint8 alHatState)
	{
		return static_cast<eGamepadHatState>(alHatState);
	}

	eGamepadBall cGamepadSDL::SDLToBall(Uint8 alBall)
	{
		return static_cast<eGamepadBall>(alBall);
	}

	//-----------------------------------------------------------------------

}
#endif // !USE_SDL2