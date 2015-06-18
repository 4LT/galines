#include "svg-writer.h"
#include <string.h>
#include <stdio.h>

static const char *SVG_HEAD =
  "<?xml version=\"1.0\" standalone=\"no\" ?>\n"
  "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\"\n"
  "  \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n"
  "<svg width=\"%upx\" height=\"%upx\" version=\"1.1\"\n"
  "  xmlns=\"http://www.w3.org/2000/svg\">\n"
  "    <rect fill=\"black\" width=\"100%%\" height=\"100%%\" />\n";

static const char *SVG_TAIL = "</svg>\n";

static const char *SVG_LINE = "    <line x1=\"%.3f%%\" y1=\"%.3f%%\" "
                              "x2=\"%.3f%%\" y2=\"%.3f%%\" "
                              "stroke=\"#%6x\" />\n";

bool seqToSVG(struct Sequence *seq, const char *fileName,
  unsigned int w, unsigned int h, float scale)
{
    FILE *svgOut = fopen(fileName, "w");
    if (svgOut == NULL)
    {
        fprintf(stderr, "\"%s\" could not be opened.", fileName);
        return false; 
    }

    fprintf(svgOut, SVG_HEAD, w, h);
    fprintf(svgOut, "\n");
    float fw = (float)(w - (w == 1 ? 0 : 1));
    float fh = (float)(h - (h == 1 ? 0 : 1));

    for (size_t i = 0; i < seq->length; i++)
    {
        struct LineDef line = seq->lines[i];
        float scaledX0 = 100 * line.x0 / fw;
        float scaledY0 = 100 * line.y0 / fh;
        float scaledX1 = 100 * line.x1 / fw;
        float scaledY1 = 100 * line.y1 / fh;
        fprintf(svgOut, SVG_LINE, scaledX0, scaledY0, scaledX1, scaledY1,
          line.color);
    }

    fprintf(svgOut, "\n");
    fprintf(svgOut, SVG_TAIL);
    fclose(svgOut);
    return true;
}
