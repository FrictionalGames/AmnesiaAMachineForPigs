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

#include "LuxEnemy_Tesla.h"
#include "LuxMapHandler.h"
#include "LuxMapHelper.h"
#include "LuxPlayer.h"
#include "LuxPlayerHelpers.h"
#include "LuxMap.h"
#include "LuxBase.h"
#include "LuxProp.h"
#include "impl/PhysicsBodyNewton.h"
#include <algorithm>

//////////////////////////////////////////////////////////////////////////
// LOADER
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cLuxEnemyLoader_Tesla::cLuxEnemyLoader_Tesla(const tString& asName) : cLuxEnemyLoader_ManPig(asName)
{
}

//-----------------------------------------------------------------------

iLuxEnemy *cLuxEnemyLoader_Tesla::CreateEnemy(const tString& asName, int alID, cLuxMap *apMap)
{
	return hplNew(cLuxEnemy_Tesla, (asName, alID,apMap) );
}

//-----------------------------------------------------------------------

void cLuxEnemyLoader_Tesla::LoadVariables(iLuxEnemy *apEnemy, cXmlElement *apRootElem)
{
    cLuxEnemyLoader_ManPig::LoadVariables( apEnemy, apRootElem );

    cLuxEnemy_Tesla * pTeslaPig = static_cast<cLuxEnemy_Tesla*>(apEnemy);

	pTeslaPig->mfLightUpdateInterval = GetVarFloat("LightUpdateInterval");
}

//-----------------------------------------------------------------------

void cLuxEnemyLoader_Tesla::LoadInstanceVariables(iLuxEnemy *apEnemy, cResourceVarsObject *apInstanceVars)
{
    cLuxEnemyLoader_ManPig::LoadInstanceVariables( apEnemy, apInstanceVars );
    cLuxEnemy_Tesla * pTeslaPig = static_cast<cLuxEnemy_Tesla*>(apEnemy);

    pTeslaPig->mfDarkeningRadius = apInstanceVars->GetVarFloat("DarkeningRadius", 10.0f);
    pTeslaPig->mfStrobeRadius = apInstanceVars->GetVarFloat("StrobeRadius", 2.0f);
    pTeslaPig->mfMinTimeBetweenStrobeFlashes  = apInstanceVars->GetVarFloat("MinTimeBetweenStrobeFlashes", 2.0f);
    pTeslaPig->mfMaxTimeBetweenStrobeFlashes  = apInstanceVars->GetVarFloat("MaxTimeBetweenStrobeFlashes", 5.0f);
    pTeslaPig->mfChanceOfDoubleStrobeFlash  = apInstanceVars->GetVarFloat("ChanceOfDoubleStrobeFlash", 0.1f);
    pTeslaPig->mfChanceOfTripleStrobeFlash  = apInstanceVars->GetVarFloat("ChanceOfTripleStrobeFlash", 0.05f);
    pTeslaPig->mfMinTimeBetweenRepeatedFlashes  = apInstanceVars->GetVarFloat("MinTimeBetweenRepeatedFlashes", 0.2f);
    pTeslaPig->mfMaxTimeBetweenRepeatedFlashes  = apInstanceVars->GetVarFloat("MaxTimeBetweenRepeatedFlashes", 0.6f);
    pTeslaPig->mfMinStrobeFlashDuration  = apInstanceVars->GetVarFloat("MinStrobeFlashDuration", 0.05f);
    pTeslaPig->mfMaxStrobeFlashDuration  = apInstanceVars->GetVarFloat("MaxStrobeFlashDuration", 0.1f);

    pTeslaPig->mbUseDarkeningEffect = apInstanceVars->GetVarBool("UseDarkeningEffect", true);
    if ( pTeslaPig->mbUseDarkeningEffect )
    {
        pTeslaPig->GetMeshEntity()->SetIlluminationAmount( pTeslaPig->mbInfluencedLightsOn ? 2.4f : 0.0f );
    }
    else
    {
        pTeslaPig->GetMeshEntity()->SetIlluminationAmount( 2.4f );
    }
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cLuxEnemy_Tesla::cLuxEnemy_Tesla(const tString &asName, int alID, cLuxMap *apMap) : cLuxEnemy_ManPig(asName,alID,apMap)
{
    mfPulseTimer = 0.0f;
    mpMouthLight = NULL;
    mpMouthBone = NULL;

    mTranslationOffset = cVector3f(0.03333f, -0.13333f,-0.01666f);
    mAngleOffset = cVector3f(1.5707964f, 0.016666666f, -2.0374627f);
    mLightColor = cColor(2.5f,4,4,4);

    mfLightUpdateTimer = 0.0f;
    mbInfluencedLightsOn = false;

    mfMinTimeBetweenStrobeFlashes = 2.0f;
    mfMaxTimeBetweenStrobeFlashes = 5.0f;
    mfChanceOfDoubleStrobeFlash = 0.1f;
    mfChanceOfTripleStrobeFlash = 0.05f;
    mfMinTimeBetweenRepeatedFlashes = 0.3f;
    mfMaxTimeBetweenRepeatedFlashes = 0.8f;
    mfMinStrobeFlashDuration = 0.1f;
    mfMaxStrobeFlashDuration = 0.2f;
    mnRepeatedFlashCount = 0;
    mfDarkeningRadius = 2.0f;
    mfTimeToNextFlashEvent = cMath::RandRectf(mfMinTimeBetweenStrobeFlashes, mfMaxTimeBetweenStrobeFlashes);
}

//-----------------------------------------------------------------------

cLuxEnemy_Tesla::~cLuxEnemy_Tesla()
{
    if ( mpMouthLight != NULL )
    {
        mpWorld->DestroyLight(mpMouthLight);
    }
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

void cLuxEnemy_Tesla::UpdateEnemySpecific(float afTimeStep)
{
    if ( mpMouthLight == NULL && !IsInvisible() )
    {
        CreateSpotLight(
            0,
            0,
            10,
            true,
            kPif / 3.0f,
            mLightColor
            );
    }

    if ( mpMouthLight != NULL )
    {
        mpMouthLight->SetMatrix(//mpMouthLight->GetLocalMatrix());    // :HACK: to trigger update of transform based on parent node
            cMath::MatrixMul( cMath::MatrixTranslate( mTranslationOffset ), cMath::MatrixRotate( mAngleOffset, eEulerRotationOrder_XYZ ) )
            );
    }
    
    mfPulseTimer += cMath::RandRectf(0.4f,0.8f) * afTimeStep;
    this->GetMeshEntity()->SetShaderTimer( mfPulseTimer );

    /*
    //float pulseFactor = 1.2f + 0.8f * sin( mfPulseTimer ) + 0.4f * fabs(sin(  mfPulseTimer ));
    float pulseFactor = 2.4f;
    this->GetMeshEntity()->SetIlluminationAmount( pulseFactor );
    mpMouthLight->SetDiffuseColor( mLightColor * pulseFactor );
    */

    if ( mbUseDarkeningEffect )
    {
        UpdateDarkening(afTimeStep);
    }

    super_class::UpdateEnemySpecific(afTimeStep);

#if _DEBUG
    if ( gpBase->mpEngine->GetInput()->GetKeyboard()->KeyIsDown(eKey_I) )
    {
        mTranslationOffset.x += afTimeStep;
    }
    if ( gpBase->mpEngine->GetInput()->GetKeyboard()->KeyIsDown(eKey_K) )
    {
        mTranslationOffset.x -= afTimeStep;
    }

    if ( gpBase->mpEngine->GetInput()->GetKeyboard()->KeyIsDown(eKey_O) )
    {
        mTranslationOffset.y += afTimeStep;
    }
    if ( gpBase->mpEngine->GetInput()->GetKeyboard()->KeyIsDown(eKey_L) )
    {
        mTranslationOffset.y -= afTimeStep;
    }

    if ( gpBase->mpEngine->GetInput()->GetKeyboard()->KeyIsDown(eKey_P) )
    {
        mTranslationOffset.z += afTimeStep;
    }
    if ( gpBase->mpEngine->GetInput()->GetKeyboard()->KeyIsDown(eKey_M) )
    {
        mTranslationOffset.z -= afTimeStep;
    }

    if ( gpBase->mpEngine->GetInput()->GetKeyboard()->KeyIsDown(eKey_Z) )
    {
        mAngleOffset.x += afTimeStep;
    }
    if ( gpBase->mpEngine->GetInput()->GetKeyboard()->KeyIsDown(eKey_X) )
    {
        mAngleOffset.x -= afTimeStep;
    }

    if ( gpBase->mpEngine->GetInput()->GetKeyboard()->KeyIsDown(eKey_C) )
    {
        mAngleOffset.y += afTimeStep;
    }
    if ( gpBase->mpEngine->GetInput()->GetKeyboard()->KeyIsDown(eKey_V) )
    {
        mAngleOffset.y -= afTimeStep;
    }

    if ( gpBase->mpEngine->GetInput()->GetKeyboard()->KeyIsDown(eKey_B) )
    {
        mAngleOffset.z += afTimeStep;
    }
    if ( gpBase->mpEngine->GetInput()->GetKeyboard()->KeyIsDown(eKey_N) )
    {
        mAngleOffset.z -= afTimeStep;
    }
#endif
}

//-----------------------------------------------------------------------

void cLuxEnemy_Tesla::CreateSpotLight(const cVector3f& avPos, const cVector3f& avAngles,float afRadius, bool abShadows, float afFOV , const cColor &aColor)
{
	mpMouthLight = mpWorld->CreateLightSpot("MouthLight");
	mpMouthLight->SetDiffuseColor(aColor);
	mpMouthLight->SetRadius(afRadius);
    mpMouthLight->SetMatrix( cMatrixf::Identity );
	//mpMouthLight->SetMatrix(cMath::MatrixRotate(avAngles, eEulerRotationOrder_XYZ));
	//mpMouthLight->SetPosition(avPos);
	mpMouthLight->SetFOV(afFOV);
    mpMouthLight->SetCastShadows(abShadows);

	mpMouthBone = GetMeshEntity()->GetBoneStateFromName( "Bip001_Jaw" );

    mpMouthLight->SetParent( mpMouthBone );

    if ( mbUseDarkeningEffect )
    {
        mpMouthLight->SetVisible( mbInfluencedLightsOn );
    }
}

//-------------------------------------------

void cLuxEnemy_Tesla::OnRenderSolidImplemented(cRendererCallbackFunctions* apFunctions)
{
	super_class::OnRenderSolidImplemented(apFunctions);

	DebugRenderMouthLight();
}

//-----------------------------------------------------------------------

void cLuxEnemy_Tesla::DebugRenderMouthLight()
{
    if ( mpMouthBone && mpMouthLight )
    {
	    cColor renderColor = cColor(0.0f, 1.0f, 1.0f);
	    iLowLevelGraphics *pLowLevelGfx = gpBase->mpEngine->GetGraphics()->GetLowLevel();
	    if ( pLowLevelGfx )
	    {
            mpMouthLight->GetFrustum()->Draw(pLowLevelGfx,renderColor);
	    }
    }
}

//-----------------------------------------------------------------------

bool cLuxEnemy_Tesla::ShouldUpdateLightList(float afTimeStep)
{
    if ( !mbInfluencedLightsOn )
    {
        mfLightUpdateTimer -= afTimeStep;

        if ( mfLightUpdateTimer <= 0.0f )
        {
            mfLightUpdateTimer += mfLightUpdateInterval;
            return true;
        }
    }

    return false;
}

//-----------------------------------------------------------------------

void cLuxEnemy_Tesla::UpdateLightList(tLightList & alstAddedLights, tLightList & alstRemovedLights)
{
    cLuxMap *pCurrentMap = gpBase->mpMapHandler->GetCurrentMap();
	if(pCurrentMap==NULL) return;

    tLightList lstIntersectingLights; 

	cWorld *pWorld = pCurrentMap->GetWorld();
	iPhysicsWorld *pPhysicsWorld = pCurrentMap->GetPhysicsWorld();

    gpBase->mpMapHelper->GetLightsAtNode(pWorld->GetRenderableContainer(eWorldContainerType_Static)->GetRoot(), lstIntersectingLights, GetCharacterBody()->GetPosition(), mfDarkeningRadius, true, true);
	gpBase->mpMapHelper->GetLightsAtNode(pWorld->GetRenderableContainer(eWorldContainerType_Dynamic)->GetRoot(), lstIntersectingLights, GetCharacterBody()->GetPosition(), mfDarkeningRadius, true, true);

    tLightList lstStrobeIntersectingLights; 

    gpBase->mpMapHelper->GetLightsAtNode(pWorld->GetRenderableContainer(eWorldContainerType_Static)->GetRoot(), lstStrobeIntersectingLights, GetCharacterBody()->GetPosition(), mfStrobeRadius, true, false);
	gpBase->mpMapHelper->GetLightsAtNode(pWorld->GetRenderableContainer(eWorldContainerType_Dynamic)->GetRoot(), lstStrobeIntersectingLights, GetCharacterBody()->GetPosition(), mfStrobeRadius, true, false);

    mlstStrobeLights = lstStrobeIntersectingLights;
    
    mlstStrobeLamps.clear();

    tLightListIt it = mlstStrobeLights.begin();
    for(; it != mlstStrobeLights.end(); ++it)
	{
        iLight* light = *it;

        if ( light->HasParentEntity() )
        {
            cPhysicsBodyNewton * pNParent = dynamic_cast<cPhysicsBodyNewton*>(light->GetParentEntity());
            if ( pNParent != NULL )
            {
                void * user_data = pNParent->GetUserData();
                if ( user_data != NULL )
                {
                    iLuxEntity * pEntity = (iLuxEntity*) user_data;

                    if ( pEntity->GetEntityType() == eLuxEntityType_Prop )
                    {
                        iLuxProp * pProp = (iLuxProp*) pEntity;

                        if ( pProp->GetPropType() == eLuxPropType_Lamp )
                        {
                            cLuxProp_Lamp * pLamp =(cLuxProp_Lamp*)pProp;
                            mlstStrobeLamps.push_back( pLamp );

                            if ( pLamp->GetLit() )
                            {
                                pLamp->SetLit( false, false );
                            }
                        }
                    }
                }
            }
        }
    }

	////////////////////////////
	//Iterate lights and get lightlevel from each
	it = lstIntersectingLights.begin();
	for(; it != lstIntersectingLights.end(); ++it)
	{
        if ( std::find( mlstInfluencedLights.begin(), mlstInfluencedLights.end(), *it ) == mlstInfluencedLights.end() )
        {
            alstAddedLights.push_back( *it );
        }
    }

    it = mlstInfluencedLights.begin();
	for(; it != mlstInfluencedLights.end(); ++it)
	{
        if ( std::find( lstIntersectingLights.begin(), lstIntersectingLights.end(), *it ) == lstIntersectingLights.end() )
        {
            alstRemovedLights.push_back( *it );
        }
    }

    mlstInfluencedLights = lstIntersectingLights;
}

//-----------------------------------------------------------------------

void cLuxEnemy_Tesla::UpdateDarkening(float afTimeStep)
{
    if ( ShouldUpdateLightList(afTimeStep) )
    {
        tLightList lstAddedLights, lstRemovedLights;
        UpdateLightList( lstAddedLights, lstRemovedLights );

        tLightListIt it = lstAddedLights.begin();
	    for(; it != lstAddedLights.end(); ++it)
	    {
            (*it)->SetVisible(mbInfluencedLightsOn);

            /* if we want to, place to play lamp disabled effects
            if ( !mbInfluencedLightsOn )
            {
                cWorld *pWorld = mpMap->GetWorld();

                cParticleSystem *pPS = pWorld->CreateParticleSystem("Bzzt", "ps_electro_switch_sparks",1);
		        if (pPS) pPS->SetPosition((*it)->GetWorldPosition());
            }*/
        }

        it = lstRemovedLights.begin();
	    for(; it != lstRemovedLights.end(); ++it)
	    {
            (*it)->SetVisible(true);
        }
    }

    bool oldInfluencedLightsOn = mbInfluencedLightsOn;

    UpdateStrobeState( afTimeStep );

    if ( oldInfluencedLightsOn != mbInfluencedLightsOn )
    {
        tLightListIt it = mlstStrobeLights.begin();
	    for(; it != mlstStrobeLights.end(); ++it)
	    {
            (*it)->SetVisible(mbInfluencedLightsOn);
        }

        /*cWorld *pWorld = gpBase->mpMapHandler->GetCurrentMap()->GetWorld();

	    pWorld->SetFogActive(mbInfluencedLightsOn);*/

        std::list<cLuxProp_Lamp *>::iterator lampIt = mlstStrobeLamps.begin();
	    for(; lampIt != mlstStrobeLamps.end(); ++lampIt)
	    {
            (*lampIt)->SetLit(mbInfluencedLightsOn, false);
        }

        if ( mbInfluencedLightsOn )
        {
            const float pulseFactor = 2.4f;
            GetMeshEntity()->SetIlluminationAmount( pulseFactor );

            if ( mpMouthLight != NULL )
            {
                mpMouthLight->SetVisible(true);
                mpMouthLight->SetDiffuseColor( mLightColor * pulseFactor );
            }
        }
        else
        {
            GetMeshEntity()->SetIlluminationAmount( 0.0f );
            if ( mpMouthLight != NULL )
            {
                mpMouthLight->SetVisible(false);
            }
        }
    }
}

//-----------------------------------------------------------------------

void cLuxEnemy_Tesla::UpdateStrobeState(float afTimeStep)
{
    mfTimeToNextFlashEvent -= afTimeStep;

    while ( mfTimeToNextFlashEvent < 0 )
    {
        if ( mbInfluencedLightsOn )
        {
            // we are in a flash. see if next up is a repeated flash or not
            mbInfluencedLightsOn = false;
            
            if ( mnRepeatedFlashCount > 0 )
            {
                mnRepeatedFlashCount--;
                mfTimeToNextFlashEvent += cMath::RandRectf(mfMinTimeBetweenRepeatedFlashes, mfMaxTimeBetweenRepeatedFlashes);
            }
            else
            {
                mfTimeToNextFlashEvent += cMath::RandRectf(mfMinTimeBetweenStrobeFlashes, mfMaxTimeBetweenStrobeFlashes);

                float randomizer = cMath::RandRectf(0.0f, 1.0f);

                if ( randomizer <= mfChanceOfDoubleStrobeFlash )
                {
                    mnRepeatedFlashCount = 1;
                }
                else
                {
                    randomizer -= mfChanceOfDoubleStrobeFlash;

                    if ( randomizer <= mfChanceOfTripleStrobeFlash )
                    {
                        mnRepeatedFlashCount = 2;
                    }
                    else
                    {
                        mnRepeatedFlashCount = 0;
                    }
                }
            }
        }
        else
        {
            mbInfluencedLightsOn = true;
            mfTimeToNextFlashEvent += cMath::RandRectf(mfMinStrobeFlashDuration, mfMaxStrobeFlashDuration);
        }
    }
}


//-----------------------------------------------------------------------

bool cLuxEnemy_Tesla::CanSeeMovingPlayer()
{
    if ( mbUseDarkeningEffect )
    {
        cLuxPlayerIsMoving * player_is_moving_helper = gpBase->mpPlayer->GetHelperIsMoving();

        if ( mbInfluencedLightsOn )
        {
            if ( !player_is_moving_helper->PlayerIsMovingSlowly() )
            {
                return false;
            }
        }
        else
        {
            if ( !player_is_moving_helper->PlayerIsMovingFast() )
            {
                return false;
            }
        }
    }

    return mbCanSeePlayer;
}

//-----------------------------------------------------------------------

bool cLuxEnemy_Tesla::StateEventImplement(int alState, eLuxEnemyStateEvent aEvent, cLuxStateMessage *apMessage)
{
    //kLuxBeginStateMachine
    return super_class::StateEventImplement(alState, aEvent, apMessage);
}


