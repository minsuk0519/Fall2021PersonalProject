//standard library
#include <iostream>

#include "Engine/Application.hpp"
#include "Engine/Graphic/Graphic.hpp"

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
    //_CrtSetBreakAlloc(157);

    Application app;

    try {
        app.init();

        app.AddSystem<Graphic>();

        app.postinit();
        
        app.update();
    } catch (const std::exception& e) {
        app.close();
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    app.close();

    return EXIT_SUCCESS;
}
