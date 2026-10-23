cmake_minimum_required(VERSION 3.20)

# Compilar con: clang-cl + Ninja + lld-link (ver runtime/tools/build.cmd)
set(CMAKE_C_COMPILER clang-cl CACHE STRING "")
set(CMAKE_CXX_COMPILER clang-cl CACHE STRING "")
set(CMAKE_LINKER "C:/Program Files/LLVM/bin/lld-link.exe" CACHE FILEPATH "")

project(SwapForceRT CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Paralelismo de compilacion (Fase 0: pico medido ~410 MB por clang-cl en el mayor fichero;
# 20 hilos logicos, 34 GB de RAM). Ajustable: -DSWAPFORCE_COMPILE_JOBS=N
set(SWAPFORCE_COMPILE_JOBS 18 CACHE STRING "Trabajos de compilacion simultaneos")
set_property(GLOBAL PROPERTY JOB_POOLS compile_pool=${SWAPFORCE_COMPILE_JOBS} link_pool=1)
set(CMAKE_JOB_POOL_COMPILE compile_pool)
set(CMAKE_JOB_POOL_LINK link_pool)

get_filename_component(SWAPFORCE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)
# Codigo recompilado: runtime/ppc_gen (regenerado con XenonAnalyse ampliado, ver PROGRESS.md) si existe;
# en caso contrario ppc/ del proyecto.
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ppc_gen/ppc_recomp_shared.h")
  set(PPC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ppc_gen")
else()
  set(PPC_DIR "${SWAPFORCE_ROOT}/ppc")
endif()
set(SIMDE_DIR "${SWAPFORCE_ROOT}/tools/XenonRecomp/thirdparty/simde")

file(GLOB PPC_SOURCES CONFIGURE_DEPENDS "${PPC_DIR}/ppc_recomp.*.cpp" "${PPC_DIR}/ppc_func_mapping.cpp")
include("${CMAKE_CURRENT_SOURCE_DIR}/ppc_sources.cmake")
foreach(i ${PPC_STALE_INDICES})
  list(REMOVE_ITEM PPC_SOURCES "${PPC_DIR}/ppc_recomp.${i}.cpp")
endforeach()
file(GLOB_RECURSE RT_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable(SwapForceRT ${RT_SOURCES} ${PPC_SOURCES})
option(SWAPFORCE_TRACE "Registrar destinos de llamadas indirectas (diagnostico)" ON)
if(SWAPFORCE_TRACE)
  set_source_files_properties(${PPC_SOURCES} PROPERTIES COMPILE_OPTIONS "/FI${CMAKE_CURRENT_SOURCE_DIR}/src/trace/trace_hooks.h")
endif()
target_include_directories(SwapForceRT PRIVATE
  "${PPC_DIR}" "${SIMDE_DIR}"
  "${CMAKE_CURRENT_SOURCE_DIR}/src")
foreach(d kernel memory loader util thread vfs trace gpu)
  target_include_directories(SwapForceRT PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/${d}")
endforeach()

target_compile_options(SwapForceRT PRIVATE
  /bigobj /EHsc /O2
  /clang:-msse4.1 /clang:-mssse3 /clang:-mavx /clang:-mf16c /clang:-w)
target_compile_definitions(SwapForceRT PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN _CRT_SECURE_NO_WARNINGS)

# Pila del hilo principal grande (la recompilacion produce llamadas anidadas profundas).
# M8: tablas de lineas solo para el runtime (perfilado con llvm-symbolizer); /OPT:REF,ICF = valores por defecto sin /DEBUG
set_source_files_properties(${RT_SOURCES} PROPERTIES COMPILE_OPTIONS "/Z7;/clang:-gline-tables-only")
target_link_options(SwapForceRT PRIVATE /STACK:268435456 /SUBSYSTEM:CONSOLE /INCREMENTAL:NO /MAP:SwapForceRT.map /DEBUG /OPT:REF /OPT:ICF)
target_link_libraries(SwapForceRT PRIVATE setupapi)

set(SWAPFORCE_OUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/build" CACHE PATH "Directorio de salida del ejecutable")
set_target_properties(SwapForceRT PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${SWAPFORCE_OUT_DIR}")

# DXC (H2): compilador HLSL -> DXIL junto al ejecutable (de la copia de XenosRecomp, MIT/NCSA)
# Orden: DXC descargado por setup.ps1 (tools/dxc), si no, la copia de XenosRecomp del arbol de desarrollo.
set(SWAPFORCE_DXC_BIN "" CACHE PATH "Directorio con dxcompiler.dll y dxil.dll ya disponibles")
if(SWAPFORCE_DXC_BIN)
  set(DXC_BIN "${SWAPFORCE_DXC_BIN}")
elseif(EXISTS "${SWAPFORCE_ROOT}/tools/dxc/bin/x64/dxcompiler.dll")
  set(DXC_BIN "${SWAPFORCE_ROOT}/tools/dxc/bin/x64")
else()
  set(DXC_BIN "${SWAPFORCE_ROOT}/tools/XenosRecomp/thirdparty/dxc-bin/bin/x64")
endif()
add_custom_command(TARGET SwapForceRT POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${DXC_BIN}/dxcompiler.dll" "$<TARGET_FILE_DIR:SwapForceRT>/dxcompiler.dll"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${DXC_BIN}/dxil.dll" "$<TARGET_FILE_DIR:SwapForceRT>/dxil.dll")

# FFmpeg/libavcodec LGPL (A2): DLL cargadas DINAMICAMENTE con LoadLibrary (sin enlace). Las cabeceras solo se usan en compilacion y la
# carpeta third_party_bin esta ignorada por git (descarga: THIRD_PARTY_NOTICES.md). Sin cabeceras el runtime compila y XMA queda en silencio.
set(SWAPFORCE_FFMPEG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party_bin/ffmpeg" CACHE PATH "Carpeta con avcodec/avutil/swresample (DLL) e include/")
if(EXISTS "${SWAPFORCE_FFMPEG_DIR}/include/libavcodec/avcodec.h")
  target_include_directories(SwapForceRT PRIVATE "${SWAPFORCE_FFMPEG_DIR}/include")
  target_compile_definitions(SwapForceRT PRIVATE SWAPFORCE_FFMPEG_HEADERS=1)
  file(GLOB SWAPFORCE_FFMPEG_DLLS "${SWAPFORCE_FFMPEG_DIR}/*.dll")
  foreach(dll ${SWAPFORCE_FFMPEG_DLLS})
    add_custom_command(TARGET SwapForceRT POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "${dll}" "$<TARGET_FILE_DIR:SwapForceRT>")
  endforeach()
  message(STATUS "FFmpeg (dinamico): cabeceras y DLL de ${SWAPFORCE_FFMPEG_DIR}")
else()
  message(STATUS "FFmpeg: sin cabeceras en ${SWAPFORCE_FFMPEG_DIR}; XMA solo silencio")
endif()
