# PLAN_60FPS — Juego a 60 FPS hasta el menú inclusive (solo planificación; nada ejecutado)

Alcance: vídeos de arranque (Bink 1 `gsbp_atvi.bik`, Bink 2 `gsbp_vvisions.bk2`), título, menú principal, selección de modo y dificultad.
Reglas: siempre `--headless`; visor en puertos 8771-8779; no tocar `runtime\build_live2` (puerto 8765); salida estándar a fichero; `[NV]` = no verificado.

## 0. Hechos leídos en código y `PROGRESS.md` (Rendimiento M8) — base del plan
- `VdSwap` (`xboxkrnl_video.cpp:121`) no recibe ni usa un intervalo: construye 64 palabras PM4 (fetch 0x4800 + `XE_SWAP` 0x64, firma `SWAP`). El CP llama a `OnSwap` al procesarlo (`gpu.cpp:168`) sin esperar. Por tanto el runtime no impone el intervalo de 2 en el volteo.
- M8 midió que el hilo de render del juego gira en `sub_82515D68` (bucle en 82515E6C → `sub_82F5CB30`) hasta que `[dev+16968]-[dev+16976] < 1`; `[+16976]` lo incrementa el manejador VBLANK (`sub_82F5C128`, ejecutado por `gpu-vsync`). Cadencia: 2 vblanks por swap. El valor del intervalo en memoria guest [NV: no leído].
- `VsyncMain` (`gpu.cpp:355`) genera 60 vblanks/s fijos (QPC, `Sleep(1)`), `DispatchInterrupt(0,2)`. Es la única fuente de vblank: si el juego espera 2 vblanks, el techo es 30 fps con este reloj.
- `Present(1,0)` (`d3d12_backend.cpp:402`) solo existe con ventana; con `--headless` no se ejecuta (coste con ventana: 0,23 ms CPU/fotograma; bloqueo por vsync [NV]).
- `KeTimeStampBundle` +16 = milisegundos de QPC (reloj real, hilo de 1 ms, `kernel_vars.cpp:81`). Bink lo usa (GetTickCount, `sub_82F42528`). La opción `--det-clock` NO aparece en `main.cpp`/`kernel_vars.cpp` de este worktree [NV: confirmar en la rama principal antes de usarla].
- Estado de rendimiento: ~28-30 VdSwap/s en vídeos; menú ~600 draws y ~40 resolves/fotograma; el menú no se ha medido a 30 s sostenidos con M8 completo [NV]. Hay un bloqueo no determinista tras `XMACreateContext` (M4) que puede impedir medir la escena posterior.

## Hito 1 — Localizar quién fija el intervalo de 2 vblanks
Instrumentación nueva (opción `--trace-swap-interval`, solo lectura, ficheros del runtime tocados: `xboxkrnl_video.cpp`, `gpu.cpp`):
1. Registrar por cada `VdSwap` los 10 argumentos (r3-r10 + pila) y `lr`; si el 2.º…4.º (`unk2-4`) varían entre 1 y 2, el juego codifica intervalo. Criterio: tabla de valores distintos y frecuencia.
2. Registrar `VdSetGraphicsInterruptCallback`, `VdInitializeEngines`, `VdSetDisplayMode`/`VdQueryVideoMode` (frecuencia declarada: 60 Hz) y cada escritura de la zona `[dev+16960..16990]` (hilo y `lr` por valor distinto; punto de vigilancia por muestreo en `gpu-vsync`).
3. Volcar `[dev+0..0x4400]` (dispositivo D3D guest; `dev` = r3 de `sub_82515D68`) y buscar el valor 2 / `D3DPRESENT_INTERVAL_TWO` (0x2) en `PresentationInterval`. Comprobación: `strings`/búsqueda en `default.xex` descifrado de `D3DPRESENT_PARAMETERS` por XDK (referencia: documentación XDK D3D9 `D3DPRESENT_INTERVAL_*`, [NV] disponible en local).
4. Contar las esperas: histograma de iteraciones del bucle 82515E6C por swap y llamadas a `KeWaitFor*`/`VdWaitForVBlank` (stubs en `import_stubs.gen.cpp`); verificar si el 2 proviene de comparar `[+16968]-[+16976] < 2` en `sub_82F5CB30` (desensamblar `ppc_gen` solo para lectura).
Salida: una de (A) el juego escribe intervalo 2 (parcheable en memoria), (B) el juego pide 1 y el 2 sale de que el manejador solo voltea cada 2.º vblank, (C) el 2 sale de cálculo de carga (el juego baja a 30 fps por no llegar a 16,7 ms; consistente con ~1,78 ms/draw histórico). Aceptación del hito: informe con valores observados y `lr`; comando: `SwapForceRT.exe --headless --trace-swap-interval --timeout 40 --perf-nosample > r1.out`.

## Hito 2 — Capacidad real del runtime sin espera
Opción nueva `--swap-no-wait` (`gpu.cpp`): `VsyncMain` genera un vblank por cada `OnSwap` completado (o a ≥240 Hz), sin pacing de 16,7 ms; no altera el cálculo de draws. Medir por escena (30 s cada una, 3 repeticiones, `--perf-nosample` para fps y `--perf` con muestreo para el desglose):
| Escena | Cómo alcanzarla | Métrica |
|---|---|---|
| Bink 1 / Bink 2 | arranque, fases por `NtCreateFile` del vídeo | VdSwap/s, `fps_vdswap` de `PERF` |
| Título | tras vídeos, antes de START | ídem |
| Menú principal | Enter, izquierda+Z (perfil), Story Mode | ídem |
| Dificultad | Story Mode → EASY/MEDIUM/… | ídem |
Desglose (`--perf`): ms/fotograma de `OnDraw_total`, `resolve`, `subida_texturas`, `fb_decode`, `espera_valla`, `OnSwap_total`; ocupación por hilo (`gpu-cp`, render guest). Criterio de decisión: con `--swap-no-wait` fps ≥ 70 en una escena → el límite es el pacing; si < 60 → hay que optimizar (Hito 4). Nota: con vblank acelerado, la lógica guest ligada a vblank/reloj se desordena; la medida es solo de capacidad, no de fidelidad.

## Hito 3 — Estrategias para 60 y experimento de fidelidad
Opciones (cada una exige el resultado del Hito 1):
- (a) Parchear el intervalo guest a 1 (si el Hito 1 da caso A). Riesgo: si la lógica es de paso fijo a 30 Hz, el juego acelera ×2; el audio XMA/Bink y la sincronización A/V pueden desfasarse.
- (b) Acortar la espera de vblank (cambiar el umbral de 2 a 1 en el manejador/bucle, o vblank a 120 Hz con cuenta de 2). Mismos riesgos que (a); en el caso 120 Hz+cuenta 2 el comportamiento guest no cambia (equivale a no hacer nada).
- (c) Presentar a 60 Hz repitiendo cada fotograma (la lógica queda a 30 Hz): no cambia la tasa de juego, solo la de presentación; el contenido nuevo seguirá siendo 30/s, por lo que **no equivale a 60 FPS de juego**; solo evita pérdida de presentación. Es lo que el usuario debe confirmar si acepta (ver Hito 6). Métrica: presentaciones/s (llamadas a `OnSwap` + repeticiones), separando «fotogramas únicos/s» (hash del frontbuffer distinto).
- (d) Interpolación: descartada (no fiel) salvo petición expresa.
Experimento de fidelidad para (a)/(b) (dos ejecuciones, base 30 vs. parcheada, mismos argumentos, `--det-clock` si existe [NV], si no con registro de tiempos):
1. Vídeo Bink: fotogramas Bink mostrados por segundo real (`BINKPLANE` con `--bink-trace`: hashes Y/U/V distintos/s) y duración total del vídeo en s de reloj; esperado = fps nativo del `.bik/.bk2` (leer cabecera) en ambas.
2. Título/transición de menú: tiempo de reloj entre dos PNG de referencia (`--dump-frames` por índice y viewer `/frame.jpg` con marca de tiempo) comparando animaciones (aparición de «PRESS START», logotipo); si la duración se reduce ≈50 % con (a), la lógica es de paso fijo.
3. Uso del tiempo por el juego: contar lecturas de `KeTimeStampBundle+16` (GetTickCount, `sub_82F42528`) y `mftb`/`KeQueryPerformanceCounter` por fotograma (traza nueva `--trace-time`); delta variable ⇒ lógica por tiempo (60 FPS seguro); sin lecturas por fotograma ⇒ paso fijo.
Criterio: duración de animaciones en tiempo de reloj idéntica (±3 %, [NV] umbral) entre 30 y 60 ⇒ (a)/(b) válidas; si no, solo (c).

## Hito 4 — Optimizaciones para sostener 60 (solo si Hito 2 < 60, priorizar por `--perf`)
Presupuesto: 16,7 ms/fotograma en CPU del CP + render guest. Candidatas (M8 ya hecho: resolve diferido, atajos, hashes, `pshufb`): CP por trampa en lugar de sondeo (`gpu-cp` al 100 % de un núcleo), hilos de resolve, caché de PSO en disco (`ID3D12PipelineLibrary`; tirón de ~280 ms), reducción de draws (600/fotograma menú), decodificación de texturas multihilo, spinlocks que cedan al SO. Cada cambio con su comparación byte a byte (`--tex-verify`, `--resolve-verify`, `--verify-shortcut`, `--fb-verify`: 0 distintos) y medición antes/después en las mismas escenas. Tarea previa: resolver o aislar el bloqueo tras `XMACreateContext` (M4).

## Hito 5 — Verificación y aceptación
Métrica sin ambigüedad: **VdSwap/s** = incrementos por segundo de `fps_vdswap` en líneas `PERF t=N` (hilo `perf-fps`, `--perf-nosample`). Para (c): **presentaciones/s** = número de llamadas efectivas de presentación por segundo, junto a fotogramas únicos/s.
Aceptación por escena (Bink 1, Bink 2, título, menú principal, dificultad): mínimo 30 s consecutivos con cada valor por segundo ≥ 58 (`python -I tools\m8\fpsstat.py salida.out`, ventanas de 30 s; sin procesos ajenos, `tools\m8\loadwatch.ps1`, «ajenos=0»); 3 ejecuciones consecutivas sin incumplimiento. Imagen: PNG por `--dump-frames`/`--frames-dir` abiertos y descritos por escena (sin banda roja, sin pantalla blanca, texto legible); 0 `shader_fail`, 0 `pso_fail`, 0 `removed`. Fidelidad: experimento del Hito 3 aprobado.
Script `tools\m60\measure_scene.sh` (nuevo): lanza `SwapForceRT.exe --headless --viewer 8771 --viewer-input --perf-nosample --perf perf.txt --timeout 240 --frames-dir f --log r.log > run.out`; por `curl http://127.0.0.1:8771/key?down=enter` (liberar con petición vacía) avanza: START (Enter) ×N hasta el diálogo de perfil → `down=left,z` (CONTINUE) → `down=z` (Story Mode) → captura `/frame.jpg` → `down=z` (dificultad) → captura. Marca de tiempo de cada fase en un CSV; fps por ventana de cada fase. Limitación conocida: tras seleccionar dificultad el juego carga `TA_VolcanoIntro` y muestra pantalla negra (≥2 min, [NV] si es carga o bloqueo): la escena «dificultad» se mide antes de pulsar Z.
Cierre: ningún `SwapForceRT.exe` propio vivo.

## Hito 6 — Riesgos y decisiones del usuario
1. Definición de «60 FPS»: (i) lógica a 60 (a/b; puede acelerar el juego), o (ii) presentación a 60 Hz (c; contenido a 30). Decisión requerida tras el Hito 1.
2. Fidelidad: si el juego es de paso fijo a 30 Hz, forzar 60 acelera lógica, animaciones y audio ⇒ no admisible sin autorización.
3. Ventana: la medida con ventana (`Present(1,0)`, vsync del monitor) no se ha hecho; el bloqueo de `Present` puede limitar a 30/60 según monitor [NV]. Requiere autorización para abrir ventana (`--window-noactivate`).
4. Bloqueo de M4 (`XMACreateContext`) y pantalla negra post-dificultad pueden impedir cubrir toda la ruta; incluir su resolución o declarar el alcance reducido.
5. Carga concurrente del equipo falsea las medidas (paradas de 1-58 s documentadas): mediciones solo con `loadwatch` limpio.
6. Cambios en `runtime\src\gpu\**` y `main.cpp` tocan áreas con propietario (AGENTS.md): avisar y anotar en `PROGRESS.md`.
Orden: Hito 1 → 2 (paralelizables, ≈1 sesión cada uno) → decisión del usuario → 3 → 4 si procede → 5.
