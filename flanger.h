#ifndef Flanger_LIB
#define Flanger_LIB 1

#include <math.h>

#include "fixedpoint.h"

typedef struct Flanger {
  int32_t delayLine[4098];  // Fixed-size buffer for the delay line
  unsigned int maxDelay;    // Maximum delay in samples
  unsigned int lfoIndex;    // Current index for the LFO
  unsigned int lfoRate;     // Rate of LFO
  float depth;              // Depth of modulation
  float feedback;           // Feedback amount
} Flanger;
Flanger *Flanger_malloc(float feedback) {
  Flanger *self = (Flanger *)malloc(sizeof(Flanger));
  if (self == NULL) {
    // Handle memory allocation failure
    return NULL;
  }

  // Initialize the Flanger structure
  self->maxDelay = 400;       // Adjust as needed, but should not exceed 2048
  self->lfoRate = 512;        // Example LFO rate, adjust as needed
  self->depth = 0.5f;         // Example depth, adjust as needed
  self->feedback = feedback;  // Set feedback

  // Initialize delayLine with zeros
  memset(self->delayLine, 0, sizeof(self->delayLine));

  return self;
}

void Flanger_process(Flanger *self, int32_t *buf, unsigned int nr_samples) {
  for (unsigned int i = 0; i < nr_samples; i++) {
    // Calculate current delay using LFO
    float lfoValue =
        (sin(2 * M_PI * self->lfoIndex / (float)self->lfoRate) + 1) / 2;
    unsigned int currentDelay = lfoValue * self->depth * self->maxDelay;
    self->lfoIndex = (self->lfoIndex + 1) % self->lfoRate;

    // Calculate read index for the delay line
    int readIndex = i - currentDelay;
    if (readIndex < 0) {
      readIndex += self->maxDelay;  // Wrap around if negative
    }

    // Read from delay line
    int32_t delayedSample = self->delayLine[readIndex % self->maxDelay];

    // Apply feedback
    self->delayLine[i % self->maxDelay] =
        buf[i] + (int32_t)(self->feedback * delayedSample);

    // Mix delayed signal with the original signal
    buf[i] = (buf[i] + delayedSample) / 2;
  }
}
#endif