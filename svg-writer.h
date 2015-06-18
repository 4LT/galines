#ifndef _SVG_WRITER_H_
#define _SVG_WRITER_H_

#include "evolver.h"
#include <stdbool.h>

bool seqToSVG(struct Sequence *seq, const char *fileName,
  unsigned int w, unsigned int h, float scale);

#endif
