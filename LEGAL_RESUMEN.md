# LEGAL_RESUMEN — documento de consulta para asesoría jurídica

Fecha: 2026-10-11. Documento de hechos y citas; **no contiene opiniones ni conclusiones jurídicas**. **[NV]** = no verificado.
Textos normativos leídos el 2026-10-11 en: BOE (TRLPI, `BOE-A-1996-8930`; Código Penal, `BOE-A-1995-25444`), EUR-Lex (CELEX 32009L0024 y 32001L0029, versión EN) y Cornell LII (17 U.S.C. §§1201, 117, 512).

## 1. Hechos del proyecto

### 1.1 Programa de partida
- Obra: *Skylanders: Swap Force*, versión Xbox 360. Titular de derechos: Activision [NV: titularidad exacta de código y de cada recurso].
- Insumo del usuario: una imagen ISO (la utilizada: `Skylanders - Swap Force (Europe) (En,De).iso`, 8.738.846.720 B, firma `MICROSOFT*XBOX*MEDIA`, XGD2). Origen y legitimidad de la ISO: la aporta el usuario; el proyecto no la distribuye ni indica cómo obtenerla.

### 1.2 Qué se descifra y descomprime
1. `extract-xiso` (herramienta de terceros) extrae el sistema de ficheros de la ISO (2474 ficheros, 8.448.047.409 B: 2231 `.pak`, 92 `.bk2`, 112 `.xzp`, 24 `.xex`, 4 `.lex`, 2 `.bik`). Esta herramienta no se aplica a protecciones del disco; trabaja sobre la ISO ya existente.
2. `analysis/dump_image.py` descifra `default.xex` (18.563.072 B; cifrado "retail", compresión básica de bloques, base 0x82000000, entrada 0x82F422E8) con AES-128-CBC: una clave de 16 bytes constante en el script (la "retail key" del formato XEX2) descifra la clave de fichero almacenada en la cabecera, y esta descifra el cuerpo; después reconstruye la imagen PE descomprimida en `analysis/image.bin` (21.659.648 B).
3. La misma constante de clave figura en el upstream de XenonRecomp: `XenonUtils/xex.h` línea 5 (`Xex2RetailKey`), usada por `XenonUtils/xex.cpp` para cargar el XEX en memoria. Verificado en el commit `ddd128b` del upstream (MIT).
4. El runtime carga `analysis/image.bin` en la memoria emulada al ejecutarse (`runtime/config/paths.ini`) y lee los ficheros de `iso_extract\` en modo solo lectura.

### 1.3 Qué se genera (todo en local, excluido del repositorio por `.gitignore`)
| Producto | Origen | Tamaño / cifra |
|---|---|---|
| `analysis/image.bin` | descifrado y descompresión de `default.xex` | 21.659.648 B |
| `runtime/ppc_gen/` (`ppc_recomp.N.cpp`, `ppc_recomp_shared.h`, `ppc_func_mapping.cpp`) | traducción automática del código PowerPC de `.text` a C++ con XenonRecomp (patcheado) | 486 ficheros (482 `ppc_recomp.N.cpp`), ≈ 241 MB, 123.134 funciones (último recuento propio) |
| `ppc/` | primera pasada de XenonRecomp (usada para inferir límites de función) | 489 ficheros, ≈ 242 MB (en el proyecto original) |
| `runtime/src/kernel/import_stubs.gen.cpp` | lista de 267 imports de kernel Xbox 360 del XEX | generado por script; **no se publica** |
| Tablas de saltos y extensiones de función (`.toml`, `extents.txt`, `pruned.txt`) | análisis del binario: direcciones, tamaños y etiquetas | solo números hexadecimales; los `.toml` generados no se publican; `extents.txt` (88 líneas) y `pruned.txt` (10 líneas) sí se publican |
| `runtime/userdata/shaders` | volcado en ejecución de microcódigo de shaders (contenedores `xenos_*.bin`, `ucode_*.bin`) tomado de la memoria del proceso | 758 contenedores únicos y 55 ucode en la medición documentada; **no se publica** |
| Fotogramas, texturas, `resolve_*.png`, `runtime.log` | salida de depuración | **no se publica** |
| `SwapForceRT.exe` (54 MB) | enlace de código propio + `ppc_gen` | contiene el código recompilado: **no se publica** |

### 1.4 Qué se distribuiría (contenido de `publish/`)
Código propio del runtime (`runtime/src`), scripts de análisis y regeneración (`analysis/*.py`, `runtime/recomp/*.py|sh`, `runtime/tools`), `runtime/CMakeLists.txt`, `analysis/SwapForce_base.toml` (rutas y 4 direcciones de funciones auxiliares del XDK), `runtime/recomp/extents.txt` y `pruned.txt`, `patches/XenonRecomp-swapforce.patch` (diff contra upstream MIT), 9 ficheros copiados de Xenia (BSD-3, cabecera conservada), 3 cabeceras de DXC, documentación (`PROGRESS.md`, `PLAN_*.md`), `setup.ps1`, `.gitignore`, licencias.
No se distribuiría: ISO, ficheros extraídos, `default.xex`, `image.bin`, código recompilado, shaders, volcados, binarios, DLL de DXC (descarga en `setup.ps1`).

### 1.5 Comprobaciones realizadas sobre el contenido a publicar (2026-10-11)
- Búsqueda en `runtime/src`, `runtime/tools`, `runtime/recomp`, `analysis/*.py|toml`: 0 ficheros con bytes NUL, 0 líneas > 2000 caracteres, 0 ocurrencias de matrices de ≥ 16 bytes hexadecimales consecutivos en `runtime/src`.
- `extents.txt` y `pruned.txt` solo contienen valores hexadecimales (direcciones/tamaños). `SwapForce_base.toml` contiene rutas y direcciones.
- Referencias residuales al juego en el contenido a publicar (metadatos, no contenido): cadena de título de ventana `"Skylanders Swap Force (PC port - runtime)"` (`runtime/src/gpu/d3d12_backend.cpp`, línea 34); `PROGRESS.md` y `PLAN_*.md` citan nombres de ficheros del disco (p. ej. `CollectibleIcons.pak`, `loosefiles.pak`), tamaños, direcciones virtuales, hashes de shaders/fotogramas, nombres de hilos/clases observados en ejecución (p. ej. "igWin32StorageDevice", "igFileRedirector") y el nombre de la ISO.
- Ruta de usuario de Windows eliminada de `PROGRESS.md`, `PLAN_RUNTIME.md` y de los scripts.

### 1.6 Estado técnico (relevante para el alcance de uso)
No jugable; imagen negra (vídeo Bink 2 sin decodificar); sin audio ni portal USB; 7–9 fps. Ver `README.md` y `PROGRESS.md`.

## 2. Normas citadas (texto verificado)

### 2.1 Unión Europea
**Directiva 2009/24/CE** (DO L 111, 5.5.2009, p. 16), programas de ordenador:
- **Art. 4.1**: derechos exclusivos del titular de (a) reproducción permanente o temporal, total o parcial; (b) traducción, adaptación, arreglo y cualquier otra transformación y reproducción de sus resultados; (c) distribución. **Art. 4.2**: agotamiento del derecho de distribución en la primera venta en la Comunidad.
- **Art. 5.1**: no requieren autorización (a falta de disposiciones contractuales específicas) los actos de 4.1.a/b necesarios para la utilización por el adquirente legítimo conforme a su finalidad. **5.2**: copia de seguridad. **5.3**: derecho a observar, estudiar o verificar el funcionamiento del programa durante las operaciones de carga, visualización, ejecución, transmisión o almacenamiento que tenga derecho a hacer.
- **Art. 6.1**: la reproducción del código y traducción de su forma (4.1.a/b) no requieren autorización si son indispensables para obtener la información necesaria para la interoperabilidad de un programa creado de forma independiente, con tres condiciones: (a) actos realizados por el licenciatario o persona con derecho a usar una copia, (b) información no disponible previamente de forma fácil y rápida, (c) actos limitados a las partes necesarias. **6.2**: la información obtenida no puede (a) usarse para fines distintos de la interoperabilidad, (b) comunicarse a terceros salvo cuando sea necesario para la interoperabilidad, (c) usarse para desarrollar, producir o comercializar un programa sustancialmente similar en su expresión ni para otro acto que infrinja derechos de autor. **6.3**: no puede interpretarse de modo que perjudique injustificadamente los intereses legítimos del titular o contradiga la explotación normal.
- **Art. 7.1.c**: recursos apropiados contra la puesta en circulación o posesión con fines comerciales de "cualquier medio cuya única finalidad sea facilitar la supresión o neutralización no autorizadas de cualquier dispositivo técnico utilizado para proteger un programa de ordenador". **Art. 8, párrafo 2**: nulidad de las cláusulas contractuales contrarias al art. 6 o a las excepciones del art. 5.2 y 5.3.

**Directiva 2001/29/CE** (DO L 167, 22.6.2001), sociedad de la información:
- **Art. 1.2.a**: la Directiva deja intactas las disposiciones comunitarias sobre protección jurídica de programas de ordenador.
- **Art. 6.1**: protección frente a la elusión de medidas tecnológicas eficaces "a sabiendas o con motivos razonables para saber"; **6.2**: actos preparatorios (fabricación, importación, distribución, venta, alquiler, publicidad, posesión comercial de dispositivos, productos o componentes o prestación de servicios) con los supuestos a, b y c; **6.3**: definición de "medida tecnológica" y de "eficaz" (control de acceso o protección como "codificación, aleatorización u otra transformación"); **6.4**: obligación de facilitar a beneficiarios de ciertos límites los medios para disfrutarlos.

### 2.2 España
**Real Decreto Legislativo 1/1996, de 12 de abril (TRLPI)**, texto consolidado consultado: "Última actualización" del texto 30/03/2022 [NV: reformas posteriores].
- **Art. 21** (Transformación): comprende traducción, adaptación y cualquier otra modificación de forma de la que se derive una obra diferente (21.1); los derechos sobre la obra resultante corresponden a su autor "sin perjuicio del derecho del autor de la obra preexistente de autorizar" su explotación (21.2).
- **Art. 99**: derechos de explotación del programa: (a) reproducción total o parcial, incluso para uso personal, permanente o transitoria, cuando la carga, presentación, ejecución, transmisión o almacenamiento necesiten reproducción; (b) traducción, adaptación, arreglo o transformación y reproducción de sus resultados; (c) distribución pública incl. alquiler; agotamiento por primera venta en la UE.
- **Art. 100**: 100.1 (reproducción o transformación necesarias para la utilización por el usuario legítimo); 100.2 (copia de seguridad); 100.3 (observar, estudiar o verificar el funcionamiento); 100.5 (reproducción del código y traducción indispensables para la interoperabilidad, con requisitos a, b, c); 100.6 (uso de la información obtenida: a, b, c); 100.7 (no perjudicar injustificadamente intereses legítimos).
- **Art. 102.c**: infractores quienes pongan en circulación o tengan con fines comerciales "cualquier instrumento cuyo único uso sea facilitar la supresión o neutralización no autorizadas de cualquier dispositivo técnico utilizado para proteger un programa de ordenador".
- **Art. 196**: acciones contra la elusión de medidas tecnológicas eficaces (196.1) y actos preparatorios (196.2); definición (196.3). **Art. 196.4: "Lo dispuesto en los apartados anteriores no es de aplicación a las medidas tecnológicas utilizadas para la protección de programas de ordenador, que quedarán sujetas a su propia normativa."**
- **Art. 31.2 y 31.3.c** (copia privada; excluye programas de ordenador "en aplicación de la letra a) del artículo 99"); **art. 138** (acciones; responsable también quien induzca o coopere a sabiendas). Citados por relación con la ISO del usuario [NV: aplicabilidad].

**Ley Orgánica 10/1995, Código Penal**, art. 270 (redacción vigente por LO 1/2015, en vigor desde 01/07/2015) [NV: aplicabilidad a este proyecto]:
- 270.1: pena de prisión de 6 meses a 4 años y multa por reproducir, plagiar, distribuir, comunicar públicamente o explotar económicamente una obra "con ánimo de obtener un beneficio económico directo o indirecto y en perjuicio de tercero", sin autorización.
- 270.5.c: favorecer o facilitar esas conductas "eliminando o modificando, sin autorización ... las medidas tecnológicas eficaces"; 270.5.d: con ánimo de beneficio económico, eludir o facilitar la elusión de medidas tecnológicas eficaces para facilitar a terceros el acceso a un ejemplar.
- 270.6: prisión de 6 meses a 3 años a quien fabrique, importe, ponga en circulación o posea "con una finalidad comercial" cualquier medio principalmente concebido, producido, adaptado o realizado para facilitar la supresión no autorizada o la neutralización de cualquier dispositivo técnico utilizado para proteger programas de ordenador u otras obras.

### 2.3 Estados Unidos (17 U.S.C.)
- **§1201(a)(1)(A)**: prohibición de eludir una medida tecnológica que controle eficazmente el acceso a una obra. **(a)(1)(B)–(D)**: exenciones por clases de obras determinadas en procedimiento trienal de la Librarian of Congress (a propuesta del Register of Copyrights). Lista vigente de exenciones (37 C.F.R. § 201.40): [NV, no consultada].
- **§1201(a)(2)** y **(b)(1)**: prohibición de fabricar, importar, ofrecer al público, proporcionar o traficar con tecnología, producto, servicio, dispositivo, componente o parte que (A) esté principalmente diseñado o producido para eludir, (B) tenga un propósito comercialmente significativo limitado al margen de la elusión, o (C) se comercialice para eludir. **(a)(3)**: definiciones de "circumvent" (incluye "decrypt an encrypted work") y de "effectively controls access".
- **§1201(f)(1)–(4)**: ingeniería inversa para interoperabilidad: quien haya obtenido lícitamente el derecho a usar una copia de un programa puede eludir una medida de acceso "for the sole purpose of identifying and analyzing those elements of the program that are necessary to achieve interoperability of an independently created computer program" en la medida en que no constituya infracción; (f)(2) medios para ello; (f)(3) comunicación a terceros solo para interoperabilidad; (f)(4) definición.
- **§117(a)**: no es infracción que el propietario de una copia de un programa realice otra copia o adaptación (1) como paso esencial para su utilización con una máquina y sin otro uso, o (2) con fines de archivo, destruyéndose las copias de archivo si cesa la posesión legítima. **§117(b)**: transferencia solo con todos los derechos; las adaptaciones solo con autorización del titular.
- **§512(c)(1)(C)** y **(c)(3)**: respuesta expeditiva a notificación de infracción y elementos de la notificación; **§512(g)(1)–(3)**: exención por retirada de buena fe, aviso al usuario, contranotificación y reposición del material entre 10 y 14 días hábiles salvo que el notificante informe de una acción judicial. **§512(c)(2)**: agente designado.

### 2.4 Otras normas/jurisprudencia a revisar [NV: no leídas en esta sesión]
Marcas (uso del nombre del juego en README y título de ventana); jurisprudencia del TJUE sobre programas de ordenador y medidas tecnológicas (p. ej. asuntos C-355/12 y C-406/10: citas sin verificar); condiciones de uso de la plataforma de alojamiento (GitHub) y su política DMCA.

## 3. Precedentes públicos (modelo de distribución, según sus propios ficheros)

| Proyecto | Licencia (API de GitHub, 2026-10-11) | Modelo de distribución documentado |
|---|---|---|
| **UnleashedRecomp** (hedge-dev), README en `main` @ `cf829a9eca8f` (2026-06-29): https://github.com/hedge-dev/UnleashedRecomp/blob/main/README.md | GPL-3.0 | README: "This project does not include any game assets. You must provide the files from your own legally acquired copy of the game to install or build Unleashed Recompiled." Para compilar, el usuario copia `default.xex`, `default.xexp` y `shader.ar` a `UnleashedRecompLib/private/` (https://github.com/hedge-dev/UnleashedRecomp/blob/main/docs/BUILDING.md). Las versiones publicadas usan un instalador que procesa contenedores/imágenes del usuario. Si los binarios publicados incorporan código derivado del juego: **[NV]**. Usa XenonRecomp y XenosRecomp (MIT) de hedge-dev. |
| **N64Recomp** (herramienta), README en `main` @ `ffb39cdad1da` (2026-05-27): https://github.com/N64Recomp/N64Recomp | MIT | El README de la herramienta no contiene declaración sobre ficheros del juego (búsqueda de términos legales/ROM/assets sin resultados relevantes). |
| **Zelda64Recomp** (proyecto de juego sobre N64Recomp), README en `dev` @ `b65c482ed672` (2026-09-25): https://github.com/Zelda64Recomp/Zelda64Recomp | GPL-3.0 | README: "This repository and its releases do not contain game assets. The original game is required to build or run this project." El usuario aporta su ROM en el menú; `BUILDING.md` exige una ROM descomprimida del usuario (hash SHA-1 indicado) y ejecutar N64Recomp para generar el C a compilar (https://github.com/Zelda64Recomp/Zelda64Recomp/blob/dev/BUILDING.md). |

Estado jurídico, notificaciones recibidas o litigios de estos proyectos: **[NV]** (no consultados).

## 4. Preguntas abiertas para el abogado
1. ¿Qué actos del flujo de `setup.ps1` (extracción de la ISO, descifrado de `default.xex`, descompresión a `image.bin`, traducción a C++ con XenonRecomp, compilación, ejecución) quedan bajo los arts. 5.1, 5.3 y 6 de la Directiva 2009/24/CE / arts. 100.1, 100.3, 100.5 TRLPI, y cuáles bajo el art. 4.1.b / 99.b / 21 TRLPI, cuando los realiza el propio usuario con su copia?
2. ¿Se cumplen las condiciones de interoperabilidad (arts. 6.1 y 6.2 Directiva; 100.5–100.7 TRLPI; §1201(f) USC) para un runtime que ejecuta código recompilado del propio juego ("programa creado de forma independiente")? ¿Afecta que la información obtenida se comunique a terceros mediante scripts/configuración publicados (art. 6.2.b)?
3. ¿Es el cifrado de `default.xex` una "medida tecnológica eficaz" a efectos de la Directiva 2001/29/CE art. 6, o se rige por la normativa específica de programas de ordenador (TRLPI art. 196.4; Directiva 2009/24 art. 7.1.c; TRLPI art. 102.c)? ¿Y en EE. UU., §1201(a)(1)(A) y exenciones vigentes?
4. ¿La inclusión en el repositorio de la constante de 16 bytes de la clave XEX2 (`analysis/dump_image.py`; ya presente en el upstream `XenonUtils/xex.h`) y del script que descifra el XEX constituye "medio"/"instrumento"/"tecnología" a efectos de Directiva 2009/24 art. 7.1.c, TRLPI art. 102.c, CP art. 270.6 o §1201(a)(2)/(b)(1)? ¿Conviene sustituir el script por la carga del XEX que ya realiza XenonRecomp?
5. ¿Constituye reproducción o "transformación" (arts. 4.1.b Directiva, 21 y 99.b TRLPI) la distribución de: (a) `runtime/recomp/extents.txt`, `pruned.txt` y `SwapForce_base.toml` (direcciones, tamaños, direcciones de funciones auxiliares del XDK); (b) el patch sobre XenonRecomp (no contiene código del juego); (c) metadatos en `PROGRESS.md` (nombres de ficheros, de clases/hilos, hashes, direcciones)?
6. ¿Condiciones y riesgos si en el futuro se distribuyera un binario (`SwapForceRT.exe`), que incorpora el código recompilado (123.134 funciones)? ¿Y el código C++ generado (`ppc_gen`)?
7. ¿Qué efecto tiene que el usuario obtenga la ISO mediante copia de su disco (TRLPI art. 31.2, 31.3.c, art. 99.a; Directiva 2009/24 art. 5.2; §117(a)(2))? ¿Debe el README limitarse a "copia legítima" sin más indicaciones, como el modelo de los precedentes de la sección 3?
8. Marcas y nombre del juego: uso en el título del repositorio, README y cadena de ventana (`d3d12_backend.cpp:34`). ¿Se requiere descargo de no afiliación o cambio de nombre?
9. Licencia propia (MIT propuesta): ¿es compatible con la inclusión de ficheros de Xenia (BSD-3) y del patch MIT? Grado de derivación de los 29 ficheros que citan Xenia y de `ucode_hlsl.cpp` respecto a XenosRecomp: [NV]. ¿Procede usar GPL-3.0 como los precedentes de la sección 3 (el repositorio de XenonRecomp que se descarga incluye código derivado de binutils bajo GPL y libmspack LGPL-2.1, aunque no se redistribuye)?
10. DXC: ¿procede en el futuro redistribuir `dxcompiler.dll` (NCSA) y `dxil.dll` (licencia Microsoft: requisitos de DISTRIBUTABLE CODE, indemnización)? Hoy no se redistribuyen; se descargan del release oficial.
11. Titular del repositorio y responsabilidad: ¿a nombre de persona física o de la entidad (FeeX)? ¿Aviso legal, procedimiento de retirada y respuesta a notificaciones DMCA §512(c)(3) y contranotificación §512(g), dirección de contacto, y designación de agente si se alojase en servidor propio?
12. Jurisdicciones: ¿qué ley resulta aplicable al alojamiento en GitHub (EE. UU.) y a usuarios en la UE/España (art. 8 Directiva 2009/24: nulidad de cláusulas contrarias a 5.2/5.3/6)? ¿Afecta el CLUF de la ISO/juego [NV: no leído]?
13. Ámbito penal (CP art. 270.1, 270.5, 270.6): confirmar si la ausencia de ánimo de lucro y de finalidad comercial es relevante para el proyecto tal como está planteado (gratuito, sin binarios del juego).
