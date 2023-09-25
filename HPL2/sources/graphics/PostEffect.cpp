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

#include "graphics/PostEffect.h"

#include "graphics/Graphics.h"

#include "graphics/FrameBuffer.h"
#include "graphics/LowLevelGraphics.h"
#include "graphics/PostEffectComposite.h"
#include "graphics/Texture.h"

namespace hpl
{

    iPostEffectType::iPostEffectType(const tString& asName, cGraphics* apGraphics, cResources* apResources)
    {
        mpGraphics = apGraphics;
        mpResources = apResources;

        msName = asName;
    }


    iPostEffectType::~iPostEffectType()
    {
    }

    iPostEffect::iPostEffect(cGraphics* apGraphics, cResources* apResources, iPostEffectType* apType)
    {
        mpGraphics = apGraphics;
        mpResources = apResources;
        mpType = apType;

        mbActive = true;
        mbDisabled = false;
    }

    //-----------------------------------------------------------------------

    iPostEffect::~iPostEffect()
    {
    }

    void iPostEffect::SetActive(bool abX)
    {
        if (mbActive == abX)
            return;

        mbActive = abX;

        OnSetActive(abX);
    }

    //-----------------------------------------------------------------------

    void iPostEffect::SetParams(iPostEffectParams* apSrcParams)
    {
        if (mpType == NULL)
            return;

        // Make sure the type is correct!
        if (apSrcParams->GetName() != mpType->GetName())
            return;

        GetTypeSpecificParams()->LoadFrom(apSrcParams);
        OnSetParams();
    }

    void iPostEffect::GetParams(iPostEffectParams* apDestParams)
    {
        if (mpType == NULL)
            return;

        // Make sure the type is correct!
        if (apDestParams->GetName() != mpType->GetName())
            return;

        GetTypeSpecificParams()->CopyTo(apDestParams);
    }

} // namespace hpl
