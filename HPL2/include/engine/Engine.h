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

#pragma once

#include <engine/IUpdateEventLoop.h>
#include "system/SystemTypes.h"
#include "engine/EngineTypes.h"
#include <engine/RTTI.h>
#include <engine/EngineInterface.h>
#include <memory>
#include <mutex>


#include <Common_3/Graphics/Interfaces/IGraphics.h>
#include <FixPreprocessor.h>

namespace hpl {

	class cUpdater;
	class iLowLevelEngineSetup;
	class iLowLevelSystem;
	class cLogicTimer;

	class cSystem;
	class cInput;
	class cResources;
	class cGraphics;
	class cScene;
	class cSound;
	class cPhysics;
	class cAI;
	class cHaptic;
	class cGui;
	class cGenerate;
	class cEngine;
	class cEngineInitVars;
	class iTimer;

	//------------------------------------------------------

	class cFPSCounter
	{
	public:
		cFPSCounter(iLowLevelSystem* apLowLevelSystem);

		void AddFrame();

		float mfFPS;
		float mfUpdateRate;
	private:
		iLowLevelSystem* mpLowLevelSystem;
		int mlFramecounter;
		float mfFrametimestart;
		float mfFrametime;
	};

	//---------------------------------------------------

	class cSetupVarContainer
	{
	public:
		cSetupVarContainer();

		void AddString(const tString& asName, const tString& asValue);

		void AddInt(const tString& asName, int alValue);
		void AddFloat(const tString& asName, float afValue);
		void AddBool(const tString& asName, bool abValue);

		const tString& GetString(const tString& asName);

		float GetFloat(const tString& asName, float afDefault);
		int GetInt(const tString& asName,int alDefault);
		bool GetBool(const tString& asName, bool abDefault);
	private:
		std::map<tString, tString>  m_mapVars;
		tString msBlank;
	};

	//---------------------------------------------------

	extern cEngine* CreateHPLEngine(eHplAPI aApi, tFlag alHplModuleFlags, cEngineInitVars *apVars);
	extern void DestroyHPLEngine(cEngine* apGame);

	//---------------------------------------------------

	class cEngine : public EngineInterface
	{
		HPL_RTTI_IMPL_CLASS(EngineInterface, cEngine, "{a7b5b5a0-1b9f-11df-8c4e-0800200c9a66}")
	public:

		cEngine(iLowLevelEngineSetup *apGameSetup,tFlag alHplSetupFlags, cEngineInitVars *apVars);
		~cEngine();
	private:
		void GameInit(iLowLevelEngineSetup *apGameSetup,tFlag alHplSetupFlags, cEngineInitVars *apVars);

	public:

		/**
		 * Starts the game loop. To make stuff run they must be added as updatables..
		 */
		void Run();
		/**
		 * Exists the game.
		 * \todo is this a good way to do it? Should game be global. If so, make a singleton.
		 */
		void Exit();
		bool GetGameIsDone();

		virtual cScene* GetScene() override { return mpScene;}
		virtual cResources* GetResources() override { return mpResources;}
		virtual cUpdater* GetUpdater() override { return mpUpdater;}
		virtual cSystem* GetSystem() override { return mpSystem;}
		virtual cInput* GetInput() override { return mpInput;}
		virtual cGraphics* GetGraphics() override { return mpGraphics;}
		virtual cSound* GetSound() override { return mpSound;}
		virtual cPhysics* GetPhysics() override { return mpPhysics;}
		virtual cAI* GetAI() override { return mpAI;}
		virtual cGui* GetGui() override { return mpGui;}
		virtual cHaptic* GetHaptic() override { return mpHaptic;}

		void ResetLogicTimer();
		void SetUpdatesPerSec(int alUpdatesPerSec);
		int GetUpdatesPerSec();
		float GetStepSize();

		cLogicTimer* GetLogicTimer(){ return mpLogicTimer;}

		void SetSpeedMul(float afX);

		float GetFPS();
		float GetAvgFrameTimeInMS();

		void SetFPSUpdateRate(float afSec);
		float GetFPSUpdateRate();

		void SetRenderOnce(bool abX){mbRenderOnce = abX;}

		float GetFrameTime(){ return mfFrameTime;}

		double GetGameTime(){ return mfGameTime;}

		void SetLimitFPS(bool abX){ mbLimitFPS = abX;}
		bool GetLimitFPS(){ return mbLimitFPS;}

		void SetWaitIfAppOutOfFocus(bool abX){ mbWaitIfAppOutOfFocus =abX;}
		bool GetWaitIfAppOutOfFocus(){ return mbWaitIfAppOutOfFocus;}

		void SetPaused(bool abPaused);
		bool GetPaused();

		static void SetDeviceWasPlugged() { mbDevicePlugged = true; }
		static void SetDeviceWasRemoved() { mbDeviceRemoved = true; }

		///// SCRIPT VAR METHODS ////////////////////

		cScriptVar* CreateLocalVar(const tString& asName);
		cScriptVar* GetLocalVar(const tString& asName);
		tScriptVarMap* GetLocalVarMap();
		cScriptVar* CreateGlobalVar(const tString& asName);
		cScriptVar* GetGlobalVar(const tString& asName);
		tScriptVarMap* GetGlobalVarMap();

		void ClearAllVariables();

		////// ENGINE VARIABLES /////////////////////

		eVariableType GetEngineTypeFromString(const tString& asType);
		eVariableType GetEngineTypeFromStringW(const tWString& asType);
		const tString& GetEngineTypeString(eVariableType aType) { return mvEngineTypeStrings[aType]; }


	private:
		void UpdateFrameTimer();

		void CheckAndBroadcastFocusChange();

		void CheckIfAppInFocusElseWait();

		void CheckAndBroadcastDeviceChange();

		bool mbGameIsDone;

		bool mbRenderOnce;

		bool mbPaused;

		float mfFrameTime;

		double mfGameTime;

		iLowLevelEngineSetup *mpGameSetup;
		cUpdater *mpUpdater;
		cLogicTimer *mpLogicTimer;

		std::mutex m_mutex;

		cFPSCounter* mpFPSCounter;

		iTimer *mpFrameTimer;

		bool mbLimitFPS;

		tScriptVarMap m_mapLocalVars;
		tScriptVarMap m_mapGlobalVars;

		bool mbApplicationHasInputFocus;
		bool mbApplicationHasMouseFocus;
		bool mbApplicationIsVisible;

		bool mbWaitIfAppOutOfFocus;

		static bool mbDevicePlugged;
		static bool mbDeviceRemoved;

		tStringVec mvEngineTypeStrings;

		//Modules that Game connnect to:
        SwapChain* m_swapChain;
		Renderer* m_renderer;

		cResources *mpResources;
		cSystem *mpSystem;
		cInput *mpInput;
		cGraphics *mpGraphics;
		cScene *mpScene;
		cSound *mpSound;
		cPhysics *mpPhysics;
		cAI *mpAI;
		cHaptic *mpHaptic;
		cGui *mpGui;

		IUpdateEventLoop::UpdateEvent::Handler m_soundUpdateHandler;
		IUpdateEventLoop::UpdateEvent::Handler m_sceneUpdateHandler;
		IUpdateEventLoop::UpdateEvent::Handler m_physicsUpdateHandler;
		IUpdateEventLoop::UpdateEvent::Handler m_graphicsUpdateHandler;
		IUpdateEventLoop::UpdateEvent::Handler m_inputUpdateHandler;
		IUpdateEventLoop::UpdateEvent::Handler m_guiUpdateHandler;
		IUpdateEventLoop::UpdateEvent::Handler m_resourcesUpdateHandler;

	};

};
