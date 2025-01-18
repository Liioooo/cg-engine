#include <Scripts/FlyingCameraScript.h>
#include <Scripts/FPSCounter.h>
#include <Scripts/Ocean.h>
#include <Scripts/GrassScript.h>
#include <Scripts/LoadingScreenScript.h>
#include <Scripts/PlayerScript.h>
#include <Scripts/ControlScript.h>
#include "CgEngine/Application.h"

int main(int argc, char **argv) {
    auto* application = new CgEngine::Application("assets/game/settings.ini");

    application->registerNativeScript<RTR::FlyingCameraScript>("flyingCameraScript");
    application->registerNativeScript<RTR::FPSCounter>("fpsCounter");
    application->registerNativeScript<RTR::Ocean>("ocean");
    application->registerNativeScript<RTR::GrassScript>("grass");
    application->registerNativeScript<RTR::LoadingScreenScript>("loadingScreen");
    application->registerNativeScript<RTR::ControlScript>("control");
    application->registerNativeScript<RTR::PlayerScript>("player");

    application->init();
    application->run();

    delete application;

    return 0;
}
