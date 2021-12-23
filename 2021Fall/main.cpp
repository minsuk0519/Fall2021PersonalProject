//standard library
#include <iostream>

#include "Engine/Common/Application.hpp"
#include "Engine/Graphic/Graphic.hpp"
#include "Engine/Level/LevelManager.hpp"
#include "Engine/Input/Input.hpp"

//for memory debug
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#define _CRTDBG_MAP_ALLOC
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)

#include <crtdbg.h>

int main() 
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    //_CrtSetBreakAlloc(1353837);

    Application* app = Application::APP();

    try {
        app->init();

        app->AddSystem<Input>();
        app->AddSystem<LevelManager>();
        app->AddSystem<Graphic>();

        app->postinit();
        
        app->update();
    } catch (const std::exception& e) {
        app->close();
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    app->close();

    return EXIT_SUCCESS;
}
