#include <stdio.h>

#include <nds.h>
#include <nf_lib.h>

#include "audio.h"
#include "ids.h"
#include "reactor.h"
#include "sprites.h"

#define REACTOR_CORE_W 32
#define REACTOR_CORE_H 32

#define REACTOR_CORE_SPRITE 12

#define REACTOR_CORE_X 64
#define REACTOR_CORE_Y 64

#define REACTOR_MIN_TEMP 0
#define REACTOR_MAX_TEMP 200

#define BLOWING_THRESHOLD 10000
#define BLOWING_DECREASE_AMOUNT 20

// The sample rate used for the recording (samples per second)
#define SAMPLE_RATE 8000

#define BLOWING_FRAMES_REQUIRED 20

#define REACTOR_THRESHOLD_1 50
#define REACTOR_HEAT_2_SPEED_PENALTY 0
#define REACTOR_THRESHOLD_2 100
#define REACTOR_HEAT_3_SPEED_PENALTY 8
#define REACTOR_THRESHOLD_3 150
#define REACTOR_HEAT_4_SPEED_PENALTY 16
#define REACTOR_HEAT_4_WATER_LEAK 25

#define FIRE_HEAT_DISTANCE 36
#define FIRE_HEAT_MULTIPLIER 200

#define FIRE_HEAT_COOLDOWN 14

int blowing_counter = 0;
int reactor_temp;
int fire_heat_cooldown;
int heat_speed_penalty;
int water_leak_rate;

// This is the size of the temporary buffer that the ARM7 will use to record
// audio. When the callback is called, you will get a pointer to some address
// inside this buffer. It's your responsibility to copy the data out of this
// buffer to another buffer if you want to use that data.
//
// Note that this is a double buffer, so the size needs to be double the size of
// the time you want to record each callback. For example, if you want the
// callback to be called every 2 frames:
//
//     samples rate * number of channels * number of seconds
//
// The number of seconds is 1 / 30 or (2 / 60).
#define MICROPHONE_BUFFER_SIZE (SAMPLE_RATE * 2 / 30)

#define SPRITE_ID 0

static uint16_t temporary_buffer[MICROPHONE_BUFFER_SIZE];
static int cnt = 0;

#define BLOW_DEBUG 0
#define HEAT_DEBUG 0

#if BLOW_DEBUG
#define DEBUG_CALLBACKS_UPDATE_TEXT 60
static int debug_callbacks_cnt = DEBUG_CALLBACKS_UPDATE_TEXT;
static char blowing_debug_str[160];
#endif

void microphone_handler(void *completed_buffer, int length) {
  cnt = 0;
  s16 *wave_buf = memUncached(completed_buffer);

  int blowing_samples = 0;
  for (int i = 0; i < length; i++) {
    s32 sample = wave_buf[i];
    if (abs(sample) > BLOWING_THRESHOLD) {
      blowing_samples++;
    }
  }

#if BLOW_DEBUG
  if (++debug_callbacks_cnt > DEBUG_CALLBACKS_UPDATE_TEXT) {
    debug_callbacks_cnt = 0;
    s32 min = 0;
    s32 max = 0;
    for (int i = 0; i < length; i++) {
      s32 s = wave_buf[i];
      if (s < min)
        min = s;
      if (s > max)
        max = s;
    }
    sprintf(blowing_debug_str,
            "SAMPLES %08d\nBLOW SP %08d\n PCT: %03d\nMIN: %016ld\nMAX: %016ld",
            length, blowing_samples, blowing_samples * 100 / length, min, max);
  }
#endif

  if (blowing_samples < 0.4 * length) {
    // not blowing enough, reset the counter
    blowing_counter = 0;
    return;
  }

  blowing_counter += 1;
  // this function is called every 2 frames, we want to check every 10 frames
  if (blowing_counter * 2 >= BLOWING_FRAMES_REQUIRED) {
    reactor_decrease_temp(BLOWING_DECREASE_AMOUNT);
    blowing_counter = 0;
  }
}

void reactor_init(void) {
  reactor_temp = 0;
  blowing_counter = 0;
  fire_heat_cooldown = 0;
  heat_speed_penalty = 0;
  water_leak_rate = 0;

  soundEnable();

  // We need to ensure that the temporary buffer isn't cached so that the ARM7
  // can use it without issues. We also need to flush the recording and
  // playback buffers so that we can use DMA between them. We also need to
  // flush the playback buffer so that the audio hardware can see the
  // up-to-date values of the buffer always.
  DC_FlushAll();

  // The microphone (especially on the DS) requires about a second to get its
  // input levels to a valid baseline. Normally you could just discard the
  // first half second, but in this example we enable it at the beginning of
  // main() so that we can always draw the waveform.
  soundMicRecord(temporary_buffer, sizeof(temporary_buffer), MicFormat_12Bit,
                 SAMPLE_RATE, microphone_handler);
}

void reactor_restart(void) {}

static u16 last_frame = 0;

void UpdateReactorImg() {
  u16 frame = 0;
  if (reactor_temp < REACTOR_THRESHOLD_1) {
    frame = 0;
  } else if (reactor_temp < REACTOR_THRESHOLD_2) {
    frame = 1;
  } else if (reactor_temp < REACTOR_THRESHOLD_3) {
    frame = 2;
  } else {
    frame = 3;
  }

  if (frame == last_frame) {
    return;
  }
  last_frame = frame;

  mm_word sfx = 0;

  switch (frame) {
  case 0:
    sfx = SFX_SFX_HEAT_PHASE_1;
    break;
  case 1:
    sfx = SFX_SFX_HEAT_PHASE_2;
    break;
  case 2:
    sfx = SFX_SFX_HEAT_PHASE_3;
    break;
  case 3:
    sfx = SFX_SFX_HEAT_PHASE_4;
    break;
  default:
    break;
  }
  audio_play_sfx(sfx, false, IGNORED_LEN, 255);

  NF_DeleteTiledBg(SCR_CHAMBER, LAYER_CHAMBER_REACTOR);
  NF_UnloadTiledBg(REACTOR_BG_NAME);
  NF_LoadTiledBg(REACTOR_LEVEL_IMG_FILE[frame], REACTOR_BG_NAME, 256, 256);
  NF_CreateTiledBg(SCR_CHAMBER, LAYER_CHAMBER_REACTOR, REACTOR_BG_NAME);
}

void reactor_update(void) {
  /* char buffer[64];
  snprintf(buffer, sizeof(buffer), "TEMP: %03d", reactor_temp);
  NF_WriteText(SCR_CHAMBER, LAYER_CHAMBER_TEXT, 2, 6, buffer); */

#if BLOW_DEBUG
  NF_WriteText(SCR_CHAMBER, LAYER_CHAMBER_TEXT, 0, 8, blowing_debug_str);
#endif

  UpdateReactorImg();

  if (fire_heat_cooldown > 0) {
    fire_heat_cooldown--;
  }
}

void reactor_deinit(void) {
  // Turn off the microphone when you're done.
  soundMicOff();
}

void resolve_heat_penalties() {
  if (reactor_temp > REACTOR_THRESHOLD_3) {
    heat_speed_penalty = REACTOR_HEAT_4_SPEED_PENALTY;
    water_leak_rate = REACTOR_HEAT_4_WATER_LEAK;
  } else if (reactor_temp > REACTOR_THRESHOLD_2) {
    heat_speed_penalty = REACTOR_HEAT_3_SPEED_PENALTY;
    water_leak_rate = 0;
  } else if (reactor_temp > REACTOR_THRESHOLD_1) {
    heat_speed_penalty = REACTOR_HEAT_2_SPEED_PENALTY;
    water_leak_rate = 0;
  } else {
    heat_speed_penalty = 0;
    water_leak_rate = 0;
  }
}

void reactor_increase_temp(int amount) {
  reactor_temp += amount;
  if (reactor_temp > REACTOR_MAX_TEMP) {
    reactor_temp = REACTOR_MAX_TEMP;
  }
  resolve_heat_penalties();
}

void reactor_decrease_temp(int amount) {
  reactor_temp -= amount;
  if (reactor_temp < REACTOR_MIN_TEMP) {
    reactor_temp = REACTOR_MIN_TEMP;
  }
  resolve_heat_penalties();
}

void reactor_heat_from_fire(s32 distance_sq) {
  int heat_amount = 0;
  if (distance_sq < (FIRE_HEAT_DISTANCE * FIRE_HEAT_DISTANCE)) {
    // scales up to 10
    heat_amount = (FIRE_HEAT_DISTANCE * FIRE_HEAT_DISTANCE - distance_sq) /
                  FIRE_HEAT_MULTIPLIER;
    if (heat_amount == 0) {
      heat_amount = 1;
    }
    reactor_increase_temp(heat_amount);
    fire_heat_cooldown = FIRE_HEAT_COOLDOWN;
  }
#if HEAT_DEBUG
  char buffer[128];
  snprintf(buffer, sizeof(buffer), "HEAT: %03d  DIST: %010d COOLDN: %03d",
           heat_amount, distance_sq, fire_heat_cooldown);
  NF_WriteText(SCR_CHAMBER, LAYER_CHAMBER_TEXT, 0, 8, buffer);
#endif
}
