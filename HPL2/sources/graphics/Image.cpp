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

#include <graphics/Image.h>
#include <graphics/Bitmap.h>
#include <vector>

namespace hpl
{


    Image::Image() :
        iResourceBase("", _W(""), 0)
    {
        // m_textures = std::make_shared(Texture{0})
    }

    Image::Image(const tString& asName, const tWString& asFullPath):
        iResourceBase(asName, asFullPath, 0)
    {
    }

    bool Image::Reload() {
        return false;
    }

    void Image::Unload() {
    }

    void Image::Destroy() {
    }

    Image::Image(Image&& other) :
        iResourceBase(other.GetName(), other.GetFullPath(), 0) {
        m_texture = std::move(other.m_texture);
    }

    Image::~Image()
    {
    }

    void Image::operator=(Image&& other) {
        m_texture = std::move(other.m_texture);
    }
} // namespace hpl
