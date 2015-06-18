#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "evolver.h"

static SDL_Window *win = NULL;
static SDL_Renderer *rend = NULL;

// for mutate thread
//static SDL_Thread *mutThread = NULL;
//static SDL_mutex *mutLock = NULL;
//static SDL_cond *mutCond = NULL;
//static bool mutDone = false;
//static bool mutWait;
//static SDL_atomic_t *mutStop;

static void cleanup()
{
    if (rend != NULL)
        SDL_DestroyRenderer(rend);
    if (win != NULL)
        SDL_DestroyWindow(win);
//    if (mutCond != NULL)
//        SDL_DestroyCond(mutCond);
//    if (mutLock != NULL)
//        SDL_DestroyMutex(mutLock);
}

static int sdlErr()
{
    fprintf(stderr, "SDL Error: %s\n", SDL_GetError());
    return EXIT_FAILURE;
}

/*
static int mutate(void *evol)
{
    SDL_LockMutex(mutLock);
    while (!mutDone)
    {
        mutWait = true;
        while (!SDL_AtomicGet(mutStop))
        {
            mutateAndSelect((struct Evolver *)evol);
        }
        SDL_AtomicSet(mutStop, 0);
        while (mutWait)
            SDL_CondWait(mutCond, mutLock);
    }
    SDL_UnlockMutex(mutLock);

    return 0;
}
*/

int main(int argc, char *argv[])
{
    int lineCount;
    int frameSkip = 0;
    SDL_Surface *loadedImg = NULL;
    SDL_Surface *source = NULL;
    struct Evolver *evol;

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        fprintf(stderr, "Failed to init SDL: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    if (argc != 3 && argc != 4)
    {
        fprintf(stderr, "Wrong number of arguments.\n");
        fprintf(stderr, "Usage: genetic <image> <line count> "
                        "[frames to skip]\n");
        return EXIT_FAILURE;
    }

    lineCount = (int) strtol(argv[2], NULL, 10);
    if (lineCount < 1)
    {
        fprintf(stderr, "Line count must be an integer that is at least 1.\n");
        return EXIT_FAILURE;
    }

    if (argc == 4)
        frameSkip = (int) strtol(argv[3], NULL, 10);
    if (frameSkip < 0)
    {
        fprintf(stderr, "Frame skip must be a positive integer.\n");
        return EXIT_FAILURE;
    }

    atexit(SDL_Quit);
    atexit(cleanup);
    
    loadedImg = IMG_Load(argv[1]);
    if (loadedImg == NULL)
        return sdlErr();

    source = SDL_ConvertSurfaceFormat(loadedImg, SDL_PIXELFORMAT_ARGB8888, 0);
    SDL_FreeSurface(loadedImg);
    if (source == NULL)
        return sdlErr();
    evol = createEvolver(source->pixels, source->w, source->h, lineCount);
    SDL_FreeSurface(source);
    
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
    win = SDL_CreateWindow("Genetic Drawing", SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED, evol->w, evol->h,
            SDL_WINDOW_OPENGL);
    if (win == NULL)
        return sdlErr();

    rend = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    if (rend == NULL)
        return sdlErr();

//    mutLock = SDL_CreateMutex();
//    mutCond = SDL_CreateCond();

//    mutStop = malloc(sizeof *mutStop);
//    mutStop->value = 0;
//    mutThread = SDL_CreateThread(mutate, "Mutate Thread", (void *)evol);
//    if (mutThread == NULL)
//        return sdlErr();

    bool done = false;
    struct Sequence *seq; 
    while (!done)
    {
        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_WINDOWEVENT &&
                    event.window.event == SDL_WINDOWEVENT_CLOSE)
            {
                done = true;
            }
        }

        for (int skip = 0; skip <= frameSkip; skip++)
        {
            mutateAndSelect(evol);
        }

//        SDL_AtomicSet(mutStop, 1);
//        SDL_LockMutex(mutLock);
        seq = copyBestFit(evol);
//        if (done)
//            mutDone = true;
//        mutWait = false;
//        SDL_CondSignal(mutCond);
//        SDL_UnlockMutex(mutLock);

        SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
        SDL_RenderClear(rend);

        for (int i = 0; i < seq->length; i++)
        {
            struct LineDef line = seq->lines[i];
            int red = line.color >> 16;
            int green = line.color >> 8;
            int blue = line.color;

            SDL_SetRenderDrawColor(rend, red, green, blue, 255);
            SDL_RenderDrawLine(rend, line.x0, line.y0, line.x1, line.y1);
        }

        if (!done)
            freeSequence(seq);

        SDL_RenderPresent(rend);
//        SDL_Delay(50);
    }

//    SDL_DetachThread(mutThread);

    seqToSVG(seq, "temp.svg", evol->w, evol->h, 1.0f);
    freeSequence(seq);
    freeEvolver(evol);

    return 0;
}
