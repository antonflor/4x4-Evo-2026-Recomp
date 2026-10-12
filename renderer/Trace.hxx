#pragma once
// Lightweight call tracer for evo2026. Enabled via [evo2026] trace=1 in system\render.ini.
namespace Evo { void Trace(const char* fmt, ...); extern int TraceEnabled; }
#define EVO_TRACE(...) do { if (Evo::TraceEnabled) { Evo::Trace(__VA_ARGS__); } } while (0)
