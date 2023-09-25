#pragma once

#include <graphics/Image.h>
#include <vector>

#include <span>

namespace hpl {
    class cTextureManager;

    class AnimatedImage : public iResourceBase {
        HPL_RTTI_IMPL_CLASS(iResourceBase, AnimatedImage, "{17f7c0e8-f266-4381-b395-84c057f745da}")
    public:
        AnimatedImage();
        AnimatedImage(const tString& asName, const tWString& asFullPath);
        virtual ~AnimatedImage();
        void Initialize(std::span<std::unique_ptr<hpl::Image>> images);

        Image* GetImage() const;

        virtual bool Reload() override;
        virtual void Unload() override;
        virtual void Destroy() override;

        void Update(float afTimeStep);
        void SetFrameTime(float frameTime);
        float GetFrameTime();
        eTextureAnimMode GetAnimMode();
        void SetAnimMode(eTextureAnimMode aMode);

    private:
        std::vector<std::unique_ptr<hpl::Image>> m_images;

        float m_frameTime = 0.0f;
        float m_timeCount = 0.0f;
        float m_timeDir = 1.0f;
        eTextureAnimMode m_animMode = eTextureAnimMode_Loop;
        cTextureManager* m_textureManager = nullptr;
    };
} // namespace hpl
