#include <Scripts/FlyingCameraScript.h>
#include <Scripts/CustomMeshTestScript.h>
#include <Scripts/FPSCounter.h>
#include "CgEngine/Application.h"

int main(int argc, char **argv) {
    auto* application = new CgEngine::Application("assets/game/settings.ini");

    application->registerNativeScript<RTR::FlyingCameraScript>("flyingCameraScript");
    application->registerNativeScript<RTR::CustomMeshTestScript>("customMeshTestScript");
    application->registerNativeScript<RTR::FPSCounter>("fpsCounter");

    application->init();
    application->run();

    delete application;

    system("pause");
    return 0;
}
