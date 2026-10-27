#ifndef EAGLE_AUDIO_H
#define EAGLE_AUDIO_H
void AudioInit(void);
void AudioEffect(unsigned char effect);
void AudioToggle(void);
extern volatile unsigned char music_enabled,music_scc,music_loops;
extern volatile unsigned int music_clock;
#define FX_SHOT 1
#define FX_PICKUP 2
#define FX_DOOR 3
#define FX_HIT 4
#define FX_KILL 5
#endif
