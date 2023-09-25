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

#include <graphics/ImageResourceWrapper.h>

#include <resources/TextureManager.h>
#include <graphics/Image.h>
#include <engine/RTTI.h>
#include <graphics/AnimatedImage.h>

#include "Common_3/Utilities/Interfaces/ILog.h"
#include <FixPreprocessor.h>

namespace hpl {

    ImageResourceWrapper::ImageResourceWrapper()
        : m_imageResource(nullptr)
        , m_textureManager(nullptr) {
    }
    ImageResourceWrapper::ImageResourceWrapper(cTextureManager* m_textureManager, hpl::iResourceBase* resource, bool autoDestroyTextures)
        : m_imageResource(resource)
        , m_textureManager(m_textureManager)
        , m_autoDestroyResource(autoDestroyTextures) {
        ASSERT(m_imageResource && "ImageResourceWrapper: Image resource is null");
        ASSERT(m_textureManager && "ImageResourceWrapper: Texture manager is null");
    }
    ImageResourceWrapper::ImageResourceWrapper(ImageResourceWrapper&& other) {
        if (m_imageResource && m_autoDestroyResource) {
            m_textureManager->Destroy(m_imageResource);
        }
        m_imageResource = other.m_imageResource;
        m_textureManager = other.m_textureManager;
        m_autoDestroyResource = other.m_autoDestroyResource;

        other.m_imageResource = nullptr;
        other.m_textureManager = nullptr;
    }

    ImageResourceWrapper::~ImageResourceWrapper() {
        if (m_imageResource && m_autoDestroyResource) {
            m_textureManager->Destroy(m_imageResource);
        }
    }

    void ImageResourceWrapper::operator=(ImageResourceWrapper&& other) {
        if (m_imageResource && m_autoDestroyResource) {
            m_textureManager->Destroy(m_imageResource);
        }
        m_imageResource = other.m_imageResource;
        m_textureManager = other.m_textureManager;
        m_autoDestroyResource = other.m_autoDestroyResource;

        other.m_imageResource = nullptr;
        other.m_textureManager = nullptr;
    }

    void ImageResourceWrapper::SetAutoDestroyResource(bool autoDestroyResource) {
        m_autoDestroyResource = autoDestroyResource;
    }

    Image* ImageResourceWrapper::GetImage() const {
        if (m_imageResource == nullptr) {
            return nullptr;
        }

        if (TypeInfo<Image>().IsType(*m_imageResource)) {
            return static_cast<Image*>(m_imageResource);
        } else if (TypeInfo<AnimatedImage>().IsType(*m_imageResource)) {
            return static_cast<AnimatedImage*>(m_imageResource)->GetImage();
        }
        return nullptr;
    }
} // namespace hpl
