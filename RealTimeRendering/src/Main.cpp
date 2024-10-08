#include <Scripts/FlyingCameraScript.h>
#include "CgEngine/Application.h"

int main(int argc, char **argv) {
    auto* application = new CgEngine::Application("assets/game/settings.ini");

    application->registerNativeScript<RTR::FlyingCameraScript>("flyingCameraScript");

    application->init();
    application->run();

    delete application;

    system("pause");
    return 0;
}
