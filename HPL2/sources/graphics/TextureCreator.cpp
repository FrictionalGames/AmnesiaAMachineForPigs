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

#include "graphics/TextureCreator.h"

#include "graphics/Image.h"
#include "system/LowLevelSystem.h"

#include "graphics/LowLevelGraphics.h"
#include "graphics/Texture.h"

#include "resources/Resources.h"

#include "math/Math.h"

#include <algorithm>
#include <vector>

namespace hpl {

    static bool SortFunc_ScatterOffset(const cVector2f& avVec1, const cVector2f& avVec2) {
        return avVec1.Length() > avVec2.Length();
    }

    namespace TextureCreator {

        void GenerateScatterDiskMap3D(int alSize, int alSamples, bool abSortSamples, Texture** ppTexture) {
            ASSERT(cMath::IsPow2(alSize) && "OffsetTexture 2D size size non-pow2");
            ASSERT(cMath::IsPow2(alSamples) && "OffsetTexture 2D samples size non-pow2");

            ////////////////////
            // Calculate size of grid
            int lGridSize = (int)(sqrt((float)alSamples) + 0.5f);
            std::vector<tVector2fVec> vOffsetsVec;
            vOffsetsVec.resize(alSize * alSize);
            tVector2fVec vTempGridArray;
            vTempGridArray.resize(lGridSize * lGridSize);

            ////////////////////////////
            // Setup texture data
            std::vector<unsigned char> textureData;
            cVector3l vTextureSize(alSize, alSize, alSamples / 2);
            textureData.resize(4 * vTextureSize.x * vTextureSize.y * vTextureSize.z, 0);

            TextureDesc desc{};
            desc.mWidth = vTextureSize.x;
            desc.mHeight = vTextureSize.y * vTextureSize.z;
            desc.mDepth = 1;
            desc.mFormat = TinyImageFormat_R8G8B8A8_UNORM;
            desc.mMipLevels = 1;
            desc.mArraySize = 1;
            desc.mSampleCount = SAMPLE_COUNT_1;
            desc.mStartState = RESOURCE_STATE_COMMON;
            desc.mDescriptors = DESCRIPTOR_TYPE_TEXTURE;

            TextureLoadDesc textureLoadDesc = {};
            textureLoadDesc.ppTexture = ppTexture;
            textureLoadDesc.pDesc = &desc;
            addResource(&textureLoadDesc, nullptr);

            ////////////////////////////
            // Generate all the different sample collections
            TextureUpdateDesc update = {*ppTexture};
            beginUpdateResource(&update);

            for (size_t sample_num = 0; sample_num < vOffsetsVec.size(); ++sample_num) {
                ////////////////////////////
                // Fills a square "grid" size, and then normalizes each pos to be 0 - 1
                for (int y = 0; y < lGridSize; ++y) {
                    for (int x = 0; x < lGridSize; ++x) {
                        //+0.5, because we want center
                        cVector2f vPos((float)x + 0.5f, (float)y + 0.5f);

                        // Randomize point from, but let it never leave the grid square.
                        vPos += cMath::RandRectVector2f(-0.5f, 0.5f);

                        // Normalize position
                        vPos *= 1.0f / (float)lGridSize;

                        // Add to array
                        vTempGridArray[y * lGridSize + x] = vPos;
                    }
                }

                // Get the offsets and put the once we want from the grid here.
                tVector2fVec& vOffsets = vOffsetsVec[sample_num];
                vOffsets.resize(alSamples);

                float fSampleAdd = ((float)vTempGridArray.size()) / ((float)alSamples);
                float fCurrentSample = 0;

                for (size_t i = 0; i < vOffsets.size(); ++i) {
                    int lIdx = (int)fCurrentSample;

                    vOffsets[i] = vTempGridArray[lIdx];

                    fCurrentSample += fSampleAdd;
                }

                ////////////////////////////
                // Warp grid so coordinates become spherical
                for (size_t i = 0; i < vOffsets.size(); ++i) {
                    cVector2f vPos = vOffsets[i];

                    vOffsets[i].x = sqrtf(vPos.y) * cos(k2Pif * vPos.x);
                    vOffsets[i].y = sqrtf(vPos.y) * sin(k2Pif * vPos.x);
                }

                ///////////////////////////////
                // Sort by length (if set)
                if (abSortSamples) {
                    std::sort(vOffsets.begin(), vOffsets.end(), SortFunc_ScatterOffset);
                }

                // Set all samples between 0 and 1
                for (size_t i = 0; i < vOffsets.size(); ++i) {
                    vOffsets[i].x = (vOffsets[i].x + 1) * 0.5f;
                    vOffsets[i].y = (vOffsets[i].y + 1) * 0.5f;
                }

                ///////////////////////////////
                // Add the samples to the texture data
                for (int depth = 0; depth < vTextureSize.z; ++depth) {
                    int lOffset = depth * vTextureSize.x * vTextureSize.y * 4 + (int)sample_num * 4;

                    update.pMappedData[lOffset + 0] = static_cast<uint8_t>(vOffsets[(depth * 2)].x * 255.0f);
                    update.pMappedData[lOffset + 1] = static_cast<uint8_t>(vOffsets[(depth * 2)].y * 255.0f);
                    update.pMappedData[lOffset + 2] = static_cast<uint8_t>(vOffsets[(depth * 2) + 1].x * 255.0f);
                    update.pMappedData[lOffset + 3] = static_cast<uint8_t>(vOffsets[(depth * 2) + 1].y * 255.0f);
                }
            }
            endUpdateResource(&update, nullptr);
        }

        void GenerateScatterDiskMap2D(int alSize, int alSamples, bool abSortSamples, Texture** ppTexture) {
            ASSERT(cMath::IsPow2(alSize) && "OffsetTexture 2D size size non-pow2");
            ASSERT(cMath::IsPow2(alSamples) && "OffsetTexture 2D samples size non-pow2");

            ////////////////////
            // Calculate size of grid
            int lGridSize = (int)(sqrt((float)alSamples) + 0.5f);
            std::vector<tVector2fVec> vOffsetsVec;
            vOffsetsVec.resize(alSize * alSize);
            tVector2fVec vTempGridArray;
            vTempGridArray.resize(lGridSize * lGridSize);

            ////////////////////////////
            // Setup texture data
            // std::vector<unsigned char> vTextureData;
            cVector3l vTextureSize(alSize, alSize, alSamples / 2);
            // vTextureData.resize(4 * vTextureSize.x * vTextureSize.y * vTextureSize.z, 0);

            TextureDesc desc{};
            desc.mWidth = vTextureSize.x;
            desc.mHeight = vTextureSize.y * vTextureSize.z;
            desc.mFormat = TinyImageFormat_R8G8B8A8_UNORM;
            desc.mMipLevels = 1;
            desc.mArraySize = 1;
            desc.mDepth = 1;
            desc.mSampleCount = SAMPLE_COUNT_1;
            desc.mStartState = RESOURCE_STATE_COMMON;
            desc.mDescriptors = DESCRIPTOR_TYPE_TEXTURE;

            TextureLoadDesc textureLoadDesc = {};
            textureLoadDesc.ppTexture = ppTexture;
            textureLoadDesc.pDesc = &desc;
            addResource(&textureLoadDesc, nullptr);

            TextureUpdateDesc update = {*ppTexture};
            beginUpdateResource(&update);

            ////////////////////////////
            // Generate all the different sample collections
            for (size_t sample_num = 0; sample_num < vOffsetsVec.size(); ++sample_num) {
                ////////////////////////////
                // Fills a square "grid" size, and then normalizes each pos to be 0 - 1
                for (int y = 0; y < lGridSize; ++y)
                    for (int x = 0; x < lGridSize; ++x) {
                        //+0.5, because we want center
                        cVector2f vPos((float)x + 0.5f, (float)y + 0.5f);

                        // Randomize point from, but let it never leave the grid square.
                        vPos += cMath::RandRectVector2f(-0.5f, 0.5f);

                        // Normalize position
                        vPos *= 1.0f / (float)lGridSize;

                        // Add to array
                        vTempGridArray[y * lGridSize + x] = vPos;
                    }

                // Get the offsets and put the once we want from the grid here.
                tVector2fVec& vOffsets = vOffsetsVec[sample_num];
                vOffsets.resize(alSamples);

                float fSampleAdd = ((float)vTempGridArray.size()) / ((float)alSamples);
                float fCurrentSample = 0;

                for (size_t i = 0; i < vOffsets.size(); ++i) {
                    int lIdx = (int)fCurrentSample;

                    vOffsets[i] = vTempGridArray[lIdx];

                    fCurrentSample += fSampleAdd;
                }

                ////////////////////////////
                // Warp grid so coordinates become spherical
                for (size_t i = 0; i < vOffsets.size(); ++i) {
                    cVector2f vPos = vOffsets[i];

                    vOffsets[i].x = sqrtf(vPos.y) * cos(k2Pif * vPos.x);
                    vOffsets[i].y = sqrtf(vPos.y) * sin(k2Pif * vPos.x);
                }

                ///////////////////////////////
                // Sort by length (if set)
                if (abSortSamples) {
                    std::sort(vOffsets.begin(), vOffsets.end(), SortFunc_ScatterOffset);
                }

                // Set all samples between 0 and 1
                for (size_t i = 0; i < vOffsets.size(); ++i) {
                    vOffsets[i].x = (vOffsets[i].x + 1) * 0.5f;
                    vOffsets[i].y = (vOffsets[i].y + 1) * 0.5f;
                }

                ///////////////////////////////
                // Add the samples to the texture data
                for (int depth = 0; depth < vTextureSize.z; ++depth) {
                    size_t lOffset = (depth * vTextureSize.x * vTextureSize.y) * 4 + (sample_num * 4);
                    update.pMappedData[lOffset + 0] = static_cast<uint8_t>(vOffsets[(depth * 2)].x * 255.0f);
                    update.pMappedData[lOffset + 1] = static_cast<uint8_t>(vOffsets[(depth * 2)].y * 255.0f);
                    update.pMappedData[lOffset + 2] = static_cast<uint8_t>(vOffsets[(depth * 2) + 1].x * 255.0f);
                    update.pMappedData[lOffset + 3] = static_cast<uint8_t>(vOffsets[(depth * 2) + 1].y * 255.0f);

                    // unsigned char* pPixelData = &vTextureData[lOffset];

                    // int lSample = depth * 2;

                    // // RG
                    // pPixelData[0] = (int)(vOffsets[lSample].x * 255.0f);
                    // pPixelData[1] = (int)(vOffsets[lSample].y * 255.0f);

                    // // BA
                    // pPixelData[2] = (int)(vOffsets[lSample + 1].x * 255.0f);
                    // pPixelData[3] = (int)(vOffsets[lSample + 1].y * 255.0f);
                }
            }
            endUpdateResource(&update, nullptr);

        }

    } // namespace TextureCreator

} // namespace hpl
