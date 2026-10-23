# THIRD_PARTY_NOTICES

Fecha de verificación: 2026-10-11. Marca **[NV]** = no verificado. Los textos íntegros de licencia están en `third_party/licenses/`.
Este repositorio no contiene ficheros de Activision ni binarios de terceros (`.exe`, `.dll`, `.lib`).

## 1. Código de terceros incluido en este repositorio

### 1.1 Xenia (xenia-project/xenia) — BSD-3-Clause
- Licencia: `third_party/licenses/Xenia-LICENSE.txt` (obtenida de `https://raw.githubusercontent.com/xenia-project/xenia/master/LICENSE`; "Copyright (c) 2015, Ben Vanik").
- Ficheros copiados sin modificar a `runtime/src/gpu/xenia/gpu/`: `xenos.h`, `ucode.h`, `registers.h`, `register_file.h`, `register_table.inc`, `texture_address.h`, `xenos_tables.cpp`, `ucode_tables.cpp`, `registers_tables.cpp`.
- Verificación (2026-10-11, SHA-256): los 9 ficheros son idénticos byte a byte a `xenia-project/xenia`, rama `master`, commit `95a5c3ee250f80c3b9d139658649d9ffb6db3eec` (2026-02-18): `src/xenia/gpu/{xenos.h, ucode.h, registers.h, register_file.h, register_table.inc, texture_address.h}` y `{xenos.cc, ucode.cc, registers.cc}` (renombrados `*_tables.cpp`). Commit exacto desde el que se copiaron originalmente: [NV].
- Cabecera BSD original conservada en los 9 ficheros ("Copyright <año> Ben Vanik. All rights reserved. Released under the BSD license - see LICENSE in the root for more details."): verificado por lectura.
- `runtime/src/gpu/xenia/base/*.h` (5 ficheros): sustitutos mínimos escritos para este proyecto (no son copia de Xenia; el comentario de `assert.h` los describe como "shim"). Se publican bajo la licencia propuesta del proyecto.
- Ficheros propios que citan o portan lógica de Xenia (consultada en `runtime/ref/`, no incluido): 29 ficheros de `runtime/src/gpu`, `kernel`, `memory`, `thread` (búsqueda `grep -il xenia`). Grado de derivación respecto a Xenia: **[NV]**; se recomienda mantener la atribución a Xenia (BSD-3 exige conservar aviso de copyright y condiciones en las partes derivadas).
- `runtime/ref/` (copias de ficheros de Xenia y de XenosRecomp) NO se incluye: se cita en lugar de copiarse.

### 1.2 DirectX Shader Compiler — cabeceras en `runtime/src/gpu/dxc/`
- `dxcapi.h`, `dxcerrors.h`: cabecera "distributed under the University of Illinois Open Source License" (NCSA); texto en `third_party/licenses/DXC-LICENSE-LLVM-NCSA.txt`.
- `d3d12shader.h`: cabecera "Copyright (c) Microsoft Corporation. Licensed under the MIT license."; texto en `third_party/licenses/DXC-LICENSE-MIT.txt`.
- Verificación: SHA-256 idéntico a `inc/` del paquete oficial `dxc_2024_07_31.zip` (release `v1.8.2407` de `microsoft/DirectXShaderCompiler`).

## 2. Componentes NO incluidos (descargados por `setup.ps1` con SHA-256 fijado, o solo citados)

| Componente | Uso | Licencia (verificada en el fichero indicado) | Fuente |
|---|---|---|---|
| hedge-dev/XenonRecomp, commit `ddd128bcca99fe8bfbb99bea583c972351fa6ace` (2025-08-04) | Recompilador PPC→C++ y analizador. Se clona y se compila en local | MIT, "Copyright (c) 2025 hedge-dev and contributors" (`XenonRecomp-LICENSE.md`) | https://github.com/hedge-dev/XenonRecomp |
| `patches/XenonRecomp-swapforce.patch` | Modificaciones propias de `XenonAnalyse/{main.cpp,function.cpp,function.h}` y `XenonRecomp/recompiler.cpp` (472 inserciones, 4 supresiones). Obra derivada de código MIT: conserva la licencia MIT de hedge-dev | MIT | ver arriba |
| Submódulos de XenonRecomp (commits en `git submodule status`): fmt (MIT), xxHash (BSD-2-Clause), tomlplusplus (MIT), tiny-AES-c (Unlicense), simde (MIT), libmspack (LGPL-2.1, `COPYING.LIB`), TinySHA1, `thirdparty/disasm` (derivado de GNU binutils: cabecera GPL) | Dependencias del build de XenonRecomp; no se redistribuyen. El runtime solo usa `simde` por ruta de inclusión | Leídas las cabeceras/ficheros LICENSE locales; versión exacta de la GPL en `disasm`, y licencia de TinySHA1: **[NV]** | repositorios de cada submódulo |
| hedge-dev/XenosRecomp (MIT, `XenosRecomp-LICENSE.md`) | Solo referencia (el traductor de ucode del runtime es propio; el nivel de inspiración/derivación de `ucode_hlsl.cpp` respecto a XenosRecomp es **[NV]**). No se descarga | MIT | https://github.com/hedge-dev/XenosRecomp (commit local `990d03b28a27b50277ee5d8d942e1c5f873869d1`) |
| extract-xiso `build-202609111233` (`extract-xiso-Win64_Release.zip`, SHA-256 `fc891dfa72f08e3838a2f8b7ec34a8e4d61678da1cc8fee879677f00bf114ce3`) | Extracción de la ISO | BSD modificado con **cláusula de publicidad** (apartado 3) y copyright "in <in@fishtank.com>" (`extract-xiso-LICENSE.txt`). No se redistribuye el binario | https://github.com/XboxDev/extract-xiso/releases/tag/build-202609111233 |
| DirectX Shader Compiler `v1.8.2407` (`dxc_2024_07_31.zip`, SHA-256 `e2627f004f0f9424d8c71ea1314d04f38c5a5096884ae9217f1f18bd320267b5`) | `dxcompiler.dll` y `dxil.dll` (x64), copiadas junto a `SwapForceRT.exe` | Véase 2.1 | https://github.com/microsoft/DirectXShaderCompiler/releases/tag/v1.8.2407 |
| Herramientas de compilación: VS Build Tools 2022 (MSVC 14.44, SDK 10.0.22621/26100), LLVM/clang-cl 23.1.3, CMake 4.4.4, Ninja 1.13.2, Python 3.13.14, pycryptodome 3.24.0 | Requisitos del entorno; no se redistribuyen | Licencias de cada uno: **[NV]** (no leídas) | — |
| Xenia `runtime/ref/*` y UnleashedRecomp | Referencias de consulta (no copiadas) | Xenia BSD-3; UnleashedRecomp GPL-3.0 (campo `license` de la API de GitHub, 2026-10-11) | https://github.com/xenia-project/xenia · https://github.com/hedge-dev/UnleashedRecomp |

### 2.1 `dxcompiler.dll` y `dxil.dll`: condiciones de redistribución
Fuente: ficheros del paquete oficial `dxc_2024_07_31.zip` (leídos íntegramente) y README del repositorio.
- El paquete incluye `LICENSE-LLVM.txt` (NCSA), `LICENSE-MIT.txt` y `LICENSE-MS.txt`; `ReleaseNotes.md` indica que `LICENSE-MS.txt` aplica a `dxil.dll`.
- `LICENSE-MS.txt` ("MICROSOFT SOFTWARE LICENSE TERMS – MICROSOFT DIRECTX SHADER COMPILER"), sección DISTRIBUTABLE CODE: se puede distribuir la forma objeto del software "listed in the distributables file list in the software", con requisitos: añadir "significant primary functionality", exigir a distribuidores y usuarios finales términos que protejan al software y a Microsoft "at least as much as this agreement", e indemnizar a Microsoft. Además, SCOPE OF LICENSE prohíbe "share, publish, distribute ... the software (except for any distributable code, subject to the terms above)".
- **[NV]** Si `dxil.dll` figura en la "distributables file list" (no se ha localizado esa lista en el paquete) y si el cumplimiento de los requisitos de distribución es viable para este proyecto.
- **Decisión adoptada: no se incluyen las DLL en el repositorio.** `setup.ps1` las descarga del release oficial y verifica el SHA-256 del ZIP. Comprobado: las DLL del paquete oficial son idénticas (SHA-256) a las de `renderbag/dxc-bin` usadas hasta ahora (`dxcompiler.dll` `1220478c87af97551d2342906c5a08d888ee9fa26f6f99b6072d674801f492d1`, `dxil.dll` `e038a9755387fc396a9a5f393d292b9318cf6ac7e520e9283bbc3088c5f8ac93`, versión de fichero 1.8.2407.7); `renderbag/dxc-bin` no contiene fichero de licencia.
- `dxcompiler.dll`: LLVM/NCSA (README de DXC: "distributed under the terms of the University of Illinois Open Source License").

### 2.2 FFmpeg / libavcodec (LGPL) — decodificación XMA2 (descargado 2026-10-23; NO incluido en el repositorio)
- Uso: `runtime/src/audio/xma_ffmpeg.cpp` carga con `LoadLibrary` (en tiempo de ejecución, sin enlace estático ni de importación) `avcodec-62.dll`, `avutil-60.dll` y `swresample-6.dll`, y usa el decodificador `xma2` (`AV_CODEC_ID_XMA2`, `libavcodec/wmaprodec.c`). Las cabeceras de `include/` solo se usan al compilar. Aceptado por el usuario (`CLAUDE.md`: FFmpeg/libavcodec LGPL como DLL dinámica).
- Origen: `https://github.com/BtbN/FFmpeg-Builds/releases/download/latest/ffmpeg-n8.1-latest-win64-lgpl-shared-8.1.zip` (BtbN/FFmpeg-Builds, variante `win64-lgpl-shared`, rama de lanzamiento n8.1). Tamaño 80 986 075 B. SHA-256 del ZIP: `54f1ae665c29bce59894cc0cbe965286494c1c24981ac09bb744e6ba369de6db` (coincide con el `digest` publicado por la API de GitHub el mismo día).
- Versión: `ffmpeg version n8.1.3-24-g92611900bb-20261023`; `libavcodec 62.28.103`, `libavutil 60.26.103` (salida de `ffmpeg.exe -version`).
- Licencia verificada en el paquete: `LICENSE.txt` es la GNU LGPL v3 (29 junio 2007); `ffmpeg.exe -buildconf` incluye `--enable-version3` y NO incluye `--enable-gpl` ni `--enable-nonfree` (LGPL v3). Texto en `runtime/third_party_bin/ffmpeg/LICENSE.txt` (carpeta ignorada por git).
- SHA-256 de las DLL usadas: `avcodec-62.dll` `c0286533a1445f57358f274ba060ceb5f0596cd23db3fc6873ed3c5092dfd446`; `avutil-60.dll` `4275ae1e121f8d8b1af6a180905b3036287df6c5164e5fdb83865029a4b6fefd`; `swresample-6.dll` `37626c307191d44a9eea9bd8d1c365245085dfed2811cabb7d9652578e05286a`. (Sustituir la DLL por otra compatible es posible: carga dinámica, requisito LGPL de relinking.)
- Ubicación local: `runtime\third_party_bin\ffmpeg\` (en `.gitignore`); el post-build de CMake copia las DLL junto a `SwapForceRT.exe` si existen. Sin cabeceras/DLL el runtime compila y XMA queda en silencio.
- Obligaciones de redistribución del binario (aviso, texto LGPL, acceso al código fuente de FFmpeg `n8.1`, posibilidad de sustituir las DLL): **[NV]** pendientes de asesoría; hoy no se redistribuye nada.
