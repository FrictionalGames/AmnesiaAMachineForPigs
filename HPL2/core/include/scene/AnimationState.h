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

#ifndef HPL_ANIMATION_STATE_H
#define HPL_ANIMATION_STATE_H

#include "math/MathTypes.h"
#include "system/SystemTypes.h"
#include "graphics/GraphicsTypes.h"

#include "engine/SaveGame.h"

namespace hpl {

	class cAnimation;
	class cAnimationManager;
	class cMeshEntity;

	//-----------------------------------------------------------------------

	class cSkeletonAABB
	{
	public:
		cSkeletonAABB() { cSkeletonAABB(cVector3f(100000), cVector3f(-100000)); }
		cSkeletonAABB(cVector3f avMin, cVector3f avMax) { mvMin = avMin; mvMax = avMax; }

		inline void Expand(cSkeletonAABB aBox) { Expand(aBox.mvMin, aBox.mvMax); }
		inline void Expand(cVector3f avMin, cVector3f avMax) 
		{ 
			if(mvMax.x < avMax.x) mvMax.x = avMax.x;
			if(mvMax.y < avMax.y) mvMax.y = avMax.y;
			if(mvMax.z < avMax.z) mvMax.z = avMax.z;

			if(mvMin.x > avMin.x) mvMin.x = avMin.x;
			if(mvMin.y > avMin.y) mvMin.y = avMin.y;
			if(mvMin.z > avMin.z) mvMin.z = avMin.z;
		}
		void SetTime(float afTime) { mfTime = afTime; }

		cVector3f mvMin;
		cVector3f mvMax;
		float mfTime;
	};

	typedef std::vector<cSkeletonAABB> tSkeletonBoundsVec;
	typedef tSkeletonBoundsVec::iterator tSkeletonBoundsVecIt;

	//---------------------------------------------
	
	class cAnimationEvent
	{
	public:
		float mfTime;
		eAnimationEventType mType;
		tString msValue;
	};

	//---------------------------------------------
	
	class cAnimationTransition
	{
	public:
		cAnimationTransition(){}
		cAnimationTransition(int alAnimId, int alPreviousAnimId, float afMinTime, float afMaxTime) :
		mlPreviousAnimId(alPreviousAnimId), mlAnimId(alAnimId), mfMinTime(afMinTime), mfMaxTime(afMaxTime){}

		int mlPreviousAnimId;	//-1= default! The animation that is played before, for this to be used.
		int mlAnimId; //The transitional animation.
		float mfMinTime;
		float mfMaxTime;
	};

	//---------------------------------------------

	class cAnimationState
	{
	public:
		cAnimationState(cAnimation* apAnimation,const tString &asName,
						cAnimationManager *apAnimationManager);
		~cAnimationState();

        const tString& GetName(){ return msName;}

		void Update(float afTimeStep);

		bool DataIsInMeshFile(){return mpAnimationManager==NULL;}

		bool IsFading();
		bool IsFadingOut(){ return mfFadeStep<0;}

		/**
		 * If the animation has reached the end.
		 */
		bool IsOver();

		void FadeIn(float afTime);
		void FadeOut(float afTime);

		void FadeInSpeed(float afTime);
		void FadeOutSpeed(float afTime);

		void SetLength(float afLength);
		float GetLength();

		void SetWeight(float afWeight);
		float GetWeight();

        void SetSpeed(float afSpeed);
		float GetSpeed();

		void SetBaseSpeed(float afSpeed);
		float GetBaseSpeed();

		void SetTimePosition(float afPosition);
        float GetTimePosition();
		float GetPreviousTimePosition();

		/**
		 * Set the relative postion. 0 = start, 1 = end
		 * \param afPosition
		 */
		void SetRelativeTimePosition(float afPosition);

		/**
		* Get the relative postion. 0 = start, 1 = end
		*/
		float GetRelativeTimePosition();

		bool IsActive();
		void SetActive(bool abActive);

		bool IsLooping();
		void SetLoop(bool abLoop);

		bool IsPaused();
		void SetPaused(bool abPaused);

		void SetSpecialEventTime(float afT){ mfSpecialEventTime = afT;}
		float GetSpecialEventTime(){ return mfSpecialEventTime;}
		bool IsAfterSpecialEvent();
		bool IsBeforeSpecialEvent();

		void CreateSkeletonBoundsFromMesh(cMeshEntity * apMesh, tBoneStateVec * apvBoneStates);
		bool TryGetBoundingVolumeAtTime(float afTime, cVector3f & avMin, cVector3f & avMax);

		void AddTimePosition(float afAdd);

		cAnimation* GetAnimation();

		cAnimationEvent *CreateEvent();
		cAnimationEvent *GetEvent(int alIdx);
		int GetEventNum();

		/**
		  * If either time is -1 then no limits are checked.
		  */
		void AddTransition(int alAnimId, int alPreviousAnimId, float afMinTime, float afMaxTime);
		cAnimationTransition* GetTransitionFromPrevAnim(int alPreviousAnimId, float afPreviousTimePos);
		cAnimationTransition* GetTransition(int alIdx);
		int GetTransitionNum();

		float GetFadeStep(){ return mfFadeStep;}
		void SetFadeStep(float afX){ mfFadeStep = afX;}

		bool CanBlend() { return mbCanBlend; }
		void SetCanBlend( bool abCanBlend ) { mbCanBlend = abCanBlend; }
	
	private:
		tString msName;

		cAnimationManager *mpAnimationManager;

		cAnimation* mpAnimation;

		std::vector<cAnimationEvent*> mvEvents;

		std::vector<cAnimationTransition> mvTransitions;

		tSkeletonBoundsVec mvSkeletonBounds;

		//Properties of the animation
		float mfLength;
		float mfWeight;
		float mfSpeed;
		float mfTimePos;
		float mfPrevTimePos;

		float mfBaseSpeed;

		float mfSpecialEventTime;

		bool mbActive;
		bool mbLoop;
		bool mbPaused;
		bool mbCanBlend;

		//properties for update
		float mfFadeStep;
		float mfFadeSpeed;
	};

};
#endif // HPL_ANIMATION_STATE_H
