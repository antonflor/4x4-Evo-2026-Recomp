/*
evo2026 — HD extensions for the 4x4 Evolution Direct3D 9 renderer. See Evo.hxx.
*/
#include "Evo.hxx"
#include "Trace.hxx"

#include <stdlib.h>
#include <string.h>

namespace Evo
{
    ConfigState Config;
    ViewState View;

    static u32 ReadInt(const char* name, const u32 value)
    {
        return (u32)GetPrivateProfileIntA(EVO_SECTION, name, (INT)value, EVO_INI);
    }

    void LoadConfig(void)
    {
        Config.Width = ReadInt("Width", Config.Width);
        Config.Height = ReadInt("Height", Config.Height);
        Config.Window = ReadInt("Window", Config.Window);
        Config.Aspect = ReadInt("Aspect", Config.Aspect);
        Config.VSync = ReadInt("VSync", Config.VSync);
        Config.Anisotropy = ReadInt("Anisotropy", Config.Anisotropy);
        Config.Filter2D = ReadInt("Filter2D", Config.Filter2D);

        if (Config.Window > 2) { Config.Window = 2; }
        if (Config.Anisotropy > 16) { Config.Anisotropy = 16; }

        EVO_TRACE("evo2026 config: %ux%u window=%u aspect=%u vsync=%u aniso=%u filter2d=%u",
            Config.Width, Config.Height, Config.Window, Config.Aspect, Config.VSync, Config.Anisotropy, Config.Filter2D);
    }

    void ComputeView(const u32 gameWidth, const u32 gameHeight, const u32 outWidth, const u32 outHeight)
    {
        View.GameWidth = gameWidth;
        View.GameHeight = gameHeight;
        View.OutWidth = outWidth;
        View.OutHeight = outHeight;

        if (Config.Aspect == 0)
        {
            View.X = 0; View.Y = 0; View.W = outWidth; View.H = outHeight;
        }
        else
        {
            // Fit the game's aspect ratio inside the output, centred.
            const f64 game = (f64)gameWidth / (f64)gameHeight;
            const f64 out = (f64)outWidth / (f64)outHeight;

            if (out > game)
            {
                View.H = outHeight;
                View.W = (u32)((f64)outHeight * game + 0.5);
            }
            else
            {
                View.W = outWidth;
                View.H = (u32)((f64)outWidth / game + 0.5);
            }

            View.X = (outWidth - View.W) / 2;
            View.Y = (outHeight - View.H) / 2;
        }

        View.SX = (f32)View.W / (f32)gameWidth;
        View.SY = (f32)View.H / (f32)gameHeight;
        View.OX = (f32)View.X;
        View.OY = (f32)View.Y;

        EVO_TRACE("evo2026 view: game %ux%u -> out %ux%u, image %u,%u %ux%u scale %.3f,%.3f",
            gameWidth, gameHeight, outWidth, outHeight, View.X, View.Y, View.W, View.H, View.SX, View.SY);
    }

    void PrepareWindow(const HWND hwnd, const u32 gameWidth, const u32 gameHeight, u32* outWidth, u32* outHeight)
    {
        // Old games are not DPI aware; without this, Windows reports a scaled-down desktop
        // (e.g. 2560x1440 on a 4K screen at 150%) and blurs the result.
        {
            typedef BOOL(WINAPI* SETPROCESSDPIAWARE)(void);
            const auto user = GetModuleHandleA("user32.dll");
            const auto fn = user != NULL ? (SETPROCESSDPIAWARE)GetProcAddress(user, "SetProcessDPIAware") : NULL;
            if (fn != NULL) { fn(); }
        }

        MONITORINFO mi;
        ZeroMemory(&mi, sizeof(mi));
        mi.cbSize = sizeof(mi);
        GetMonitorInfoA(MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY), &mi);

        const u32 monitorWidth = (u32)(mi.rcMonitor.right - mi.rcMonitor.left);
        const u32 monitorHeight = (u32)(mi.rcMonitor.bottom - mi.rcMonitor.top);

        u32 width = Config.Width != 0 ? Config.Width : monitorWidth;
        u32 height = Config.Height != 0 ? Config.Height : monitorHeight;

        if (Config.Window == 1 && Config.Width == 0)
        {
            // Windowed with no explicit size: largest integer multiple of the game size that fits.
            u32 scale = 1;
            while ((scale + 1) * gameWidth <= monitorWidth * 9 / 10 && (scale + 1) * gameHeight <= monitorHeight * 9 / 10) { scale++; }
            width = gameWidth * scale;
            height = gameHeight * scale;
        }

        if (hwnd != NULL)
        {
            if (Config.Window == 1)
            {
                const DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE;
                RECT rect = { 0, 0, (LONG)width, (LONG)height };
                AdjustWindowRect(&rect, style, FALSE);
                SetWindowLongA(hwnd, GWL_STYLE, style);
                SetWindowPos(hwnd, HWND_NOTOPMOST,
                    mi.rcWork.left + ((mi.rcWork.right - mi.rcWork.left) - (rect.right - rect.left)) / 2,
                    mi.rcWork.top + ((mi.rcWork.bottom - mi.rcWork.top) - (rect.bottom - rect.top)) / 2,
                    rect.right - rect.left, rect.bottom - rect.top, SWP_FRAMECHANGED | SWP_SHOWWINDOW);
            }
            else
            {
                SetWindowLongA(hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
                SetWindowPos(hwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top,
                    (s32)monitorWidth, (s32)monitorHeight, SWP_FRAMECHANGED | SWP_SHOWWINDOW);
            }
        }

        *outWidth = width;
        *outHeight = height;
    }

    void SetViewViewport(IDirect3DDevice9* device)
    {
        D3DVIEWPORT9 vp = { View.X, View.Y, View.W, View.H, 0.0f, 1.0f };
        device->SetViewport(&vp);
    }

    void SetFullViewport(IDirect3DDevice9* device)
    {
        D3DVIEWPORT9 vp = { 0, 0, View.OutWidth, View.OutHeight, 0.0f, 1.0f };
        device->SetViewport(&vp);
    }

    // ---------------------------------------------------------------------------------
    // Framebuffer access.
    //
    // The game sometimes writes pixels straight into the frame (Bink videos, the setup
    // screen, some text). It expects a GameWidth x GameHeight buffer, but our back buffer
    // is the full output size. On lock we hand it a small copy of the current frame; on
    // unlock we composite back only the pixels it actually changed, so the HD 3D image
    // underneath stays sharp.
    // ---------------------------------------------------------------------------------

    static struct
    {
        IDirect3DSurface9* Target = NULL;   // GameWidth x GameHeight render target (downscaled copy)
        IDirect3DSurface9* System = NULL;   // system-memory copy the game writes into
        IDirect3DTexture9* Overlay = NULL;  // changed pixels, uploaded for compositing
        IDirect3DStateBlock9* States = NULL;
        u8* Before = NULL;
        u32 Width = 0, Height = 0, BPP = 0;
        BOOL Locked = FALSE;
    } Frame;

    void ReleaseFrame(void)
    {
        if (Frame.Locked && Frame.System != NULL) { Frame.System->UnlockRect(); }
        if (Frame.Target != NULL) { Frame.Target->Release(); Frame.Target = NULL; }
        if (Frame.System != NULL) { Frame.System->Release(); Frame.System = NULL; }
        if (Frame.Overlay != NULL) { Frame.Overlay->Release(); Frame.Overlay = NULL; }
        if (Frame.States != NULL) { Frame.States->Release(); Frame.States = NULL; }
        if (Frame.Before != NULL) { free(Frame.Before); Frame.Before = NULL; }
        Frame.Width = Frame.Height = Frame.BPP = 0;
        Frame.Locked = FALSE;
    }

    static BOOL EnsureFrame(IDirect3DDevice9* device, const u32 bpp)
    {
        if (Frame.Target != NULL && Frame.Width == View.GameWidth && Frame.Height == View.GameHeight && Frame.BPP == bpp) { return TRUE; }

        ReleaseFrame();

        const D3DFORMAT format = bpp == 16 ? D3DFMT_R5G6B5 : D3DFMT_X8R8G8B8;

        if (device->CreateRenderTarget(View.GameWidth, View.GameHeight, format, D3DMULTISAMPLE_NONE, 0, FALSE, &Frame.Target, NULL) != D3D_OK) { ReleaseFrame(); return FALSE; }
        if (device->CreateOffscreenPlainSurface(View.GameWidth, View.GameHeight, format, D3DPOOL_SYSTEMMEM, &Frame.System, NULL) != D3D_OK) { ReleaseFrame(); return FALSE; }
        if (device->CreateTexture(View.GameWidth, View.GameHeight, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &Frame.Overlay, NULL) != D3D_OK) { ReleaseFrame(); return FALSE; }
        if (device->CreateStateBlock(D3DSBT_ALL, &Frame.States) != D3D_OK) { ReleaseFrame(); return FALSE; }

        Frame.Before = (u8*)malloc((size_t)View.GameWidth * View.GameHeight * (bpp / 8));
        if (Frame.Before == NULL) { ReleaseFrame(); return FALSE; }

        Frame.Width = View.GameWidth;
        Frame.Height = View.GameHeight;
        Frame.BPP = bpp;

        return TRUE;
    }

    BOOL LockFrame(IDirect3DDevice9* device, const u32 bpp, u8** bits, u32* pitch)
    {
        if (Frame.Locked) { return FALSE; }
        if (!EnsureFrame(device, bpp)) { return FALSE; }

        IDirect3DSurface9* back = NULL;
        if (device->GetRenderTarget(0, &back) != D3D_OK) { return FALSE; }

        const RECT source = { (LONG)View.X, (LONG)View.Y, (LONG)(View.X + View.W), (LONG)(View.Y + View.H) };
        device->StretchRect(back, &source, Frame.Target, NULL, D3DTEXF_LINEAR);
        back->Release();

        device->GetRenderTargetData(Frame.Target, Frame.System);

        D3DLOCKED_RECT rect;
        if (Frame.System->LockRect(&rect, NULL, 0) != D3D_OK) { return FALSE; }

        const u32 row = Frame.Width * (bpp / 8);
        for (u32 y = 0; y < Frame.Height; y++)
        {
            memcpy(Frame.Before + (size_t)y * row, (u8*)rect.pBits + (size_t)y * rect.Pitch, row);
        }

        Frame.Locked = TRUE;

        *bits = (u8*)rect.pBits;
        *pitch = (u32)rect.Pitch;

        return TRUE;
    }

    void UnlockFrame(IDirect3DDevice9* device)
    {
        if (!Frame.Locked) { return; }

        Frame.Locked = FALSE;

        // The game has finished writing; reopen the system copy read-only to diff it.
        D3DLOCKED_RECT src;
        Frame.System->UnlockRect();
        if (Frame.System->LockRect(&src, NULL, D3DLOCK_READONLY) != D3D_OK) { return; }

        D3DLOCKED_RECT dst;
        if (Frame.Overlay->LockRect(0, &dst, NULL, D3DLOCK_DISCARD) != D3D_OK) { Frame.System->UnlockRect(); return; }

        u32 changed = 0;
        const u32 bytes = Frame.BPP / 8;
        const u32 row = Frame.Width * bytes;

        for (u32 y = 0; y < Frame.Height; y++)
        {
            const u8* now = (const u8*)src.pBits + (size_t)y * src.Pitch;
            const u8* was = Frame.Before + (size_t)y * row;
            u32* out = (u32*)((u8*)dst.pBits + (size_t)y * dst.Pitch);

            if (memcmp(now, was, row) == 0)
            {
                memset(out, 0, (size_t)Frame.Width * 4);
                continue;
            }

            if (bytes == 2)
            {
                const u16* n = (const u16*)now; const u16* w = (const u16*)was;
                for (u32 x = 0; x < Frame.Width; x++)
                {
                    if (n[x] == w[x]) { out[x] = 0; continue; }
                    const u32 c = n[x];
                    const u32 r = ((c >> 11) & 0x1f) * 255 / 31, g = ((c >> 5) & 0x3f) * 255 / 63, b = (c & 0x1f) * 255 / 31;
                    out[x] = 0xff000000 | (r << 16) | (g << 8) | b;
                    changed++;
                }
            }
            else
            {
                const u32* n = (const u32*)now; const u32* w = (const u32*)was;
                for (u32 x = 0; x < Frame.Width; x++)
                {
                    if ((n[x] & 0xffffff) == (w[x] & 0xffffff)) { out[x] = 0; continue; }
                    out[x] = 0xff000000 | (n[x] & 0xffffff);
                    changed++;
                }
            }
        }

        Frame.Overlay->UnlockRect(0);
        Frame.System->UnlockRect();

        if (changed == 0) { return; }

        // Composite the changed pixels over the HD frame.
        Frame.States->Capture();

        struct QuadVertex { f32 X, Y, Z, RHW, U, V; };
        const f32 x0 = (f32)View.X - 0.5f, y0 = (f32)View.Y - 0.5f;
        const f32 x1 = x0 + (f32)View.W, y1 = y0 + (f32)View.H;
        const QuadVertex quad[4] =
        {
            { x0, y0, 0.0f, 1.0f, 0.0f, 0.0f },
            { x1, y0, 0.0f, 1.0f, 1.0f, 0.0f },
            { x0, y1, 0.0f, 1.0f, 0.0f, 1.0f },
            { x1, y1, 0.0f, 1.0f, 1.0f, 1.0f },
        };

        device->BeginScene();
        device->SetVertexShader(NULL);
        device->SetPixelShader(NULL);
        device->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);
        device->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
        device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
        device->SetRenderState(D3DRS_FOGENABLE, FALSE);
        device->SetRenderState(D3DRS_LIGHTING, FALSE);
        device->SetRenderState(D3DRS_SPECULARENABLE, FALSE);
        device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
        // Overlay is premultiplied (unchanged pixels are 0,0,0,0), so filtered edges blend cleanly.
        device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
        device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
        device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        device->SetTexture(0, Frame.Overlay);
        device->SetTexture(1, NULL);
        device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
        device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        device->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
        device->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
        device->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        const DWORD filter = Config.Filter2D ? D3DTEXF_LINEAR : D3DTEXF_POINT;
        device->SetSamplerState(0, D3DSAMP_MAGFILTER, filter);
        device->SetSamplerState(0, D3DSAMP_MINFILTER, filter);
        device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
        device->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
        device->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
        device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, quad, sizeof(QuadVertex));
        device->EndScene();

        Frame.States->Apply();
    }
}
