// SPDX-License-Identifier: CC0-1.0

#include <stdio.h>

#include "audio.h"
#include "mm_types.h"

mm_word lopped_names[] = {0, 0, 0, 0, 0, 0};
u16 looped_lengths[] = {0, 0, 0, 0, 0, 0};
u16 looped_remaining[] = {0, 0, 0, 0, 0, 0};
u8 looped_volumes[] = {0, 0, 0, 0, 0, 0};
mm_word looped_handles[] = {0, 0, 0, 0, 0, 0};
u8 current_size = 0;

typedef struct WAVHeader {
  // "RIFF" chunk descriptor
  uint32_t chunkID;
  uint32_t chunkSize;
  uint32_t format;
  // "fmt" subchunk
  uint32_t subchunk1ID;
  uint32_t subchunk1Size;
  uint16_t audioFormat;
  uint16_t numChannels;
  uint32_t sampleRate;
  uint32_t byteRate;
  uint16_t blockAlign;
  uint16_t bitsPerSample;
  // "data" subchunk
  uint32_t subchunk2ID;
  uint32_t subchunk2Size;
} WAVHeader_t;

FILE *wavFile = NULL;

char stream_buffer[BUFFER_LENGTH];
int stream_buffer_in;
int stream_buffer_out;

mm_word streamingCallback(mm_word length, mm_addr dest,
                          mm_stream_formats format) {
  size_t multiplier = 0;

  if (format == MM_STREAM_8BIT_MONO)
    multiplier = 1;
  else if (format == MM_STREAM_8BIT_STEREO)
    multiplier = 2;
  else if (format == MM_STREAM_16BIT_MONO)
    multiplier = 2;
  else if (format == MM_STREAM_16BIT_STEREO)
    multiplier = 4;

  size_t size = length * multiplier;

  size_t bytes_until_end = BUFFER_LENGTH - stream_buffer_out;

  if (bytes_until_end > size) {
    char *src_ = &stream_buffer[stream_buffer_out];

    memcpy(dest, src_, size);
    stream_buffer_out += size;
  } else {
    char *src_ = &stream_buffer[stream_buffer_out];
    char *dst_ = dest;

    memcpy(dst_, src_, bytes_until_end);
    dst_ += bytes_until_end;
    size -= bytes_until_end;

    src_ = &stream_buffer[0];
    memcpy(dst_, src_, size);
    stream_buffer_out = size;
  }

  return length;
}

// This reads bytes from wavFile into the provided buffer. If the end of the
// file is reached, it starts from the start again.
void readFile(char *buffer, size_t size) {
  while (size > 0) {
    int res = fread(buffer, 1, size, wavFile);
    size -= res;
    buffer += res;

    if (feof(wavFile)) {
      // Loop back when song ends
      fseek(wavFile, sizeof(WAVHeader_t), SEEK_SET);
      res = fread(buffer, 1, size, wavFile);
      size -= res;
      buffer += res;

      printf("Restarting...\n");
    }
  }
}

void streamingFillBuffer(bool force_fill) {
  if (!force_fill) {
    if (stream_buffer_in == stream_buffer_out)
      return;
  }

  if (stream_buffer_in < stream_buffer_out) {
    size_t size = stream_buffer_out - stream_buffer_in;
    readFile(&stream_buffer[stream_buffer_in], size);
    stream_buffer_in += size;
  } else {
    size_t size = BUFFER_LENGTH - stream_buffer_in;
    readFile(&stream_buffer[stream_buffer_in], size);
    stream_buffer_in = 0;

    size = stream_buffer_out - stream_buffer_in;
    readFile(&stream_buffer[stream_buffer_in], size);
    stream_buffer_in += size;
  }

  if (stream_buffer_in >= BUFFER_LENGTH)
    stream_buffer_in -= BUFFER_LENGTH;
}

int checkWAVHeader(const WAVHeader_t header) {
  if (header.chunkID != RIFF_ID) {
    printf("Wrong RIFF_ID %lx\n", header.chunkID);
    return 1;
  }

  if (header.format != WAVE_ID) {
    printf("Wrong WAVE_ID %lx\n", header.format);
    return 1;
  }

  /* if (header.subchunk1ID != FMT_ID) {
    printf("Wrong FMT_ID %lx\n", header.subchunk1ID);
    return 1;
  }

  if (header.subchunk2ID != DATA_ID) {
    printf("Wrong Subchunk2ID %lx\n", header.subchunk2ID);
    return 1;
  } */

  return 0;
}

mm_stream_formats getMMStreamType(uint16_t numChannels,
                                  uint16_t bitsPerSample) {
  if (numChannels == 1) {
    if (bitsPerSample == 8)
      return MM_STREAM_8BIT_MONO;
    else
      return MM_STREAM_16BIT_MONO;
  } else if (numChannels == 2) {
    if (bitsPerSample == 8)
      return MM_STREAM_8BIT_STEREO;
    else
      return MM_STREAM_16BIT_STEREO;
  }
  return MM_STREAM_8BIT_MONO;
}

void waitForever(void) {
  while (1)
    swiWaitForVBlank();
}

void audio_init_wav(char *path) {
  wavFile = fopen(path, "rb");
  if (wavFile == NULL) {
    perror("fopen");
    waitForever();
  }

  WAVHeader_t wavHeader = {0};
  if (fread(&wavHeader, 1, sizeof(WAVHeader_t), wavFile) !=
      sizeof(WAVHeader_t)) {
    perror("fread");
    waitForever();
  }
  if (checkWAVHeader(wavHeader) != 0) {
    printf("WAV file header is corrupt!\n");
    waitForever();
  }

  // Fill the buffer before we start doing anything
  streamingFillBuffer(true);

  // We are not using a soundbank so we need to manually initialize
  // mm_ds_system.
  mmInitDefault("nitro:/soundbank.bin");
  mmSelectMode(MM_MODE_B);

  // Open the stream
  mm_stream stream = {
      .sampling_rate = wavHeader.sampleRate,
      .buffer_length = 2048,
      .callback = streamingCallback,
      .format = getMMStreamType(wavHeader.numChannels, wavHeader.bitsPerSample),
      .timer = MM_TIMER0,
      .manual = false,
  };
  mmStreamOpen(&stream);
}

void audio_update_wav(void) { streamingFillBuffer(false); }

void audio_close_wav(void) {
  mmStreamClose();

  if (fclose(wavFile) != 0) {
    perror("fclose");
    waitForever();
  }
}

void audio_init_SB(void) {
  mmInitDefault("nitro:/soundbank.bin");
  soundEnable();
}
void audio_play_sfx(mm_word sample_name, bool loop, u16 length, u8 volume) {
  mmLoadEffect(sample_name);
  mm_word handle = mmEffect(sample_name);
  if (handle == MM_SFXHAND_INVALID) {
    printf("Failed to play sample");
    waitForever();
  }
  if (loop) {
    lopped_names[current_size] = sample_name;
    looped_lengths[current_size] = length;
    looped_remaining[current_size] = length;
    looped_volumes[current_size] = volume;
    looped_handles[current_size] = handle;
    current_size++;
  } else {
    mmEffectVolume(handle, volume);
  }
}

void audio_update_loops(void) {
  for (u8 i = 0; i < current_size; ++i) {
    if (looped_remaining[i] > 0) {
      looped_remaining[i]--;
      if (looped_remaining[i] <= 0) {
        mm_word handle = mmEffect(lopped_names[i]);
        looped_remaining[i] = looped_lengths[i];
        looped_handles[i] = handle;
        mmEffectVolume(handle, looped_volumes[i]);
      }
    }
  }
}

void audio_set_looped_volume(mm_word sample_name, u8 volume) {
  for (u8 i = 0; i < current_size; ++i) {
    if (lopped_names[i] == sample_name) {
      looped_volumes[i] = volume;
      mmEffectVolume(looped_handles[i], volume);
      return;
    }
  }
}

void audio_stop_looped_sfx(mm_word sample_name) {
  int found_index = -1;
  for (u8 i = 0; i < current_size; ++i) {
    if (lopped_names[i] == sample_name) {
      found_index = i;
      lopped_names[i] = 0;
      looped_lengths[i] = 0;
      looped_remaining[i] = 0;
      looped_volumes[i] = 0;
      looped_handles[i] = 0;
      break;
    }
  }
  mmUnloadEffect(sample_name);
  if (found_index != -1) {
    if (found_index < current_size - 1) {
      lopped_names[found_index] = lopped_names[current_size - 1];
      looped_lengths[found_index] = looped_lengths[current_size - 1];
      looped_remaining[found_index] = looped_remaining[current_size - 1];
      looped_volumes[found_index] = looped_volumes[current_size - 1];
      looped_handles[found_index] = looped_handles[current_size - 1];
    }
    current_size--;
  }
}

void audio_stop_all_sfx(void) {
  mmEffectCancelAll();
  current_size = 0;
}

void audio_unload_all_sfx(void) {
  audio_stop_all_sfx();
  for (u8 i = 0; i < MSL_BANKSIZE; ++i) {
    mmUnloadEffect(i);
  }
}
