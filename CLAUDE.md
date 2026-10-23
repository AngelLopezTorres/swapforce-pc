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
- Imagen (2026-10-22): los vídeos Bink 1 (`gsbp_atvi.bik`, logotipo Activision) y Bink 2 (`gsbp_vvisions.bk2`) se decodifican y se ven en los volcados PNG tras actualizar `KeTimeStampBundle.tick_count` cada 1 ms (antes constante: BinkWait nunca avanzaba). Siguiente bloqueo: `XMACreateContext` (stub) -> escritura en dirección guest 0 y el juego se detiene (M4). Detalle en `PROGRESS.md`, apartado «Bink».
- Audio mínimo (M2): 187,49 callbacks/s (sumidero nulo, no suena). Con audio activo el flujo cambia: un hilo muere con `__builtin_debugtrap` en `vupkd3d128 ...,20` (FLOAT16_4) en `sub_826D2AB8` y el hilo principal queda bloqueado en una sección crítica de ese hilo.
- Rendimiento (M8, 2026-10-23, `--headless`): Bink 1 media 29,6 VdSwap/s y Bink 2 30,0 (tope del juego: 30, volteo cada 2 vblanks); 118 s consecutivos >= 20 sin carga concurrente. Detalle, opciones y verificaciones en `PROGRESS.md`, apartado «Rendimiento M8». Simplificaciones registradas como `SIMPLIFY` (MSAA 1X, sin EDRAM, sin resolve de profundidad, sin mips).

## Estado verificado (2026-10-23)
- M1 y M3 cerrados: `ppc_gen` con 0 `__builtin_debugtrap`; `--selftest-cpu` fail=0 (690 funciones, 62 familias). Quedan sin implementar los tipos 1-4 y 6 de vupkd3d128/vpkd3d128 y otras instrucciones heredadas del upstream con convencion de 32 bits sin evaluar.
- Bink: causa y arreglo documentados en PROGRESS.md (apartado Bink). M8: ~28 fps de VdSwap en --headless (tope del juego 30).
- Visor en vivo: `--viewer [puerto]` sirve el ultimo fotograma por HTTP en 127.0.0.1 (opcion `--viewer-input` para teclas); ver PROGRESS.md (apartado Visor). Caida de fps medida 0,44 %.
- Pendiente principal: el juego no progresa tras el video Bink 2 (imagen blanca fija). Diagnostico en curso en la rama codex-m4m5.

## Siguientes pasos (orden)
1. Diagnostico de la pantalla blanca y del bloqueo tras gsbp_vvisions.bk2 (hilos, entrada START, contenido/perfil, SIMPLIFY del backend).
2. M4 decodificacion XMA2 real; firmas GetRaw/SetRaw del Portal (sin escribir al dispositivo).
3. M6 fidelidad grafica (EDRAM, MSAA, mips, formatos, logotipo invertido), M7 escala 2K/4K. Criterios en `PLAN_FIDELIDAD.md`.

## Entorno
Windows 11, VS Build Tools 2022, LLVM/clang-cl, CMake, Ninja (rutas fijadas en `runtime\tools\vsenv.cmd`). Probado solo con RTX 4070 SUPER (y WARP). Salida estándar del exe muy voluminosa: redirigir a fichero.

## Actualizacion 2026-10-23 (tras unir ramas)
- Rama codex-m4m5 unida a main (XMA contextos, Portal USB selftest, entrada XInput). Compilado y ejecutado en un worktree con ppc_gen: --selftest-input/portal/xma/cpu OK, --selftest-gpu OK; ejecucion headless 150 s: ~28 fps de VdSwap medios, 0 excepciones guest (el hilo 41 ya no muere), pero la imagen queda en BLANCO uniforme (hash fijo 21AEC9254EADB383, ~596 draws y 46 resolves por frame) desde el fotograma ~650, justo tras el video Bink 2 de Vicarious Visions; el juego no carga mas .pak despues de gsbp_vvisions.bk2. Causa sin determinar [NV]: el juego queda parado (hash identico durante miles de fotogramas) o la escena final se renderiza blanca; hipotesis: fundido a blanco congelado, aproximaciones SIMPLIFY del backend, shader de composicion, textura vacia.
- Siguiente paso: localizar con volcados de resolves (--dump-resolves) y volcado de hilos (--perf) donde espera el hilo principal y que pasada produce el blanco.
- Reparto de agentes previsto: planifica Sonnet, ejecuta Haiku (Codex: gpt-6-luna esfuerzo max cuando haya tokens). Antigravity descartado (cuota agotada). Rama agy-pak (analisis .pak) NO unida: contiene CSV con nombres de assets del juego (decision legal pendiente).

## Actualizacion 2026-10-23 (ronda 7: PANTALLA DE TITULO)
- Corregido en el traductor: inversion de W con VTX_W0_FMT=1 (ronda 6) y resolve de profundidad + comparacion Z (ronda 7). Resultado verificado con PNG: pantalla de titulo "SKYLANDERS SWAP FORCE / PRESS START" (frames 850-1200, ~1956 frames en 70 s).
- Con --viewer-input, Enter inyecta START (INPUT buttons=0x0010) y el juego avanza a la pantalla "PORTAL DISCONNECTED - Please plug the Portal of Power back into your console". Siguiente bloqueo: el juego sondea el Portal por los ordinales xam 0x4A1/0x4A2 (XamInputNonControllerGetRaw/SetRaw), hoy STATUS_NOT_SUPPORTED. Requiere protocolo del Portal (escrituras al dispositivo fisico: pendiente de autorizacion explicita del usuario).
- Las secciones anteriores de este fichero sobre el blanco estan superadas por esta.
