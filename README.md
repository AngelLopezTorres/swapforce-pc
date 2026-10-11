# SwapForce-PC

Port experimental a PC (Windows, Direct3D 12) de la versión Xbox 360 de *Skylanders: Swap Force*, mediante recompilación estática (XenonRecomp) y un runtime propio (kernel, memoria, E/S, GPU Xenos→D3D12).
Proyecto no oficial, sin relación con Activision ni con sus licenciantes. Los nombres de producto se citan solo para identificar el software compatible.

## Contenido del repositorio y exclusiones

**Este repositorio NO contiene ningún fichero del juego ni derivado de su código o recursos.** No incluye: la ISO, `default.xex`, ningún `.pak`/`.bk2`/`.xzp`, la imagen descifrada, el código C++ generado por el recompilador, shaders ni fotogramas volcados, ni binarios (`.exe`, `.dll`).
Incluye únicamente: código propio del runtime (`runtime/src`), scripts de análisis y generación de configuración (`analysis/`, `runtime/recomp/`, `runtime/tools/`), el patch propio sobre XenonRecomp (`patches/`), y documentación.

**Se requiere la ISO del propio usuario** (copia legítima del juego). Todo lo derivado del juego se genera en local con `setup.ps1` y está excluido por `.gitignore`.

## Estado real (2026-10-11)

- **No jugable.** El juego arranca, inicializa vídeo, carga y sube shaders, y ejecuta su bucle principal; no hay audio, ni entrada (H6 no iniciado), ni portal USB.
- La imagen presentada es **negra**: la composición final usa los planos Y/U/V de un vídeo **Bink 2** que permanecen constantes (Y=0x10, U=V=0x80); no se ha decodificado Bink ni se ha investigado por qué el vídeo no avanza.
- Rendimiento medido: **7–9 fotogramas/s** (sincronización por *resolve* y destilado del frontbuffer por CPU).
- Traductor de shaders: 55/55 shaders observados compilan a DXIL; solo los de limpieza se han contrastado numéricamente. Simplificaciones de EDRAM, MSAA y profundidad registradas en `PROGRESS.md`.
- Verificado únicamente en: Windows 11 Pro 10.0.26200, NVIDIA GeForce RTX 4070 SUPER (también WARP). Otros equipos: [NV].
- Detalle, cifras y puntos no verificados [NV]: `PROGRESS.md`, `PLAN_RUNTIME.md`, `PLAN_VIDEO.md`, `PLAN_GPU.md`.

## Requisitos

| Requisito | Versión usada en el desarrollo |
|---|---|
| Windows 11 x64, GPU con Direct3D 12 | 10.0.26200 |
| Visual Studio Build Tools 2022 (MSVC, Windows SDK) | MSVC 14.44, SDK 10.0.22621/26100; ruta fijada en `runtime/tools/vsenv.cmd` |
| LLVM (clang-cl, lld-link) | 23.1.3, en `C:\Program Files\LLVM\bin` (fijado en `vsenv.cmd` y `runtime/CMakeLists.txt`) |
| CMake, Ninja | 4.4.4, 1.13.2 |
| Python y `pycryptodome` | 3.13.14, 3.24.0 |
| Git | — |
| ISO propia de *Skylanders: Swap Force* (Xbox 360) | Probada: Europe (En,De), 8.738.846.720 B, XGD2. Otras ediciones: [NV] |
| Espacio en disco / RAM | ≥ 20 GB libres (se estima; extracción ≈ 8,4 GB, `ppc_gen` ≈ 0,24 GB, build ≈ 0,3 GB); ~410 MB de RAM por trabajo de compilación (medido) |
| Acceso a internet | GitHub (XenonRecomp, extract-xiso, DXC) |

## Uso

```
powershell -NoProfile -ExecutionPolicy Bypass -File setup.ps1 -IsoPath "D:\ruta\a\mi_copia.iso"
```
`setup.ps1` ejecuta, en este orden: descarga de `extract-xiso` (SHA-256 fijado) → extracción de la ISO a `iso_extract\` → descifrado/descompresión de `default.xex` a `analysis\image.bin` → clonado de `hedge-dev/XenonRecomp` en el commit `ddd128b…` + `patches/XenonRecomp-swapforce.patch` + compilación → XenonAnalyse → `iterate_extents.py` → `find_missing_switch.py` → `prune_loop.py` (XenonRecomp → `runtime\ppc_gen`) → descarga de DXC v1.8.2407 (SHA-256 fijado) → compilación (`runtime\build\SwapForceRT.exe`, ≈ 125 s con 18 trabajos en el equipo de desarrollo).
Ejecución (desde `runtime\`): `build\SwapForceRT.exe` (opciones en `PROGRESS.md`, sección GPU).
Los resultados intermedios deterministas se resumen en `setup_hashes.txt` (SHA-256 de `runtime\ppc_gen`).

## Licencias

- Código propio: licencia MIT **propuesta, no impuesta** (`LICENSE`); pendiente de decisión del titular.
- Terceros (Xenia BSD-3, XenonRecomp MIT, DXC NCSA/MIT/Microsoft, extract-xiso, etc.): `THIRD_PARTY_NOTICES.md` y `third_party/licenses/`.
- Consideraciones jurídicas pendientes de asesoría: `LEGAL_RESUMEN.md`.
