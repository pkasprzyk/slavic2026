// SPDX-License-Identifier: CC0-1.0

#include <filesystem.h>
#include <nds.h>
#include <nf_lib.h>
#include <stdlib.h>

#include "dswifi9.h"
#include "game.h"
#include "ids.h"

const u32 GAME_ID = 0xFF00FF0F;
bool found_other = false;
const s16 found_lifetime_max = 5 * 60;
s16 found_lifetime = found_lifetime_max;
const s16 timer_to_swap_min = 60;
const s16 timer_to_swap_max = 3 * 60;
s16 timer_to_swap = 0;
u8 network_state = 0;      // 1-host, 2-client, 0-none
u8 network_last_state = 0; // 1-host, 2-client, 0-none

void enterHost() {
  timer_to_swap =
      timer_to_swap_min + rand() % (timer_to_swap_max - timer_to_swap_min);
  Wifi_IdleMode();
  Wifi_MultiplayerHostMode(2, 1, 1);
  network_state = 1; // Set to host mode
  network_last_state = 1;
}

void enterClient() {
  timer_to_swap =
      timer_to_swap_min + rand() % (timer_to_swap_max - timer_to_swap_min);
  Wifi_IdleMode();
  Wifi_MultiplayerClientMode(1);
  network_state = 2; // Set to client mode
  network_last_state = 2;
}

void updateNetworkState() {
  if (found_other) {
    found_lifetime--;
    if (found_lifetime <= 0) {
      found_other = false;
      // found_lifetime = found_lifetime_max;
    }
  }
  if (timer_to_swap > 0) {
    timer_to_swap--;
  } else {
    if (network_last_state == 1) {
      enterClient();
    } else if (network_last_state == 2) {
      enterHost();
    }
  }
  if (network_state == 0)
    return;
  if (network_state == 1) {
    if (!Wifi_LibraryModeReady())
      return;

    Wifi_SetChannel(7);
    Wifi_MultiplayerAllowNewClients(true);

    Wifi_BeaconStart("RHCB", GAME_ID);
    network_state = 0; // Reset to none after starting host
    NF_WriteText(SCR_CHAMBER, LAYER_CHAMBER_TEXT, 2, 2, "Host started!");
    NF_UpdateTextLayers();
    /* consoleDemoInit();
    printf("Host mode started, waiting for clients...\n");
    while (1) {
      swiWaitForVBlank();
    } */
  } else if (network_state == 2) {
    if (!Wifi_LibraryModeReady())
      return;
    Wifi_ScanMode();
    int count = Wifi_GetNumAP();
    for (int i = 0; i < count; i++) {
      NF_WriteText(SCR_CHAMBER, LAYER_CHAMBER_TEXT, 2, 2, "Found some AP!");
      NF_UpdateTextLayers();
      Wifi_AccessPoint ap;
      Wifi_GetAPData(i, &ap);
      /* consoleDemoInit();
      printf("found %d APs, gameID: %u\n", count, ap.nintendo.game_id);
      while (1) {
        swiWaitForVBlank();
      } */
      if (ap.nintendo.game_id != GAME_ID) {
        continue;
      } else {
        found_other = true;
        found_lifetime = found_lifetime_max;
        network_state = 0;
        /* consoleDemoInit();
        printf("Found other DS with game ID");
        while (1) {
          swiWaitForVBlank();
        } */
      }
    }
  }
}

int main(void) {
  Wifi_InitDefault(INIT_ONLY | WIFI_LOCAL_ONLY);

  // enterHost();
  //  enterClient();
  int mode = rand() % 2;
  if (mode == 0) {
    enterHost();
  } else {
    enterClient();
  }

  game_init();
  while (1) {
    updateNetworkState();
    if (found_other) {
      NF_WriteText(SCR_CHAMBER, LAYER_CHAMBER_TEXT, 2, 2, "Found other DS!");
      NF_UpdateTextLayers();
    } else {
      // NF_WriteText(SCR_CHAMBER, LAYER_CHAMBER_TEXT, 2, 2, " ");
      // NF_UpdateTextLayers();
    }
    scanKeys();
    game_update();
    NF_SpriteOamSet(SCR_WORLD);
    NF_SpriteOamSet(SCR_CHAMBER);
    swiWaitForVBlank();
    oamUpdate(&oamMain);
    oamUpdate(&oamSub);
  }

  game_deinit();
}
