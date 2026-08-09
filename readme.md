# Red Hot Chilling Bunnies - Slavic Game Jam 2026

A Nintendo DS game about a moss-covered guardian mech rescuing forest animals from a wildfire. Blow into the DS microphone to cool your overheating reactor. Stay calm while everything burns.

The theme is <em>chill</em>, and the whole game is designed around staying chill (in more ways than one) while everything around you burns.

---

## Controls

| Input             | Action                                           |
|-------------------|--------------------------------------------------|
| D-Pad             | Move the mech                                    |
| Touch (stylus)    | Tap / drag to aim the water spray                |
|                   | Fill up your tank while near the water           |
| DS microphone     | Blow to cool the overheating reactor             |

---

## Build

Requires [BlocksDS](https://github.com/blocksds) with [NightFox's Lib](https://github.com/nflib).

```sh
make          # build ROM (slavic2026.nds)
make run      # build + run in melonDS
make clean    # remove artifacts
```

Asset pipeline: PNGs in `assets/` → GRIT → `nitrofiles/` → NitroFS. Graphics are 256-color tiled backgrounds and sprites via NFLib. Audio via mmutil.

---

## Tech

- **Platform**: Nintendo DS (ARM9)
- **Language**: C (GNU17)
- **SDK**: BlocksDS
- **Graphics**: NightFox's Lib (NFLib)
- **Asset pipeline**: GRIT + mmutil

---

## Contributors

Made for **Slavic Game Jam 2026**.

| Role   | People                                                         |
|--------|----------------------------------------------------------------|
| Code   | NamespaceV, Mateusz Choiński, Szymon Rzosiński, Piotr Kasprzyk |
| Art    | yreron                                                         |
| Design | all                                                            |
| Audio  | Justyna Kryścio-Rzosińska                                      |

---
