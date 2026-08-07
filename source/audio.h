// SPDX-License-Identifier: CC0-1.0

#ifndef AUDIO_H
#define AUDIO_H

#define DATA_ID 0x61746164
#define FMT_ID 0x20746d66
#define RIFF_ID 0x46464952
#define WAVE_ID 0x45564157

#define BUFFER_LENGTH 16384

void audio_init_wav(char *path);
void audio_update(void);
void audio_close_wav(void);

#endif
