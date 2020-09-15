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

#ifndef LUX_JOURNAL_H
#define LUX_JOURNAL_H

//----------------------------------------------

#include "LuxBase.h"

//----------------------------------------

enum eLuxJournalState
{
	eLuxJournalState_Main,
	eLuxJournalState_Notes,
	eLuxJournalState_Hints,
	eLuxJournalState_Diaries,
	eLuxJournalState_QuestLog,
	eLuxJournalState_OpenNote,
	eLuxJournalState_OpenHint,
	eLuxJournalState_OpenDiary,
	eLuxJournalState_OpenNarratedDiary,

	eLuxJournalState_LastEnum,
};

//----------------------------------------

class cLuxDiaryContainer
{
public:
	~cLuxDiaryContainer();

	tString msType;
	std::vector<cLuxDiary*> mvDiaries;
};
//----------------------------------------

class cLuxNotePage
{
public:
	tString msVoice;
	tWStringVec mvRows;
};

//----------------------------------------

class iLuxJournalWidgetData
{
public:
	iLuxJournalWidgetData(iWidget *apWidget) : mpWidget(apWidget) {}
	virtual ~iLuxJournalWidgetData(){}

	virtual void Update(float afTimeStep)=0;

	iWidget *mpWidget;
	void *mpExtraData;
};

typedef std::list<iLuxJournalWidgetData*> tLuxJournalWidgetData;
typedef tLuxJournalWidgetData::iterator tLuxJournalWidgetDataIt;

class cLuxJournal;

//----------------------------------------

class cLuxJournalTextData : public iLuxJournalWidgetData
{
public:	
	cLuxJournalTextData(iWidget *apWidget, eLuxJournalState aType) : iLuxJournalWidgetData(apWidget), mType(aType), mfEffectfAlpha(0){}

	void Update(float afTimeStep);

	eLuxJournalState mType;
	float mfEffectfAlpha;
};

class cLuxJournalImageData : public iLuxJournalWidgetData
{
public:	
	cLuxJournalImageData(iWidget *apWidget, eLuxJournalState aType) : iLuxJournalWidgetData(apWidget), mType(aType), mfEffectfAlpha(0){}

	void Update(float afTimeStep);

	eLuxJournalState mType;
	float mfEffectfAlpha;
};


//----------------------------------------


class cLuxJournalStateData
{
public:
	cLuxJournalStateData(cLuxJournal *apJournal, eLuxJournalState aState);

	void Reset();
	void OnEnter();
	void DestroySessionWidgets();
	void Update(float afTimeStep);
	void OnDraw(float afFrameTime);

	tWidgetList mlstSessionWidgets;
	cWidgetDummy* mpRootWidget;
	float mfAlpha;

private:
	cLuxJournal *mpJournal;
	eLuxJournalState mState;
};

//----------------------------------------

class cLuxJournal_ListEntry
{
public:
	cLuxJournal_ListEntry() : mpJournal(NULL) {}
	cLuxJournal_ListEntry(cLuxJournal *apJournal) : mpJournal(apJournal) {} 
	void AddWidget(iWidget *apWidget);
	void AddIcon(iWidget *apWidget, int alHintID);
	void SetVisible(bool abX);
	
	tWidgetList mlstWidgets;
	tWidgetList mlstIcons;
	tIntList mlstIconHint;
	cLuxJournal *mpJournal;
};

class cLuxJournal_ListPage
{
public:
	void SetVisible(bool abX);

	std::vector<cLuxJournal_ListEntry> mvEntries;
};

//----------------------------------------

class cLuxJournal : public iLuxUpdateable
{
friend class cLuxMusicHandler_SaveData;
friend class cLuxJournalStateData;
friend class cLuxJournal_SaveData;
public:	
	cLuxJournal();
	~cLuxJournal();
	
	void OnClearFonts();
	void LoadFonts();

	void OnStart();
	void Reset();

	void OnGameStart();

	void Update(float afTimeStep);

	void OnEnterContainer(const tString& asOldContainer);
	void OnLeaveContainer(const tString& asNewContainer);

	void OnDraw(float afFrameTime);

	cGuiSet* GetSet() { return mpGuiSet; }

	void ExitPressed(bool abInstantExit);

	void Exit();

	void RemoveAllHints();

	void SetForceInstantExit(bool abX){ mbForceInstantExit = abX;}
	void SetOpenedFromInventory(bool abX){ mbOpenedFromInventory = abX;}

    cLuxNote* AddNote(const tString& asNameAndTextEntry, const tString& asImage);
    cLuxHint* AddHint(const tString& asNameAndTextEntry, const tString& asImage);
	void RemoveHint(const tString& asNameAndTextEntry);
	
	cLuxDiary* AddDiary(const tString& asNameAndTextEntry, const tString& asImage, int &alCurrentEntryIdx);
	
	bool AddQuestNote(const tString& asName, const tString& asNameAndTextEntry);
	bool DisableQuestNote(const tString& asName);
	cLuxQuestNote* GetQuestNote(const tString& asName);

	void ChangeState(eLuxJournalState aState);

	void OpenNote(cLuxNote *apNote, bool abNarration);
	cLuxNote* GetNote(int alIdx){ return mvNotes[alIdx];}


    void OpenHint(cLuxHint *apHint);
	cLuxHint* GetHint(int alIdx){ return mvHints[alIdx];}
	int GetHintNum(){ return (int)mvHints.size();}
	bool OpenLatestHint();

	void OpenDiary(cLuxDiary *apDiary, bool abNarration);
	void SetDiaryAsLastRead(cLuxDiary *apDiary);

	bool GetDisabled() { return mbDisabled; }
	void SetDisabled(bool abX) { mbDisabled = abX; }

	void OpenLastReadText();

	int GetLastReadHint() { return mlLastReadHint; }
	int GetLastReadDocument() { return mlLastReadDocument; }
	
private:
	cLuxDiaryContainer* CreateDiaryContainer(const tString& asType);

	void SetupLabel(cWidgetLabel *apLabel, const cVector2f& avSize, int alIdx, eLuxJournalState aState, iFontData *apFont=NULL, eFontAlign aFontAlign=eFontAlign_Center);
	void SetupImage(cWidgetImage *apImage, int alIdx, eLuxJournalState aState);
	void SetupNavigationWidgets(eLuxJournalState aState, int alListIndex, int alForwardIndex, int alBackwardIndex, cWidgetDummy *apRoot);

	void AddSessionWidget(eLuxJournalState aState, iWidget *apWidget);
	void ResetSessionVars();

	void SetStateBackgroundGfx(const tString& asFile);

	void LoadText(const tWString &asName ,const tWString &asText);
	void LoadNarrationText(const tWString &asName ,const tWString &asText);
	void SetNotePage(int alPageNum);
	
	int GetNoteListIndex(eLuxJournalState aState);//Return values: 0=notes, 1=diaries, 2=quests, 3 = hints
	void SetNoteListPage(int alPageNum, eLuxJournalState aState);
	void SetNoteListPageFromHint(int alHint);
	void SetNoteListPageFromNote(int alDiary);

	void CreateGui();
	void DestroyGui();

	void CreateMainGui();
	void CreateNotesGui();
	void CreateHintsGui();
	void CreateDiariesGui();
	void CreateQuestNotesGui();
	void CreateOpenNoteGui();

	void CreateBackground();
	void RenderBackgroundImage();
	void CreateScreenTextures();
	void DestroyBackground();

	///////////////////////
	// Gui callbacks
	bool MainMenuTextClick(iWidget* apWidget, const cGuiMessageData& aData);
	kGuiCallbackDeclarationEnd(MainMenuTextClick);

	bool MainMenuUIButtonPress(iWidget* apWidget, const cGuiMessageData& aData);
	kGuiCallbackDeclarationEnd(MainMenuUIButtonPress);

	bool MainMenuTextOnDraw(iWidget* apWidget, const cGuiMessageData& aData);
	kGuiCallbackDeclarationEnd(MainMenuTextOnDraw);
	
	bool ListTextOnDraw(iWidget* apWidget, const cGuiMessageData& aData);
	kGuiCallbackDeclarationEnd(ListTextOnDraw);

	bool NoteTextClick(iWidget* apWidget, const cGuiMessageData& aData);
	kGuiCallbackDeclarationEnd(NoteTextClick);

	bool NoteBackClick(iWidget* apWidget, const cGuiMessageData& aData);
	kGuiCallbackDeclarationEnd(NoteBackClick);

    bool HintTextClick(iWidget* apWidget, const cGuiMessageData& aData);
	kGuiCallbackDeclarationEnd(HintTextClick);

	bool DiaryTextClick(iWidget* apWidget, const cGuiMessageData& aData);
	kGuiCallbackDeclarationEnd(DiaryTextClick);
	
	bool ImageButtonOnDraw(iWidget* apWidget, const cGuiMessageData& aData);
	kGuiCallbackDeclarationEnd(ImageButtonOnDraw);

	bool NoteArrowClick(iWidget* apWidget, const cGuiMessageData& aData);
	kGuiCallbackDeclarationEnd(NoteArrowClick);

	bool NoteClickFrameClick(iWidget* apWidget, const cGuiMessageData& aData);
	kGuiCallbackDeclarationEnd(NoteClickFrameClick);
	
	bool JournalItemUIButtonPress(iWidget* apWidget, const cGuiMessageData& aData);
	kGuiCallbackDeclarationEnd(JournalItemUIButtonPress);
	
	bool JournalHintUIFocus(iWidget* apWidget, const cGuiMessageData& aData);
	kGuiCallbackDeclarationEnd(JournalHintUIFocus);

	bool JournalDocumentUIFocus(iWidget* apWidget, const cGuiMessageData& aData);
	kGuiCallbackDeclarationEnd(JournalDocumentUIFocus);

	bool UIListenerArrowPress(iWidget* apWidget, const cGuiMessageData& aData);
	kGuiCallbackDeclarationEnd(UIListenerArrowPress);

	bool UIListenerButtonPress(iWidget* apWidget, const cGuiMessageData& aData);
	kGuiCallbackDeclarationEnd(UIListenerButtonPress);

	bool NoteBackUIButtonPress(iWidget* apWidget, const cGuiMessageData& aData);
	kGuiCallbackDeclarationEnd(NoteBackUIButtonPress);

	//////////////
	// Variables
	bool mbActive;
	float mfAlpha;
	bool mbDisabled;

	bool mbForceInstantExit;
	bool mbOpenedFromInventory;

	int mlLastReadTextCat; //Only used by diary
	int mlLastReadTextEntry;
	int mlLastReadTextType;	//0=note 1=diary
	
	int mlLastReadHint;
	int mlLastReadDocument;

	std::vector<cLuxNote*> mvNotes;
	std::vector<cLuxDiaryContainer*> mvDiaryContainers;
	std::vector<cLuxQuestNote*> mvQuestNotes;

	std::vector<cLuxHint*> mvHints;

	
	cGuiGfxElement *mpStateBackgroundGfx;

	float mfMouseOverPulse;

	tWString msHeader;
	std::vector<cLuxNotePage> mvPages;
	int mlCurrentNotePage;
	
	cSoundEntry *mpVoiceEntry;
	int mlVoiceEntryID;

	std::vector<cLuxJournal_ListPage> mvNoteListPages[4];//0=notes, 1=diaries, 2=quests, 3=hints
	int mlCurrentNoteListPage[4];

	//////////////
	// Data
	cGui *mpGui;
	cScene *mpScene;
	cGraphics *mpGraphics;

	cViewport *mpViewport;
	cGuiSkin *mpGuiSkin;
	cGuiSet *mpGuiSet;

	eLuxJournalState mCurrentState;
	std::vector<cLuxJournalStateData*> mvStateData;

	tLuxJournalWidgetData mlstSessionWidgetData;

	iWidget		 *mpWidgetDefaultNav[eLuxJournalState_LastEnum];
	cWidgetImage *mpImageForward[eLuxJournalState_LastEnum];
	iWidget		 *mpWidgetReturn[eLuxJournalState_LastEnum];
	cWidgetImage *mpImageBackward[eLuxJournalState_LastEnum];
	
	iTexture *mpScreenTexture;
	cGuiGfxElement *mpScreenGfx;
	iTexture *mpScreenBgTexture;
	cGuiGfxElement *mpScreenBgGfx;

	cGuiGfxElement *mpWhiteGfx;

	iFontData *mpFontDefault;
	iFontData *mpFontMenu;

	iGpuProgram *mpEffectProgram;

	cVector2f mvScreenSize;
	cVector2f mvGuiSetCenterSize;//Size of the part that is inside a 4:3 ratio!
	cVector2f mvGuiSetSize;
	cVector2f mvGuiSetOffset;
	cVector3f mvGuiSetStartPos;

	float mfNoteTextWidth;
	int mlNoteMaxPageRows;
	cVector2f mvNoteFontSize;
	float mfNoteRowDist;
	cVector2f mfNoteHeaderFontSize;
	float mfNoteHeaderStartY;
	float mfNoteTextStartY;

	float mfNoteListHeaderY;
	float mfMaxNoteListY;

	float mfBackTextY;
	cVector2f mvBackTextFontSize;

};

//----------------------------------------------


#endif // LUX_JOURNAL_H
