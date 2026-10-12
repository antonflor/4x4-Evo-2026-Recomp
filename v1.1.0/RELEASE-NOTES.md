## 4x4 Evolution 2026 Edition, v1.1.0

**New in this version:** true widescreen and the race HUD at the screen edges.

- **Widescreen (Hor+):** on 16:9 and 21:9 screens you see more scenery at the sides. The vertical view and proportions are unchanged, and nothing is stretched. The game is patched in memory at startup and only on the exact Build 57 exe; anything else runs in classic 4:3.
- **HUD at the edges:** lap times and place sit on the left, the map on the right, and the gauges in the bottom corners. The checkpoint arrow, messages and pause menu stay centred.
- **Menus:** the 3D backgrounds and the blue title/footer bars fill the screen. The menu text keeps its 4:3 layout.
- **Settings moved** to `system\evo2026.ini`, because the game rewrites `render.ini` on exit. The new keys are `Widescreen=1` (`0` = classic 4:3) and `HudToEdges=1`.

Also included from earlier work: native-resolution output in a borderless window, 16× anisotropic filtering, vsync, and AI-upscaled HD textures for the Arizona track, the trucks and the UI.

### Downloads

| File | What it is |
|---|---|
| `4x4Evolution2026-Setup.exe` + `-1.bin` … `-11.bin` | **All-in-one installer**: the game pre-patched to Build 57, evo2026 and the HD textures. Download all 12 files into one folder and run the exe. |
| `evo2026-kit-1.1.0.zip` | Renderer DLL, `install.ps1` / `uninstall.ps1`, texture tools, README and source, for an existing Build 57 install |
| `textures-arizona-01..05.zip` | HD texture pack (the kit's `install.ps1` unpacks these into `textures\`) |
| `SHA256SUMS.txt` | Checksums for every file |
| `*.png` | Screenshots |

The installer contains game files from a personal copy of the game. Keep this repository private.
