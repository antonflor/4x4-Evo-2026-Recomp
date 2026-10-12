# evo2026: 4x4 Evolution at native HD on modern Windows

evo2026 is a drop-in Direct3D 9 renderer for **4x4 Evolution Build 57**. You don't need to modify the game's exe.

The game keeps running at its own 640×480 "logical" resolution, so menus and HUD stay laid out exactly as designed. evo2026 then draws everything at your monitor's native resolution (1080p, 1440p, 4K). 3D geometry is rasterized at full detail rather than upscaled.

It also adds:

- borderless fullscreen
- vsync
- 16× anisotropic filtering
- an HD texture-replacement system with an AI upscaling pipeline

Built on the open-source [4x4e](https://github.com/americusmaximus/4x4e) reimplementation's Direct3D 9 renderer (MIT).

## Install

1. **Install the game** from your disc or ISO. You can mount the ISO in Windows by double-clicking it. Install to a folder **outside Program Files**, e.g. `C:\Games\4x4 Evolution`.
2. **Apply the Build 57 patch**: run `4x4patchGB57.exe` and point it at the install.
3. **Run the installer** from this folder in PowerShell:
   ```powershell
   powershell -ExecutionPolicy Bypass -File .\install.ps1 -GamePath "C:\Games\4x4 Evolution"
   ```
4. **Launch `4x4.exe`.** If the blue "4x4 Setup Options" screen appears, Renderer should read **Direct3D 7** and 3D Card **evo2026 Direct3D 9**. Pick **Run 4x4**.

To go back to stock, run `uninstall.ps1 -GamePath "..."`.

### Why the DLL is called TRIDX7.DLL

The exe only accepts renderers by fixed file names. It compares `rendererDLLPath` in `system\metal.ini` against `tridx7.dll`, `trigl.dll`, `triglide.dll` and `tridx8tl.dll`. Any other name silently falls back to 320×240 software rendering. evo2026 implements the Direct3D 7 slot, so it has to be installed as `TRIDX7.DLL`. The installer keeps your original as `TRIDX7.original.dll`.

## Settings

The settings live in `system\render.ini`, section `[evo2026]`:

| Key | Default | Meaning |
|---|---|---|
| `Width` / `Height` | `0` | Output resolution. `0` = monitor's native resolution |
| `Window` | `2` | `0` exclusive fullscreen · `1` window · `2` borderless fullscreen |
| `Aspect` | `1` | `1` keep 4:3 with side bars · `0` stretch to fill |
| `VSync` | `1` | Sync to the monitor's refresh rate |
| `Anisotropy` | `16` | Anisotropic filtering level (`0` = off) |
| `Filter2D` | `1` | `1` smooth scaling of 2D framebuffer art · `0` sharp pixels |
| `ReplaceTextures` | `1` | Load HD textures from `textures\` |
| `DumpTextures` | `0` | Save every texture the game uploads to `textures_dump\` |
| `Trace` | `0` | Write a renderer call log to `system\evo2026.log` |

Game-side settings are in `system\metal.ini` `[Graphics]`. Keep `gamePIXX=640`, `gamePIXY=480` and `gameBPP=32`.

**Don't** set `gamePIXY` to 1080. The game's font code only accepts 4:3 heights (480, 600, 768, 864, 960, 1200) and stops with "Invalid screen resolution". The logical-resolution design is how evo2026 gets around that.

## HD textures

The game identifies textures by name. evo2026 matches replacements by name plus a CRC of the pixels, so the same texture with a different palette (e.g. truck colors) can be replaced separately:

- `textures\<name>__<crc>.png` replaces only that exact variant.
- `textures\<name>.png` replaces every variant of that texture.

Replacements can be any size (4× the original is a good target). Mip-maps are generated automatically, and sub-folders are fine.

### Workflow

1. **Get the originals**, either way:
   - From the game files (no play-through needed). This gives `<name>.png` files:
     ```
     python tools\pod_textures.py "C:\Games\4x4 Evolution" C:\evo-textures\original
     ```
   - Or exactly as the game uses them: set `DumpTextures=1`, play the tracks and trucks you care about, then set it back to `0`. Files land in `textures_dump\`. This is the only way to get the runtime-built truck skins (`truck0512`…).
2. **Upscale.** With an NVIDIA GPU, install CUDA PyTorch first:
   ```
   pip install torch --index-url https://download.pytorch.org/whl/cu128
   pip install pillow numpy
   python tools\upscale_textures.py "C:\Games\4x4 Evolution\textures_dump" "C:\Games\4x4 Evolution\textures"
   ```
   Add `--model x4plus` for a sharper (heavier) model. Add `--match zoa_` to do one track's tiles only.
3. **Launch the game.** Replacements load automatically.

The upscaler wrap-pads tileable terrain so it stays seamless, edge-pads atlases (truck skins, UI) instead, and keeps cut-out alpha masks crisp.

## Known limitations and next steps

- **Widescreen is 4:3 with side bars** (or stretched). A true wider field of view needs a small patch to the exe's projection code. That's next on the list.
- **Lighting is still the original per-vertex lighting** baked by the game on the CPU. The exe also supports an unshipped hardware lighting renderer (`tridx8tl.dll`) with `setLightVector`, `setAmbientLight`, `addDynamicLight` and transform calls. That's the way into real per-pixel lighting.
- **Intro videos and the setup screen** are drawn by the game's own DirectDraw code before the renderer starts, so they run at 640×480. They're unaffected by evo2026.
- **The setup screen comes back** if the previous session crashed or was force-closed. That's the game's own crash recovery.
- **Mouse/gamepad** are unchanged from the original (DirectInput). Modern-controller support is a planned add-on.

## Troubleshooting

- **The game runs at 320×240 or looks software-rendered.** `system\metal.ini` must say `rendererDLLPath=tridx7.dll` and `useDirect3D=1`. If the game is under Program Files, Windows may be using a copy in `%LOCALAPPDATA%\VirtualStore\…\system\metal.ini` instead.
- **The image is blurry on a high-DPI screen.** The installer marks `4x4.exe` DPI-aware. Check Properties → Compatibility → "Change high DPI settings".
- **Anything else:** set `Trace=1`, reproduce, and look at `system\evo2026.log`.

## Building from source

The renderer source is in `evo2026-src.zip` (git history included). It cross-compiles with MinGW-w64:

```
make            # needs i686-w64-mingw32-g++ (Linux/WSL: apt install g++-mingw-w64-i686)
```

Output: `build/evo2026.dll`. 32-bit only, because the game is 32-bit.

## Credits and licenses

- **Renderer base:** 4x4e by Americus Maximus, MIT (see `LICENSE.4x4e` in the source).
- **PNG loading/saving:** stb_image / stb_image_write by Sean Barrett, public domain.
- **Upscaling models:** Real-ESRGAN by Xintao Wang et al., BSD-3-Clause. Downloaded by the tool on first use, not bundled.
- **4x4 Evolution** © 2000 Terminal Reality. You need your own copy of the game. No game files are included.
