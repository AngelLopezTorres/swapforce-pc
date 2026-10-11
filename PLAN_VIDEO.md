# Plan capa de vídeo mínima (elaborado por Opus; ejecuta Sonnet). [NV] = comprobar antes de implementar.
Referencias: Xenia (xboxkrnl_video.cc, graphics_system.cc, command_processor.cc, xenos.h, memory.cc, mmio_handler.cc) -> descargar con curl a runtime\ref\. Microsoft Learn (VirtualAlloc2, MapViewOfFile3).

## Fase 1 — causa de la violación en 0x7FC8112C (diagnosticada)
- No la provocan los stubs Vd*: el juego lee un registro GPU directamente. runtime\ppc_gen\ppc_recomp.443.cpp, sub_82F68160 (~línea 24444): `lis r7,32712; lwz r7,4396(r7)` = lee 0x7FC80000+0x112C. GuestMemory no mapea 0x7FC80000-0x7FC8FFFF.
- Xenia: rango 0x7FC80000-0x7FC8FFFF = registros GPU, índice = (addr&0xFFFF)/4 [NV].
- Accesos (grep -c "= 2143813632;" = 8, en 442/443/444.cpp): escritura 0x714 (idx 0x1C5, CP_RB_WPTR) en sub_82F580B8/sub_82F588D0; lectura 0x6544 (idx 0x1951, bit0 = estado interrupción; Xenia devuelve 1) en sub_82F58198 (callback gráfico); escritura 0x6110 (idx 0x1844, D1GRPH_PRIMARY_SURFACE_ADDRESS = presentación) en sub_82F5C128/82F5C258; lectura 0x112C (idx 0x44B) y 0xE5C (0x397) en sub_82F68160/sub_82F6DFB0; escritura 0x3214/0x3408 (idx 0xC85/0xD02) en sub_82F6EC10. Puede haber más accesos (base en otro registro): medir en ejecución.
- Args Vd* del log: VdInitializeRingBuffer(0x0BF20000,0xD); VdEnableRingBufferRPtrWriteBack(0x014E003C,7); VdSetSystemCommandBufferGpuIdentifierAddress(0xA14E0008); VdSetGraphicsInterruptCallback(0x82F58198,0x40003980); VdInitializeEngines(0x53020000,0x82F6EC10,0,0x821DB018).
- MmAllocatePhysicalMemoryEx r5=0x20000004/0x20000404; Xenia: heap por tamaño de página (4K->0xE0000000, 64K->0xA0000000, 16M->0xC0000000) [NV]. El juego usa vista 0xC0000000 y +0x1000 para 0xE0000000.
- 1.1 Mapear 0x7FC80000-0x7FC8FFFF; aceptación: `grep dirGuest=7FC8 runtime.log` vacío y imports > 709025.
- 1.2 Dos copias (la del juego y una interna GPU); `--gpu-regdump` registra cada índice que cambia (`GPUREG W idx=0x1C5`).
- 1.3 El handler SEH clasifica accesos en 0xA0000000-0xFFFFFFFF para saber si hacen falta vistas 0xC0/0xE0.

## Fase 2 — Vd*, interrupción, ring buffer, rptr
- 2.1 Memoria física: sección 512 MB (CreateFileMapping) en 0xA0/0xC0/0xE0000000 (+0x1000 en la última [NV]) con VirtualAlloc2 placeholder + MapViewOfFile3. Aplazar si 1.3 mide 0 accesos fuera de 0xA0. `--selftest-phys`: escribir 0xA0001000, leer en 0xC0001000 y 0xE0002000.
- 2.2 kernel\xboxkrnl_video.cpp [NV firma de cada una]: XGetVideoMode/VdQueryVideoMode 1280x720 60Hz; VdInitializeEngines->1; VdSetGraphicsInterruptCallback; VdInitializeRingBuffer (tamaño 1<<(log2+3) [NV]); VdEnableRingBufferRPtrWriteBack; VdGetSystemCommandBuffer, VdSetSystemCommandBufferGpuIdentifierAddress, VdPersistDisplay, VdRetrainEDRAM, VdIsHSIOTrainingSucceeded, VdGetCurrentDisplayInformation/Gamma, VdEnableDisableClockGating, VdShutdownEngines, VdSwap [NV formato]; VdGpuClockInMHz [NV]. Aceptación: `grep -E "import Vd.* stub" runtime.log` vacío.
- 2.3 Hilo GPU (gpu\command_processor): sondea CP_RB_WPTR (1 ms), avanza rptr y lo escribe big-endian en la dirección de write-back. Aceptación: log `RB wptr=… rptr=…` con rptr=wptr; el juego no se bloquea esperando ring.
- 2.4 Hilo vsync 60 Hz ejecuta el callback con r3=0, r4=argumento, contexto guest propio; paquetes INTERRUPT -> r3=1. Registro 0x1951 lee 1; 0x194C y 0x1961 [NV].
- 2.5 KeInitializeDpc/KeInsertQueueDpc/KeRemoveQueueDpc con hilo DPC (ref runtime\ref\xboxkrnl_threading.cc:1130-1185).
- 2.6 Frames: contar escrituras en idx 0x1844 o VdSwap. Aceptación: `grep -c "FRAME " runtime.log` > 60 con `--timeout 60`.

## Fase 3 — PM4 con log, sin dibujar
- gpu\pm4_parser: tipo en bits 31:30; tipo 0 reg base 14:0, n=29:16+1; tipo 1 dos regs; tipo 2 NOP; tipo 3 opcode 15:8, n=29:16+1 [NV]. Opcodes copiados de xenos.h: INDIRECT_BUFFER, ME_INIT, NOP, WAIT_REG_MEM, REG_RMW, COND_WRITE, EVENT_WRITE(+SHD/EXT), DRAW_INDX(_2), SET_CONSTANT(2), LOAD_ALU_CONSTANT, SET_SHADER_CONSTANTS, IM_LOAD(_IMMEDIATE), INVALIDATE_STATE, INTERRUPT, presentación. Registros 0x5003 [NV]. Log gpu.log con límite por frame.
- Aceptación: histograma de opcodes por frame sin UNKNOWN; ≥1 DRAW_INDX por frame; direcciones de IM_LOAD en memoria asignada.

## Fase 4 — shaders (medido)
- iso_extract\archives\shaders_xenon.pak: 592025 B, cabecera BE 1A414749, versión 10, 62 entradas, sector 0x800, tabla de nombres 0x90000, 61 `shaders\*.igz`; contenido ilegible, compresión sin identificar [NV]. Contenedor Xenos 0x102A11xx: 0 veces en 2231 .pak.
- analysis\image.bin: 19 contenedores (criterio XenosRecomp main.cpp:91) desde 0x82254FF8 (.rdata), probablemente shaders internos D3D [NV]. XEX: 62 rutas `shaders:/*.fx`; D3D9 de la XDK enlazada.
- 4a Captura en ejecución: tras cargar el .pak buscar 0x102A11xx en memoria guest y volcar a runtime\userdata\shaders\. Aceptación: >19 volcados. 4b Volcar microcódigo de IM_LOAD (falta tabla de constantes). 4c Formato IGA/IGZ: ingeniería inversa [NV].

## Fase 5 — backend (decidir con datos de 3 y 4; NO ejecutar sin confirmación)
- A: emulación bajo nivel (Xenia gpu/d3d12, vulkan). B: traducción alto nivel interceptando D3D9 de la XDK + shaders XenosRecomp (DXIL/SPIR-V). Sin pantalla: tests del parser sobre ring grabado, render offscreen a PNG con hash ≠ negro.

## Riesgos
Lista de registros incompleta; sondeo de wptr sin orden; posible cambio del modelo de memoria física; compresión IGA; 49 opcodos no verificados en ejecución.
