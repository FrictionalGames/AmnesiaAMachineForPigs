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

#include "launcher.h"
#include "hpl.h"
#include "QualityChooser.h"

#include "FL/fl_ask.H"

#ifdef WIN32
	#include <Windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>

#include <SDL2/SDL.h>

using namespace hpl;

//--------------------------------------------------------------------------------

#include "../game/LuxBasePersonal.h"

//-----------------------------------------------------------------------

cEngine* gpEngine = NULL;

#ifdef USERDIR_RESOURCES
tWString gsUserResourceDir;
#endif

//tWString gsInitConfigFile;
tWString gsMainSaveFolder;

tString gsDefaultBaseLanguage;
tString gsDefaultGameLanguage;

tString gsBaseLanguageFolder;
tString gsGameLanguageFolder;
tString gsCustomStoriesPath;

tWString gsDefaultMainConfigPath;
tWString gsDefaultMainConfigPathLow;
tWString gsDefaultMainConfigPathMedium;
tWString gsDefaultMainConfigPathHigh;
tString gsGameName;
tWString gsBaseSavePath;
tWString gsCrashFlagPath;

std::vector<cConfigFile*> gvPresets;

bool InitPaths(const tWString& asInitConfigFile)
{
    // Load this up as a global to search alternate resource path
	tWString sPersonalDir = cPlatform::GetSystemSpecialPath(eSystemPath_Personal);

    tWString sInitConfigFile = asInitConfigFile;
#ifdef USERDIR_RESOURCES
    gsUserResourceDir = sPersonalDir+PERSONAL_RELATIVEROOT PERSONAL_RELATIVEGAME_PARENT PERSONAL_RESOURCES;
    if (cPlatform::FileExists(gsUserResourceDir + asInitConfigFile) {
        sInitConfigFile = gsUserResourceDir + asInitConfigFile;
    }
#endif
	/////////////////////////
	// Load the Init file
	cConfigFile *pInitCfg = hplNew(cConfigFile, (sInitConfigFile));
	if(pInitCfg->Load()==false){
		fl_message("%s",cString::To8Char((_W("Could not load main init file: ")+asInitConfigFile)).c_str());
		return false;
	}
	
	
	//Set the name of the folder (in Lux) that all save stuff will be put.
	gsMainSaveFolder = pInitCfg->GetStringW("Directories","MainSaveFolder",_W(""));
	
	
	//Get the config file paths
	gsDefaultMainConfigPath = pInitCfg->GetStringW("ConfigFiles", "DefaultMainSettingsSDL2",_W(""));


	// Get presets paths
	gsDefaultMainConfigPathLow = pInitCfg->GetStringW("ConfigFiles", "DefaultMainSettingsLow",_W(""));
	gsDefaultMainConfigPathMedium = pInitCfg->GetStringW("ConfigFiles", "DefaultMainSettingsMedium",_W(""));
	gsDefaultMainConfigPathHigh = pInitCfg->GetStringW("ConfigFiles", "DefaultMainSettingsHigh",_W(""));

	gsDefaultBaseLanguage = pInitCfg->GetString("ConfigFiles", "DefaultBaseLanguage", "");
	gsDefaultGameLanguage = pInitCfg->GetString("ConfigFiles", "DefaultGameLanguage", "");

	//Directories
	gsBaseLanguageFolder = pInitCfg->GetString("Directories","BaseLanguageFolder","");
	gsGameLanguageFolder = pInitCfg->GetString("Directories","GameLanguageFolder","");
    gsCustomStoriesPath = pInitCfg->GetString("Directories","CustomStoryPath","");

	//Various variables
	gsGameName = pInitCfg->GetString("Variables","GameName","");
	

	//Delete the config file
	hplDelete(pInitCfg);

    //////////////////////////////
    //Set up the directories to be created
    tWStringVec vDirs;
    hpl::SetupBaseDirs(vDirs, PERSONAL_RELATIVEGAME_PARENT, gsMainSaveFolder
#ifdef USERDIR_RESOURCES
                       , true, hpl::cString::To16Char(gsCustomStoriesPath)
#endif
                       );
    //Create directories	
    hpl::CreateBaseDirs(vDirs, sPersonalDir);

	//Set the base directory from which all saving will take place.
	gsBaseSavePath = sPersonalDir+PERSONAL_RELATIVEROOT PERSONAL_RELATIVEGAME_PARENT + gsMainSaveFolder + _W("/");

	//Set Crash flag file path
	gsCrashFlagPath = gsBaseSavePath + _W("crash_flag");

	// Store the log file in the users personal directory not in the install directory
	SetLogFile(gsBaseSavePath + _W("launcher.log"));
	return true;
}


//--------------------------------------------------------------------------------

cConfigFile* LoadConfigFile(const tWString& asDefaultPath, const tWString& asWantedPath, bool abForceLoadDefault)
{
	cConfigFile *pConfig;
	bool bLoadedWantedPath = false;
	
	//////////////////////
	//Check if wanted exist and created config using existing file
	if(abForceLoadDefault==false && cPlatform::FileExists(asWantedPath))
	{
		pConfig = hplNew( cConfigFile, (asWantedPath) );
		bLoadedWantedPath = true;
	}
	else
	{
#ifdef USERDIR_RESOURCES
		pConfig = hplNew( cConfigFile, (asDefaultPath, gsUserResourceDir) );
#else
		pConfig = hplNew( cConfigFile, (asDefaultPath) );
#endif
	}

	////////////////////////////////////
	// Load the settings config file
	if(pConfig->Load()==false)
	{
		//msErrorMessage = _W("Failed to load config file!");
		return NULL;
	}

	//Set correct path for saving.
	if(bLoadedWantedPath==false)
	{
		pConfig->SetFileLocation(asWantedPath);
	}

	return pConfig;
}

//--------------------------------------------------------------------------------

bool LoadLanguage(cEngine* apEngine, const tString& asName, bool abForceReload)
{
	cResources *pResources = apEngine->GetResources();

	////////////////////////////////////////////
	//Check if the language is already loaded.
	//tString sLowName = cString::ToLowerCase(asName);
	//if(gsCurrentLanguage == sLowName && abForceReload==false) return false;

	//if(gsCurrentLanguage != "")
	//{
		pResources->ClearTranslations();	
	//}

	//msCurrentLanguage = sLowName;

	tString sGameFileName = cString::SetFileExt(asName,"lang");
	tString sBaseFileName = "base_"+sGameFileName;

	////////////////////////////////////////////
	//Load the language files
	pResources->AddLanguageFile(gsBaseLanguageFolder + sBaseFileName, true);

    
	////////////////////////////////////////////
	//If not default language, add default to so only missing entries are filled in
    if(sGameFileName != gsDefaultGameLanguage)	
		pResources->AddLanguageFile(gsGameLanguageFolder + gsDefaultGameLanguage, false);

	if(sBaseFileName != gsDefaultBaseLanguage)	
	pResources->AddLanguageFile(gsBaseLanguageFolder + gsDefaultBaseLanguage, false);
    
	return true;
}

tString SDL2GetRenderer() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

    SDL_Window* sdlwin = SDL_CreateWindow("GL Info",0, 0, 50, 50, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
    SDL_GLContext sdlctx = SDL_GL_CreateContext(sdlwin);
    tString ret;
    const char* pCardString = (const char*)glGetString(GL_RENDERER);
    if (pCardString)
    {
        ret = pCardString;
    }
    SDL_GL_DeleteContext(sdlctx);
    SDL_DestroyWindow(sdlwin);
    return ret;
}

//--------------------------------------------------------------------------------

int hplMain(const tString &asCommandLine)
{
	tWString sParams = cString::To16Char(asCommandLine);
	tWString sCfgFile = _W("config/main_init.cfg");
#if 0 // Pigs doesn't use cfg file as command arg
	if(sParams==_W("ptest"))
		sCfgFile = _W("config/ptest_main_init.cfg");
	else if(! sParams.empty())
		sCfgFile = sParams;
#endif

	if(InitPaths(sCfgFile)==false)
		return -1;

	// Create video card database handler
	cQualityChooser* pChooser = hplNew(cQualityChooser,("launcher/launcher_card_database.cfg"));

    tString sCardString = SDL2GetRenderer();

	tWString sConfigFilePath = gsBaseSavePath + _W("main_settings.cfg");

	bool bConfigFileExists = cPlatform::FileExists(sConfigFilePath);
	bool bLastInitCrashed = cPlatform::FileExists(gsCrashFlagPath);
	/////////////////////////////////////////////////
	// Load the main settings
	cConfigFile* pMainConfig = LoadConfigFile(gsDefaultMainConfigPath, sConfigFilePath, bLastInitCrashed);
	if(pMainConfig==NULL) return -1;
	cEngineInitVars vars;
	vars.mSound.mlSoundDeviceID = pMainConfig->GetInt("Sound", "Device", -1);
	//////////////////////////////////////////////////////////////////////////
	// If sound devices should be filtered, set this before creating engine
	#if defined(WIN32)
	iLowLevelSound::SetSoundDeviceNameFilter("software");
	#endif

	gpEngine = CreateHPLEngine(eHplAPI_OpenGL, eHplSetup_Video, &vars);
	
	/////////////////////////////////////////////////
	// Load presets
	gvPresets.push_back(hplNew(cConfigFile, (gsDefaultMainConfigPathLow)));
	gvPresets.push_back(hplNew(cConfigFile, (gsDefaultMainConfigPathMedium)));
	gvPresets.push_back(hplNew(cConfigFile, (gsDefaultMainConfigPathHigh)));
	gvPresets.push_back(NULL);
	
	for(int i=0;i<(int)gvPresets.size();++i)
	{
		if(gvPresets[i]) gvPresets[i]->Load();
	}

	//////////////////////////////////////////////////
	// Run FLTK window in a loop
	int exitflag = -1;
	bool bShowLauncher = pMainConfig->GetBool("Main", "ShowLauncher", true);

	if(bShowLauncher)
	{
		while(exitflag==-1)
		{
			LoadLanguage(gpEngine, pMainConfig->GetString("Main", "StartLanguage", "english.lang"), true);
			cUserInterface* win = hplNew(cUserInterface,(exitflag, pMainConfig, bConfigFileExists, bLastInitCrashed, sCardString, pChooser, gvPresets, gpEngine ));

			int ret = win->Run();
			if(ret) 
			{

			} 
			else 
			{
				ret = exitflag;
			}

			hplDelete(win);
		}
	}

	////////////////////////////////////////////
	// Set up game exe
	tWString sGameExe;
#ifdef WIN32
	sGameExe = _W("aamfp.exe");
#elif __linux__
	#if (defined(i386) && !defined(__LP64__))
		sGameExe = _W("./AmnesiaAMFP.bin.x86");
	#else
		sGameExe = _W("./AmnesiaAMFP.bin.x86_64");
	#endif
#endif


	//////////////////////////////
	// PTEST arg
	if(exitflag==2)
		sParams = _W("ptest");

	///////////////////////////////
	// Call program
    if(exitflag!=0)
		cPlatform::RunProgram(sGameExe, sParams);
		
	///////////////////////////////////////
	// Clean up
	DestroyHPLEngine(gpEngine);

	hplDelete(pMainConfig);
	hplDelete(pChooser);

	return 0;
}
