# Coordinación de agentes (Claude + Codex) — leer antes de tocar nada

Contexto completo del proyecto: `CLAUDE.md`. Detalle de hitos y criterios de aceptación: `PLAN_FIDELIDAD.md`. Resultados: `PROGRESS.md`.

## Reglas de trabajo (organización FeeX)
Español formal y escueto; solo datos objetivos; citar documentación oficial/fuentes; no inventar: marcar [NV]; no afirmar que algo funciona sin ejecutar el comando de aceptación; distinguir "compila" de "se ha ejecutado".

## Reparto por propietario (no editar ficheros de otro propietario sin avisar)
| Propietario | Alcance | Rutas |
|---|---|---|
| CPU (Claude) | M1, M3: recompilador, tablas de salto, tests de opcodes | `tools\XenonRecomp\**`, `runtime\ppc_gen\**` (SOLO este propietario regenera), `runtime\recomp\**`, `runtime\src\selftest\**` |
| Audio/entrada (Codex) | M4 XMA, M5 entrada y Portal USB | `runtime\src\kernel\xboxkrnl_audio.cpp`, `runtime\src\kernel\xam_input.cpp`, ficheros nuevos `runtime\src\audio\**`, `runtime\src\hid\**` |
| Gráficos | M6, M7 (fase posterior) | `runtime\src\gpu\**` |
| Compartido (cambios mínimos, avisar) | `runtime\CMakeLists.txt`, `runtime\src\main.cpp`, `runtime\src\thread\**` | — |

## Aislamiento obligatorio
1. Un `git worktree` y una rama por agente; unión a `main` solo tras pasar su aceptación por comando.
2. Directorio de build propio: `-DSWAPFORCE_OUT_DIR=<ruta propia>` (CMake lo admite). Nunca compilar sobre el `runtime\build` de otro.
3. `userdata\` y `runtime.log` propios por ejecución (el volcado de PNG sobrescribe).
4. `runtime\ppc_gen` es de solo lectura para todos salvo el propietario CPU. Si necesitas una función que falta, pídelo y deja nota en `HANDOFF` del apartado de tu propietario en `PROGRESS.md`.
5. `PROGRESS.md`: cada agente escribe solo en su apartado (`## <Propietario>`); no reescribir los ajenos.
6. Ningún contenido del juego en el repositorio (ISO, XEX, `.pak`, `ppc_gen`, shaders volcados, PNG): ver `.gitignore` y `LEGAL_RESUMEN.md`.
7. Sin `git push` ni cambios de visibilidad del repositorio sin autorización expresa del usuario. Sin instalar software sin autorización.
8. Procesos con timeout; sin `SwapForceRT.exe` vivos al terminar; redirigir la salida estándar a fichero (es muy voluminosa).

## Dependencias entre hitos
M4 y M5 dependen en parte de que el flujo del juego avance (M1: `vupkd3d128`, `vsldoi128`). Hasta que M1 cierre, M4/M5 se verifican con pruebas aisladas (selftests propios), no con el juego completo.
