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

#include "LuxProp_Lamp.h"

#include "LuxPlayer.h"
#include "LuxPlayerState.h"
#include "LuxMap.h"
#include "LuxInventory.h"
#include "LuxMessageHandler.h"
#include "LuxHelpFuncs.h"
#include "LuxGlobalDataHandler.h"

//////////////////////////////////////////////////////////////////////////
// LOADER
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cLuxPropLoader_Lamp::cLuxPropLoader_Lamp(const tString& asName) : iLuxPropLoader(asName)
{
}

//-----------------------------------------------------------------------

iLuxProp *cLuxPropLoader_Lamp::CreateProp(const tString& asName, int alID, cLuxMap *apMap)
{
	return hplNew(cLuxProp_Lamp, (asName, alID,apMap) );
}

//-----------------------------------------------------------------------

void cLuxPropLoader_Lamp::LoadVariables(iLuxProp *apProp, cXmlElement *apRootElem)
{
	cLuxProp_Lamp  *pLamp = static_cast<cLuxProp_Lamp*>(apProp);

	///////////////////////////
	// General
	pLamp->mbCanBeLitByPlayer = GetVarBool("CanBeLitByPlayer", true);

	pLamp->mbCanBeGrabbed = GetVarBool("CanBeGrabbed", false);

	pLamp->mbCanBeTurnedOff = GetVarBool("CanBeTurnedOff", false);
	pLamp->mbNeedsTinderbox = GetVarBool("NeedsTinderbox", false);

	///////////////////////////
	// Grab specific
	pLamp->mGrabData.mbGrabUseDepth = GetVarBool("GrabUseDepth", false);
	pLamp->mGrabData.mfGrabDepth = GetVarFloat("GrabDepth", 1.5f);
	pLamp->mGrabData.mfGrabDepthInc = GetVarFloat("GrabDepthInc", 0.1f);
	pLamp->mGrabData.mfGrabMinDepth = GetVarFloat("GrabMinDepth", 1.0f);
	pLamp->mGrabData.mfGrabMaxDepth = GetVarFloat("GrabMaxDepth", 2.0f);
	pLamp->mGrabData.mvGrabPositionOffset = GetVarVector3f("GrabPositionOffset", 0.0f);
	pLamp->mGrabData.mvGrabRotationOffset = cMath::Vector3ToRad(GetVarVector3f("GrabRotationOffset", 0.0f));
	pLamp->mGrabData.mfGrabThrowImpulse = GetVarFloat("GrabThrowImpulse", 10.0f);
	pLamp->mGrabData.mbGrabUseOffset = GetVarBool("GrabUseOffset", true);
	pLamp->mGrabData.mfGrabMassMul = GetVarFloat("GrabMassMul", 0.1f);
	pLamp->mGrabData.mfForceMul = GetVarFloat("GrabForceMul", 1);
	pLamp->mGrabData.mfTorqueMul = GetVarFloat("GrabTorqueMul", 1);
	pLamp->mGrabData.mbUseRotation = GetVarBool("GrabUseRotation", true);
}

//-----------------------------------------------------------------------

void cLuxPropLoader_Lamp::LoadInstanceVariables(iLuxProp *apProp, cResourceVarsObject *apInstanceVars)
{
    cLuxProp_Lamp  *pLamp = static_cast<cLuxProp_Lamp*>(apProp);

	pLamp->SetLit(apInstanceVars->GetVarBool("Lit",true), false);

	/////////////////////////////////
	//Connect a light to the lamp
	pLamp->msConnectionLight = apInstanceVars->GetVarString("ConnectedLight","");
	pLamp->mfConnectionLightAmount = apInstanceVars->GetVarFloat("ConnectionLightAmount",0);
	pLamp->mbConnectionLightUseOnColor = apInstanceVars->GetVarBool("ConnectionLightUseOnColor",false);
	pLamp->mbConnectionLightUseSpec = apInstanceVars->GetVarBool("ConnectionLightUseSpec",false);

	pLamp->msConnectionLight2 = apInstanceVars->GetVarString("ConnectedLight2","");
	pLamp->mfConnectionLight2Amount = apInstanceVars->GetVarFloat("ConnectionLight2Amount",0);
	pLamp->mbConnectionLight2UseOnColor = apInstanceVars->GetVarBool("ConnectionLight2UseOnColor",false);
	pLamp->mbConnectionLight2UseSpec = apInstanceVars->GetVarBool("ConnectionLight2UseSpec",false);
	pLamp->mbSynchronizeFlickering = apInstanceVars->GetVarBool("SynchronizeFlickering",false);

	pLamp->SetFlickerActive( apInstanceVars->GetVarBool("FlickerActive", false) );
}
//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// LAMP LIGHT CONNECTION
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cLuxLampLightConnection::cLuxLampLightConnection(iLight *apLight)
{
	mpLight = apLight;
}

cLuxLampLightConnection::~cLuxLampLightConnection()
{
	tLuxLampLightConnection_LampListIt it = mlstLamps.begin();
	for(; it != mlstLamps.end(); ++it)
	{
		cLuxLampLightConnection_Lamp *pLampConnection = *it;
		pLampConnection->mpLamp->mpLightConnection1 = NULL;
		pLampConnection->mpLamp->mpLightConnection2 = NULL;
	}

	STLDeleteAll(mlstLamps);
}

//-----------------------------------------------------------------------

void cLuxLampLightConnection::Update(float afTimeStep)
{
	cColor finalColor = mpLight->GetDefaultDiffuseColor();
	
	tLuxLampLightConnection_LampListIt it = mlstLamps.begin();
	for(; it != mlstLamps.end(); ++it)
	{
		cLuxLampLightConnection_Lamp *pLampConnection = *it;
		cLuxProp_Lamp *pLamp = pLampConnection->mpLamp;
		if(pLamp->mvLights.empty()) continue;

		cColor lightColor = pLampConnection->mbUseLightOnColor ?	pLamp->mvEffectLightData[0].mOnColor :
																	pLamp->mvLights[0]->GetDiffuseColor();
		if(pLampConnection->mbUseLightSpec==false) lightColor.a = 0;

	    finalColor = finalColor + lightColor * pLampConnection->mfAmount * pLampConnection->mpLamp->GetEffectsAlpha();
	}

	mpLight->SetDiffuseColor(finalColor);
}

//-----------------------------------------------------------------------

void cLuxLampLightConnection::AddLamp(cLuxProp_Lamp *apLamp, float afAmount, bool abUseOnColor, bool abUseSpec)
{
	cLuxLampLightConnection_Lamp *pLampConnection = hplNew( cLuxLampLightConnection_Lamp, () );
	pLampConnection->mpLamp =apLamp;
	pLampConnection->mfAmount = afAmount;
	pLampConnection->mbUseLightOnColor = abUseOnColor;
	pLampConnection->mbUseLightSpec = abUseSpec;

	mlstLamps.push_back(pLampConnection);
}

//-----------------------------------------------------------------------

void cLuxLampLightConnection::RemoveLamp(cLuxProp_Lamp *apLamp)
{
	tLuxLampLightConnection_LampListIt it = mlstLamps.begin();
	for(; it != mlstLamps.end(); )
	{
		cLuxLampLightConnection_Lamp *pLampConnection = *it;
		if(pLampConnection->mpLamp == apLamp)
		{
			it = mlstLamps.erase(it);
        }
		else
		{
			++it;
		}
	}
}

//-----------------------------------------------------------------------


//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cLuxProp_Lamp::cLuxProp_Lamp(const tString &asName,int alID, cLuxMap *apMap) : iLuxProp(asName,alID,apMap, eLuxPropType_Lamp)
{
	mbLit = true;
	mpLightConnection1 = NULL;
	mpLightConnection2 = NULL;
	mbLightConnectionSetup = false;
}

//-----------------------------------------------------------------------

cLuxProp_Lamp::~cLuxProp_Lamp()
{
	if(mpLightConnection1)
		mpLightConnection1->RemoveLamp(this);

	if(mpLightConnection2)
		mpLightConnection2->RemoveLamp(this);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

bool cLuxProp_Lamp::CanInteract(iPhysicsBody *apBody)
{
	if(mbCanBeLitByPlayer && ( mbLit==false || mbCanBeTurnedOff ) ) return true;
	if(msInteractCallback != "") return true;
	
	return false;
}

//-----------------------------------------------------------------------

bool cLuxProp_Lamp::OnInteract(iPhysicsBody *apBody, const cVector3f &avPos)
{
	//////////////////////
	//Turn off
	if(mbLit)
	{
		if ( mbCanBeTurnedOff )
		{
			SetLit(false, true);
		}

		//Nothing...
	}
	//////////////////////
	//Ignite
	else
	{
		if ( mbNeedsTinderbox )
		{
			/////////////////////
			// Check so has enough tinderboxes
			if(gpBase->mpPlayer->GetTinderboxes()<=0)
			{
				gpBase->mpMessageHandler->SetMessage(kTranslate("Game","NoMoreTinderboxes"), 0);
				return false;
			}

			////////////////////
			// Negate tinderboxes
			gpBase->mpPlayer->AddTinderboxes(-1);
		
			gpBase->mpHelpFuncs->PlayGuiSoundData("ui_use_tinderbox", eSoundEntryType_Gui);
		}

		RunCallbackFunc("OnIgnite");
		
		SetLit(true, true);
	}

	return true;
}

//-----------------------------------------------------------------------

void cLuxProp_Lamp::OnResetProperties()
{

}

//-----------------------------------------------------------------------

void cLuxProp_Lamp::OnSetupAfterLoad(cWorld *apWorld)
{
	
}

//-----------------------------------------------------------------------

void cLuxProp_Lamp::SynchronizeFlickering()
{
    if ( mvLights.size() > 0 )
    {
        for(size_t i=0; i<mvBillboards.size(); ++i) 
	    {
            mvBillboards[i]->SetVisible( mvLights[0]->GetFlickerOn() );
        }

		if ( mbSynchronizeFlickering )
		{
			for(size_t i=0; i<mvLights.size(); ++i)
			{
				if ( mvLights[0]->GetFlickerOn() )
				{
					mvLights[i]->SetDiffuseColor(mvLights[i]->GetFlickerOnColor() );
					mvLights[i]->SetRadius(mvLights[i]->GetFlickerOnRadius() );

				}
				else
				{
					mvLights[i]->SetDiffuseColor(mvLights[i]->GetFlickerOffColor() );
					mvLights[i]->SetRadius(mvLights[i]->GetFlickerOffRadius() );
				}
			}
		}
	}
}

//-----------------------------------------------------------------------

void cLuxProp_Lamp::UpdatePropSpecific(float afTimeStep)
{
	if(mbLightConnectionSetup == false)
	{
		SetupLampLightConnection();
	}

	if ( mbFlickerActive  && mvLights.size() > 0 )
	{
        SynchronizeFlickering();

		bool flickerOn = mvLights[0]->GetFlickerOn();

		for(size_t i=0; i<mvParticleSystems.size(); ++i) 
		{
			cParticleSystem *pPS = mvParticleSystems[i];
			if(pPS && mpWorld->ParticleSystemExists(pPS)) pPS->SetFlickerMultiplier( flickerOn ? 1.0f : 0.25f );
		}
	}

	if(mpMeshEntity)
	{
		if ( mbFlickerActive && mvLights.size() > 0 )
		{
			if ( mvLights[0]->GetFlickerFade() )
			{
				mpMeshEntity->SetIlluminationAmount(mfEffectsAlpha * (mvLights[0]->GetFlickerOn() ? mvLights[0]->GetFadeProgress() : 0.0f ) );
			}
			else
			{
				mpMeshEntity->SetIlluminationAmount(mfEffectsAlpha * (mvLights[0]->GetFlickerOn() ? 1.0f : 0.0f ) );
			}
		}
		else
		{
			mpMeshEntity->SetIlluminationAmount( mfEffectsAlpha );
		}
	}
}

//-----------------------------------------------------------------------

void cLuxProp_Lamp::FadeTo( float afR, float afG, float afB, float afA, float afRadius, float afTime )
{
	SetFlickerActive(false);

	for(size_t i=0; i<mvLights.size(); ++i)
	{
		iLight *pLight = mvLights[i];

		cColor newColor(
			afR >=0 ? afR : pLight->GetDiffuseColor().r,	
			afG >=0 ? afG : pLight->GetDiffuseColor().g,
			afB >=0 ? afB : pLight->GetDiffuseColor().b,
			afA >=0 ? afA : pLight->GetDiffuseColor().a
			);
	
		float fNewRadius = afRadius >=0 ? afRadius : pLight->GetRadius();

		pLight->SetVisible(true);
		pLight->FadeTo(newColor, fNewRadius, afTime);
	}
}

//-----------------------------------------------------------------------

void cLuxProp_Lamp::BeforePropDestruction()
{
}

//-----------------------------------------------------------------------

eLuxFocusCrosshair cLuxProp_Lamp::GetFocusCrosshair(iPhysicsBody *apBody, const cVector3f &avPos)
{
	if ( !mbNeedsTinderbox && mbCanBeLitByPlayer && ( mbCanBeTurnedOff || !mbLit ) )
	{
		return eLuxFocusCrosshair_Grab;
	}

	//if(CanBeIgnitByPlayer())	return eLuxFocusCrosshair_Ignite;
	if(mCustomFocusCrossHair != eLuxFocusCrosshair_Default && mbLit==true)
	{
		return mCustomFocusCrossHair;
	}

	if(CanInteract(apBody)) return eLuxFocusCrosshair_Ignite;

	return eLuxFocusCrosshair_Default;
}

tWString cLuxProp_Lamp::GetFocusText()
{
	if( mbNeedsTinderbox && CanInteract(GetMainBody()) && mbLit==false ) 
	{
		return _W("x ") + cString::ToStringW(gpBase->mpPlayer->GetTinderboxes());
	}
	return _W("");
}

//-----------------------------------------------------------------------

void cLuxProp_Lamp::SetFlickerActive(bool abFlickerActive)
{
	mbFlickerActive = abFlickerActive;

    if ( mvLights.size() > 0 )
    {
		mvLights[0]->SetFlickerActive( abFlickerActive );
	    mvEffectLightData[0].mbFlickering = abFlickerActive;

		if ( !abFlickerActive )
		{
			for(size_t i=0; i<mvBillboards.size(); ++i) 
			{
				mvBillboards[i]->SetVisible( true );
			}

			if ( mbSynchronizeFlickering )
			{
				for(size_t i=0; i<mvLights.size(); ++i)
				{
					mvLights[i]->SetDiffuseColor(mvLights[i]->GetFlickerOnColor() );
					mvLights[i]->SetRadius(mvLights[i]->GetFlickerOnRadius() );
				}
			}
		}
	}

        /*if ( abFlickerActive )
        {
            mvLights[0]->SetFlickerActive( abFlickerActive );
	    	mvEffectLightData[0].mbFlickering = true;
        }
        else
        {
            for(size_t i=0; i<mvLights.size(); ++i)
	        {
				mvLights[0]->SetFlickerActive( false );
	    	    mvEffectLightData[i].mbFlickering = false;
	        }
        }

		for(size_t i=0; i<mvBillboards.size(); ++i) 
	    {
            mvBillboards[i]->SetVisible( mvLights[0]->GetFlickerOn() );
        }

		if(mpMeshEntity)
		{
			mpMeshEntity->SetIlluminationAmount(mfEffectsAlpha * (mvLights[0]->GetFlickerOn() ? mvLights[0]->GetFadeProgress() : 0.0f ));
		}
    }
	else
	{
		if(mpMeshEntity)
		{
			mpMeshEntity->SetIlluminationAmount(mfEffectsAlpha);
		}
	}*/
}

//-----------------------------------------------------------------------

void cLuxProp_Lamp::SetLit(bool abX, bool abUseEffects)
{
	if(mbLit == abX) return;

    mbLit =  abX;
	
	SetEffectsActive(mbLit, abUseEffects);
}

//-----------------------------------------------------------------------

void cLuxProp_Lamp::OnConnectionStateChange(iLuxEntity *apEntity, int alState)
{
	if(alState > 0)	SetLit(true, true);
	if(alState < 0)	SetLit(false, true);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cLuxProp_Lamp::SetupLampLightConnection()
{
	if(msConnectionLight != "")
	{
		iLight *pConnectionLight = NULL;
		//Iterate all lights and get light with name and with NO parent (otherwise a light connected to an entity is gotten).
		cLightListIterator lightIt = mpWorld->GetLightIterator();
		while(lightIt.HasNext())
		{
			iLight *pLight = lightIt.Next();
            if(pLight->GetEntityParent() == NULL && pLight->GetName() == msConnectionLight)
			{
				pConnectionLight = pLight;
				break;
			}
		}
		
			
		if(pConnectionLight)
		{
			mpMap->AddLampLightConnection(this, pConnectionLight, mfConnectionLightAmount, mbConnectionLightUseOnColor, mbConnectionLightUseSpec);
		}
		else
		{
			Error("Light '%s' that is connected to lamp '%s' does not exist!\n", msConnectionLight.c_str(), msName.c_str());
		}
	}

	if(msConnectionLight2 != "")
	{
		iLight *pConnectionLight2 = NULL;
		//Iterate all lights and get light with name and with NO parent (otherwise a light connected to an entity is gotten).
		cLightListIterator lightIt = mpWorld->GetLightIterator();
		while(lightIt.HasNext())
		{
			iLight *pLight = lightIt.Next();
            if(pLight->GetEntityParent() == NULL && pLight->GetName() == msConnectionLight2)
			{
				pConnectionLight2 = pLight;
				break;
			}
		}
		
			
		if(pConnectionLight2)
		{
			mpMap->AddLampLightConnection(this, pConnectionLight2, mfConnectionLight2Amount, mbConnectionLight2UseOnColor, mbConnectionLight2UseSpec);
		}
		else
		{
			Error("Light '%s' that is connected to lamp '%s' does not exist!\n", msConnectionLight2.c_str(), msName.c_str());
		}
	}

	mbLightConnectionSetup = true;
}

//-----------------------------------------------------------------------

bool cLuxProp_Lamp::CanBeIgnitByPlayer()
{
	if(mbLit || mbCanBeLitByPlayer==false) return false;
	return true;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// SAVE DATA STUFF
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

kBeginSerialize(cLuxProp_Lamp_SaveData, iLuxProp_SaveData)
kSerializeVar(mbLit,	eSerializeType_Bool)
kSerializeVar(mbLightConnectionSetup,	eSerializeType_Bool)
kSerializeVar(msConnectionLight,			eSerializeType_String)
kSerializeVar(mfConnectionLightAmount,		eSerializeType_Float32)
kSerializeVar(mbConnectionLightUseOnColor,	eSerializeType_Bool)
kSerializeVar(mbConnectionLightUseSpec,		eSerializeType_Bool)
kSerializeVar(msConnectionLight2,			eSerializeType_String)
kSerializeVar(mfConnectionLight2Amount,		eSerializeType_Float32)
kSerializeVar(mbConnectionLight2UseOnColor,	eSerializeType_Bool)
kSerializeVar(mbConnectionLight2UseSpec,		eSerializeType_Bool)
kSerializeVar(mbCanBeLitByPlayer,		eSerializeType_Bool)
kSerializeVar(mbCanBeGrabbed,		eSerializeType_Bool)
kSerializeVar(mbCanBeTurnedOff,		eSerializeType_Bool)
kSerializeVar(mbNeedsTinderbox,		eSerializeType_Bool)
kSerializeVar(mbSynchronizeFlickering,		eSerializeType_Bool)
kSerializeVar(mbFlickerActive,		eSerializeType_Bool)
kEndSerialize()

//-----------------------------------------------------------------------

iLuxEntity_SaveData* cLuxProp_Lamp::CreateSaveData()
{
	return hplNew(cLuxProp_Lamp_SaveData, ());
}

//-----------------------------------------------------------------------

void cLuxProp_Lamp::SaveToSaveData(iLuxEntity_SaveData* apSaveData)
{
	//////////////////
	//Init
	super_class::SaveToSaveData(apSaveData);
	cLuxProp_Lamp_SaveData *pData = static_cast<cLuxProp_Lamp_SaveData*>(apSaveData);

	//////////////////
	//Set variables
	kCopyToVar(pData,mbLit);
	kCopyToVar(pData,mbLightConnectionSetup);
	kCopyToVar(pData,msConnectionLight);
	kCopyToVar(pData,mfConnectionLightAmount);
	kCopyToVar(pData,mbConnectionLightUseOnColor);
	kCopyToVar(pData,mbConnectionLightUseSpec);
	kCopyToVar(pData,msConnectionLight2);
	kCopyToVar(pData,mfConnectionLight2Amount);
	kCopyToVar(pData,mbConnectionLight2UseOnColor);
	kCopyToVar(pData,mbConnectionLight2UseSpec);
	kCopyToVar(pData,mbCanBeLitByPlayer);
	kCopyToVar(pData,mbCanBeGrabbed);
	kCopyToVar(pData,mbCanBeTurnedOff);
	kCopyToVar(pData,mbNeedsTinderbox);
	kCopyToVar(pData,mbSynchronizeFlickering);
	kCopyToVar(pData,mbFlickerActive);
}

//-----------------------------------------------------------------------

void cLuxProp_Lamp::LoadFromSaveData(iLuxEntity_SaveData* apSaveData)
{
	//////////////////
	//Init
	super_class::LoadFromSaveData(apSaveData);
	cLuxProp_Lamp_SaveData *pData = static_cast<cLuxProp_Lamp_SaveData*>(apSaveData);
	
	//////////////////
	//Set variables
	kCopyFromVar(pData,	mbLit);
	kCopyFromVar(pData,mbLightConnectionSetup);
	kCopyFromVar(pData,msConnectionLight);
	kCopyFromVar(pData,mfConnectionLightAmount);
	kCopyFromVar(pData,mbConnectionLightUseOnColor);
	kCopyFromVar(pData,mbConnectionLightUseSpec);
	kCopyFromVar(pData,msConnectionLight2);
	kCopyFromVar(pData,mfConnectionLight2Amount);
	kCopyFromVar(pData,mbConnectionLight2UseOnColor);
	kCopyFromVar(pData,mbConnectionLight2UseSpec);
	kCopyFromVar(pData,mbCanBeLitByPlayer);
	kCopyFromVar(pData,mbCanBeGrabbed);
	kCopyFromVar(pData,mbCanBeTurnedOff);
	kCopyFromVar(pData,mbNeedsTinderbox);
	kCopyFromVar(pData,mbSynchronizeFlickering);
	kCopyFromVar(pData,mbFlickerActive);
}

//-----------------------------------------------------------------------

void cLuxProp_Lamp::SetupSaveData(iLuxEntity_SaveData *apSaveData)
{
	super_class::SetupSaveData(apSaveData);

	SetupLampLightConnection();
}

//-----------------------------------------------------------------------
