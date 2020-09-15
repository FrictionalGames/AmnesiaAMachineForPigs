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

#ifndef HPL_LOWLEVELINPUT_SDL_H
#define HPL_LOWLEVELINPUT_SDL_H

#include <list>
#include "input/LowLevelInput.h"

#if USE_SDL2
#include "SDL2/SDL_events.h"
#else
#include "SDL/SDL_events.h"
#endif

namespace hpl {

	class iLowLevelGraphics;

	class cLowLevelInputSDL : public iLowLevelInput
	{
	public:
		cLowLevelInputSDL(iLowLevelGraphics *apLowLevelGraphics);
		~cLowLevelInputSDL();

		void LockInput(bool abX);
		void RelativeMouse(bool abX);

		void BeginInputUpdate();
		void EndInputUpdate();

		void InitGamepadSupport();
		void DropGamepadSupport();

		bool DirtyGamepads();
		int GetPluggedGamepadNum();

		iMouse* CreateMouse();
		iKeyboard* CreateKeyboard();
		iGamepad* CreateGamepad(int alIndex);

		iLowLevelGraphics* GetLowLevelGraphics() { return mpLowLevelGraphics; }

		
		bool isQuitMessagePosted();
		void resetQuitMessagePosted();
	public:
		std::list<SDL_Event> mlstEvents;
		int mlConnectedDevices;
		int mlCheckDeviceChange;
		bool mbDirtyGamepads;
		
	private: 
		iLowLevelGraphics *mpLowLevelGraphics;
		bool mbQuitMessagePosted;
	};
};
#endif // HPL_LOWLEVELINPUT_SDL_H
