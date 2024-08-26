#include "CgEngine/Application.h"

#include "Scripts/FlyingCameraScript.h"
#include "Scripts/GhostsController.h"
#include "Scripts/PlayerScript.h"
#include "Scripts/ProjectileScript.h"
#include "Scripts/EndScript.h"
#include "Scripts/StartScript.h"
#include "Scripts/FPSCounter.h"

int main(int argc, char **argv) {
    auto* application = new CgEngine::Application("assets/game/settings.ini");

    application->registerNativeScript<Game::FlyingCameraScript>("flyingCameraScript");
    application->registerNativeScript<Game::GhostsController>("ghostsController");
    application->registerNativeScript<Game::PlayerScript>("player");
    application->registerNativeScript<Game::ProjectileScript>("projectileScript");
    application->registerNativeScript<Game::EndScript>("endScript");
    application->registerNativeScript<Game::StartScript>("startScript");
    application->registerNativeScript<Game::FPSCounter>("fpsCounter");

    application->init();
    application->run();

    delete application;

    system("pause");
    return 0;
}
