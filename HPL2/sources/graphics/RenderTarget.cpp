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

#include "graphics/Image.h"
#include <array>
#include <folly/small_vector.h>
#include <graphics/RenderTarget.h>
#include <memory>
#include <span>

namespace hpl
{
    const LegacyRenderTarget LegacyRenderTarget::EmptyRenderTarget = LegacyRenderTarget();

    LegacyRenderTarget::LegacyRenderTarget(std::shared_ptr<Image> image)
    {
    }

    LegacyRenderTarget::LegacyRenderTarget(std::span<std::shared_ptr<Image>> images)
    {
    }

    LegacyRenderTarget::LegacyRenderTarget()
    {
    }

    LegacyRenderTarget::LegacyRenderTarget(LegacyRenderTarget&& target)
    {
    }

    void LegacyRenderTarget::operator=(LegacyRenderTarget&& target)
    {
    }

    LegacyRenderTarget::~LegacyRenderTarget()
    {
    }

    void LegacyRenderTarget::Initialize(std::span<std::shared_ptr<Image>> images) {
        ASSERT(false && "This is deprecated");
    }

    void LegacyRenderTarget::Invalidate() {
        ASSERT(false && "This is deprecated");
    }

    const bool LegacyRenderTarget::IsValid() const
    {
        ASSERT(false && "This is deprecated");
        return false;
    }

    std::shared_ptr<Image> LegacyRenderTarget::GetImage(size_t index){
        ASSERT(false && "This is deprecated");
        return std::shared_ptr<Image>();
    }

    std::span<std::shared_ptr<Image>> LegacyRenderTarget::GetImages()
    {
        ASSERT(false && "This is deprecated");
        return std::span<std::shared_ptr<Image>>{};
    }

    const std::shared_ptr<Image> LegacyRenderTarget::GetImage(size_t index) const {
        ASSERT(false && "This is deprecated");
        return std::shared_ptr<Image>();
    }

    const std::span<const std::shared_ptr<Image>> LegacyRenderTarget::GetImages() const
    {
        ASSERT(false && "This is deprecated");
        return std::span<std::shared_ptr<Image>>();
    }

} // namespace hpl
