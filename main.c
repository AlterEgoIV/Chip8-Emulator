#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <SDL.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

SDL_Window* window = NULL;
SDL_Surface* screenSurface = NULL;

void drawPixel(SDL_Surface*, int, int, uint8_t);
void drawSprite(SDL_Surface*, uint8_t*, int, int, int);
bool initialise();
void close();
int emulator();

int main(int argc, char* args[])
{
    uint8_t sprite[] = {0b11000011, 0b01000010, 0b01111110, 0b01011010, 0b01111110};

    if(!initialise())
    {
        printf("Failed to initialise!\n");
    }
    else
    {
        bool quit = false;

        emulator();

        SDL_Event event;

        while(!quit)
        {
            while(SDL_PollEvent(&event) != 0)
            {
                if(event.type == SDL_QUIT) quit = true;
            }

            drawSprite(screenSurface, sprite, sizeof(sprite) / sizeof(sprite[0]), SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
            SDL_UpdateWindowSurface(window);
        }
    }

    close();

    return 0;
}

void drawPixel(SDL_Surface* surface, int x, int y, uint8_t bitValue)
{
    Uint32* p_screen = (Uint32*)surface->pixels;

    p_screen += y * surface->w + x;

    if(bitValue == 1)
    {
        *p_screen = SDL_MapRGBA(surface->format, 255, 255, 255, 128); // white pixel
    }
    else
    {
        *p_screen = SDL_MapRGBA(surface->format, 0, 0, 0, 128); // black pixel
    }
}

void drawSprite(SDL_Surface* surface, uint8_t* sprite, int length, int x, int y)
{
    //int line; // The line of pixels of the sprite
    //int bit; // The individual pixel on the line
    int currentByte; // The current byte/line we are drawing
    //length = sizeof(sprite) / sizeof(sprite[0]);
    //printf("%d", sizeof(sprite) / sizeof(sprite[0]));

    // For every line of the sprite
    for(int line = 0; line < length; ++line)
    {
        currentByte = sprite[line]; // Set the current byte to the current line/element of the sprite

        // For every bit on the current line/element of the sprite
        for(int bit = 0; bit < 8; ++bit)
        {
            /*
                Draw a pixel to the given SDL_Surface

                Draw the pixel at:
                    x plus the value of bit going right: x + (0, 1, 2, 3, 4, 5, 6, 7)

                    Incorrect: y plus the value of line going down: y + (0... length - 1)

                    SDL has its co-ordinate system origin at the top left.
                    This means that the x value decreases to the left and increases to the right.
                    It also means that the y value decreases going up and increases going down.

                    Subtracting from x moves things left. Adding to x moves things right.
                    Subtracting from y moves things up. Adding to y moves things down.

                    In order to draw the sprite with the first line/element at the bottom
                    and the last line/element at the top along the y axis,
                    we must subtract the value of line from y in the third parameter of drawPixel().

                    Doing this will cause the y co-ordinate each line is drawn at to be decreased,
                    causing the location of each line/element of the sprite to move up along the y axis
                    as the sprite is drawn.

                    Correct: y minus the value of line going up: y - (0... length - 1)

                Pass in the current byte with the seven most significant bits cleared
            */
            drawPixel(surface, x + bit, y - line, currentByte & 0b00000001);

            // Right shift the current byte by 1, losing the least significant bit
            currentByte = currentByte >> 1;
        }
    }
}

bool initialise()
{
    bool success = true;

    if(SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL could not initialise. Error: %s", SDL_GetError());
        success = false;
    }
    else
    {
        window = SDL_CreateWindow("Title", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

        if(window == NULL)
        {
            printf("The window could not be created. Error: %s", SDL_GetError());
            success = false;
        }
        else
        {
            screenSurface = SDL_GetWindowSurface(window);
        }
    }

    return success;
}

void close()
{
    SDL_DestroyWindow(window);
    window = NULL;

    SDL_Quit();
}
