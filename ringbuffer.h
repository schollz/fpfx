#ifndef RINGBUFFER_LIB_H
#define RINGBUFFER_LIB_H

typedef struct Ringbuffer {
  unsigned int nr_samples;
  int32_t* samples;
  unsigned int pos;
} Ringbuffer;

Ringbuffer* Ringbuffer_malloc(unsigned int nr_samples) {
  Ringbuffer* fb = (Ringbuffer*)malloc(sizeof(Ringbuffer));
  if (fb == NULL) {
    return NULL;
  }
  fb->nr_samples = nr_samples;
  fb->samples = (int32_t*)malloc(nr_samples * sizeof(int32_t));
  if (fb->samples == NULL) {
    free(fb);
    return NULL;
  }

  // Initialize the allocated memory to zero
  memset(fb->samples, 0, nr_samples * sizeof(int32_t));

  fb->pos = 0;
  return fb;
}

void Ringbuffer_free(Ringbuffer* fb) {
  if (fb != NULL) {
    free(fb->samples);
    free(fb);
  }
}

int32_t Ringbuffer_get(const Ringbuffer* fb) { return fb->samples[fb->pos]; }

void Ringbuffer_add(Ringbuffer* fb, int32_t sample) {
  while (fb->pos > fb->nr_samples) {
    fb->pos -= fb->nr_samples;
  }
  fb->samples[fb->pos] = sample;

  /* If we reach the end of the buffer, wrap around */
  if (++fb->pos == fb->nr_samples) fb->pos = 0;
}

#endif