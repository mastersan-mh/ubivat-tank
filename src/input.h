#ifndef INPUT_H
#define INPUT_H

#include <SDL2/SDL_scancode.h>

#include <stdbool.h>

void input_key_setState(SDL_Scancode key, bool state);

#endif // INPUT_H
