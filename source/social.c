#include "ids.h"
#include <nf_lib.h>
#include <stdlib.h>

#include "dswifi9.h"
#include "social.h"
#include "sprites.h"

#define SPRITE_ID GUARDIAN_ICON_OAM_ID

const u32 GAME_ID = 0xFF00FF0F;
int found_others = 0;
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
  if (found_others > 0) {
    found_lifetime--;
    if (found_lifetime <= 0) {
      found_others = 0;
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
    // NF_WriteText(SCR_CHAMBER, LAYER_CHAMBER_TEXT, 2, 2, "Host started!");
    // NF_UpdateTextLayers();
    /* consoleDemoInit();
    printf("Host mode started, waiting for clients...\n");
    while (1) {
      swiWaitForVBlank();
    } */
  } else if (network_state == 2) {
    if (!Wifi_LibraryModeReady())
      return;
    Wifi_ScanMode();
    bool found_match = false;
    int count = Wifi_GetNumAP();
    for (int i = 0; i < count; i++) {
      // NF_WriteText(SCR_CHAMBER, LAYER_CHAMBER_TEXT, 2, 2, "Found some AP!");
      // NF_UpdateTextLayers();
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
        if (!found_match) {
          found_match = true;
          found_others = 0;
          found_lifetime = found_lifetime_max;
        }
        found_others++;
        // network_state = 0;
        /* consoleDemoInit();
        printf("Found other DS with game ID");
        while (1) {
          swiWaitForVBlank();
        } */
      }
    }
  }
}

void social_init_hw() {
  Wifi_InitDefault(INIT_ONLY | WIFI_LOCAL_ONLY);

  // enterHost();
  //  enterClient();
  int mode = rand() % 2;
  if (mode == 0) {
    enterHost();
  } else {
    enterClient();
  }
}

void social_init_gfx() {
  NF_CreateSprite(SCR_WORLD, SPRITE_ID, GUARDIAN_ICON, DEFAULT_SPRITE_PALETTE,
                  210, 154);
  NF_ShowSprite(SCR_WORLD, SPRITE_ID, false);
  NF_SpriteLayer(SCR_WORLD, SPRITE_ID, LAYER_WORLD_TEXT);
}


void social_hide_info() {
  NF_WriteText(SCR_WORLD, LAYER_WORLD_TEXT, 26, 20, "   ");
  NF_ShowSprite(SCR_WORLD, SPRITE_ID, false);
  NF_UpdateTextLayers();
}

void social_update() {
  updateNetworkState();

//   found_others = rand() % 5;

  char buffer[3];
  if (found_others > 0) {
    snprintf(buffer, sizeof(buffer), "%d", found_others);
    NF_WriteText(SCR_WORLD, LAYER_WORLD_TEXT, 26, 20, buffer);
    NF_ShowSprite(SCR_WORLD, SPRITE_ID, true);
    NF_UpdateTextLayers();
  } else {
    social_hide_info();
  }
}