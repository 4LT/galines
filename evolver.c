#include <string.h>
#include <time.h>
#include "evolver.h"

#define MB_SWAP(a, b)\
{\
    b ^= a;\
    a ^= b;\
    b ^= a;\
}

struct PointList
{
    int *pts; // array of points, sequence of x/y pairs
    unsigned int count;
    uint32_t color;
};

static void drawPoints(uint32_t *img, unsigned int imgWidth,
        struct PointList points);

static struct PointList lineToPoints(struct LineDef line);
static struct LineDef randomLine(const struct Evolver *evol);
static int colorDiff(uint32_t c1, uint32_t c2);
static unsigned int fitness(struct Evolver *evol, uint32_t *img,
        struct PointList newPts, struct PointList oldPts);

static void drawPoints(uint32_t *img, unsigned int imgWidth,
        struct PointList points)
{
    for (int i = 0; i < points.count; i++)
        img[points.pts[2*i] + imgWidth * points.pts[2*i+1]] = points.color;
}

static struct PointList lineToPoints(struct LineDef line)
{
    int dx = line.x1-line.x0; // delta x
    int dy = line.y1-line.y0; // delta y
    int depSign = 1; // dependent var delta sign
    struct PointList points;
    points.color = line.color;

    // independent/dependent start/stop and deltas [dependent = f(independent)]
    int ind0, ind1, dInd, dep0, dep1, dDep;

    // true: line is steep, false: line is shallow
    int steep = abs(dx) < abs(dy);

    // if steep, y is independent and x is dependent, else x is independent and
    // y is dependent
    if (steep)
    {
        ind0 = line.y0;
        ind1 = line.y1;
        dInd = dy;
        dep0 = line.x0;
        dep1 = line.x1;
        dDep = dx;
    }
    else
    {
        ind0 = line.x0;
        ind1 = line.x1;
        dInd = dx;
        dep0 = line.y0;
        dep1 = line.y1;
        dDep = dy;
    }

    // reverse direction of dependent if f'(independent) is < 0
    if (dep1 < dep0)
    {
        depSign = -1;
        dDep = -dDep;
    }

    // correct coordinates and directions if line drawn from lower-right to
    // upper-left
    if (ind1 < ind0)
    {
        MB_SWAP(ind0, ind1);
        MB_SWAP(dep0, dep1);
        dInd = -dInd;
        depSign = -depSign;    
    }

    points.count = dInd+1;
    points.pts = calloc(2 * points.count, sizeof *(points.pts));

    int dl = 2*dDep; // delta lower
    int du = 2*(dDep - dInd); // delta upper
    int d = 2*dDep - dInd; // error
    int pixDep = dep0; // initial dependent coordinate for pixel

    // iterate over independent variable
    int i, pixInd;
    for (i = 0, pixInd = ind0; pixInd <= ind1; i++, pixInd++)
    {        
        // if steep, dependent = x and independent = y
        // else dependent = y and independent = x
        if (steep)
        {
            points.pts[i*2] = pixDep;
            points.pts[i*2 + 1] = pixInd;
        }
        else
        {
            points.pts[i*2] = pixInd;
            points.pts[i*2 + 1] = pixDep;
        }

        if (d <= 0)
        {
            d+= dl;
        }
        else
        {
            pixDep+= depSign;
            d+= du;
        }
            
    }

    return points;
} 

static struct LineDef randomLine(const struct Evolver *evol)
{
    struct LineDef line;
    line.x0 = rand() % evol->w;
    line.x1 = rand() % evol->w;
    line.y0 = rand() % evol->h;
    line.y1 = rand() % evol->h;

    int red = rand() & 0xFF;
    int green = rand() & 0xFF;
    int blue = rand() & 0xFF;
    line.color = red << 16 | green << 8 | blue;

    return line;
}

static int colorDiff(uint32_t c1, uint32_t c2)
{
    return abs( (c1>>16 & 0xFF) - (c2>>16 & 0xFF) ) +
            abs( (c1>>8 & 0xFF) - (c2>>8 & 0xFF) ) +
            abs((c1 & 0xFF) - (c2 & 0xFF));
}

static unsigned int fitness(struct Evolver *evol, uint32_t *img,
        struct PointList newPts, struct PointList oldPts)
{
    int fitness = 0;
    int x, y;
    
    for (int i = 0; i < newPts.count; i++)
    {
        x = newPts.pts[2*i];
        y = newPts.pts[2*i + 1];
        fitness-= colorDiff(evol->source[y*evol->w + x], img[y*evol->w + x]);
    }

    for (int i = 0; i < oldPts.count; i++)
    {
        x = oldPts.pts[2*i];
        y = oldPts.pts[2*i + 1];
        fitness-= colorDiff(evol->source[y*evol->w + x], img[y*evol->w + x]);
    }
    
    return fitness;
}

struct Evolver *createEvolver(const uint32_t *sourcePix, unsigned int width,
        unsigned int height, size_t seqLength)
{
    size_t imgPixCount = width * height;
    size_t imgByteCount = imgPixCount * sizeof (uint32_t);
    struct Evolver *evol = malloc(sizeof (struct Evolver));
    evol->source = malloc(imgByteCount);
    evol->_bestImg = calloc(imgPixCount, sizeof (uint32_t));
    memcpy(evol->source, sourcePix, imgByteCount);
    evol->w = width;
    evol->h = height;

    evol->_bestSeq.length = seqLength;
    evol->_bestSeq.lines = malloc(seqLength * sizeof (struct LineDef));
    srand(time(NULL));

    for (int i = 0; i < seqLength; i++)
    {
          struct LineDef line = randomLine(evol);
          struct PointList linePoints = lineToPoints(line);
          evol->_bestSeq.lines[i] = line;
          drawPoints(evol->_bestImg, evol->w, linePoints);
          free(linePoints.pts);
    }

    return evol;
}

void mutateAndSelect(struct Evolver *evol)
{
    uint32_t *testImg = calloc(evol->w * evol->h, sizeof (uint32_t));
    struct LineDef newLine = randomLine(evol);
    struct PointList newLinePts = lineToPoints(newLine);
    int selectedIndex = rand() % evol->_bestSeq.length;
    struct PointList selectedPts
            = lineToPoints(evol->_bestSeq.lines[selectedIndex]);

    for (int i = 0; i < evol->_bestSeq.length; i++)
    {
        if (i == selectedIndex)
            drawPoints(testImg, evol->w, newLinePts);
        else
        {
            struct PointList tmpPts = lineToPoints(evol->_bestSeq.lines[i]);
            drawPoints(testImg, evol->w, tmpPts);
            free(tmpPts.pts);
        }
    }

    int oldFitness = fitness(evol, evol->_bestImg, newLinePts, selectedPts);
    int newFitness = fitness(evol, testImg, newLinePts, selectedPts);

    if (newFitness > oldFitness)
    {
        evol->_bestSeq.lines[selectedIndex] = newLine;
        free(evol->_bestImg);
        evol->_bestImg = testImg;
    }
    else
    {
        free(testImg);
    }
    
    free(newLinePts.pts);
    free(selectedPts.pts);
}

struct Sequence *copyBestFit(const struct Evolver *evol)
{
    size_t linesByteCount = evol->_bestSeq.length * sizeof (struct LineDef);
    struct Sequence *seq = malloc(sizeof (struct Sequence));
    seq->lines = malloc(linesByteCount);
    memcpy(seq->lines, evol->_bestSeq.lines, linesByteCount);
    seq->length = evol->_bestSeq.length;
    return seq;
}

void freeSequence(struct Sequence *seq)
{
    free(seq->lines);
    free(seq);
}

void freeEvolver(struct Evolver *evol)
{
    free(evol->source);
    free(evol->_bestImg);
    free(evol->_bestSeq.lines);
    free(evol);
}
