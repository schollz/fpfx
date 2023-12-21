#ifndef BITCRUSH_LIB
#define BITCRUSH_LIB 1

#include "fixedpoint.h"

typedef struct Bitcrush {
  uint8_t bits;
  uint8_t reduce;
} Bitcrush;

Bitcrush *Bitcrush_malloc() {
  Bitcrush *bitcrush = (Bitcrush *)malloc(sizeof(Bitcrush));
  if (bitcrush == NULL) {
    return NULL;
  }
  bitcrush->bits = 8;
  bitcrush->reduce = 5;

  return bitcrush;
}

void Bitcrush_process(Bitcrush *bitcrush, int32_t *buf,
                      unsigned int nr_samples) {
  for (int i = 0; i < nr_samples; i++) {
    if (i % bitcrush->reduce == 0) {
      // bitcrush fixedpoint
      buf[i] = buf[i] >> (16 - bitcrush->bits) << (16 - bitcrush->bits);
    } else {
      buf[i] = buf[i - 1];
    }
  }
}

void Bitcrush_free(Bitcrush *bitcrush) {
  if (bitcrush != NULL) {
    free(bitcrush);
  }
}

#endif