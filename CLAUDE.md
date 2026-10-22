# Contexto del proyecto (traspaso entre equipos) — actualizado 2026-10-12

Proyecto: port nativo a PC de Skylanders Swap Force (Xbox 360) por recompilación estática (XenonRecomp + runtime propio en `runtime\` + backend D3D12). Requiere la ISO propia del usuario; el repositorio NO contiene contenido del juego (ver `LEGAL_RESUMEN.md`). Repositorio PUBLICO desde 2026-10-12 (decision del usuario); sin contenido del juego.

## Reglas de trabajo (organización FeeX)
Formal, escueto, metódico; solo datos objetivos y documentables, sin opiniones; citar normativa/documentación oficial; no inventar: preguntar lo necesario; marcar [NV] lo no verificado; no afirmar que algo funciona sin ejecutar el comando; distinguir "compila" de "se ha ejecutado".

## Decisiones del usuario
- Objetivo: juego fiel al original, nativo en PC, con escalado de resolución interna 2K (x2) y luego 4K (x3). Mods: más adelante. Unity/Bevy descartados (cambiarían el comportamiento).
- Repositorio publicado (público) por decisión del usuario para que otros lo jueguen (modelo: herramientas + ISO propia). La asesoría jurídica sigue pendiente (`LEGAL_RESUMEN.md`, 13 preguntas abiertas). `LICENSE` es propuesta con `[TITULAR]` sin definir.
- Portal of Power: físico (paso directo USB; referencia Xenia Canary `hid/portal`). Modelo exacto: pendiente de preguntar.
- Aceptado: FFmpeg/libavcodec (LGPL, DLL dinámica) para XMA. Pendiente: capturas de referencia del juego original y umbral PSNR (propuesta 40 dB).
- Método usado: planificar con agente Opus (`Plan`, model opus), ejecutar con agente Sonnet; planes en `PLAN_RUNTIME.md`, `PLAN_VIDEO.md`, `PLAN_GPU.md`, `PLAN_FIDELIDAD.md`. Detalle de resultados en `PROGRESS.md` (apartados Runtime, Vídeo, GPU, Fidelidad).

## Estado medido (última verificación completa)
- Runtime: el juego arranca `_xstart`, ejecuta millones de imports y entra en su bucle principal. GPU: PM4 sin opcodes desconocidos, 55 shaders traducidos a DXIL (55/55), ventana D3D12, volcado de frames a PNG, resolve y texturas básicas.
- NO hay imagen visible: todos los frames salen negros. Causa en investigación: la composición final usa planos Y/U/V de vídeo Bink 1 (`gsbp_atvi.bik`) que no avanza.
- Audio mínimo (M2): 187,49 callbacks/s (sumidero nulo, no suena). Con audio activo el flujo cambia: un hilo muere con `__builtin_debugtrap` en `vupkd3d128 ...,20` (FLOAT16_4) en `sub_826D2AB8` y el hilo principal queda bloqueado en una sección crítica de ese hilo.
- Rendimiento ~7-9 fps. Simplificaciones registradas como `SIMPLIFY` (MSAA 1X, sin EDRAM, sin resolve de profundidad, sin mips).

## Trabajo en curso / sin verificar (importante)
- M1 (decodificación CPU) quedó a medias cuando se cortó la sesión: `recompiler.cpp` modificado y `ppc_gen` regenerado con 0 `__builtin_debugtrap`, PERO no se verificó que compile ni que el juego avance. `runtime\build` puede no incluir M0/M2 (había un build aparte `build_fid`). El parche `patches\XenonRecomp-swapforce.patch` incluye ese estado sin comprobar.
- Hallazgos de M1 a confirmar: las 2 `mulchw.` (0x83088AE0 y 0x83088C0C) son `vsldoi128` VX128_5 mal decodificadas; 129 `vupkd3d128` tipo 5; 1 `blrl`.
- M3 (`--selftest-cpu` de los 49 opcodos vectoriales contra ALTIVECPEM/Power ISA) no iniciado; los 49 opcodos siguen sin verificar en ejecución.

## Siguientes pasos (orden)
1. Ejecutar `setup.ps1` con la ISO (ver README) y reconstruir; comprobar que compila con los cambios de M1.
2. Cerrar M1: `grep -c __builtin_debugtrap runtime\ppc_gen\*.cpp`; repetir `SwapForceRT.exe --headless --bink-trace --d3d-stats --dump-frames 60,200,400 --timeout 90` y registrar hasta dónde avanza (hilos, excepciones, BINKPLANE, hashes Y distintos, nonblack_px).
3. M3 (tests de CPU), M4 XMA, M5 entrada y Portal (ordinales 0x4A1/0x4A2 de xam), M6 fidelidad gráfica (EDRAM, MSAA, mips, formatos, logotipo invertido), M7 escala 2K/4K, M8 rendimiento ≥30 fps. Detalle y criterios de aceptación en `PLAN_FIDELIDAD.md`.

## Entorno
Windows 11, VS Build Tools 2022, LLVM/clang-cl, CMake, Ninja (rutas fijadas en `runtime\tools\vsenv.cmd`). Probado solo con RTX 4070 SUPER (y WARP). Salida estándar del exe muy voluminosa: redirigir a fichero.
