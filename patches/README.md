# patches/

`XenonRecomp-swapforce.patch`: diff unificado contra el upstream **hedge-dev/XenonRecomp**, commit
`ddd128bcca99fe8bfbb99bea583c972351fa6ace` (rama `main`, 2025-08-04, autor Skyth (Asilkan)). Licencia del upstream: MIT.

Ficheros modificados (`git diff --stat`): `XenonAnalyse/function.cpp` (+34), `XenonAnalyse/function.h` (+5), `XenonAnalyse/main.cpp` (+127), `XenonRecomp/recompiler.cpp` (+310); 472 inserciones, 4 supresiones.
Generado con `git diff HEAD` en el árbol de trabajo del proyecto; verificado que las copias `*.orig` (estado previo a cada edición) son idénticas al commit upstream. Las copias intermedias `main.cpp.pre_nop`, `main.cpp.pre_cmpwi` y `recompiler.cpp.pre_clamp` son estados intermedios y no se publican; el patch recoge el estado final.

Aplicación (la hace `setup.ps1`): `git clone https://github.com/hedge-dev/XenonRecomp && git -C XenonRecomp checkout ddd128bcca99fe8bfbb99bea583c972351fa6ace && git -C XenonRecomp submodule update --init --recursive && git -C XenonRecomp apply XenonRecomp-swapforce.patch`.

Contenido funcional (según `PROGRESS.md`): 49 opcodos añadidos a `recompiler.cpp`; patrón de tablas de salto `lis, rlwinm, addi, lwzx, mtctr, bctr` y escaneo tolerante a `nop` en XenonAnalyse; seguimiento de casos de tablas de salto y recorte de funciones al siguiente inicio conocido. El opcode `mulchw.` emite `__builtin_debugtrap()` (semántica no verificada).
