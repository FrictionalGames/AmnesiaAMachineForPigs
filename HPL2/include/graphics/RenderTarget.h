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

#include "math/MathTypes.h"
#include <cstdint>
#include <graphics/Image.h>
#include <memory>
#include <span>

namespace hpl
{

    class LegacyRenderTarget
    {
    public:
        static const LegacyRenderTarget EmptyRenderTarget;

	    [[deprecated("replaced with wrapper ForgeRenderTarget ")]]
        LegacyRenderTarget(std::shared_ptr<Image> image);
	    [[deprecated("replaced with wrapper ForgeRenderTarget ")]]
        LegacyRenderTarget(std::span<std::shared_ptr<Image>> images);

	    [[deprecated("replaced with wrapper ForgeRenderTarget ")]]
        LegacyRenderTarget(LegacyRenderTarget&& target);
	    [[deprecated("replaced with wrapper ForgeRenderTarget ")]]
        LegacyRenderTarget(const LegacyRenderTarget& target) = delete;
	    [[deprecated("replaced with wrapper ForgeRenderTarget ")]]
        LegacyRenderTarget();
        ~LegacyRenderTarget();

	    [[deprecated("replaced with wrapper ForgeRenderTarget ")]]
        void Initialize(std::span<std::shared_ptr<Image>> descriptor);
	    [[deprecated("replaced with wrapper ForgeRenderTarget ")]]
        void Invalidate();
	    [[deprecated("replaced with wrapper ForgeRenderTarget ")]]
        const bool IsValid() const;

        void operator=(const LegacyRenderTarget& target) = delete;
        void operator=(LegacyRenderTarget&& target);

        std::span<std::shared_ptr<Image>> GetImages();
        std::shared_ptr<Image> GetImage(size_t index = 0);

        const std::span<const std::shared_ptr<Image>> GetImages() const;
        const std::shared_ptr<Image> GetImage(size_t index = 0) const;
    };

} // namespace hpl
