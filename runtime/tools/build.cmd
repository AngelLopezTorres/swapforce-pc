@echo off
rem Uso: build.cmd [objetivo ninja]   (configura si hace falta y compila)
python "%~dp0gen_ppc_sources.py"
if errorlevel 1 exit /b 1
call "%~dp0vsenv.cmd" cmake -S "%~dp0.." -B "%~dp0..\build" -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl
if errorlevel 1 exit /b 1
call "%~dp0vsenv.cmd" cmake --build "%~dp0..\build" %*
