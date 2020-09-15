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

#ifndef HPL_TIMER_SDL_H
#define HPL_TIMER_SDL_H

#include "system/Timer.h"

#ifdef WIN32   // Windows system specific
	#include <windows.h>
#else          // Unix based system specific
	#include <sys/time.h>
#endif

namespace hpl {

	class cTimerSDL : public iTimer
	{
	public:
		
		cTimerSDL();
		~cTimerSDL();
		
        void Start();
		void Stop();

		double GetTimeInMicroSec(); 
	private:
		double mfStartTimeInMicroSec;
		double mfEndTimeInMicroSec;
		bool   mbStopped;
		
		#ifdef WIN32
			LARGE_INTEGER mFrequency; 
			LARGE_INTEGER mStartCount;
			LARGE_INTEGER mEndCount;  
		#else
			double mFrequency;
			timeval mStartCount;
			timeval mEndCount;
		#endif
	};

};
#endif // HPL_TIMER_SDL_H
