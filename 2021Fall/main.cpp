//standard library
#include <iostream>

#include "Engine/Application.hpp"
#include "Engine/Graphic/Graphic.hpp"

int main() 
{
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