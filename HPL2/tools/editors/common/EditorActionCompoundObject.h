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

#ifndef HPLEDITOR_EDITOR_ACTION_COMPOUND_OBJECT_H
#define HPLEDITOR_EDITOR_ACTION_COMPOUND_OBJECT_H

#include "EditorAction.h"

//---------------------------------------------------------------------

class cEditorActionCompoundObjectAddEntities : public iEditorActionAggregateAddComponents
{
public:
	cEditorActionCompoundObjectAddEntities(iEditorWorld* apEditorWorld, int alID, const tIntList& alstEntityIDs);
	//~cEditorActionCompoundObjectAddEntities();

	//void DoModify();
	//void UndoModify();

protected:
	iEntityWrapperAggregate* GetAggregateFromEntity(iEntityWrapper*);
	//int mlID;
	//tIntVec mvEntityIDs;
	//tEntityDataVec mvOldCompoundData;
};

//---------------------------------------------------------------------

class cEditorActionCompoundObjectRemoveEntities : public iEditorActionAggregateRemoveComponents
{
public:
	cEditorActionCompoundObjectRemoveEntities(iEditorWorld* apEditorWorld,const tIntList& alstEntityIDs);
	//~cEditorActionCompoundObjectRemoveEntities();

//	void DoModify();
//	void UndoModify();

protected:
	iEntityWrapperAggregate* GetAggregateFromEntity(iEntityWrapper*);
//	tEntityDataVec mvOldCompoundData;

//	tIntVec mvEntityIDs;
};

//---------------------------------------------------------------------

#endif // HPLEDITOR_EDITOR_ACTION_COMPOUND_OBJECT_H
