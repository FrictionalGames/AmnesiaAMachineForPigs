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

#include "sound/Sound.h"
#include "engine/IUpdateEventLoop.h"
#include "engine/Interface.h"
#include "system/LowLevelSystem.h"
#include "sound/LowLevelSound.h"
#include "resources/Resources.h"
#include "sound/SoundHandler.h"
#include "sound/MusicHandler.h"


namespace hpl {

	cSound::cSound(iLowLevelSound *apLowLevelSound) :
		mpLowLevelSound(apLowLevelSound)
	{
	}

	//-----------------------------------------------------------------------

	cSound::~cSound()
	{
		Log("Exiting Sound Module\n");
		Log("--------------------------------------------------------\n");

		hplDelete(mpSoundHandler);
		hplDelete(mpMusicHandler);

		Log("--------------------------------------------------------\n\n");
	}

	void cSound::Update(float afTimeStep)
	{
		mpSoundHandler->Update(afTimeStep);
		mpMusicHandler->Update(afTimeStep);

		mpLowLevelSound->UpdateSound(afTimeStep);
	}
	
	void cSound::Init(	cResources *apResources, int alSoundDeviceID, bool abUseEnvAudio, int alMaxChannels,
						int alStreamUpdateFreq, bool abUseThreading, bool abUseVoiceManagement,
						int alMaxMonoSourceHint, int alMaxStereoSourceHint,
						int alStreamingBufferSize, int alStreamingBufferCount, bool abEnableLowLevelLog)
	{
		mpResources = apResources;

		Log("Initializing Sound Module\n");
		Log("--------------------------------------------------------\n");

		mpLowLevelSound->Init(	alSoundDeviceID, abUseEnvAudio, alMaxChannels, alStreamUpdateFreq, abUseThreading,
								abUseVoiceManagement, alMaxMonoSourceHint, alMaxStereoSourceHint,
								alStreamingBufferSize, alStreamingBufferCount, abEnableLowLevelLog);

		mpSoundHandler = hplNew( cSoundHandler, (mpLowLevelSound, mpResources) );
		mpMusicHandler = hplNew( cMusicHandler, (mpLowLevelSound, mpResources) );

		Log("--------------------------------------------------------\n\n");
	}

}
