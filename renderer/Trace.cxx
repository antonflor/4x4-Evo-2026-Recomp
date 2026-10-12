#include "Native.Basic.hxx"
#include "Trace.hxx"
#include <stdio.h>
#include <stdarg.h>
namespace Evo
{
    int TraceEnabled = -1;
    void Trace(const char* fmt, ...)
    {
        static FILE* f = NULL;
        static unsigned long count = 0;
        if (TraceEnabled < 0) { TraceEnabled = GetPrivateProfileIntA("evo2026", "Trace", 0, ".\\system\\render.ini"); if (!TraceEnabled) return; }
        if (++count > 20000) return; // cap log size
        if (f == NULL) { f = fopen(".\\system\\evo2026.log", "w"); if (f == NULL) return; }
        va_list a; va_start(a, fmt); vfprintf(f, fmt, a); va_end(a); fputc('\n', f); fflush(f);
    }
}
