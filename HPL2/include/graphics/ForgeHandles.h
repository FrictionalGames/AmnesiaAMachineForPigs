#pragma once

#include "graphics/GraphicsTypes.h"

#include "Common_3/Graphics/Interfaces/IGraphics.h"
#include "Common_3/Resources/ResourceLoader/Interfaces/IResourceLoader.h"
#include "FixPreprocessor.h"

#include <atomic>
#include <functional>
#include <span>
#include <folly/hash/Hash.h>


namespace hpl {
    class cBitmap;

    template<class CRTP, class T>
    struct UniqueHandle {
    public:
        using Base = UniqueHandle<CRTP, T>;
        // NOTE: This is a pointer to the resource
        //      Just how the API works for the-forge this is exposed to the user so be careful
        T* m_handle = nullptr;
    };

    // a handle that can be used to reference a resource
    template<class CRTP, class T>
    struct RefHandle {
    public:
        using Base = RefHandle<CRTP, T>;
        // NOTE: This is a pointer to the resource
        //      Just how the API works for the-forge this is exposed to the user so be careful
        T* m_handle = nullptr;

        struct RefCounter {
        public:
            RefCounter() = default;
            RefCounter(const RefCounter&) = delete;
            RefCounter& operator=(const RefCounter&) = delete;
            RefCounter(RefCounter&&) = delete;
            RefCounter& operator=(RefCounter&&) = delete;
            std::atomic<uint32_t> m_refCount = 0;
        };

        RefHandle() = default;

        RefHandle(const RefHandle& other) {
            m_handle = other.m_handle;
            m_refCounter = other.m_refCounter;
            m_initialized = other.m_initialized;
            if(m_initialized) {
                ASSERT(m_refCounter && "RefCounter is null");
                m_refCounter->m_refCount++;
            }
        }

        ~RefHandle() {
            TryFree();
        }

        RefHandle(RefHandle&& other) {
            m_handle = other.m_handle;
            m_refCounter = other.m_refCounter;
            m_initialized = other.m_initialized;
            other.m_handle = nullptr;
            other.m_refCounter = nullptr;
            other.m_initialized = false;
        }

        void Load(std::function<bool(T** handle)> load) {
            TryFree();
            if(!m_initialized) {
                ASSERT(!m_handle && "Handle is not null");
                if(load(&m_handle)) {
                    m_owning = true;
                    ASSERT(m_handle && "Handle is null");
                    ASSERT(!m_refCounter && "Trying to Initialize a handle with references");
                    if(!m_refCounter) {
                        m_refCounter = new RefCounter();
                    }
                    m_refCounter->m_refCount++;
                    m_initialized = true;
                } else {
                    ASSERT(!m_handle && "Handle is not null");
                }
            }
        }

        void TryFree() {
            // we don't own the handle so we can't free it
            if(!m_initialized) {
                ASSERT(!m_handle && "Handle is not null");
                // we only have a refcounter if we own the handle
                ASSERT(((!m_refCounter && m_owning) || !m_owning) && "RefCounter is not null");
                return;
            }
            if(!m_owning) {
                ASSERT(!m_refCounter && "RefCounter is not null"); // we should have a refcounter if we don't own the handle
                m_handle = nullptr;
                m_initialized = false;
                return;
            }

            ASSERT(m_initialized && "Trying to free a handle that has not been initialized");
            ASSERT(m_refCounter && "Trying to free a handle that has not been initialized");
            ASSERT(m_refCounter->m_refCount > 0 && "Trying to free resource that is still referenced");
            if((--m_refCounter->m_refCount) == 0) {
                static_cast<CRTP*>(this)->Free(); // Free the underlying resource
                delete m_refCounter;
            }
            m_handle = nullptr;
            m_initialized = false;
            m_refCounter = nullptr;
        }

        void operator= (const RefHandle& other) {
            TryFree(); // Free the current handle
            m_handle = other.m_handle;
            m_refCounter = other.m_refCounter;
            m_initialized = other.m_initialized;
            m_owning = other.m_owning;
            if(m_initialized && m_owning) {
                ASSERT(m_handle && "Handle is null");
                ASSERT(m_refCounter && "RefCounter is null");
                m_refCounter->m_refCount++;
            }
        }

        void operator= (RefHandle&& other) {
            TryFree(); // Free the current handle
            m_handle = other.m_handle;
            m_refCounter = other.m_refCounter;
            m_initialized = other.m_initialized;
            m_owning = other.m_owning;
            other.m_owning = false;
            other.m_handle = nullptr;
            other.m_refCounter = nullptr;
            other.m_initialized = false;
        }

        bool IsValid() const {
            return m_handle != nullptr;
        }

    protected:
        bool m_owning = true;
        bool m_initialized = false;
        RefCounter* m_refCounter = nullptr;
        friend CRTP;
    };

    struct SharedRenderTarget : public RefHandle<SharedRenderTarget, RenderTarget> {
    public:
        using Base = RefHandle<SharedRenderTarget, RenderTarget>;
        SharedRenderTarget()
            : Base() {
        }
        SharedRenderTarget(Renderer* renderer)
            : m_renderer(renderer)
            , Base() {
        }
        SharedRenderTarget(const SharedRenderTarget& other)
            : Base(other)
            , m_renderer(other.m_renderer) {
        }
        SharedRenderTarget(SharedRenderTarget&& other)
            : Base(std::move(other))
            , m_renderer(other.m_renderer) {
        }
        ~SharedRenderTarget() {
        }
        void operator=(const SharedRenderTarget& other) {
            Base::operator=(other);
            m_renderer = other.m_renderer;
        }
        void operator=(SharedRenderTarget&& other) {
            Base::operator=(std::move(other));
            m_renderer = other.m_renderer;
        }

        void Load(Renderer* renderer, std::function<bool(RenderTarget** handle)> load) {
            ASSERT(renderer && "Renderer is null");
            m_renderer = renderer;
            static_cast<Base*>(this)->Load(load);
        }
    private:
        void Free();
        Renderer* m_renderer = nullptr;
        friend struct RefHandle<SharedRenderTarget, RenderTarget>;
    };

    struct SharedTexture: public RefHandle<SharedTexture, Texture> {
    public:
        struct BitmapLoadOptions {
        public:
            bool m_useCubeMap = false;
            bool m_useArray = false;
            bool m_useMipmaps = false;
        };

        struct BitmapCubmapLoadOptions {
            bool m_useMipmaps: 1;
        };

        static SharedTexture LoadFromHPLBitmap(cBitmap& bitmap, const BitmapLoadOptions& options);
        static SharedTexture CreateCubemapFromHPLBitmaps(const std::span<cBitmap*> bitmaps, const BitmapCubmapLoadOptions& options);
        static TinyImageFormat FromHPLPixelFormat(ePixelFormat format);

        SharedTexture():
            Base() {
        }
        SharedTexture(const SharedTexture& other):
            Base(other), m_renderTarget(other.m_renderTarget){
        }
        SharedTexture(SharedTexture&& other):
            Base(std::move(other)),
            m_renderTarget(std::move(other.m_renderTarget)) {
        }
        ~SharedTexture() {
        }

        void operator= (const SharedTexture& other) {
            Base::operator=(other);
            m_renderTarget = other.m_renderTarget;
        }

        void operator= (SharedTexture&& other) {
            Base::operator=(std::move(other));
            m_renderTarget = std::move(other.m_renderTarget);
        }

        void SetRenderTarget(SharedRenderTarget renderTarget) {
            TryFree();
            m_initialized = true;
            m_owning = false; // We don't own the handle we are attaching
            m_handle = renderTarget.m_handle->pTexture;
            m_renderTarget = renderTarget; // We don't own the handle
        }
    private:
        void Free();
        SharedRenderTarget m_renderTarget{};
        friend struct RefHandle<SharedTexture, Texture>;
    };

    struct SharedDescriptorSet final: public RefHandle<SharedDescriptorSet, DescriptorSet> {
    public:
        SharedDescriptorSet():
            Base() {
        }
        SharedDescriptorSet(const SharedDescriptorSet& other):
            Base(other),
            m_renderer(other.m_renderer) {

        }
        SharedDescriptorSet(SharedDescriptorSet&& other):
            Base(std::move(other)),
            m_renderer(other.m_renderer){
        }
        ~SharedDescriptorSet() {
        }
        void operator= (const SharedDescriptorSet& other) {
            Base::operator=(other);
            m_renderer = other.m_renderer;
        }
        void operator= (SharedDescriptorSet&& other) {
            Base::operator=(std::move(other));
            m_renderer = other.m_renderer;
        }
        void Load(Renderer* renderer, std::function<bool(DescriptorSet** handle)> load) {
            ASSERT(renderer && "Renderer is null");
            m_renderer = renderer;
            Base::Load(load);
        }

    private:
        void Free();
        Renderer* m_renderer = nullptr;
        friend struct RefHandle<SharedDescriptorSet, DescriptorSet>;
    };

    struct SharedBuffer : public RefHandle<SharedBuffer, Buffer> {
    public:
        SharedBuffer():
            Base() {
        }
        SharedBuffer(const SharedBuffer& other):
            Base(other) {
        }
        SharedBuffer(SharedBuffer&& other):
            Base(std::move(other)) {
        }
        ~SharedBuffer() {
        }

        void operator= (const SharedBuffer& other) {
            Base::operator=(other);
        }
        void operator= (SharedBuffer&& other) {
            Base::operator=(std::move(other));
        }
    private:
        void Free();
        friend struct RefHandle<SharedBuffer, Buffer>;
    };

    struct SharedPipeline: public RefHandle<SharedPipeline, Pipeline> {
    public:
        SharedPipeline():
            Base() {
        }
        SharedPipeline(const SharedPipeline& other):
            Base(other),
            m_renderer(other.m_renderer) {
        }
        SharedPipeline(SharedPipeline&& other):
            Base(std::move(other)){
        }
        ~SharedPipeline() {
        }

        void operator= (const SharedPipeline& other) {
            Base::operator=(other);
            m_renderer = other.m_renderer;
        }
        void operator= (SharedPipeline&& other) {
            Base::operator=(std::move(other));
            m_renderer = other.m_renderer;
        }

        void Load(Renderer* renderer, std::function<bool(Pipeline** handle)> load) {
            ASSERT(renderer && "Renderer is null");
            m_renderer = renderer;
            Base::Load(load);
        }
    private:
        void Free();
        Renderer* m_renderer = nullptr;
        friend struct RefHandle<SharedPipeline, Pipeline>;
    };

    struct SharedShader: public RefHandle<SharedShader, Shader> {
    public:
        SharedShader():
            Base() {
        }
        SharedShader(const SharedShader& other):
            Base(other),
            m_renderer(other.m_renderer) {
        }
        SharedShader(SharedShader&& other):
            Base(std::move(other)){
        }
        ~SharedShader() {
        }

        void operator= (const SharedShader& other) {
            Base::operator=(other);
            m_renderer = other.m_renderer;
        }
        void operator= (SharedShader&& other) {
            Base::operator=(std::move(other));
            m_renderer = other.m_renderer;
        }

        void Load(Renderer* renderer, std::function<bool(Shader** handle)> load) {
            ASSERT(renderer && "Renderer is null");
            m_renderer = renderer;
            Base::Load(load);
        }
    private:
        void Free();
        Renderer* m_renderer = nullptr;
        friend struct RefHandle<SharedShader, Shader>;
    };
    struct SharedSampler: public RefHandle<SharedSampler, Sampler> {
    public:
        SharedSampler():
            Base() {
        }
        SharedSampler(const SharedSampler& other):
            Base(other) {
            m_renderer = other.m_renderer;
        }
        SharedSampler(SharedSampler&& other):
            Base(std::move(other)),
            m_renderer(other.m_renderer) {

        }
        ~SharedSampler() {
        }

        void Load(Renderer* renderer, std::function<bool(Sampler** handle)> load) {
            ASSERT(renderer && "Renderer is null");
            m_renderer = renderer;
            Base::Load(load);
        }

        void operator= (const SharedSampler& other) {
            Base::operator=(other);
            m_renderer = other.m_renderer;
        }
        void operator= (SharedSampler&& other) {
            Base::operator=(std::move(other));
            m_renderer = other.m_renderer;
        }
    private:
        void Free();
        Renderer* m_renderer = nullptr;
        friend struct RefHandle<SharedSampler, Sampler>;
    };

    struct SharedSwapChain: public RefHandle<SharedSwapChain, SwapChain> {

        SharedSwapChain():
            Base() {
        }
        SharedSwapChain(const SharedSwapChain& other):
            Base(other) {
            m_renderer = other.m_renderer;
        }
        SharedSwapChain(SharedSwapChain&& other):
            Base(std::move(other)),
            m_renderer(other.m_renderer) {

        }
        ~SharedSwapChain() {
        }

        void Load(Renderer* renderer, std::function<bool(SwapChain** handle)> load) {
            ASSERT(renderer && "Renderer is null");
            m_renderer = renderer;
            Base::Load(load);
        }

        void operator= (const SharedSwapChain& other) {
            Base::operator=(other);
            m_renderer = other.m_renderer;
        }
        void operator= (SharedSwapChain&& other) {
            Base::operator=(std::move(other));
            m_renderer = other.m_renderer;
        }
    private:
        void Free();
        Renderer* m_renderer = nullptr;
        friend struct RefHandle<SharedSwapChain, SwapChain>;
    };

    struct SharedRootSignature: public RefHandle<SharedRootSignature, RootSignature> {
    public:
        SharedRootSignature():
            Base() {
        }
        SharedRootSignature(const SharedRootSignature& other):
            Base(other) {
            m_renderer = other.m_renderer;
        }
        SharedRootSignature(SharedRootSignature&& other):
            Base(std::move(other)),
            m_renderer(other.m_renderer) {
        }
        ~SharedRootSignature() {
        }

        void Load(Renderer* renderer, std::function<bool(RootSignature** handle)> load) {
            ASSERT(renderer && "Renderer is null");
            m_renderer = renderer;
            Base::Load(load);
        }
        void operator= (const SharedRootSignature& other) {
            Base::operator=(other);
            m_renderer = other.m_renderer;
        }
        void operator= (SharedRootSignature&& other) {
            Base::operator=(std::move(other));
            m_renderer = other.m_renderer;
        }
    private:
        void Free();
        Renderer* m_renderer = nullptr;
        friend struct RefHandle<SharedRootSignature, RootSignature>;
    };

    struct SharedFence: public RefHandle<SharedFence, Fence> {
    public:
        SharedFence():
            Base() {
        }
        SharedFence(const SharedFence& other):
            Base(other) {
            m_renderer = other.m_renderer;
        }
        SharedFence(SharedFence&& other):
            Base(std::move(other)),
            m_renderer(other.m_renderer) {
        }
        ~SharedFence() {
        }

        void Load(Renderer* renderer, std::function<bool(Fence** handle)> load) {
            ASSERT(renderer && "Renderer is null");
            m_renderer = renderer;
            Base::Load(load);
        }
        void operator= (const SharedFence& other) {
            Base::operator=(other);
            m_renderer = other.m_renderer;
        }
        void operator= (SharedFence&& other) {
            Base::operator=(std::move(other));
            m_renderer = other.m_renderer;
        }
    private:
        void Free();
        Renderer* m_renderer = nullptr;
        friend struct RefHandle<SharedFence, Fence>;
    };

    struct SharedCmdPool: public RefHandle<SharedCmdPool, CmdPool> {
        public:
            SharedCmdPool():
                Base() {
            }
            SharedCmdPool(const SharedCmdPool& other):
                Base(other) {
                m_renderer = other.m_renderer;
            }
            SharedCmdPool(SharedCmdPool&& other):
                Base(std::move(other)),
                m_renderer(other.m_renderer) {
            }
            ~SharedCmdPool() {
            }

            void Load(Renderer* renderer, std::function<bool(CmdPool** handle)> load) {
                ASSERT(renderer && "Renderer is null");
                m_renderer = renderer;
                Base::Load(load);
            }
            void operator= (const SharedCmdPool& other) {
                Base::operator=(other);
                m_renderer = other.m_renderer;
            }
            void operator= (SharedCmdPool&& other) {
                Base::operator=(std::move(other));
                m_renderer = other.m_renderer;
            }
        private:
            void Free();
            Renderer* m_renderer = nullptr;
            friend struct RefHandle<SharedCmdPool, CmdPool>;
        };

    struct SharedCmd: public RefHandle<SharedCmd, Cmd> {
        public:
            SharedCmd():
                Base() {
            }
            SharedCmd(const SharedCmd& other):
                Base(other) {
                m_renderer = other.m_renderer;
            }
            SharedCmd(SharedCmd&& other):
                Base(std::move(other)),
                m_renderer(other.m_renderer) {
            }
            ~SharedCmd() {
            }

            void Load(Renderer* renderer, std::function<bool(Cmd** handle)> load) {
                ASSERT(renderer && "Renderer is null");
                m_renderer = renderer;
                Base::Load(load);
            }
            void operator= (const SharedCmd& other) {
                Base::operator=(other);
                m_renderer = other.m_renderer;
            }
            void operator= (SharedCmd&& other) {
                Base::operator=(std::move(other));
                m_renderer = other.m_renderer;
            }
        private:
            void Free();
            Renderer* m_renderer = nullptr;
            friend struct RefHandle<SharedCmd, Cmd>;
    };

    struct SharedQueryPool: public RefHandle<SharedQueryPool, QueryPool> {
        public:
            SharedQueryPool():
                Base() {
            }
            SharedQueryPool(const SharedQueryPool& other):
                Base(other) {
                m_renderer = other.m_renderer;
            }
            SharedQueryPool(SharedQueryPool&& other):
                Base(std::move(other)),
                m_renderer(other.m_renderer) {
            }
            ~SharedQueryPool() {
            }

            void Load(Renderer* renderer, std::function<bool(QueryPool** handle)> load) {
                ASSERT(renderer && "Renderer is null");
                m_renderer = renderer;
                Base::Load(load);
            }
            void operator= (const SharedQueryPool& other) {
                Base::operator=(other);
                m_renderer = other.m_renderer;
            }
            void operator= (SharedQueryPool&& other) {
                Base::operator=(std::move(other));
                m_renderer = other.m_renderer;
            }
        private:
            void Free();
            Renderer* m_renderer = nullptr;
            friend struct RefHandle<SharedQueryPool, QueryPool>;
    };
}

static bool operator==(const SamplerDesc& lhs, const SamplerDesc& rhs) {
   return rhs.mMinFilter  ==   lhs.mMinFilter &&
       rhs.mMagFilter  ==   lhs.mMagFilter &&
       rhs.mMipMapMode ==   lhs.mMipMapMode &&
       rhs.mAddressU   ==   lhs.mAddressU &&
       rhs.mAddressV   ==   lhs.mAddressV &&
       rhs.mAddressW   ==   lhs.mAddressW &&
       rhs.mMipLodBias ==   lhs.mMipLodBias &&
       rhs.mSetLodRange==   lhs.mSetLodRange &&
       rhs.mMinLod     ==   lhs.mMinLod &&
       rhs.mMaxLod     ==   lhs.mMaxLod &&
       rhs.mMaxAnisotropy== lhs.mMaxAnisotropy &&
       rhs.mCompareFunc  == lhs.mCompareFunc;
}

namespace std {
    template<>
    struct hash<SamplerDesc> {
        size_t operator()(const SamplerDesc& desc) const {
            return folly::hash::fnv32_buf(&desc, sizeof(desc));
        }
    };
}

