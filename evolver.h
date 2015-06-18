#ifndef _EVOLVER_H_
#define _EVOLVER_H_

#include <stdlib.h>
#include <stdint.h>

// Line defined by 2 points (x/y 0, and x/y 1) and the line's color.
struct LineDef
{
    int x0;
    int y0;
    int x1;
    int y1;
    
    uint32_t color;
};

struct Sequence
{
    size_t length;
    struct LineDef *lines;
};

struct Evolver
{
    uint32_t *source;
    unsigned int w;
    unsigned int h;
    struct Sequence _bestSeq;
    uint32_t *_bestImg;
};

struct Evolver *createEvolver(const uint32_t *sourcePix, unsigned int width,
        unsigned int height, size_t seqLength);

void mutateAndSelect(struct Evolver *evol);

struct Sequence *copyBestFit(const struct Evolver *evol);

void freeSequence(struct Sequence *seq);

void freeEvolver(struct Evolver *evol);

#endif
