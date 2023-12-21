#ifndef REVERB_LIB
#define REVERB_LIB 1

#include "fixedpoint.h"
#include "ringbuffer.h"

typedef struct Reverb {
  Ringbuffer *fb0;
  Ringbuffer *fb1;
  Ringbuffer *fb2;
  Ringbuffer *fb3;
  Ringbuffer *fb4;
  Ringbuffer *fb5;
} Reverb;

Reverb *Reverb_malloc() {
  Reverb *reverb = (Reverb *)malloc(sizeof(Reverb));
  if (reverb == NULL) {
    return NULL;
  }
  reverb->fb0 = Ringbuffer_malloc(1229);
  reverb->fb1 = Ringbuffer_malloc(1559);
  reverb->fb2 = Ringbuffer_malloc(1907);
  reverb->fb3 = Ringbuffer_malloc(4057);
  reverb->fb4 = Ringbuffer_malloc(33123);
  reverb->fb5 = Ringbuffer_malloc(19993);

  // Check if any Ringbuffer creation failed
  if (!reverb->fb0 || !reverb->fb1 || !reverb->fb2 || !reverb->fb3 ||
      !reverb->fb4 || !reverb->fb5) {
    // Free any successfully allocated buffers
    Ringbuffer_free(reverb->fb0);
    Ringbuffer_free(reverb->fb1);
    Ringbuffer_free(reverb->fb2);
    Ringbuffer_free(reverb->fb3);
    Ringbuffer_free(reverb->fb4);
    Ringbuffer_free(reverb->fb5);
    free(reverb);
    return NULL;
  }

  return reverb;
}

void Reverb_process(Reverb *reverb, int32_t *buf, unsigned int nr_samples) {
  for (int i = 0; i < nr_samples; i++) {
    int32_t x = q16_16_multiply(Q16_16_0_125, buf[i]);
    x += q16_16_multiply(Q16_16_0_125, Ringbuffer_get(reverb->fb0));
    x += q16_16_multiply(Q16_16_0_125, Ringbuffer_get(reverb->fb1));
    x += q16_16_multiply(Q16_16_0_125, Ringbuffer_get(reverb->fb2));
    x += q16_16_multiply(Q16_16_0_125, Ringbuffer_get(reverb->fb3));
    x += q16_16_multiply(Q16_16_0_125, Ringbuffer_get(reverb->fb4));
    x += q16_16_multiply(Q16_16_0_125, Ringbuffer_get(reverb->fb5));
    Ringbuffer_add(reverb->fb0, x);
    Ringbuffer_add(reverb->fb1, x);
    Ringbuffer_add(reverb->fb2, x);
    Ringbuffer_add(reverb->fb3, x);
    Ringbuffer_add(reverb->fb4, x);
    Ringbuffer_add(reverb->fb5, x);
    buf[i] = q16_16_multiply(Q16_16_8, x);
  }
}

void Reverb_free(Reverb *reverb) {
  if (reverb != NULL) {
    Ringbuffer_free(reverb->fb0);
    Ringbuffer_free(reverb->fb1);
    Ringbuffer_free(reverb->fb2);
    Ringbuffer_free(reverb->fb3);
    Ringbuffer_free(reverb->fb4);
    Ringbuffer_free(reverb->fb5);
    free(reverb);
  }
}

#endif