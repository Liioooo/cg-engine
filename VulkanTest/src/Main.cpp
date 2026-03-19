#include "CgEngine/Application.h"
#include "Scripts/FlyingCameraScript.h"

int main(int argc, char **argv) {
    auto* application = new CgEngine::Application("assets/game/settings.ini");

    application->registerNativeScript<VulkanTest::FlyingCameraScript>("flyingCameraScript");

    application->init();
    application->run();

    delete application;

    return 0;
}
