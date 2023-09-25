#pragma once

#include <engine/RTTI.h>

namespace hpl {

    class cScene;
    class cResources;
    class cUpdater;
    class cSystem;
    class cInput;
    class cGraphics;
    class cSound;
    class cPhysics;
    class cAI;
    class GraphicsContext;
    class cGui;
    class cHaptic;
    class cGenerate;
    class GraphicsContext;

    class EngineInterface {
        HPL_RTTI_CLASS(EngineInterface, "{4a770995-9604-41dc-9f81-79b4a230cf24}")

    public:
        virtual cScene* GetScene() = 0;
        virtual cResources* GetResources() = 0;
        virtual cUpdater* GetUpdater() = 0;
        virtual cSystem* GetSystem() = 0;
        virtual cInput* GetInput() = 0;
        virtual cGraphics* GetGraphics() = 0;
        virtual cSound* GetSound() = 0;
        virtual cPhysics* GetPhysics() = 0;
        virtual cAI* GetAI() = 0;
        virtual cGui* GetGui() = 0;
        virtual cHaptic* GetHaptic() = 0;
    };
} // namespace hpl
