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

#include "system/Thread.h"

#include "math/Math.h"

namespace hpl {

	//////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	iThread::iThread()
	{
		mlSleepTime = 0;
		mbThreadActive = false;
		mPrio = eThreadPrio_Normal;
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PUBLIC METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	void iThread::SetUpdateRate(float afUpdateRate)
	{
		 SetSleepTime(cMath::RoundToInt(1000.0f/afUpdateRate));
	}
	
	//-----------------------------------------------------------------------

	void iThread::SetPriority(eThreadPrio aPrio)
	{
		mPrio = aPrio;
	}

	//-----------------------------------------------------------------------
	
	//////////////////////////////////////////////////////////////////////////
	// PRIVATE METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// MAIN THREAD CODE
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	int iThread::MainThreadFunc(void* apThread)
	{
		if(apThread==NULL)
			return -1;

		iThread* pThread = (iThread*)apThread;
		iThreadClass* pThreadClass = pThread->mpThreadClass;

		while(pThread->IsActive())
		{
			pThreadClass->UpdateThread();
			pThread->Sleep(pThread->GetSleepTime());
		}

		return 0;
	}

}
