// SPDX-License-Identifier: CC0-1.0

#include <stdio.h>
#include <time.h>

#include <filesystem.h>
#include <maxmod9.h>
#include <nds.h>
#include <soundbank.h>

#include <nf_lib.h>

#include "level.h"
#include "mech.h"

#include <nds/arm9/dldi.h>

#define DATA_ID 0x61746164
#define FMT_ID 0x20746d66
#define RIFF_ID 0x46464952
#define WAVE_ID 0x45564157

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

#define BUFFER_LENGTH 16384

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

  if (header.subchunk1ID != FMT_ID) {
    printf("Wrong FMT_ID %lx\n", header.subchunk1ID);
    return 1;
  }

  if (header.subchunk2ID != DATA_ID) {
    printf("Wrong Subchunk2ID %lx\n", header.subchunk2ID);
    return 1;
  }

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

int main(void) {
  NF_Set2D(0, 0);
  NF_Set2D(1, 0);
  consoleDemoInit();
  printf("\n Chilling Mech\n\n NitroFS init...\n");
  swiWaitForVBlank();

  if (!nitroFSInit(NULL)) {
    perror("nitroFSInit()");
    while (1)
      swiWaitForVBlank();
  }
  NF_SetRootFolder("NITROFS");

  NF_Set2D(0, 0);
  NF_Set2D(1, 0);

  NF_InitTiledBgBuffers();
  NF_InitTiledBgSys(0);
  NF_InitTiledBgSys(1);

  NF_InitTextSys(0);
  NF_InitSpriteBuffers();
  NF_InitSpriteSys(1);

  level_init();
  mech_init();

  NF_LoadTextFont("fnt/default", "normal", 256, 256, 0);
  NF_CreateTextLayer(0, 2, 0, "normal");
  NF_WriteText(0, 2, 2, 2, "CHILLING MECH");
  NF_WriteText(0, 2, 2, 4, "D-Pad moves the mech");
  NF_UpdateTextLayers();

  srand(time(NULL));

  wavFile = fopen("nitro:/audio/AuldLangSyne.wav", "rb");

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

  mm_ds_system mmSys = {.mod_count = 0,
                        .samp_count = 0,
                        .mem_bank = 0,
                        .fifo_channel = FIFO_MAXMOD};
  mmInit(&mmSys);

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
  // mmInitDefault("nitro:/soundbank.bin");
  // mmLoad(MOD_JOINT_PEOPLE);

  // mmStart(MOD_JOINT_PEOPLE, MM_PLAY_LOOP);

  while (1) {
    scanKeys();

    mech_update();

    NF_SpriteOamSet(1);

    swiWaitForVBlank();
    streamingFillBuffer(false);

    oamUpdate(&oamSub);
  }

  // mmStop();
  // soundDisable();

  mmStreamClose();

  if (fclose(wavFile) != 0) {
    perror("fclose");
    waitForever();
  }

  soundDisable();

  return 0;
}
