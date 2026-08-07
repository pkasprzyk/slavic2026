# NightFox's Lib — Official Documentation

Vendored copy of the official [NightFox's Lib](https://github.com/knightfox75/nds_nflib)
documentation for **version 1.1.13**, the version linked by this project
(`-lnflib`, installed via `wf-pacman -Sy blocksds-nflib`).

## Contents

- `nflib-api.html` — the full Doxygen API reference as a single standalone
  HTML file. Open it directly in a browser (no server or other files needed).
- `Doxyfile` — the official Doxygen configuration used to build the reference.
- `readme.rst` / `changelog.rst` — the official library README and changelog.
- `licenses/` — the official license texts (MIT library, CC-BY-4.0 assets,
  CC0 examples).

## Regenerating

The multi-page reference is generated from the official sources
(`include/` + `source/`) with Doxygen 1.16.1:

```
git clone --depth 1 https://github.com/knightfox75/nds_nflib.git
cd nds_nflib && doxygen Doxyfile   # outputs docs/html
```

The single-file `nflib-api.html` is produced by merging that HTML output
(all pages, inline CSS/JS) into one self-contained file.
