#ifndef TapeDelay_LIB
#define TapeDelay_LIB 1

#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#include "fixedpoint.h"

typedef struct TapeDelay {
  int32_t buffer[44100];  // Fixed circular buffer of 44100 samples
  size_t buffer_size;     // Size of the circular buffer
  size_t write_index;     // Current write index
  float delay_time;       // Delay time in samples (can be fractional)
  int32_t feedback;       // Feedback amount
  unsigned int oversampling_factor;  // Oversampling factor
} TapeDelay;

TapeDelay *TapeDelay_malloc(float feedback, float delay_time) {
  TapeDelay *tapeDelay = (TapeDelay *)malloc(sizeof(TapeDelay));
  if (tapeDelay == NULL) {
    return NULL;
  }

  tapeDelay->feedback = q16_16_float_to_fp(feedback);
  tapeDelay->delay_time = delay_time;
  tapeDelay->oversampling_factor = 8;  // Hardcoded to 8x oversampling
  tapeDelay->buffer_size = 44100;      // Fixed buffer size
  tapeDelay->write_index = 0;

  // Initialize the buffer to zero
  for (size_t i = 0; i < tapeDelay->buffer_size; i++) {
    tapeDelay->buffer[i] = 0;
  }

  return tapeDelay;
}

void TapeDelay_set_feedback(TapeDelay *tapeDelay, float feedback) {
  tapeDelay->feedback = q16_16_float_to_fp(feedback);
}

void TapeDelay_set_delay_time(TapeDelay *tapeDelay, float delay_time) {
  tapeDelay->delay_time = delay_time;
}

// Linear interpolation helper
static inline int32_t linear_interpolation(int32_t y1, int32_t y2, float frac) {
  return y1 + (int32_t)((y2 - y1) * frac);
}

void TapeDelay_process(TapeDelay *tapeDelay, int32_t *buf,
                       unsigned int nr_samples) {
  unsigned int oversampling_factor = tapeDelay->oversampling_factor;
  unsigned int oversampled_samples = nr_samples * oversampling_factor;

  // Temporary oversampled buffer
  int32_t *oversampled_buf =
      (int32_t *)malloc(oversampled_samples * sizeof(int32_t));
  if (!oversampled_buf) {
    return;  // Allocation failed
  }

  // Oversample the input buffer (simple zero-stuffing)
  for (unsigned int i = 0; i < nr_samples; i++) {
    oversampled_buf[i * oversampling_factor] = buf[i];
    for (unsigned int j = 1; j < oversampling_factor; j++) {
      oversampled_buf[i * oversampling_factor + j] = 0;  // Zero-stuffing
    }
  }

  for (unsigned int i = 0; i < oversampled_samples; i++) {
    // Calculate fractional read index
    float fractional_read_index = (float)tapeDelay->write_index -
                                  tapeDelay->delay_time * oversampling_factor;
    if (fractional_read_index < 0) {
      fractional_read_index += tapeDelay->buffer_size;
    }

    size_t base_read_index =
        (size_t)fractional_read_index % tapeDelay->buffer_size;
    size_t next_read_index = (base_read_index + 1) % tapeDelay->buffer_size;
    float frac = fractional_read_index - (size_t)fractional_read_index;

    // Read the delayed sample with interpolation
    int32_t delayed_sample =
        linear_interpolation(tapeDelay->buffer[base_read_index],
                             tapeDelay->buffer[next_read_index], frac);

    // Add feedback to the current sample and write it to the buffer
    int32_t input_sample = oversampled_buf[i];
    int32_t processed_sample =
        input_sample + q16_16_multiply(tapeDelay->feedback, delayed_sample);

    tapeDelay->buffer[tapeDelay->write_index] = processed_sample;

    // Update write index
    tapeDelay->write_index =
        (tapeDelay->write_index + 1) % tapeDelay->buffer_size;

    // Store the processed sample back into the oversampled buffer
    oversampled_buf[i] = processed_sample;
  }

  // Downsample back to the original buffer
  for (unsigned int i = 0; i < nr_samples; i++) {
    buf[i] = oversampled_buf[i * oversampling_factor];
  }

  free(oversampled_buf);
}

void TapeDelay_free(TapeDelay *tapeDelay) {
  if (tapeDelay != NULL) {
    free(tapeDelay);
  }
}

#endif
