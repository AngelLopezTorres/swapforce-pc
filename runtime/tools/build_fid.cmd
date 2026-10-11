@echo off
rem Compila en runtime\build_fid (no toca runtime\build ni ppc_gen; no ejecuta gen_ppc_sources.py).
call "%~dp0vsenv.cmd" cmake -S "%~dp0.." -B "%~dp0..\build_fid" -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl -DSWAPFORCE_OUT_DIR=%~dp0..\build_fid
if errorlevel 1 exit /b 1
call "%~dp0vsenv.cmd" cmake --build "%~dp0..\build_fid" %*
