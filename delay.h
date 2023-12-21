#ifndef Delay_LIB
#define Delay_LIB 1

#include "fixedpoint.h"
#include "ringbuffer.h"

typedef struct Delay {
  Ringbuffer *fb0;
  int32_t feedback;
} Delay;

Delay *Delay_malloc(float feedback) {
  Delay *delay = (Delay *)malloc(sizeof(Delay));
  if (delay == NULL) {
    return NULL;
  }
  delay->feedback = q16_16_float_to_fp(feedback);
  delay->fb0 = Ringbuffer_malloc(13230 / 2);

  if (!delay->fb0) {
    Ringbuffer_free(delay->fb0);
    free(delay);
    return NULL;
  }

  return delay;
}

void Delay_set_feedback(Delay *delay, float feedback) {
  delay->feedback = q16_16_float_to_fp(feedback);
}

void Delay_process(Delay *delay, int32_t *buf, unsigned int nr_samples) {
  for (int i = 0; i < nr_samples; i++) {
    int32_t x = buf[i];
    x += q16_16_multiply(delay->feedback, Ringbuffer_get(delay->fb0));
    Ringbuffer_add(delay->fb0, x);
    buf[i] = x;
  }
}

void Delay_free(Delay *delay) {
  if (delay != NULL) {
    Ringbuffer_free(delay->fb0);
    free(delay);
  }
}

#endif