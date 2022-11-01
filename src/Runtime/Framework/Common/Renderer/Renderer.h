#include "Interface/IRuntimeModule.h"
#include <memory>

namespace nilou {

    class FRendererModule : public IRuntimeModule
    {
    public:
        FRendererModule();

        virtual int StartupModule() override;
        virtual void ShutdownModule() override;

        void Draw(class FScene *Scene);

        void Draw_RenderThread(class FScene *Scene);

        class FDefferedShadingSceneRenderer *Renderer;
    };

}