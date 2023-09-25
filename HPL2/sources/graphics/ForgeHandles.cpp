
#include "graphics/ForgeHandles.h"
#include "Common_3/Graphics/Interfaces/IGraphics.h"
#include "graphics/Bitmap.h"

#include "Common_3/Resources/ResourceLoader/TextureContainers.h"
#include "tinyimageformat_base.h"
#include "tinyimageformat_query.h"
#include <FixPreprocessor.h>

#include <cstring>

namespace hpl {
    void SharedCmd::Free() {
        if (m_handle) {
            ASSERT(m_renderer && "Renderer is null");
            removeCmd(m_renderer, m_handle);
        }
    }
    void SharedShader::Free() {
        if(m_handle) {
            removeShader(m_renderer, m_handle);
        }
    }
    void SharedTexture::Free() {
        if (m_handle) {
            removeResource(m_handle);
        }
    }

    void SharedBuffer::Free() {
        if (m_handle) {
            removeResource(m_handle);
        }
    }

    void SharedRenderTarget::Free() {
        if (m_handle) {
            ASSERT(m_renderer && "Renderer is null");
            removeRenderTarget(m_renderer, m_handle);
        }
    }

    void SharedSwapChain::Free() {
        if (m_handle) {
            ASSERT(m_renderer && "Renderer is null");
            removeSwapChain(m_renderer, m_handle);
        }
    }

    void SharedDescriptorSet::Free() {
        if (m_handle) {
            ASSERT(m_renderer && "Renderer is null");
            removeDescriptorSet(m_renderer, m_handle);
        }
    }

    void SharedPipeline::Free() {
        if (m_handle) {
            ASSERT(m_renderer && "Renderer is null");
            removePipeline(m_renderer, m_handle);
        }
    }

    void SharedSampler::Free() {
        if (m_handle) {
            ASSERT(m_renderer && "Renderer is null");
            removeSampler(m_renderer, m_handle);
        }
    }

    void SharedRootSignature::Free() {
        if (m_handle) {
            ASSERT(m_renderer && "Renderer is null");
            removeRootSignature(m_renderer, m_handle);
        }
    }

    void SharedCmdPool::Free() {
        if (m_handle) {
            ASSERT(m_renderer && "Renderer is null");
            removeCmdPool(m_renderer, m_handle);
        }
    }

    void SharedQueryPool::Free() {
        if (m_handle) {
            ASSERT(m_renderer && "Renderer is null");
            removeQueryPool(m_renderer, m_handle);
        }
    }

    void SharedFence::Free() {
        if (m_handle) {
            ASSERT(m_renderer && "Renderer is null");
            removeFence(m_renderer, m_handle);
        }
    }
    TinyImageFormat SharedTexture::FromHPLPixelFormat(ePixelFormat format) {
        switch(format) {
            case ePixelFormat_Alpha:
                return TinyImageFormat_A8_UNORM;
            case ePixelFormat_Luminance:
                return TinyImageFormat_R8_UNORM;
            case ePixelFormat_LuminanceAlpha:
                return TinyImageFormat_R8G8_UNORM;
            case ePixelFormat_RGB:
                return TinyImageFormat_R8G8B8_UNORM;
            case ePixelFormat_RGBA:
                return TinyImageFormat_R8G8B8A8_UNORM;
            case ePixelFormat_BGRA:
                return TinyImageFormat_B8G8R8A8_UNORM;
            case ePixelFormat_DXT1:
                return TinyImageFormat_DXBC1_RGBA_UNORM;
            case ePixelFormat_DXT2:
            case ePixelFormat_DXT3:
                return TinyImageFormat_DXBC2_UNORM;
            case ePixelFormat_DXT4:
            case ePixelFormat_DXT5:
                return TinyImageFormat_DXBC3_UNORM;
            case ePixelFormat_Depth16:
                return TinyImageFormat_D16_UNORM;
            case ePixelFormat_Depth24:
                return TinyImageFormat_D24_UNORM_S8_UINT;
            case ePixelFormat_Depth32:
                return TinyImageFormat_D32_SFLOAT;
            case ePixelFormat_Alpha16:
                return TinyImageFormat_R16_UNORM;
            case ePixelFormat_Luminance16:
                return TinyImageFormat_R16_UNORM;
            case ePixelFormat_LuminanceAlpha16:
                return TinyImageFormat_R16G16_UNORM;
            case ePixelFormat_RGBA16:
                return TinyImageFormat_R16G16B16A16_UNORM;
            case ePixelFormat_Alpha32:
                return TinyImageFormat_R32_SFLOAT;
            case ePixelFormat_Luminance32:
                return TinyImageFormat_R32_SFLOAT;
            case ePixelFormat_LuminanceAlpha32:
                return TinyImageFormat_R32G32_SFLOAT;
            case ePixelFormat_RGBA32:
                return TinyImageFormat_R32G32B32A32_SFLOAT;
            case ePixelFormat_RGB16:
                return TinyImageFormat_R16G16B16_UNORM;
            case ePixelFormat_BGR:
                return TinyImageFormat_B8G8R8_UNORM;
            default:
                ASSERT(false && "Unsupported texture format");
                break;
        }
        return TinyImageFormat_UNDEFINED;
    }

    namespace detail {
        TinyImageFormat ToGraphicsCardSupportedFormat(ePixelFormat format) {
            switch(format) {
                case ePixelFormat_Alpha:
                case ePixelFormat_Luminance:
                    return TinyImageFormat_R8_UNORM;
                case ePixelFormat_LuminanceAlpha:
                    return TinyImageFormat_R8G8_UNORM;
                case ePixelFormat_RGB: // generally not supported most hardware does not support 24 bit formats
                case ePixelFormat_RGBA:
                    return TinyImageFormat_R8G8B8A8_UNORM;
                case ePixelFormat_BGRA:
                    return TinyImageFormat_B8G8R8A8_UNORM;
                case ePixelFormat_DXT1:
                    return TinyImageFormat_DXBC1_RGBA_UNORM;
                case ePixelFormat_DXT2:
                case ePixelFormat_DXT3:
                    return TinyImageFormat_DXBC2_UNORM;
                case ePixelFormat_DXT4:
                case ePixelFormat_DXT5:
                    return TinyImageFormat_DXBC3_UNORM;
                case ePixelFormat_Depth16:
                    return TinyImageFormat_D16_UNORM;
                case ePixelFormat_Depth24:
                    return TinyImageFormat_D32_SFLOAT_S8_UINT;
                case ePixelFormat_Depth32:
                    return TinyImageFormat_D32_SFLOAT;
                case ePixelFormat_Alpha16:
                case ePixelFormat_Luminance16:
                    return TinyImageFormat_R16_UNORM;
                case ePixelFormat_LuminanceAlpha16:
                    return TinyImageFormat_R16G16_UNORM;
                case ePixelFormat_RGBA16:
                case ePixelFormat_RGB16:
                    return TinyImageFormat_R16G16B16A16_UNORM;
                case ePixelFormat_Alpha32:
                case ePixelFormat_Luminance32:
                    return TinyImageFormat_R32_SFLOAT;
                case ePixelFormat_LuminanceAlpha32:
                    return TinyImageFormat_R32G32_SFLOAT;
                case ePixelFormat_RGBA32:
                    return TinyImageFormat_R32G32B32A32_SFLOAT;
                case ePixelFormat_BGR:
                    return TinyImageFormat_B8G8R8A8_UNORM;
                default:
                    ASSERT(false && "Unsupported texture format");
                    break;
            }
            return TinyImageFormat_UNDEFINED;
        }
    }


    #define MIP_REDUCE(s, mip) (max(1u, (uint32_t)((s) >> (mip))))


    SharedTexture SharedTexture::LoadFromHPLBitmap(cBitmap& bitmap, const BitmapLoadOptions& options) {
        SharedTexture handle;
        SyncToken token = {};

        TextureDesc desc{};
        desc.mWidth = bitmap.GetWidth();
        desc.mHeight = bitmap.GetHeight();
        desc.mDepth = bitmap.GetDepth();
        desc.mSampleCount = SAMPLE_COUNT_1;
        desc.mMipLevels = options.m_useMipmaps ? bitmap.GetNumOfMipMaps() : 1;
        desc.mArraySize = options.m_useArray ? bitmap.GetNumOfImages() : 1;
        desc.mDescriptors = DESCRIPTOR_TYPE_TEXTURE;
        desc.mFormat = detail::ToGraphicsCardSupportedFormat(bitmap.GetPixelFormat());
	    desc.mStartState = RESOURCE_STATE_COMMON;
        if(options.m_useCubeMap) {
            desc.mDescriptors |= DESCRIPTOR_TYPE_TEXTURE_CUBE;
            if(options.m_useArray) {
                ASSERT(bitmap.GetNumOfImages() % 6 == 0 && "Cube map array must have a multiple of 6 images");
                desc.mArraySize = bitmap.GetNumOfImages();
            } else {
                ASSERT(bitmap.GetNumOfImages() == 6 && "Cube map must have 6 images");
                desc.mArraySize = 6;
            }
        }

        handle.Load([&](Texture** texture) {
            TextureLoadDesc textureLoadDesc = {};
            textureLoadDesc.ppTexture = texture;
            textureLoadDesc.pDesc = &desc;
            addResource(&textureLoadDesc, &token);
            return true;
        });

        auto sourceImageFormat = FromHPLPixelFormat(bitmap.GetPixelFormat());
        auto isCompressed  = TinyImageFormat_IsCompressed(desc.mFormat);
        for(uint32_t arrIndex = 0; arrIndex < desc.mArraySize; arrIndex++) {
            for(uint32_t mipLevel = 0; mipLevel < desc.mMipLevels; mipLevel++) {
                TextureUpdateDesc update = {handle.m_handle, mipLevel, arrIndex};
                const auto& input = bitmap.GetData(arrIndex, mipLevel);
                //auto data = std::span<uint8_t>(input->mpData, static_cast<size_t>(input->mlSize));
                beginUpdateResource(&update);
         
                uint32_t sourceRowStride;
                uint32_t destRowStride;
                if (!util_get_surface_info(
                        MIP_REDUCE(desc.mWidth, mipLevel),
                        MIP_REDUCE(desc.mHeight, mipLevel),
                        sourceImageFormat,
                        nullptr,
                        &sourceRowStride,
                        nullptr)) {
                    ASSERT(false && "Failed to get surface info");
                }
                uint32_t srcElementStride = sourceRowStride / desc.mWidth;

                 if (!util_get_surface_info(
                        MIP_REDUCE(desc.mWidth, mipLevel),
                        MIP_REDUCE(desc.mHeight, mipLevel),
                        desc.mFormat,
                        nullptr,
                        &destRowStride,
                        nullptr)) {
                    ASSERT(false && "Failed to get surface info");
                }

                uint32_t dstElementStride = destRowStride / desc.mWidth;

                for (size_t z = 0; z < desc.mDepth; ++z)
                {
                    uint8_t* dstData = update.pMappedData + (update.mDstSliceStride * z);
                    auto srcData = input->mpData + update.mSrcSliceStride * z;
                    for (uint32_t row = 0; row < update.mRowCount; ++row) {
                        if(isCompressed) {
                             std::memcpy(dstData + (row * update.mDstRowStride), srcData + (row * update.mSrcRowStride), update.mSrcRowStride);
                        } else {
                            for(uint32_t column = 0;  column < desc.mWidth; column++) {
                                std::memset(dstData + (row * update.mDstRowStride + column * dstElementStride), 0xff, dstElementStride);
                                std::memcpy(dstData + (row * update.mDstRowStride + column * dstElementStride), srcData + (row * sourceRowStride + column * srcElementStride), std::min(dstElementStride, srcElementStride));
                            }
                        }
                    }
                }
                endUpdateResource(&update, &token);
            }
        }
        waitForToken(&token);
        return handle;
    }
    SharedTexture SharedTexture::CreateCubemapFromHPLBitmaps(const std::span<cBitmap*> bitmaps, const BitmapCubmapLoadOptions& options) {
        SharedTexture handle;
        ASSERT(bitmaps.size() == 6 && "Cubemap must have 6 bitmaps");
        SyncToken token = {};
        TextureDesc desc{};
        desc.mWidth = bitmaps[0]->GetWidth();
        desc.mHeight = bitmaps[0]->GetHeight();
        desc.mDepth = bitmaps[0]->GetDepth();
        desc.mArraySize = 6;
        desc.mFormat = FromHPLPixelFormat(bitmaps[0]->GetPixelFormat());
        desc.mMipLevels = options.m_useMipmaps ? bitmaps[0]->GetNumOfMipMaps() : 1;
	    desc.mStartState = RESOURCE_STATE_COMMON;
        desc.mSampleCount = SAMPLE_COUNT_1;
        desc.mDescriptors = DESCRIPTOR_TYPE_TEXTURE | DESCRIPTOR_TYPE_TEXTURE_CUBE;

        handle.Load([&](Texture** texture) {
            TextureLoadDesc textureLoadDesc = {};
            textureLoadDesc.ppTexture = texture;
            textureLoadDesc.pDesc = &desc;
            addResource(&textureLoadDesc, &token);
            return true;
        });

        for(auto& bitmap : bitmaps) {
            ASSERT((options.m_useMipmaps || (bitmap->GetNumOfMipMaps() == desc.mMipLevels)) && "All bitmaps must have the same number of mipmaps");
            ASSERT(bitmap->GetWidth() == desc.mWidth && "All bitmaps must have the same width");
            ASSERT(bitmap->GetHeight() == desc.mHeight && "All bitmaps must have the same height");
            ASSERT(bitmap->GetDepth() == desc.mDepth && "All bitmaps must have the same depth");

            auto sourceImageFormat = FromHPLPixelFormat(bitmap->GetPixelFormat());
            uint32_t sourceRowStride;
            if(!util_get_surface_info(desc.mWidth, desc.mHeight, sourceImageFormat, nullptr, &sourceRowStride, nullptr)) {
                ASSERT(false && "Failed to get surface info");
            }
            uint32_t srcElementStride = sourceRowStride / desc.mWidth;
            auto isCompressed  = TinyImageFormat_IsCompressed(desc.mFormat);

            for(uint32_t arrIndex = 0; arrIndex < desc.mArraySize; arrIndex++) {
                for(uint32_t mipLevel = 0; mipLevel < desc.mMipLevels; mipLevel++) {
                    TextureUpdateDesc update = {handle.m_handle, mipLevel, arrIndex};
                    const auto& input = bitmap->GetData(arrIndex, mipLevel);
                    auto data = std::span<unsigned char>(input->mpData, static_cast<size_t>(input->mlSize));
                    beginUpdateResource(&update);
                    for (uint32_t z = 0; z < desc.mDepth; ++z) {
                        uint32_t dstElementStride = update.mDstRowStride / desc.mWidth;
                        uint8_t* dstData = update.pMappedData + update.mDstSliceStride * z;
                        auto srcData = data.begin() + update.mSrcSliceStride * z;

                        for (uint32_t row = 0; row < update.mRowCount; ++row) {
                            if(isCompressed) {
                                std::memcpy(dstData + row * update.mDstRowStride, &srcData[row * update.mSrcRowStride], update.mSrcRowStride);
                            } else {
                                for(uint32_t column = 0;  column < desc.mWidth; column++) {
                                    std::memcpy(dstData + row * update.mDstRowStride + column * dstElementStride, &srcData[row * sourceRowStride + column * srcElementStride], std::min(dstElementStride, srcElementStride));
                                }
                            }
                        }
                    }
                    endUpdateResource(&update, &token);
                }
            }
        }
        return handle;
    }

} // namespace hpl
