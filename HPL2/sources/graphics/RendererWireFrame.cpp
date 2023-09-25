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

#include "graphics/RendererWireFrame.h"

#include "graphics/RenderTarget.h"
#include "graphics/VertexBuffer.h"
#include "math/Math.h"

#include "math/MathTypes.h"
#include "system/LowLevelSystem.h"
#include "system/String.h"
#include "system/PreprocessParser.h"

#include "graphics/Graphics.h"
#include "graphics/Texture.h"
#include "graphics/LowLevelGraphics.h"
#include "graphics/Renderable.h"
#include "graphics/RenderList.h"
#include "graphics/Material.h"
#include "graphics/MaterialType.h"
#include "graphics/Mesh.h"
#include "graphics/SubMesh.h"

#include "resources/Resources.h"

#include "scene/Camera.h"
#include "scene/World.h"
#include "scene/RenderableContainer.h"
#include <memory>

namespace hpl {

	//////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	cRendererWireFrame::cRendererWireFrame(cGraphics *apGraphics,cResources* apResources)
		: iRenderer("WireFrame",apGraphics, apResources)
	{
		////////////////////////////////////
		// Set up render specific things

//		m_boundOutputBuffer = std::move(UniqueViewportData<LegacyRenderTarget>([](cViewport& viewport) {
//				auto colorImage = [&] {
//				auto desc = ImageDescriptor::CreateTexture2D(viewport.GetSize().x, viewport.GetSize().y, false, bgfx::TextureFormat::Enum::RGBA8);
//				desc.m_configuration.m_rt = RTType::RT_Write;
//				auto image = std::make_shared<Image>();
//				image->Initialize(desc);
//				return image;
//			};
//			return std::make_unique<LegacyRenderTarget>(colorImage());
//		}, [](cViewport& viewport, LegacyRenderTarget& target) {
//			return target.GetImage()->GetImageSize() == viewport.GetSize();
//		}));

		// m_u_color.Initialize();
		// m_colorProgram = hpl::loadProgram("vs_color", "fs_color");
	}

	//-----------------------------------------------------------------------

	cRendererWireFrame::~cRendererWireFrame()
	{
	}

	bool cRendererWireFrame::LoadData()
	{
		return true;
	}

	void cRendererWireFrame::DestroyData()
	{
	}

	void cRendererWireFrame::CopyToFrameBuffer()
	{
		//Do Nothing
	}

	// Texture* cRendererWireFrame::GetOutputImage(cViewport& viewport) {
	// 	return nullptr;
	// 	// return m_boundOutputBuffer.resolve(viewport).GetImage();
	// }


	void cRendererWireFrame::Draw(const ForgeRenderer::Frame& frame, cViewport& viewport, float afFrameTime, cFrustum *apFrustum, cWorld *apWorld, cRenderSettings *apSettings, bool abSendFrameBufferToPostEffects) {
		//BeginRendering(afFrameTime, apFrustum, apWorld, apSettings, abSendFrameBufferToPostEffects);
        ASSERT(false && "TODO: need to implement bound Viewport");
		// auto& rt = m_boundOutputBuffer.resolve(viewport);

		// [&]{
		// 	GraphicsContext::ViewConfiguration viewConfig {rt};
		// 	viewConfig.m_viewRect = {0, 0, viewport.GetSize().x, viewport.GetSize().y};
		// 	viewConfig.m_clear = {0, 1, 0, ClearOp::Depth | ClearOp::Stencil | ClearOp::Color};
		// 	bgfx::touch(context.StartPass("Clear", viewConfig));
		// }();


		// mpCurrentRenderList->Setup(mfCurrentFrameTime,apFrustum);

		// rendering::detail::UpdateRenderListWalkAllNodesTestFrustumAndVisibility(
		// 	mpCurrentRenderList, apFrustum, mpCurrentWorld->GetRenderableContainer(eWorldContainerType_Static), mvCurrentOcclusionPlanes, 0);
		// rendering::detail::UpdateRenderListWalkAllNodesTestFrustumAndVisibility(
		// 	mpCurrentRenderList, apFrustum, mpCurrentWorld->GetRenderableContainer(eWorldContainerType_Dynamic), mvCurrentOcclusionPlanes, 0);

		// mpCurrentRenderList->Compile(	eRenderListCompileFlag_Diffuse |
		// 								eRenderListCompileFlag_Decal |
		// 								eRenderListCompileFlag_Translucent);

		// // START_RENDER_PASS(WireFrame);

		// ////////////////////////////////////////////
		// // Diffuse Objects
		// // SetDepthTest(true);
		// // SetDepthWrite(true);
		// // SetBlendMode(eMaterialBlendMode_None);
		// // SetAlphaMode(eMaterialAlphaMode_Solid);
		// // SetChannelMode(eMaterialChannelMode_RGBA);

		// // SetTextureRange(NULL,0);

		// int lCount =0;

		// GraphicsContext::ViewConfiguration viewConfig {rt};
		// viewConfig.m_projection = apFrustum->GetProjectionMatrix().GetTranspose();
		// viewConfig.m_view = apFrustum->GetViewMatrix().GetTranspose();
		// viewConfig.m_viewRect = {0, 0, viewport.GetSize().x, viewport.GetSize().y};
		// auto view = context.StartPass("Wireframe", viewConfig);
		// for(auto& pObject: mpCurrentRenderList->GetRenderableItems(eRenderListType_Diffuse))
		// {
		// 	GraphicsContext::LayoutStream layoutStream;
        //     GraphicsContext::ShaderProgram shaderProgram;

		// 	if(pObject == nullptr)
		// 	{
		// 		continue;
		// 	}

		// 	iVertexBuffer* vertexBuffer = pObject->GetVertexBuffer();
		// 	if(vertexBuffer == nullptr) {
		// 		continue;
		// 	}

		// 	struct {
		// 		float m_r;
		// 		float m_g;
		// 		float m_b;
		// 		float m_a;
		// 	} color = { 1.0f, 1.0f,1.0f ,1.0f };


		// 	shaderProgram.m_configuration.m_depthTest = DepthTest::LessEqual;
		// 	shaderProgram.m_configuration.m_write = Write::RGBA;
		// 	shaderProgram.m_configuration.m_cull = Cull::None;
		// 	shaderProgram.m_handle = m_colorProgram;

		// 	shaderProgram.m_uniforms.push_back({ m_u_color, &color });
		// 	shaderProgram.m_modelTransform = pObject->GetModelMatrixPtr() ?  pObject->GetModelMatrixPtr()->GetTranspose() : cMatrixf::Identity.GetTranspose();

		// 	vertexBuffer->GetLayoutStream(layoutStream, eVertexBufferDrawType_LineStrip);
		// 	GraphicsContext::DrawRequest drawRequest {layoutStream, shaderProgram};
		// 	context.Submit(view, drawRequest);

		// 	lCount++;
		// }

		// ////////////////////////////////////////////
		// // Decal Objects
		// // SetDepthWrite(false);

		// for(auto& pObject: mpCurrentRenderList->GetRenderableItems(eRenderListType_Decal))
		// {
		// 	cMaterial *pMaterial = pObject->GetMaterial();

		// 	// SetBlendMode(pMaterial->GetBlendMode());

		// 	// SetTexture(0,pMaterial->GetTexture(eMaterialTexture_Diffuse));

		// 	// SetMatrix(pObject->GetModelMatrixPtr());

		// 	// SetVertexBuffer(pObject->GetVertexBuffer());

		// 	// DrawCurrent(eVertexBufferDrawType_LineStrip);
		// }

		// // RunCallback(eRendererMessage_PostSolid);


		// ////////////////////////////////////////////
		// // Trans Objects
		// // SetDepthWrite(false);

		// for(auto& pObject: mpCurrentRenderList->GetRenderableItems(eRenderListType_Translucent))
		// {
		// 	cMaterial *pMaterial = pObject->GetMaterial();

		// 	pObject->UpdateGraphicsForViewport(apFrustum, mfCurrentFrameTime);

		// 	// SetBlendMode(pMaterial->GetBlendMode());

		// 	// SetTexture(0,pMaterial->GetTexture(eMaterialTexture_Diffuse));

		// 	// SetMatrix(pObject->GetModelMatrix(mpCurrentFrustum));

		// 	// SetVertexBuffer(pObject->GetVertexBuffer());

		// 	// DrawCurrent(eVertexBufferDrawType_LineStrip);
		// }

		// // RunCallback(eRendererMessage_PostTranslucent);


		// END_RENDER_PASS();
	}

	//-----------------------------------------------------------------------

}
