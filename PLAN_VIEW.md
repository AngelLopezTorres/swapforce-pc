# PLAN_VIEW — Visor en vivo en navegador (rama `live-view`)
Estado: SOLO PLAN. Nada implementado. [NV] = no verificado. Propietario: gráficos (`runtime\src\gpu\**`, nuevo `runtime\src\viewer\**`); `main.cpp`, `CMakeLists.txt` y `xam_input.cpp` son ajenos/compartidos: cambios mínimos y aviso (AGENTS.md).

## 1. Hechos comprobados en el código (worktree `_wt_view`)
- `Backend::OnSwap(const SwapInfo&)` (`gpu/backend.h`) se invoca desde el hilo del CP; `D3D12Backend::OnSwap` (`d3d12_backend.cpp`, l.~340-420) obtiene el RGBA en `ri->rgba` (resolve) o `fbScratch` (`DecodeFrontbuffer`) y llama `PresentFromRgba(rgba, w, h)`. Ese buffer RGBA en CPU es el punto de enganche: **no exige lectura de GPU**.
- Tamaño interno `kW x kH` (fijo, `d3d12_backend.cpp` l.133); `PresentFromRgba` recorta a `min(w,kW) x min(h,kH)`.
- Con `--headless` no se crea swapchain ni ventana (l.178): el visor funciona sin abrir la ventana del juego. Sin `--headless`, `Present(1,0)` (l.400).
- `util/image_io.cpp` ya enlaza `windowscodecs`/`ole32` y solo escribe PNG a fichero (`WritePngRGBA`). Falta codificar a memoria y JPEG.
- `xam_input.cpp`: `ReadState(user,s)` usa XInput; si no hay mando, usuario 0 usa `KeyboardState()` con `GetAsyncKeyState` (flechas, Enter, Z, X, Esc = DPAD, START, A, B, BACK). Bits: DPAD 0x0001-0x0008, START 0x0010, BACK 0x0020, A 0x1000, B 0x2000.
- `main.cpp` analiza opciones con `strcmp` en bucle (l.244-288); `Cfg()` guarda configuración GPU.
- `CMakeLists.txt` solo enlaza `setupapi`; Winsock se enlazará con `#pragma comment(lib,"ws2_32.lib")` en el nuevo fichero (patrón de `image_io.cpp`).

## 2. Diseño
### 2.1 Captura (sin frenar el CP)
- Nuevo `viewer::Publish(const uint8_t* rgba, w, h)` llamado en `OnSwap` justo antes de `PresentFromRgba`, tras la rama resolve/frontbuffer. Si el visor está desactivado: una comparación de un `std::atomic<bool>` (coste ~0).
- Filtro de ritmo en el CP: solo copia si `atomic<bool> wanted` (hay cliente conectado) **y** han pasado >= `1000/fps` ms (QPC). El resto de fotogramas no cuesta nada.
- Copia: una sola `memcpy` de `w*h*4` B (≈3,6 MB a 1280x720 [NV: kW/kH exactos]) a un búfer doble; el CP usa `try_lock` y, si el hilo del visor lo tiene, **descarta el fotograma** (nunca espera). Número de copias en el CP: 1, como máximo `fps_visor` veces/s.
- Reducción opcional (`scale=1|2|4`) en el hilo del visor, no en el CP.
### 2.2 Codificación (hilo del visor)
- WIC (`CLSID_WICJpegEncoder`, `IWICStream::InitializeFromMemory`/`CreateStreamOnHGlobal`) a memoria; propiedad `ImageQuality` (0-1) vía `IPropertyBag2` (Microsoft Learn: «JPEG Format-Specific Encoder Options», «WIC Encoder»). Entrada BGRA (`GUID_WICPixelFormat32bppBGR`) o convertir como en `WritePngRGBA`. COM se inicializa en el propio hilo (`CoInitializeEx`, como `image_io.cpp`).
- Nuevas funciones en `image_io.*`: `EncodeJpegRGBA(w,h,rgba,stride,quality,std::vector<uint8_t>& out)`. PNG a memoria opcional (`/frame.png`).
- Calidad por defecto 70; límite de tamaño de la imagen codificada 4 MiB (si se supera, se descarta el fotograma).
### 2.3 Servidor HTTP (Winsock, un hilo de aceptación + un hilo por cliente, máx. 2 clientes)
- `WSAStartup(MAKEWORD(2,2))`, `socket(AF_INET,SOCK_STREAM,IPPROTO_TCP)`, `bind` a `sockaddr_in` con `inet_pton("127.0.0.1")` **nunca** `INADDR_ANY`; `listen(s, 4)`; `accept` (Microsoft Learn: «Getting Started with Winsock», «bind function», «Winsock Server Application»).
- Rutas (solo `GET`, resto 405; ruta desconocida 404; sin acceso a ficheros, todo en memoria):
  - `/` página HTML mínima embebida en una cadena constante.
  - `/frame.jpg` último JPEG (`Cache-Control: no-store`, `Content-Type: image/jpeg`).
  - `/stream` MJPEG: `Content-Type: multipart/x-mixed-replace; boundary=frame`, una parte por fotograma (RFC 2046 §5.1.3; patrón no normativo de facto [NV en navegadores concretos]).
  - `/key?down=<códigos>` (solo si `--viewer-input`): lista separada por comas de códigos permitidos (`up,down,left,right,enter,z,x,esc`); cualquier otro valor -> 400.
- Seguridad: comprobar `Host:` == `127.0.0.1:<puerto>` o `localhost:<puerto>` (mitiga DNS rebinding) y, en `/key`, rechazar si existe `Origin:` distinto del propio (mitiga CSRF desde otras webs); cabecera de petición <= 4 KiB, sin cuerpo; `SO_RCVTIMEO`/`SO_SNDTIMEO` 5 s; cierre de cliente lento (envío con tiempo límite, no bloquea al CP porque corre en su hilo); `Connection: close` salvo MJPEG.
- Cierre ordenado: `closesocket` + `WSACleanup` en `Shutdown()`; hilos con `join`.
### 2.4 Entrada del navegador
- JS del cliente: `keydown`/`keyup` -> conjunto activo; cada cambio y un latido de 200 ms envían `/key?down=...`. Mapeo: flechas=DPAD, Enter=START, Z=A, X=B, Esc=BACK.
- Estado compartido: `std::atomic<uint16_t> g_viewerButtons` + `std::atomic<uint64_t> g_viewerStamp` (QPC del último mensaje). Caducidad: si pasan >500 ms sin latido, los botones valen 0 (evita teclas «pegadas» si se cierra la pestaña).
- Integración mínima en `xam_input.cpp` (propietario Codex; avisar): en `ReadState`, `s.pad.buttons |= viewer::Buttons()` en usuario 0 tras leer XInput/teclado. Con `--viewer-input` y sin mando, se omite `GetAsyncKeyState` para no leer teclas globales del sistema [decisión pendiente del usuario].
- `s.packet` hoy no se incrementa (`State s{}`): [NV] si el juego exige cambio de `packet`; verificar antes de dar la entrada por buena.

## 3. Opciones CLI (por defecto TODO desactivado)
`--viewer [puerto]` (defecto 8765, rango 1024-65535; no se activa sin la opción) · `--viewer-fps N` (1-30, defecto 10) · `--viewer-quality N` (10-95, defecto 70) · `--viewer-scale 1|2|4` · `--viewer-input` (acepta `/key`; sin ella `/key` -> 404). Combinable con `--headless`.

## 4. Verificación sin ver la pantalla (todas por comando; salida a fichero)
Lanzar: `SwapForceRT.exe --headless --viewer 8765 --timeout 90 > out.txt` con `userdata\` y `runtime.log` propios (AGENTS.md §3).
1. Escucha solo en loopback: `netstat -ano -p tcp | findstr :8765` -> debe aparecer SOLO `127.0.0.1:8765  LISTENING`; no debe aparecer `0.0.0.0:8765` ni `[::]:8765` ni la IP de la LAN. Prueba negativa: `curl -m 3 http://<IP-LAN>:8765/frame.jpg` -> fallo de conexión.
2. Cabecera JPEG: `curl -s -o f1.jpg -w "%{http_code} %{content_type} %{size_download}\n" http://127.0.0.1:8765/frame.jpg`; `200 image/jpeg` y primeros bytes `FF D8` y últimos `FF D9` (`certutil -encodehex f1.jpg - 4` o PowerShell `[IO.File]::ReadAllBytes`).
3. Cambio en el tiempo: dos descargas separadas >= 1 s durante el vídeo Bink (`gsbp_atvi.bik`/`gsbp_vvisions.bk2`) -> `certutil -hashfile f1.jpg SHA256` != `f2.jpg`. Criterio de coherencia: el hash del JPEG decodificado no es comparable bit a bit con el PNG volcado; comparar con `--dump-frames`: diferencia media (PSNR) >= 30 dB entre `/frame.jpg` y el PNG del mismo fotograma [NV: requiere sincronizar número de fotograma; exponer `X-Frame: <n>` en la cabecera].
4. Ritmo: `curl -m 10 http://127.0.0.1:8765/stream -o s.bin` y contar ocurrencias de `FF D8` -> entre 0,8x y 1,0x de `fps*10`.
5. No frena al CP: comparar VdSwap/s con y sin `--viewer` y con un cliente MJPEG conectado (método de `PROGRESS.md`, «Rendimiento M8»): caída <= 3 % [umbral propuesto, a confirmar con el usuario].
6. Seguridad: `curl -H "Host: evil.com" http://127.0.0.1:8765/` -> 403/400; `curl -X POST ...` -> 405; petición de 64 KiB de cabecera -> cierre; `curl http://127.0.0.1:8765/../runtime.log` -> 404; 3.er cliente simultáneo -> 503.
7. Entrada (con `--viewer-input`): `curl "http://127.0.0.1:8765/key?down=enter"` y comprobar en `runtime.log` la línea `INPUT user=0 buttons=0x0010` (ya la emite `XamInputGetState`); a los 500 ms sin latido, `buttons=0x0000`. `down=zz` -> 400. Sin `--viewer-input` -> 404.
8. Fin limpio: al terminar el proceso, `netstat` sin el puerto y sin `SwapForceRT.exe` vivo (`tasklist`).

## 5. Hitos y criterios de aceptación
| Hito | Contenido | Aceptación |
|---|---|---|
| V0 | `EncodeJpegRGBA` en `image_io` + selftest `--selftest-jpeg` | JPEG de patrón sintético empieza `FFD8`, termina `FFD9`, se decodifica con WIC con PSNR >= 30 dB a calidad 70 |
| V1 | Servidor Winsock loopback + `/` + `/frame.jpg` (fotograma sintético, `--no-d3d`) | Verif. 1, 2, 6 |
| V2 | `Publish` en `OnSwap` con filtro de ritmo y `try_lock` | Verif. 3 y 5 con el juego |
| V3 | `/stream` MJPEG + página HTML | Verif. 4; revisión visual del usuario en navegador [NV: no se puede automatizar] |
| V4 | Entrada `/key` + atomics + `xam_input.cpp` | Verif. 7 |
| V5 | Documentar en `PROGRESS.md` (apartado Gráficos), nota de `HANDOFF` a Codex por `xam_input.cpp` | Compila (`vb_verify.cmd`) y se ha ejecutado V1-V4 (distinguir ambos) |

## 6. Riesgos
- Coste de CPU de WIC JPEG a 1280x720 [NV; medir en V0]; mitigación: `--viewer-scale`, fps bajo.
- Copia de 3,6 MB bajo `try_lock` aumenta ancho de banda de memoria; medir en verif. 5.
- Puerto ocupado: `bind` falla -> registrar en `runtime.log` y continuar sin visor (nunca abortar el juego).
- Cualquier proceso local del mismo equipo puede conectar al loopback; con `--viewer-input` puede inyectar teclas: aceptado y documentado; sin autenticación [decisión pendiente: token aleatorio en la URL impreso en el log].
- Cortafuegos de Windows: el loopback no suele generar aviso [NV].
- Contenido del juego: el visor sirve imágenes del juego; no persistir en disco ni en el repositorio (AGENTS.md §6, `LEGAL_RESUMEN.md`).
- Cambios en ficheros compartidos/ajenos (`main.cpp`, `CMakeLists.txt`, `xam_input.cpp`): mínimos y con aviso.

## 7. Referencias
Microsoft Learn: Winsock (Getting Started, `bind`, `listen`, `accept`, `WSAStartup`, `setsockopt` SO_RCVTIMEO); Windows Imaging Component («JPEG Format-Specific Encoder Options», «IWICBitmapEncoder»); XInput (`XINPUT_GAMEPAD` valores de botones); RFC 9110 (HTTP semántica); RFC 2046 §5.1.3 (multipart/mixed-replace, tipo `x-mixed-replace` no registrado); WHATWG Fetch/HTML (cabecera `Origin`).
