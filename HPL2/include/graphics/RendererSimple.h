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

#include "graphics/Renderer.h"

namespace hpl {

	class iFrameBuffer;
	class iDepthStencilBuffer;
	class iTexture;
	class iLight;

	class cRendererSimple : public  iRenderer
	{
		HPL_RTTI_IMPL_CLASS(iRenderer, cRendererSimple, "{ea5e1d79-c424-47c6-bab3-ead1391f64ea}")
	public:
		cRendererSimple(cGraphics *apGraphics,cResources* apResources);
		~cRendererSimple();

		virtual bool LoadData() override;
		virtual void DestroyData() override;

		virtual void Draw(const ForgeRenderer::Frame& frame, cViewport& viewport, float afFrameTime, cFrustum *apFrustum, cWorld *apWorld, cRenderSettings *apSettings, bool abSendFrameBufferToPostEffects) override;

	private:

		void CopyToFrameBuffer();
		void SetupRenderList();
		void RenderObjects();


	};

};
