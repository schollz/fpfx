#include <stdio.h>

#define undenormalise(sample) \
  if (((*(unsigned int *)&(sample)) & 0x7f800000) == 0) (sample) = 0.0f

typedef struct FV_AllPass {
  float feedback;
  float *buffer;
  int bufsize;
  int bufidx;
} FV_AllPass;

// Function implementations
void FV_AllPass_init(FV_AllPass *self) {
  self->buffer = NULL;
  self->feedback = 0.5;
  self->bufsize = 0;
  self->bufidx = 0;
}

void FV_AllPass_setbuffer(FV_AllPass *self, float *buf, int size) {
  if (self->buffer) {
    free(self->buffer);
  }
  self->buffer = buf;
  self->bufsize = size;
}

float FV_AllPass_process(FV_AllPass *self, float input) {
  float output;
  float bufout;

  bufout = self->buffer[self->bufidx];
  undenormalise(bufout);

  output = -input + bufout;
  self->buffer[self->bufidx] = input + (bufout * self->feedback);

  if (++(self->bufidx) >= self->bufsize) self->bufidx = 0;

  return output;
}

void FV_AllPass_mute(FV_AllPass *self) {
  for (int i = 0; i < self->bufsize; i++) self->buffer[i] = 0;
}

void FV_AllPass_setfeedback(FV_AllPass *self, float val) {
  self->feedback = val;
}

float FV_AllPass_getfeedback(FV_AllPass *self) { return self->feedback; }

// comb filter

typedef struct FV_Comb {
  float feedback;
  float filterstore;
  float damp1;
  float damp2;
  float *buffer;
  int bufsize;
  int bufidx;
} FV_Comb;

void FV_Comb_init(FV_Comb *self) {
  self->feedback = 0.5f;
  self->filterstore = 0.0f;
  self->damp1 = 0.0f;
  self->damp2 = 0.0f;
  self->buffer = NULL;
  self->bufsize = 0;
  self->bufidx = 0;
}

void FV_Comb_free(FV_Comb *self) {
  if (self->buffer) {
    free(self->buffer);
  }
  free(self);
}

static inline float FV_Comb_process(FV_Comb *self, float input) {
  float output = self->buffer[self->bufidx];
  undenormalise(output);
  self->filterstore =
      (output * self->damp2) + (self->filterstore * self->damp1);
  undenormalise(self->filterstore);
  self->buffer[self->bufidx] = input + (self->filterstore * self->feedback);
  if (++self->bufidx >= self->bufsize) self->bufidx = 0;
  return output;
}

void FV_Comb_mute(FV_Comb *self) {
  for (int i = 0; i < self->bufsize; i++) self->buffer[i] = 0;
}

void FV_Comb_setbuffer(FV_Comb *self, float *buf, int size) {
  if (self->buffer) {
    free(self->buffer);
  }
  self->buffer = buf;
  self->bufsize = size;
}

void FV_Comb_setfeedback(FV_Comb *self, float val) { self->feedback = val; }

void FV_Comb_setdamp(FV_Comb *self, float val) {
  self->damp1 = val;
  self->damp2 = 1 - val;
}

float FV_Comb_getfeedback(FV_Comb *self) { return self->feedback; }

// tuning
#define FV_NUMCOMBS 8
#define FV_NUMALLPASSES 4
#define FV_MUTED 0.0f
#define FV_FIXEDGAIN 0.015f
#define FV_SCALEWET 3.0f
#define FV_SCALEDRY 2.0f
#define FV_SCALEDAMP 0.4f
#define FV_SCALEROOM 0.28f
#define FV_OFFSETROOM 0.7f
#define FV_INITIALROOM 0.8f
#define FV_INITIALDAMP 0.15f
#define FV_INITIALWET 0.7f
#define FV_INITIALDRY 0.3f
#define FV_INITIALWIDTH 1.0f
#define FV_INITIALMODE 0.0f
#define FV_FREEZEMODE 0.5f
#define FV_STEREOSPREAD 23
#define FV_COMBTUNINGL1 1116
#define FV_COMBTUNINGR1 (1116 + FV_STEREOSPREAD)
#define FV_COMBTUNINGL2 1188
#define FV_COMBTUNINGR2 (1188 + FV_STEREOSPREAD)
#define FV_COMBTUNINGL3 1277
#define FV_COMBTUNINGR3 (1277 + FV_STEREOSPREAD)
#define FV_COMBTUNINGL4 1356
#define FV_COMBTUNINGR4 (1356 + FV_STEREOSPREAD)
#define FV_COMBTUNINGL5 1422
#define FV_COMBTUNINGR5 (1422 + FV_STEREOSPREAD)
#define FV_COMBTUNINGL6 1491
#define FV_COMBTUNINGR6 (1491 + FV_STEREOSPREAD)
#define FV_COMBTUNINGL7 1557
#define FV_COMBTUNINGR7 (1557 + FV_STEREOSPREAD)
#define FV_COMBTUNINGL8 1617
#define FV_COMBTUNINGR8 (1617 + FV_STEREOSPREAD)
#define FV_ALLPASSTUNINGL1 556
#define FV_ALLPASSTUNINGR1 (556 + FV_STEREOSPREAD)
#define FV_ALLPASSTUNINGL2 441
#define FV_ALLPASSTUNINGR2 (441 + FV_STEREOSPREAD)
#define FV_ALLPASSTUNINGL3 341
#define FV_ALLPASSTUNINGR3 (341 + FV_STEREOSPREAD)
#define FV_ALLPASSTUNINGL4 225
#define FV_ALLPASSTUNINGR4 (225 + FV_STEREOSPREAD)

typedef struct FV_Reverb {
  float gain;
  float roomsize, roomsize1;
  float damp, damp1;
  float wet, wet1, wet2;
  float dry;
  float width;
  float mode;

  // Comb filters
  FV_Comb combL[FV_NUMCOMBS];
  FV_Comb combR[FV_NUMCOMBS];

  // Allpass filters
  FV_AllPass allpassL[FV_NUMALLPASSES];
  FV_AllPass allpassR[FV_NUMALLPASSES];

  // Buffers for the combs
  float bufcombL1[FV_COMBTUNINGL1], bufcombR1[FV_COMBTUNINGR1];
  float bufcombL2[FV_COMBTUNINGL2], bufcombR2[FV_COMBTUNINGR2];
  float bufcombL3[FV_COMBTUNINGL3], bufcombR3[FV_COMBTUNINGR3];
  float bufcombL4[FV_COMBTUNINGL4], bufcombR4[FV_COMBTUNINGR4];
  float bufcombL5[FV_COMBTUNINGL5], bufcombR5[FV_COMBTUNINGR5];
  float bufcombL6[FV_COMBTUNINGL6], bufcombR6[FV_COMBTUNINGR6];
  float bufcombL7[FV_COMBTUNINGL7], bufcombR7[FV_COMBTUNINGR7];
  float bufcombL8[FV_COMBTUNINGL8], bufcombR8[FV_COMBTUNINGR8];

  // Buffers for the allpasses
  float bufallpassL1[FV_ALLPASSTUNINGL1], bufallpassR1[FV_ALLPASSTUNINGR1];
  float bufallpassL2[FV_ALLPASSTUNINGL2], bufallpassR2[FV_ALLPASSTUNINGR2];
  float bufallpassL3[FV_ALLPASSTUNINGL3], bufallpassR3[FV_ALLPASSTUNINGR3];
  float bufallpassL4[FV_ALLPASSTUNINGL4], bufallpassR4[FV_ALLPASSTUNINGR4];
} FV_Reverb;

float FV_Reverb_getmode(FV_Reverb *self) {
  if (self->mode >= FV_FREEZEMODE)
    return 1.0f;
  else
    return 0.0f;
}

void FV_Reverb_mute(FV_Reverb *self) {
  if (FV_Reverb_getmode(self) >= FV_FREEZEMODE) {
    return;
  }
  for (int i = 0; i < FV_NUMCOMBS; i++) {
    FV_Comb_mute(&self->combL[i]);
    FV_Comb_mute(&self->combR[i]);
  }
  for (int i = 0; i < FV_NUMALLPASSES; i++) {
    FV_AllPass_mute(&self->allpassL[i]);
    FV_AllPass_mute(&self->allpassR[i]);
  }
}

void FV_Reverb_update(FV_Reverb *self) {
  int i;

  self->wet1 = self->wet * (self->width / 2 + 0.5);
  self->wet2 = self->wet * ((1 - self->width) / 2);

  //   if (self->mode >= FV_FREEZEMODE) {
  //     self->roomsize1 = 1;
  //     self->damp1 = 0;
  //     self->gain = FV_MUTED;
  //   } else {
  self->roomsize1 = self->roomsize;
  self->damp1 = self->damp;
  self->gain = FV_FIXEDGAIN;
  //}

  for (i = 0; i < FV_NUMCOMBS; i++) {
    FV_Comb_setfeedback(&self->combL[i], self->roomsize1);
    FV_Comb_setfeedback(&self->combR[i], self->roomsize1);
    FV_Comb_setdamp(&self->combL[i], self->damp1);
    FV_Comb_setdamp(&self->combR[i], self->damp1);
  }
}

void FV_Reverb_setroomsize(FV_Reverb *self, float value) {
  self->roomsize = value * FV_SCALEROOM + FV_OFFSETROOM;
}

void FV_Reverb_setdamp(FV_Reverb *self, float value) {
  self->damp = value * FV_SCALEDAMP;
}

void FV_Reverb_setwet(FV_Reverb *self, float value) {
  self->wet = value * FV_SCALEWET;
}

void FV_Reverb_setdry(FV_Reverb *self, float value) {
  self->dry = value * FV_SCALEDRY;
}

void FV_Reverb_setwidth(FV_Reverb *self, float value) { self->width = value; }

void FV_Reverb_setmode(FV_Reverb *self, float value) { self->mode = value; }

void FV_Reverb_init(FV_Reverb *self) {
  for (int i = 0; i < FV_NUMCOMBS; i++) {
    FV_Comb_init(&self->combL[i]);
    FV_Comb_init(&self->combR[i]);
  }

  FV_Comb_setbuffer(&self->combL[0], self->bufcombL1, FV_COMBTUNINGL1);
  FV_Comb_setbuffer(&self->combR[0], self->bufcombR1, FV_COMBTUNINGR1);
  FV_Comb_setbuffer(&self->combL[1], self->bufcombL2, FV_COMBTUNINGL2);
  FV_Comb_setbuffer(&self->combR[1], self->bufcombR2, FV_COMBTUNINGR2);
  FV_Comb_setbuffer(&self->combL[2], self->bufcombL3, FV_COMBTUNINGL3);
  FV_Comb_setbuffer(&self->combR[2], self->bufcombR3, FV_COMBTUNINGR3);
  FV_Comb_setbuffer(&self->combL[3], self->bufcombL4, FV_COMBTUNINGL4);
  FV_Comb_setbuffer(&self->combR[3], self->bufcombR4, FV_COMBTUNINGR4);
  FV_Comb_setbuffer(&self->combL[4], self->bufcombL5, FV_COMBTUNINGL5);
  FV_Comb_setbuffer(&self->combR[4], self->bufcombR5, FV_COMBTUNINGR5);
  FV_Comb_setbuffer(&self->combL[5], self->bufcombL6, FV_COMBTUNINGL6);
  FV_Comb_setbuffer(&self->combR[5], self->bufcombR6, FV_COMBTUNINGR6);
  FV_Comb_setbuffer(&self->combL[6], self->bufcombL7, FV_COMBTUNINGL7);
  FV_Comb_setbuffer(&self->combR[6], self->bufcombR7, FV_COMBTUNINGR7);
  FV_Comb_setbuffer(&self->combL[7], self->bufcombL8, FV_COMBTUNINGL8);
  FV_Comb_setbuffer(&self->combR[7], self->bufcombR8, FV_COMBTUNINGR8);

  for (int i = 0; i < FV_NUMALLPASSES; i++) {
    FV_AllPass_init(&self->allpassL[i]);
    FV_AllPass_init(&self->allpassR[i]);
  }

  FV_AllPass_setbuffer(&self->allpassL[0], self->bufallpassL1,
                       FV_ALLPASSTUNINGL1);
  FV_AllPass_setbuffer(&self->allpassR[0], self->bufallpassR1,
                       FV_ALLPASSTUNINGR1);
  FV_AllPass_setbuffer(&self->allpassL[1], self->bufallpassL2,
                       FV_ALLPASSTUNINGL2);
  FV_AllPass_setbuffer(&self->allpassR[1], self->bufallpassR2,
                       FV_ALLPASSTUNINGR2);
  FV_AllPass_setbuffer(&self->allpassL[2], self->bufallpassL3,
                       FV_ALLPASSTUNINGL3);
  FV_AllPass_setbuffer(&self->allpassR[2], self->bufallpassR3,
                       FV_ALLPASSTUNINGR3);
  FV_AllPass_setbuffer(&self->allpassL[3], self->bufallpassL4,
                       FV_ALLPASSTUNINGL4);
  FV_AllPass_setbuffer(&self->allpassR[3], self->bufallpassR4,
                       FV_ALLPASSTUNINGR4);

  FV_Reverb_setroomsize(self, FV_INITIALROOM);
  FV_Reverb_setdamp(self, FV_INITIALDAMP);
  FV_Reverb_setwet(self, FV_INITIALWET);
  FV_Reverb_setdry(self, FV_INITIALDRY);
  FV_Reverb_setwidth(self, FV_INITIALWIDTH);
  FV_Reverb_setmode(self, FV_INITIALMODE);
  FV_Reverb_update(self);

  FV_Reverb_mute(self);
}

void FV_Reverb_process(FV_Reverb *self, int32_t *buf, unsigned int nr_samples) {
  float outL, outR, input, input_gained;
  for (int i = 0; i < nr_samples; i++) {
    outL = outR = 0;
    // convert int32_t to float
    input = (float)buf[i] / 32768.0f;
    input_gained = input * self->gain;

    // accumluate comb filters in parallel
    for (int j = 0; j < FV_NUMCOMBS; j++) {
      outL += FV_Comb_process(&self->combL[j], input_gained);
      outR += FV_Comb_process(&self->combR[j], input_gained);
    }

    // feed through allpasses in series
    for (int j = 0; j < FV_NUMALLPASSES; j++) {
      outL = FV_AllPass_process(&self->allpassL[j], outL);
      outR = FV_AllPass_process(&self->allpassR[j], outR);
    }

    // calculate output mixing with anything already there
    buf[i] = (int32_t)(32768 * (((input)*self->dry) + (outL * self->wet)));
  }
}
