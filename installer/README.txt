4x4 EVOLUTION - 2026 EDITION (evo2026)
=======================================

This is 4x4 Evolution Build 57 with the evo2026 renderer: the game runs at
its own 640x480 layout, and everything is drawn at your monitor's native
resolution (1080p, 1440p, 4K) in a borderless window, with 16x anisotropic
filtering and AI-upscaled HD textures for the Arizona track and the trucks.

Already set up by the installer:
  - Build 57 patch applied
  - evo2026 installed as TRIDX7.DLL (the game only accepts renderers by this
    file name). The original renderer is kept as TRIDX7.original.dll.
  - system\metal.ini: Direct3D renderer, 32-bit colour, 640x480 layout
  - textures\: HD texture pack
  - 4x4.exe marked DPI-aware for your Windows account


SETTINGS  (Start menu > 4x4 Evolution 2026 > evo2026 Settings)
--------
The file is system\render.ini, section [evo2026]:

  Width / Height   0         output resolution, 0 = monitor's native
  Window           2         0 exclusive fullscreen, 1 window, 2 borderless
  Aspect           1         1 keep 4:3 with side bars, 0 stretch to fill
  VSync            1         sync to the monitor's refresh rate
  Anisotropy       16        texture filtering level (0 = off)
  Filter2D         1         1 smooth 2D scaling, 0 sharp pixels
  ReplaceTextures  1         load HD textures from textures\
  DumpTextures     0         1 = save every texture the game uses to
                             textures_dump\ (for making more HD textures)
  Trace            0         1 = write a renderer log to system\evo2026.log

Leave gamePIXX=640 / gamePIXY=480 in system\metal.ini. The game's font code
only accepts 4:3 heights and stops with "Invalid screen resolution" at 1080.


THE BLUE "4x4 SETUP OPTIONS" SCREEN
-----------------------------------
The original game shows this screen every time it starts - just press Enter
("Run 4x4" is already selected). It should read Renderer "Direct3D 7" and
3D Card "evo2026 Direct3D 9". Leave Resolution at 640x480: evo2026 draws it
at your monitor's resolution anyway.


MORE HD TEXTURES
----------------
The pack covers the Arizona track, the trucks and the UI. To do other tracks,
see evo2026\README.md ("HD textures"). In short:
  1. python evo2026\tools\pod_textures.py "<this folder>" <out folder>
     (or play with DumpTextures=1)
  2. python evo2026\tools\upscale_textures.py <in folder> "<this folder>\textures"
  3. Start the game; new PNGs in textures\ load automatically.
Replacement names: textures\<name>.png (all variants) or
<name>__<crc>.png (one exact variant, e.g. a single truck colour).


TROUBLESHOOTING
---------------
- Low-res / software look: system\metal.ini must contain
  rendererDLLPath=tridx7.dll and useDirect3D=1.
- Blurry on a high-DPI screen: 4x4.exe > Properties > Compatibility >
  Change high DPI settings > "Override high DPI scaling" = Application.
- Back to the stock renderer: copy TRIDX7.original.dll over TRIDX7.DLL.
- Anything else: set Trace=1, reproduce, look at system\evo2026.log.

Known limits: widescreen is 4:3 with side bars for now (a wider field of view
needs an exe patch); intro videos and the setup screen run at 640x480;
lighting is the original 2000 lighting.


CREDITS & LICENSES
------------------
evo2026 renderer: based on 4x4e by Americus Maximus (MIT, see
  evo2026\LICENSE.4x4e.txt). Source: evo2026\evo2026-src.zip
stb_image / stb_image_write: Sean Barrett, public domain.
HD textures made with Real-ESRGAN (Xintao Wang et al., BSD-3-Clause).
4x4 Evolution (c) 2000 Terminal Reality. This installer was built from a
personal copy of the game for personal use - please don't redistribute it.
