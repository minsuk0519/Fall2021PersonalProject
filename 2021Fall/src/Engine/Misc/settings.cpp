#include "settings.hpp"

unsigned int Settings::windowWidth = 1280;
unsigned int Settings::windowHeight = 720;

float Settings::GetAspectRatio()
{
    return static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
}
