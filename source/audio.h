// SPDX-License-Identifier: CC0-1.0

#ifndef AUDIO_H
#define AUDIO_H

#define DATA_ID 0x61746164
#define FMT_ID 0x20746d66
#define RIFF_ID 0x46464952
#define WAVE_ID 0x45564157

#define BUFFER_LENGTH 16384
#define IGNORED_LEN 0
#define DEFAULT_VOLUME 255

#include <filesystem.h>
#include <maxmod9.h>
#include <nds.h>
#include <nds/arm9/dldi.h>
#include <soundbank.h>

void audio_init_SB(void);
void audio_init_wav(char *path);
void audio_update_wav(void);
void audio_close_wav(void);
void audio_play_sfx(mm_word sample_name, bool loop, u16 length, u8 volume);
void audio_update_loops(void);
void audio_stop_looped_sfx(mm_word sample_name);
void audio_set_looped_volume(mm_word sample_name, u8 volume);
void audio_stop_all_sfx(void);
void audio_unload_all_sfx(void);
void waitForever(void);

#endif
