// SPDX-License-Identifier: CC0-1.0

#include <filesystem.h>
#include <nds.h>
#include <nf_lib.h>

#include "dswifi9.h"
#include "game.h"
#include "ids.h"

const u32 GAME_ID = 0xFF00FF0F;
bool found_other = false;
u8 network_state = 0; // 1-host, 2-client, 0-none

void enterHost() {
  Wifi_IdleMode();
  Wifi_MultiplayerHostMode(2, 1, 1);
  network_state = 1; // Set to host mode
}

void enterClient() {
  Wifi_IdleMode();
  Wifi_MultiplayerClientMode(1);
  network_state = 2; // Set to client mode
}

void updateNetworkState() {
  if (network_state == 0)
    return;
  if (network_state == 1) {
    if (!Wifi_LibraryModeReady())
      return;

    Wifi_SetChannel(7);
    Wifi_MultiplayerAllowNewClients(true);

    Wifi_BeaconStart("RHCB", GAME_ID);
    network_state = 0; // Reset to none after starting host
    consoleDemoInit();
    printf("Host mode set");
    while (1) {
      swiWaitForVBlank();
    }
  } else if (network_state == 2) {
    if (!Wifi_LibraryModeReady())
      return;
    Wifi_ScanMode();
    int count = Wifi_GetNumAP();
    for (int i = 0; i < count; i++) {
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
  // enterClient();

  game_init();

  while (1) {
    scanKeys();
    game_update();
    NF_SpriteOamSet(SCR_WORLD);
    NF_SpriteOamSet(SCR_CHAMBER);
    swiWaitForVBlank();
    oamUpdate(&oamMain);
    oamUpdate(&oamSub);
    updateNetworkState();
  }

  game_deinit();
}
