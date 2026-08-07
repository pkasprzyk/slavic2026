// SPDX-License-Identifier: CC0-1.0

#include <maxmod9.h>
#include <soundbank.h>

#include "audio.h"

void audio_init(void)
{
    mmInitDefault("nitro:/soundbank.bin");
    mmLoad(MOD_JOINT_PEOPLE);
    mmStart(MOD_JOINT_PEOPLE, MM_PLAY_LOOP);
}
