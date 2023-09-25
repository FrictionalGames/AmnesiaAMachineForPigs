/*
 * Copyright 2007-2010 (C) - Frictional Games
 *
 * This file is part of OALWrapper
 *
 * For conditions of distribution and use, see copyright notice in LICENSE
 */
#include "OALWrapper/OAL_EFXManager.h"
#include "OALWrapper/OAL_EffectSlot.h"
#include "OALWrapper/OAL_Effect.h"
#include "OALWrapper/OAL_Effect_Reverb.h"
#include "OALWrapper/OAL_Filter.h"
#include "OALWrapper/OAL_Device.h"

#include <SDL2/SDL_thread.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_version.h>

int SlotUpdaterThread(void* alUnusedArg);

extern cOAL_Device* gpDevice;

//  Set EFX func pointers to null (maybe this should be in OAL_Device.cpp? )

// Effect Slots

LPALGENAUXILIARYEFFECTSLOTS OAL_GenAuxiliaryEffectSlots = NULL;
LPALDELETEAUXILIARYEFFECTSLOTS OAL_DeleteAuxiliaryEffectSlots = NULL;
LPALISAUXILIARYEFFECTSLOT OAL_IsAuxiliaryEffectSlot = NULL;
LPALAUXILIARYEFFECTSLOTI OAL_AuxiliaryEffectSloti = NULL;
LPALAUXILIARYEFFECTSLOTIV OAL_AuxiliaryEffectSlotiv = NULL;
LPALAUXILIARYEFFECTSLOTF OAL_AuxiliaryEffectSlotf = NULL;
LPALAUXILIARYEFFECTSLOTFV OAL_AuxiliaryEffectSlotfv = NULL;
LPALGETAUXILIARYEFFECTSLOTI OAL_GetAuxiliaryEffectSloti = NULL;
LPALGETAUXILIARYEFFECTSLOTIV OAL_GetAuxiliaryEffectSlotiv = NULL;
LPALGETAUXILIARYEFFECTSLOTF OAL_GetAuxiliaryEffectSlotf = NULL;
LPALGETAUXILIARYEFFECTSLOTFV OAL_GetAuxiliaryEffectSlotfv = NULL;

// Effects

LPALGENEFFECTS OAL_GenEffects = NULL;
LPALDELETEEFFECTS OAL_DeleteEffects = NULL;
LPALISEFFECT OAL_IsEffect = NULL;
LPALEFFECTI OAL_Effecti = NULL;
LPALEFFECTIV OAL_Effectiv = NULL;
LPALEFFECTF OAL_Effectf = NULL;
LPALEFFECTFV OAL_Effectfv = NULL;
LPALGETEFFECTI OAL_GetEffecti = NULL;
LPALGETEFFECTIV OAL_GetEffectiv = NULL;
LPALGETEFFECTF OAL_GetEffectf = NULL;
LPALGETEFFECTFV OAL_GetEffectfv = NULL;

// Filters

LPALGENFILTERS OAL_GenFilters = NULL;
LPALDELETEFILTERS OAL_DeleteFilters = NULL;
LPALISFILTER OAL_IsFilter = NULL;
LPALFILTERI OAL_Filteri = NULL;
LPALFILTERIV OAL_Filteriv = NULL;
LPALFILTERF OAL_Filterf = NULL;
LPALFILTERFV OAL_Filterfv = NULL;
LPALGETFILTERI OAL_GetFilteri = NULL;
LPALGETFILTERIV OAL_GetFilteriv = NULL;
LPALGETFILTERF OAL_GetFilterf = NULL;
LPALGETFILTERFV OAL_GetFilterfv = NULL;

cOAL_EFXManager::cOAL_EFXManager() : mlNumSlots(0), mpvSlots(NULL), mplstEffectList(NULL), mplstFilterList(NULL)
{
}

cOAL_EFXManager::~cOAL_EFXManager()
{
}

bool cOAL_EFXManager::Initialize(int alNumSlotsHint, int alNumSends, bool abUseThreading, int alSlotUpdateFreq)
{
	DEF_FUNC_NAME(cOAL_EFXManager::Initialize);
	FUNC_USES_AL;

	ALuint lTempSlot[256];

	// Set up every EFX function pointer

	LogMsg("",eOAL_LogVerbose_Medium, eOAL_LogMsg_Info, "Initializing EFX Manager...\n" );
	LogMsg("",eOAL_LogVerbose_Medium, eOAL_LogMsg_Info, "Initializing function pointers...\n" );



	// Slot funcs
	OAL_GenAuxiliaryEffectSlots		=	(LPALGENAUXILIARYEFFECTSLOTS) alGetProcAddress ("alGenAuxiliaryEffectSlots");
	OAL_DeleteAuxiliaryEffectSlots	=	(LPALDELETEAUXILIARYEFFECTSLOTS) alGetProcAddress ("alDeleteAuxiliaryEffectSlots");
	OAL_IsAuxiliaryEffectSlot			=	(LPALISAUXILIARYEFFECTSLOT) alGetProcAddress ("alIsAuxiliaryEffectSlot");
	OAL_AuxiliaryEffectSloti			=	(LPALAUXILIARYEFFECTSLOTI) alGetProcAddress ("alAuxiliaryEffectSloti");
	OAL_AuxiliaryEffectSlotiv			=	(LPALAUXILIARYEFFECTSLOTIV) alGetProcAddress ("alAuxiliaryEffectSlotiv");
	OAL_AuxiliaryEffectSlotf			=	(LPALAUXILIARYEFFECTSLOTF) alGetProcAddress ("alAuxiliaryEffectSlotf");
	OAL_AuxiliaryEffectSlotfv			=	(LPALAUXILIARYEFFECTSLOTFV) alGetProcAddress ("alAuxiliaryEffectSlotfv");
	OAL_GetAuxiliaryEffectSloti		=	(LPALGETAUXILIARYEFFECTSLOTI) alGetProcAddress ("alGetAuxiliaryEffectSloti");
	OAL_GetAuxiliaryEffectSlotiv		=	(LPALGETAUXILIARYEFFECTSLOTIV) alGetProcAddress ("alGetAuxiliaryEffectSlotiv");
	OAL_GetAuxiliaryEffectSlotf		=	(LPALGETAUXILIARYEFFECTSLOTF) alGetProcAddress ("alGetAuxiliaryEffectSlotf");
	OAL_GetAuxiliaryEffectSlotfv		=	(LPALGETAUXILIARYEFFECTSLOTFV) alGetProcAddress ("alGetAuxiliaryEffectSlotfv");

	// Effect funcs
	OAL_GenEffects	=	(LPALGENEFFECTS) alGetProcAddress ("alGenEffects");
	OAL_DeleteEffects =	(LPALDELETEEFFECTS) alGetProcAddress ("alDeleteEffects");
	OAL_IsEffect		=	(LPALISEFFECT) alGetProcAddress ("alIsEffect");
	OAL_Effecti		=	(LPALEFFECTI) alGetProcAddress ("alEffecti");
	OAL_Effectiv		=	(LPALEFFECTIV) alGetProcAddress ("alEffectiv");
	OAL_Effectf		=	(LPALEFFECTF) alGetProcAddress ("alEffectf");
	OAL_Effectfv		=	(LPALEFFECTFV) alGetProcAddress ("alEffectfv");
	OAL_GetEffecti	=	(LPALGETEFFECTI) alGetProcAddress ("alGetEffecti");
	OAL_GetEffectiv	=	(LPALGETEFFECTIV) alGetProcAddress ("alGetEffectiv");
	OAL_GetEffectf	=	(LPALGETEFFECTF) alGetProcAddress ("alGetEffectf");
	OAL_GetEffectfv	=	(LPALGETEFFECTFV) alGetProcAddress ("alGetEffectfv");

	// Filter funcs
	OAL_GenFilters	= (LPALGENFILTERS) alGetProcAddress ("alGenFilters");
	OAL_DeleteFilters = (LPALDELETEFILTERS) alGetProcAddress ("alDeleteFilters");
	OAL_IsFilter		= (LPALISFILTER) alGetProcAddress ("alIsFilter");
	OAL_Filteri		= (LPALFILTERI) alGetProcAddress ("alFilteri");
	OAL_Filteriv		= (LPALFILTERIV) alGetProcAddress ("alFilteriv");
	OAL_Filterf		= (LPALFILTERF) alGetProcAddress ("alFilterf");
	OAL_Filterfv		= (LPALFILTERFV) alGetProcAddress ("alFilterfv");
	OAL_GetFilteri	= (LPALGETFILTERI) alGetProcAddress ("alGetFilteri");
	OAL_GetFilteriv	= (LPALGETFILTERIV) alGetProcAddress ("alGetFilteriv");
	OAL_GetFilterf	= (LPALGETFILTERF) alGetProcAddress ("alGetFilterf");
	OAL_GetFilterfv	= (LPALGETFILTERFV) alGetProcAddress ("alGetFilterfv");

	if (!(OAL_GenAuxiliaryEffectSlots && OAL_DeleteAuxiliaryEffectSlots && OAL_IsAuxiliaryEffectSlot &&
		OAL_AuxiliaryEffectSloti && OAL_AuxiliaryEffectSlotiv && OAL_AuxiliaryEffectSlotf && OAL_AuxiliaryEffectSlotfv &&
		OAL_GetAuxiliaryEffectSloti && OAL_GetAuxiliaryEffectSlotiv && OAL_GetAuxiliaryEffectSlotf && OAL_GetAuxiliaryEffectSlotfv &&

		OAL_GenEffects && OAL_DeleteEffects && OAL_IsEffect &&
		OAL_Effecti && OAL_Effectiv && OAL_Effectf && OAL_Effectfv &&
		OAL_GetEffecti && OAL_GetEffectiv && OAL_GetEffectf && OAL_GetEffectfv &&

		OAL_GenFilters && OAL_DeleteFilters && OAL_IsFilter &&
		OAL_Filteri && OAL_Filteriv && OAL_Filterf && OAL_Filterfv &&
		OAL_GetFilteri && OAL_GetFilteriv && OAL_GetFilterf && OAL_GetFilterfv))
	{
		LogMsg("",eOAL_LogVerbose_Medium, eOAL_LogMsg_Error, "Failed initializing function pointers\n" );
		return false;
	}
	LogMsg("",eOAL_LogVerbose_Medium, eOAL_LogMsg_Info, "Done\n" );

	LogMsg("",eOAL_LogVerbose_Medium, eOAL_LogMsg_Info, "Calculating max slots...\n" );

	while ( mlNumSlots < alNumSlotsHint )
	{
		RUN_AL_FUNC(OAL_GenAuxiliaryEffectSlots(1,&lTempSlot[mlNumSlots]));
		if (!AL_ERROR_OCCURED)
		{
			LogMsg("",eOAL_LogVerbose_High, eOAL_LogMsg_Info, "Effect Slot Object successfully created\n" );
			++mlNumSlots;
		}
		else
			break;
	}

	if ( mlNumSlots == 0 )
	{
		LogMsg("",eOAL_LogVerbose_Low, eOAL_LogMsg_Error, "Error creating Slots. Failed to initialize EFX.\n" );
		return false;
	}

	LogMsg("",eOAL_LogVerbose_Medium, eOAL_LogMsg_Info, "Created %d Low Level Effect Slots, %d hinted\n", mlNumSlots, alNumSlotsHint);

	RUN_AL_FUNC(OAL_DeleteAuxiliaryEffectSlots ( mlNumSlots, lTempSlot ));

	mpvSlots = new tSlotVector;
	mpvSlots->reserve(mlNumSlots);

	LogMsg("",eOAL_LogVerbose_Medium, eOAL_LogMsg_Info, "Creating High Level Effect Slots\n" );


	for ( int i = 0; i < mlNumSlots; i++ )
	{
		cOAL_EffectSlot *pSlot = new cOAL_EffectSlot(this, i);
		mpvSlots->push_back(pSlot);
	}

	LogMsg("",eOAL_LogVerbose_Medium, eOAL_LogMsg_Info, "Done creating Effect Slots\n" );


	LogMsg("",eOAL_LogVerbose_Medium, eOAL_LogMsg_Info, "Creating Filter and Effect containers\n" );
	mplstEffectList = new tOALEffectList;
	mplstFilterList = new tOALFilterList;

	if (!mplstEffectList || !mplstFilterList)
	{
		LogMsg("",eOAL_LogVerbose_Low, eOAL_LogMsg_Error, "Error creating containers. Failed to initialize EFX.\n" );
		return false;
	}
	else
		LogMsg("",eOAL_LogVerbose_Medium, eOAL_LogMsg_Info, "Done creating containers\n" );

	mlNumSends = alNumSends;
	mbUsingThread = abUseThreading;

	// Launch updater thread
	if (mbUsingThread)
	{
		LogMsg("",eOAL_LogVerbose_Medium, eOAL_LogMsg_Info, "Launching Slot updater thread...\n" );

		mlThreadWaitTime = 1000/alSlotUpdateFreq;

#if SDL_VERSION_ATLEAST(2, 0, 0)
		mpUpdaterThread = SDL_CreateThread ( SlotUpdaterThread, "EFX Slot Updater", NULL );
#else
		mpUpdaterThread = SDL_CreateThread ( SlotUpdaterThread, NULL );
#endif
	}

	LogMsg("",eOAL_LogVerbose_Medium, eOAL_LogMsg_Info, "EFX succesfully initialized.\n" );

	return true;


}

void cOAL_EFXManager::Destroy()
{
	tSlotVectorIt vSlotIterator;
	tOALEffectListIt lstEffectIterator;
	tOALFilterListIt lstFilterIterator;

	LogMsg("",eOAL_LogVerbose_Medium, eOAL_LogMsg_Command, "Destroying EFX Manager...\n" );

	if ( mbUsingThread )
	{
		LogMsg("",eOAL_LogVerbose_Medium, eOAL_LogMsg_Info, "Stopping Slot updater...\n" );
		mbUsingThread = false;
		SDL_WaitThread ( mpUpdaterThread, 0 );
		mpUpdaterThread = NULL;
	}

	LogMsg("",eOAL_LogVerbose_Medium, eOAL_LogMsg_Info, "Destroying Effect Slots...\n" );

	if (mpvSlots)
	{
		for ( vSlotIterator = mpvSlots->begin(); vSlotIterator != mpvSlots->end(); ++vSlotIterator )
		{
			(*vSlotIterator)->Reset();
			delete (*vSlotIterator);
		}
		mpvSlots->clear();
		delete mpvSlots;

		mpvSlots = NULL;
	}

	LogMsg("",eOAL_LogVerbose_Medium, eOAL_LogMsg_Info, "Destroying Effects...\n" );

	if (mplstEffectList)
	{
		for ( lstEffectIterator = mplstEffectList->begin(); lstEffectIterator != mplstEffectList->end(); ++lstEffectIterator )
		{
			delete (*lstEffectIterator);
		}
		mplstEffectList->clear();
		delete mplstEffectList;
		mplstEffectList = NULL;
	}

	LogMsg("",eOAL_LogVerbose_Medium, eOAL_LogMsg_Info, "Destroying Filters...\n" );

	if (mplstFilterList)
	{
		for ( lstFilterIterator = mplstFilterList->begin(); lstFilterIterator != mplstFilterList->end(); ++lstFilterIterator )
		{
			delete (*lstFilterIterator);
		}
		mplstFilterList->clear();
		delete mplstFilterList;
		mplstFilterList = NULL;
	}

	LogMsg("",eOAL_LogVerbose_Medium, eOAL_LogMsg_Info, "EFX Manager successfully destroyed\n" );

}

//////////////////////////////////////////////////////////////

cOAL_Filter* cOAL_EFXManager::CreateFilter()
{
    cOAL_Filter* pFilter = new cOAL_Filter;

	if(pFilter && pFilter->GetStatus())
	{
		mplstFilterList->push_back(pFilter);
		return pFilter;
	}
	else
	{
		delete  pFilter ;
		pFilter = NULL;

		return NULL;
	}
}

////////////////////////////////////////////////////////////

cOAL_Effect_Reverb* cOAL_EFXManager::CreateReverbEffect()
{
	cOAL_Effect_Reverb* pEffect = new cOAL_Effect_Reverb;

	if (pEffect && pEffect->GetStatus())
	{
		mplstEffectList->push_back(pEffect);
		return pEffect;
	}
	else
	{
        delete pEffect;
		pEffect = NULL;

		return NULL;
	}

}

////////////////////////////////////////////////////////////

void cOAL_EFXManager::DestroyFilter ( cOAL_Filter* apFilter )
{
	if (apFilter == NULL)
		return;

	mplstFilterList->remove(apFilter);
	delete apFilter;
}

////////////////////////////////////////////////////////////

void cOAL_EFXManager::DestroyEffect ( cOAL_Effect *apEffect )
{
	if (apEffect == NULL)
		return;

	mplstEffectList->remove(apEffect);
	delete apEffect;
}

////////////////////////////////////////////////////////////

int cOAL_EFXManager::UseEffect ( cOAL_Effect *apEffect )
{
	if (apEffect == NULL)
		return -1;

	cOAL_EffectSlot* pSlot = NULL;
	for (int i = 0; i < mlNumSlots; ++i)
	{
		pSlot = (*mpvSlots)[i];
		if (pSlot->IsFree())
		{
			if (pSlot->AttachEffect(apEffect))
            	return i;
			else
			{
				pSlot->Reset();
			}
		}
	}
	return -1;
}

void cOAL_EFXManager::UpdateSlots()
{
	cOAL_EffectSlot* pSlot;
	for (int i = 0; i < mlNumSlots; ++i)
	{
		pSlot = (*mpvSlots)[i];
		pSlot->Lock();
		pSlot->Update();
		pSlot->Unlock();
	}
}

inline int cOAL_EFXManager::GetThreadWaitTime()
{
	return mlThreadWaitTime;
}

int SlotUpdaterThread ( void* alUnusedArg )
{
	cOAL_EFXManager* pEFXManager = gpDevice->GetEFXManager();

	int lWaitTime = pEFXManager->GetThreadWaitTime();

	while(pEFXManager->IsThreadAlive())
	{
		//	While the thread lives, perform the update
        pEFXManager->UpdateSlots();
		//	And rest a bit
		SDL_Delay ( lWaitTime );
	}
	return 0;
}

