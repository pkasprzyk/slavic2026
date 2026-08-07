// SPDX-License-Identifier: CC0-1.0

#include <nf_lib.h>

#include "level.h"

void level_init(void)
{
    NF_LoadTiledBg("bg/nfl", "nfl", 256, 256);
    NF_CreateTiledBg(1, 3, "nfl");
}
