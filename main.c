#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "bitcrush.h"
#include "delay.h"
#include "fixedpoint.h"
#include "reverb.h"

const int block_size = 8192;

#define DO_REVERB 0
#define DO_BITCRUSH 0
#define DO_DELAY 1

int msleep(long msec) {
  struct timespec ts;
  int res;

  if (msec < 0) {
    errno = EINVAL;
    return -1;
  }

  ts.tv_sec = msec / 1000;
  ts.tv_nsec = (msec % 1000) * 1000000;

  do {
    res = nanosleep(&ts, &ts);
  } while (res && errno == EINTR);

  return res;
}

int main(int argc, char *argv[]) {
  // Initialize random number generator
  srand(time(NULL));

#if DO_REVERB == 1
  Reverb *reverb = Reverb_malloc();
#endif
#if DO_DELAY == 1
  Delay *delay = Delay_malloc(0.6);
#endif
#if DO_BITCRUSH == 1
  Bitcrush *bitcrush = Bitcrush_malloc();
#endif

  while (true) {
    int16_t buf[block_size];
    ssize_t in = read(STDIN_FILENO, buf, sizeof(buf));
    if (in == -1) {
      /* Error */
      return 1;
    }
    if (in == 0) {
      /* EOF */
      break;
    }

    int32_t buf_fp[block_size];
    for (int i = 0; i < block_size; i++) {
      buf_fp[i] = q16_16_int16_to_fp(buf[i]);
    }

    // Delay_set_feedback(delay, (float)rand() / (2 * (float)RAND_MAX));
#if DO_DELAY == 1
    Delay_process(delay, buf_fp, block_size);
#endif
#if DO_REVERB == 1
    Reverb_process(reverb, buf_fp, block_size);
#endif
#if DO_BITCRUSH == 1
    Bitcrush_process(bitcrush, buf_fp, block_size);
#endif

    for (int i = 0; i < block_size; i++) {
      buf[i] = q16_16_fp_to_int16(buf_fp[i]);
    }

    write(STDOUT_FILENO, buf, in);

    // msleep(180);
  }

#if DO_REVERB == 1
  Reverb_free(reverb);
#endif
#if DO_DELAY == 1
  Delay_free(delay);
#endif
#if DO_BITCRUSH == 1
  Bitcrush_free(bitcrush);
#endif
  return 0;
}
