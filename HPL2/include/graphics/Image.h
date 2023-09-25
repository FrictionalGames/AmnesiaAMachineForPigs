/**
 * Copyright 2023 Michael Pollind
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "graphics/Bitmap.h"
#include "graphics/ForgeRenderer.h"
#include "math/MathTypes.h"
#include "system/SystemTypes.h"
#include <engine/RTTI.h>
#include <graphics/Enum.h>
#include <graphics/GraphicsTypes.h>
#include <resources/ResourceBase.h>

#include <cstdint>
#include <memory>
#include <span>
#include <string>

#include "Common_3/Graphics/Interfaces/IGraphics.h"
#include <FixPreprocessor.h>

namespace hpl {

    class cBitmap;
    class Image : public iResourceBase {
        HPL_RTTI_IMPL_CLASS(iResourceBase, Image, "{d9cd842a-c76b-4261-879f-53f1baa5ff7c}")
    public:

        Image();
        Image(const tString& asName, const tWString& asFullPath);

        ~Image();
        Image(Image&& other);
        Image(const Image& other) = delete;

        Image& operator=(const Image& other) = delete;
        void operator=(Image&& other);

        inline void SetForgeTexture(SharedTexture&& handle) {
            m_texture = std::move(handle);
        }

        virtual bool Reload() override;
        virtual void Unload() override;
        virtual void Destroy() override;

        inline uint16_t GetWidth() const {
            ASSERT(m_texture.IsValid());
            return m_texture.m_handle->mWidth;
        }
        inline uint16_t GetHeight() const {
            ASSERT(m_texture.IsValid());
            return m_texture.m_handle->mHeight;
        }

        cVector2l GetImageSize() const {
            if (m_texture.IsValid()) {
                return cVector2l(m_texture.m_handle->mWidth, m_texture.m_handle->mHeight);
            }
            return cVector2l(0, 0);
        }

        inline SharedTexture& GetTexture() {
            return m_texture;
        }
        SharedTexture m_texture;
    };

} // namespace hpl
