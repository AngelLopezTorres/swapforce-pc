# PLAN_PORTAL — XamInputNonControllerGetRaw (0x4A1) y SetRaw (0x4A2) con paso directo al Portal físico
Fecha 2026-10-23. Solo plan; nada ejecutado salvo lectura de ficheros. [NV] = no verificado.
Fuentes: `runtime\src\hid\portal.cpp/.h`, `runtime\src\kernel\xboxkrnl_modules.cpp` (líneas 55-66, 120-135), traza `runtime\build_live\v\runtime.log` (líneas 17255-18606, 26155), y Xenia Canary rama `canary_experimental` (`xam_input.cc` l.257-326, `hid/portal/portal.h`, `hardware_portal.h`; copias de `portal.cc`/`hardware_portal.cc` en `runtime\ref\` del repositorio principal; las descargas de este plan están en `/tmp/cn`, fuera del repositorio).
Autorización de escritura: `TAREA_CODEX.md` ronda 8 (solo bytes que entrega el juego en SetRaw; sin comandos propios).

## 1. Firma (deducida y contrastada)
Xenia Canary (`xam_input.cc`):
- `XamInputNonControllerGetRaw(lpword_t state_ptr, lpdword_t buffer_length_ptr, lpdword_t buffer_ptr)` = `GetRawEx(5, buffer_ptr, length_ptr, state_ptr)`.
- `XamInputNonControllerSetRaw(dword_t buffer_length, lpdword_t buffer_ptr)` = `SetRawEx(5, buffer_ptr, length)`.
- `kPortalBufferSize = 0x20`. Validaciones: punteros no nulos; longitud 1..0x20, si no `X_ERROR_INVALID_PARAMETER` (0x57). Sin dispositivo: `X_ERROR_DEVICE_NOT_CONNECTED` (0x48F, ERROR_DEVICE_NOT_CONNECTED, winerror.h). Éxito: 0; la longitud se actualiza con los bytes leídos y `*state_ptr` (u16) = `previous_status_ == status`. Un timeout de lectura se devuelve como éxito con 0 bytes. Xenia reconoce que esta semántica de `state` es una conjetura (comentario en `portal.cc`) [NV].
Traza del juego (coincide):
- r3 = 703BFCD4 = `state_ptr` (u16, vale 0 antes de la llamada); r4 = 703BFCD0 = `buffer_length_ptr`, contiene `00000020` (entrada: capacidad 32 B; salida: bytes leídos); r5 = 703BFCE0 = `buffer_ptr` (32 B, contiene basura de pila antes de la llamada; 703BFCE0 - 703BFCD0 = 0x10, por lo que los 32 B del buffer están junto a las variables locales). r6..r10 = restos de registros, no son argumentos [NV: confirmar que no hay 4.º argumento comparando varias llamadas; las llamadas 2+ muestran r6=r7=83410000, r9=00000010, que no son punteros a datos útiles].
- SetRaw: r3 = longitud, r4 = puntero al buffer (aún no hay traza de SetRaw: el juego no la ha invocado) [NV].
- Valor de retorno actual STATUS_NOT_SUPPORTED (0xC00000BB) hace que el juego muestre «PORTAL DISCONNECTED». Qué códigos consume exactamente: [NV]; hipótesis a verificar: 0 = conectado, 0x48F = desconectado.
- Conclusión: IDs de dispositivo fijos en 5 en las variantes sin «Ex».
Discrepancia a resolver: Xenia lista VID/PID 1430:1F17 y 24C6:FA00 (Portal de Xbox 360); el dispositivo del usuario es 1430:0150 (Portal de otra plataforma). Que el protocolo de 32 B sea idéntico es [NV]; el paso directo no depende de ello.
Primer hito sin riesgo: ampliar `LogRawCall` para registrar también el valor de retorno y `*state`, `*len` posteriores.

## 2. Diseño de lectura (no bloqueante)
- Nueva clase `PortalDevice` en `runtime\src\hid\portal.cpp` (singleton, inicio perezoso en la primera llamada de GetRaw/SetRaw, no en el arranque). Reutiliza `FindPortalPath`, `LibUsbKApi` (añadir `UsbK_WritePipe`, `UsbK_AbortPipe`, `UsbK_ResetPipe`, `UsbK_FlushPipe` con `GetProcAddress`; solo Abort y Write son necesarios).
- Hilo lector propio (`std::thread`): bucle {abrir si cerrado -> `UsbK_ReadPipe(0x81, 32 B, timeout 100 ms)` -> si OK con n>0 poner el informe en una cola circular (p. ej. 64 elementos, `{n, bytes[32]}`) bajo `std::mutex`; timeout = sin dato; `ERROR_DEVICE_NOT_CONNECTED/ERROR_GEN_FAILURE/ERROR_NO_SUCH_DEVICE` = cerrar, marcar `connected=false`, reintentar abrir cada 500 ms}.
- Reabrir: `FindPortalPath` cada 500 ms; en cada apertura registrar interfaz 0, endpoints (IN 0x81, OUT: leer del descriptor con `UsbK_QueryPipe` el primer endpoint interrupt/OUT; Xenia usa 0x02 [NV: confirmar en el dispositivo]) y `MaximumPacketSize`.
- GetRaw (hilo guest, nunca toca USB): valida r3/r4/r5 como Xenia; si `!connected` -> devuelve 0x48F; si conectado, desencola un informe (si cola vacía: éxito con longitud 0 y `state` coherente con Xenia); copia `min(n, cap)` bytes con `memcpy` al buffer guest, escribe `*len = n` y `*state` con `__builtin_bswap32/16` (el guest es big-endian; el informe es un flujo de bytes, se copia tal cual sin intercambiar) y devuelve 0.
- Orden: los informes se conservan en orden FIFO; si la cola se llena se descarta el más antiguo y se cuenta (contador en log).
- `state`: reproducir la regla de Xenia (`previous_status_ == status`) como valor inicial; afinar con la traza real [NV].

## 3. Diseño de escritura
- SetRaw: valida (len 1..0x20, puntero no nulo, base+puntero legible); copia los bytes del juego a un búfer local; los entrega al hilo del dispositivo mediante cola de escritura (máx. 16) y espera con tope de 150 ms al resultado (o retorna 0 de inmediato si el resultado es opcional [NV: decidir según traza]); el hilo hace `UsbK_WritePipe(endpoint OUT, bytes, len)` con timeout 100 ms (política `PIPE_TRANSFER_TIMEOUT`). Sin modificar, sin rellenar, sin fragmentar, sin reordenar.
- Prohibido: cualquier escritura no originada por SetRaw (ni en apertura, ni en reconexión, ni en cierre; sin `SET_REPORT`, sin LED, sin reset).
- Si no hay dispositivo: devolver 0x48F y no encolar.
- Registro (`ImportLog`, una línea por transferencia, sin muestreo): `PORTAL USB dir=IN|OUT ep=0x81|0xNN len=N data[32]=HEX res=OK|TIMEOUT|ERR(win32) t_ms`. Las lecturas con timeout se cuentan en agregado (1 línea/segundo) para no inundar el log.

## 4. Verificación sin ver la pantalla
Compilar en directorio propio (`runtime\build_codex_round8`), puertos 8771-8779, nunca `build_live`:
1. Build; `--selftest-portal --headless` debe seguir en OK (lectura/enumeración, sin escrituras).
2. `SwapForceRT.exe --headless --viewer 8771 --viewer-input --timeout 240 > ronda8.out 2> ronda8.err` con log propio.
3. Esperar ~90 s (vídeos Bink) y `curl -s "http://127.0.0.1:8771/key?down=enter"` repetido cada 2 s durante 20 s (pulsación) seguido de `curl -s "http://127.0.0.1:8771/key?down="` para liberar.
4. `curl -s -o f_N.jpg http://127.0.0.1:8771/frame.jpg` a intervalos de 10 s; abrir con Read. Criterio: el texto «PORTAL DISCONNECTED» deja de verse o el hash/aspecto del frame cambia a otra pantalla.
5. En la traza: GetRaw devuelve 0 (no 0xC00000BB); líneas `PORTAL USB dir=IN` con datos; número de llamadas a GetRaw no se dispara (indicador de bucle de reintento).
6. Comparación negativa: con el Portal desenchufado, el juego debe mostrar la pantalla de desconexión y GetRaw devolver 0x48F (confirma el contrato de «desconectado»). Requiere acción del usuario [NV].
7. Fin: `Stop-Process` de cada SwapForceRT propio (nunca el PID 10348) y comprobación con `Get-Process`.

## 5. Riesgos y mitigaciones
- Bloqueo del hilo guest por USB: imposible por diseño (GetRaw no hace E/S; SetRaw espera ≤150 ms acotado). Comprobar con `--perf`.
- Timeouts: lectura 100 ms (Xenia usa 100 ms); un `ReadPipe` colgado se cancela con `UsbK_AbortPipe` al cerrar el hilo.
- Fallo de libusbK/instalación: `Load()` falla -> modo «desconectado» (0x48F), un solo aviso en log; sin reintentos agresivos.
- Dispositivo ocupado (otra aplicación/Xenia abierta) -> `CreateFile`/`Claim` falla; registrar el error Win32 y reintentar cada 500 ms.
- Cierre del proceso: señal de parada, `AbortPipe`, `join` con tope 1 s, liberar handles (`UsbDeviceHandle` ya lo hace).
- Sin figura: el informe del Portal sin figura es válido; el juego debe avanzar sin ella (si no avanza, anotar [NV] la causa y no inventar datos de figura). No se emulan figuras.
- Contrato de retorno/`state` conjeturado (§1): si el juego sigue en «DISCONNECTED» con GetRaw=0, probar variaciones de `state` (0/1) y de longitud devuelta solo en la lectura (sin tocar el dispositivo), registrando cada intento.
- Efecto en dispositivo por escrituras del juego: autorizado solo como paso directo; si aparece una escritura inesperada de gran tamaño o frecuencia (> 1000/s), limitar tasa y registrar, sin alterar contenido.
- Rutas permitidas: `hid\**`, `kernel\**`, `main.cpp`, `CMakeLists.txt`; no `ppc_gen`, no `selftest`.

## 6. Hitos y aceptación
| Hito | Contenido | Aceptación (por comando) |
|---|---|---|
| P1 | Firma real: log de retorno, `*len`, `*state` y SetRaw; sin USB | Traza de ≥ 8 llamadas con r3/r4/r5 y salidas; 0 cambios de comportamiento |
| P2 | Capa `PortalDevice` + hilo lector + cola; GetRaw real | `--selftest-portal` OK; log `PORTAL USB dir=IN` con ≥ 1 transferencia; GetRaw devuelve 0 con Portal y 0x48F sin él |
| P3 | SetRaw pass-through con registro | Toda transferencia OUT en log con len/32 B/resultado; ninguna OUT sin llamada SetRaw previa en el log (comprobación con script) |
| P4 | Reconexión | Desenchufar/enchufar (acción del usuario [NV]): log de cierre y reapertura sin reiniciar el exe |
| P5 | Verificación visual por curl (§4) | `frame.jpg` sin «PORTAL DISCONNECTED» en ≥ 2 capturas separadas 10 s |
| P6 | Siguiente bloqueo | Informe: pantalla alcanzada, siguiente fallo o límite fuera de alcance; apartado «Codex ronda 8» en `PROGRESS.md` |
