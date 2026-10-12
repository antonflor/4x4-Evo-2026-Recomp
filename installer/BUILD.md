# Building the all-in-one installer

`4x4Evolution2026.iss` is an Inno Setup 6.4+ script. It expects a `payload\game\` folder
next to it containing:

- the installed game files patched to Build 57 (`4x4.exe` = 2,238,976 bytes),
  with `TRIDX7.DLL` renamed to `TRIDX7.original.dll`
- `build\evo2026.dll` copied in as `TRIDX7.DLL`
- `system\METAL.INI` / `system\RENDER.INI` preconfigured, plus `system\evo2026.ini` (template: `installer\evo2026.ini`); the [INI] section enforces the keys anyway
- `textures\` (HD pack) and `evo2026\` (README.txt, README.md, tools, source zip, license)

Plus `art\game.ico` (the CD's `4X4.ICO`). Game files are not in this repo: build the payload
from your own disc. Then:

    ISCC.exe 4x4Evolution2026.iss

Output: `output\4x4Evolution2026-Setup.exe` + `-N.bin` slices (24 MB each; keep them together).
Inno's 64 MB LZMA dictionary doesn't fit a 32-bit build host, so the script uses lzma2/max.
