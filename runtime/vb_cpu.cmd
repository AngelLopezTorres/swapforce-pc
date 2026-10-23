@echo off
rem Compila en runtime\build_cpu (salida runtime\build_cpu\out\SwapForceRT.exe). Rutas relativas a este fichero.
set "RT=%~dp0"
call "%RT%tools\vsenv.cmd" cmake -S "%RT%." -B "%RT%build_cpu" -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl -DSWAPFORCE_OUT_DIR="%RT%build_cpu\out"
if errorlevel 1 exit /b 1
call "%RT%tools\vsenv.cmd" cmake --build "%RT%build_cpu" --parallel 8
