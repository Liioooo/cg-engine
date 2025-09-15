#include "CgEngine/Application.h"

int main(int argc, char **argv) {
    auto* application = new CgEngine::Application("assets/game/settings.ini");

    application->init();
    application->run();

    delete application;

    return 0;
}
