#include "CgEngine/Application.h"
#include "Scripts/FlyingCameraScript.h"

int main(int argc, char **argv) {
    CgEngine::Application application("assets/game/settings.ini");

    application.registerNativeScript<VulkanTest::FlyingCameraScript>("flyingCameraScript");

    application.init();
    application.run();

    return 0;
}
